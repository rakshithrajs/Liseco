#include "Liseco.h"
#include "LisecoCrypto.h"
#include "LisecoPacket.h"

LisecoClient::LisecoClient() {
    connected = false;
}

void LisecoClient::begin(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
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

void LisecoClient::send(const String& message) {
    if (connected) {
        client.println(message);
        Serial.print("Sent: ");
        Serial.println(message);
    } else {
        Serial.println("Not connected, cannot send.");
    }
}

String LisecoClient::receive() {
    String message = "";
    if (connected && client.available()) {
        message = client.readStringUntil('\n');
        if (message.length() > 0) {
        Serial.print("Received: ");
        Serial.println(message);
        }
    }
    return message;
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

    auto nonce1 = LisecoCrypto::generateNonce(16);
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), nonce1.begin(), nonce1.end());
    // TODO: add DeviceID + public key if needed

    auto mac = LisecoCrypto::computeHMAC({0x01}, payload); 
    auto hello = buildPacket(PacketType::HELLO, payload, mac);

    auto raw = serializePacket(hello);
    client.write(raw.data(), raw.size());
    Serial.println("[Liseco] HELLO packet sent!");

    LisecoPacket reply;
    unsigned long start = millis();
    while (millis() - start < 5000) { 
        if (receivePacket(reply) && reply.type == HELLO_REPLY) {
            Serial.println("[Liseco] HELLO_REPLY received!");

            if (reply.payload.size() >= nonce1.size() &&
                std::equal(nonce1.begin(), nonce1.end(), reply.payload.begin())) {
                Serial.println("[Liseco] Nonce validated, handshake complete!");
                return true;
            } else {
                Serial.println("[Liseco] Nonce mismatch! Handshake failed.");
                return false;
            }
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