#ifndef LISECO_H
#define LISECO_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "LisecoPacket.h"

class LisecoClient {
    public:
        LisecoClient();

        void begin(const char* ssid, const char* password);

        bool connect(const char* host, uint16_t port);

        bool send(const std::vector<uint8_t>& plain);

        bool receive(std::vector<uint8_t>& outPlain);

        bool receivePacket(LisecoPacket& packet);

        bool handshake();

        void loop();

        bool connected;
        bool secure;
    private:
        WiFiClient client;
        std::vector<uint8_t> session_key;
        std::vector<uint8_t> PSK;
};

#endif
