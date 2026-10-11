#include "Liseco.h"
#include "LisecoCrypto.h"
#include "LisecoPacket.h"
#define NONCE_SIZE 16

LisecoClient::LisecoClient() {
    connected = false;
    secure = false;
    PSK = { 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x20, 0x6b, 0x65, 0x79 };
}

void LisecoClient::begin(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    unsigned long start = millis();
    const unsigned long timeout = 15000; // 15 seconds
    unsigned long lastStatusPrint = 0;

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        // Print periodic status every 2s to avoid spamming
        if (millis() - lastStatusPrint > 2000) {
            lastStatusPrint = millis();
            Serial.print(" status=");
            Serial.print(WiFi.status());
            Serial.print(" rssi=");
            Serial.println(WiFi.RSSI());
        }

        if (millis() - start > timeout) {
            Serial.println("\n[Liseco] WiFi connect timed out");
            // Provide helpful diagnostics: scan for SSIDs
            int n = WiFi.scanNetworks();
            Serial.print("[Liseco] Found ");
            Serial.print(n);
            Serial.println(" networks:");
            for (int i = 0; i < n; ++i) {
                Serial.print("  ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(" dBm) ");
                Serial.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "secure");
                Serial.println();
            }
            Serial.println("[Liseco] Check SSID/password, ensure AP is 2.4GHz and in range.");
            return;
        }
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

bool LisecoClient::connect(const char* host, uint16_t port) {
    Serial.print("Connecting to server: ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);

    if (client.connect(host, port)) {
        Serial.println("Connected to server!");
        connected = true;
        return true;
    } else {
        Serial.println("Connection failed!");
        connected = false;
        return false;
    }
}

bool LisecoClient::send(const std::vector<uint8_t>& plain) {
    if (!secure) return false;

    std::vector<uint8_t> iv(12);
    for (int i=0;i<12;i++) iv[i] = (uint8_t)random(0,256);

    std::vector<uint8_t> ciphertext, tag;
    std::vector<uint8_t> aad;
    if (!LisecoCrypto::aes_gcm_encrypt(session_key, iv, plain, aad, ciphertext, tag)) {
        Serial.println("[Liseco] AES-GCM encryption failed");
        return false;
    }

    std::vector<uint8_t> payload = iv;
    payload.insert(payload.end(), ciphertext.begin(), ciphertext.end());

    auto pkt = buildPacket(PacketType::NORMAL_MSG, payload, tag);
    auto raw = serializePacket(pkt);
    client.write(raw.data(), raw.size());
    Serial.println("[Liseco] Sent NORMAL_MSG (encrypted)");
    return true;
}

bool LisecoClient::receive(std::vector<uint8_t>& outPlain) {
    LisecoPacket pkt;
    if (!receivePacket(pkt)) return false;
    if (pkt.type != NORMAL_MSG) {
        return false;
    }
    if (pkt.payload.size() < 12) return false;

    std::vector<uint8_t> iv(pkt.payload.begin(), pkt.payload.begin()+12);
    std::vector<uint8_t> ciphertext(pkt.payload.begin()+12, pkt.payload.end());
    std::vector<uint8_t> tag = pkt.mac;

    std::vector<uint8_t> aad;
    std::vector<uint8_t> plain;
    if (!LisecoCrypto::aes_gcm_decrypt(session_key, iv, ciphertext, aad, tag, plain)) {
        Serial.println("[Liseco] AES-GCM auth failed!");
        return false;
    }
    outPlain = plain;
    Serial.println("[Liseco] Decrypted NORMAL_MSG");
    return true;
}

bool LisecoClient::receivePacket(LisecoPacket& packet) {
    if (!connected || !client.available()) return false;

    std::vector<uint8_t> raw;
    while (client.available()) {
        raw.push_back(client.read());
    }

    if (parsePacket(raw, packet)) {
        Serial.println("Packet received & parsed!");
        return true;
    }

    return false;
}


bool LisecoClient::handshake() {
    if (!connected) return false;
    Serial.println("[Liseco] Starting handshake...");

    auto nonce1 = LisecoCrypto::generateNonce(NONCE_SIZE);
    std::vector<uint8_t> payload = nonce1;
    auto mac_dummy = std::vector<uint8_t>(16, 0x00); 
    auto hello = buildPacket(PacketType::HELLO, payload, mac_dummy);
    auto raw = serializePacket(hello);
    client.write(raw.data(), raw.size());
    Serial.println("[Liseco] HELLO packet sent!");
    // TODO: add DeviceID + public key if needed

    LisecoPacket reply;
    unsigned long start = millis();
    while (millis() - start < 5000) { 
        if (receivePacket(reply) && reply.type == HELLO_REPLY) {
            Serial.println("[Liseco] HELLO_REPLY received!");

            if(reply.payload.size() < NONCE_SIZE * 2){
                Serial.println("[Liseco] HELLO_REPLY payload too small!");
                return false;
            }

            std::vector<uint8_t> recv_nonce1(reply.payload.begin(), reply.payload.begin() + NONCE_SIZE);
            std::vector<uint8_t> nonce2(reply.payload.begin() + NONCE_SIZE, reply.payload.begin() + NONCE_SIZE*2);
            
            if (recv_nonce1 != nonce1) {
                Serial.println("[Liseco] Nonce1 mismatch! Handshake failed.");
                return false;
            }

            std::vector<uint8_t> psk_vec = PSK;
            std::vector<uint8_t> derived;
            if (!LisecoCrypto::derive_session_key(psk_vec, nonce1, nonce2, derived)) {
                Serial.println("[Liseco] Session key derivation failed!");
                return false;
            }
            session_key = derived;
            secure = true;

            auto confirm = buildPacket(PacketType::CONFIRM, nonce2, mac_dummy);
            auto raw_confirm = serializePacket(confirm);
            client.write(raw_confirm.data(), raw_confirm.size());
            Serial.println("[Liseco] CONFIRM sent!");

            Serial.println("[Liseco] Handshake complete, secure session established!");
            return true;
        }
    }

    Serial.println("[Liseco] Handshake timed out!");
    return false;
}


void LisecoClient::loop() {
    if (connected && !client.connected()) {
        Serial.println("Lost connection to server!");
        connected = false;
    }
}