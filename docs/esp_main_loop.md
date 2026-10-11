# ESP Main Loop and Reconnection Logic

This document describes the main loop logic for the ESP client in the Liseco
protocol implementation.

## Overview

The ESP runs a robust main loop that:

1. Connects to WiFi (with diagnostics and timeout).
2. Connects to the server via TCP.
3. Performs the handshake protocol to establish a secure session key.
4. Periodically (every 2 seconds) sends encrypted sensor data as JSON.
5. If the TCP connection is lost, automatically reconnects, re-handshakes, and
   resumes sending.

## Pseudocode

```cpp
void loop() {
    if (!client.connected) {
        if (client.connect(SERVER_HOST, SERVER_PORT)) {
            if (client.handshake()) {
                // Secure session established
            } else {
                // Handshake failed, retry after delay
                delay(2000);
                return;
            }
        } else {
            // TCP connect failed, retry after delay
            delay(2000);
            return;
        }
    }

    // Only send if secure
    if (client.secure && millis() - lastSend >= sendInterval) {
        // Build and send JSON sensor data
    }

    // Handle incoming packets, check for disconnects
    client.loop();
    delay(10);
}
```

## Key Points

-   Handshake only happens once per connection.
-   Data is only sent if the session is secure.
-   If the connection drops, the ESP will always attempt to restore a secure
    session before resuming data transmission.

## Sensor Data Format

The payload sent in NORMAL_MSG is a JSON string, e.g.:

```json
{
    "device": "esp32",
    "temp": 24.12,
    "hum": 40.23,
    "ts": 12345
}
```

This is encrypted with AES-256-GCM using the session key.
