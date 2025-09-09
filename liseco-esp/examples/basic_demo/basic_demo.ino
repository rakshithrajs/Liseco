#include <Liseco.h>

LisecoClient client;

const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASS = "YourPassword";
const char* SERVER_HOST = "192.168.1.100"; 
const uint16_t SERVER_PORT = 8765;

void setup() {
    Serial.begin(115200);

    client.begin(WIFI_SSID, WIFI_PASS);
    if (client.connect(SERVER_HOST, SERVER_PORT)) {
        client.send("Hello from ESP!");
    }
}

void loop() {
    client.loop();
    String msg = client.receive();
    if (msg.length() > 0) {
        client.send("ESP got: " + msg);
    }
    delay(1000);
}
