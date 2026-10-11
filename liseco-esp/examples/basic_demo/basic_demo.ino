#include <Arduino.h>
#include "Liseco.h"
#include "LisecoPacket.h"
#include "LisecoCrypto.h"

const char* WIFI_SSID  = "rak";
const char* WIFI_PASS  = "1234567890";
const char* SERVER_HOST = "10.113.95.44";   
const uint16_t SERVER_PORT = 8765;

LisecoClient client;

void setup() {
    Serial.begin(115200);

    client.begin(WIFI_SSID, WIFI_PASS);

    if (client.connect(SERVER_HOST, SERVER_PORT)) {
        Serial.println("[Liseco] TCP connected, starting handshake...");

        if (client.handshake()) {
            Serial.println("Handshake Complete");
        } else {
            Serial.println("[Liseco] Handshake failed at HELLO!");
        }
    }
}

void loop() {
    static unsigned long lastSend = 0;
    const unsigned long sendInterval = 2000;

    // Reconnect logic: if not connected, try to connect and handshake
    if (!client.connected) {
        if (client.connect(SERVER_HOST, SERVER_PORT)) {
            Serial.println("[Liseco] TCP connected, starting handshake...");
            if (client.handshake()) {
                Serial.println("Handshake Complete");
            } else {
                Serial.println("[Liseco] Handshake failed!");
                delay(2000);
                return;
            }
        } else {
            Serial.println("[Liseco] TCP connect failed, retrying in 2s...");
            delay(2000);
            return;
        }
    }

    // Handle incoming packets
    LisecoPacket packet;
    if (client.receivePacket(packet)) {
        Serial.print("[Liseco] Got packet, type = ");
        Serial.println(packet.type, HEX);
    }

    // Only send if secure
    if (client.secure && millis() - lastSend >= sendInterval) {
        lastSend = millis();
        float temp = 24.0 + (random(-50, 50) / 100.0);
        float hum = 40.0 + (random(-200, 200) / 100.0);
        char buf[128];
        int n = snprintf(buf, sizeof(buf), "{\"device\":\"esp32\",\"temp\":%.2f,\"hum\":%.2f,\"ts\":%lu}", temp, hum, millis());
        std::vector<uint8_t> payload(buf, buf + n);
        if (client.send(payload)) {
            Serial.print("[Liseco] Sent sensor: ");
            Serial.println(buf);
        } else {
            Serial.println("[Liseco] send() failed - session not secure or connection lost");
        }
    }

    client.loop();
    delay(10);
}