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

        void send(const String& message);

        String receive();

        bool receivePacket(LisecoPacket& packet);

        bool handshake();

        void loop();
    private:
        WiFiClient client;
        bool connected;
};

#endif
