# Lightweight Secure Comms – Packet Format

All packets follow a common structure:

```text
[ HEADER | TYPE | LENGTH | PAYLOAD | TAG ]
```

## Common Fields

| Field   | Size     | Description                             |
| ------- | -------- | --------------------------------------- |
| HEADER  | 2 bytes  | Fixed magic bytes (0xABCD)              |
| TYPE    | 1 byte   | Packet type (see below)                 |
| LENGTH  | 2 bytes  | Length of PAYLOAD (big-endian)          |
| PAYLOAD | variable | Data specific to packet type            |
| TAG     | 16 bytes | AES-GCM authentication tag (NORMAL_MSG) |

## Packet Types (as implemented)

### 1. HELLO (client → server)

```ini
TYPE = 0x01
PAYLOAD = [Nonce1 (16B)]
```

### 2. HELLO_REPLY (server → client)

```ini
TYPE = 0x02
PAYLOAD = [Nonce1 (16B) | Nonce2 (16B)]
```

### 3. CONFIRM (client → server)

```ini
TYPE = 0x03
PAYLOAD = [Nonce2 (16B)]
```

### 4. (Unused)

Type 0x04 (VERIFY) is not implemented in the current code.

### 5. NORMAL_MSG (client ↔ server)

```ini
TYPE = 0x05
PAYLOAD = [IV (12B) | Ciphertext (JSON)]
TAG = AES-GCM tag (16B)
```

-   Encryption: AES-256-GCM (IV is 12 bytes, tag is 16 bytes)
-   Ciphertext is a JSON string, e.g.
    `{ "device": "esp32", "temp": 24.12, "hum": 40.23, "ts": 12345 }`

#### Periodic Sensor Data Sending

After handshake, the ESP sends NORMAL_MSG packets every 2 seconds with
encrypted sensor data. If the connection drops, the ESP reconnects,
re-handshakes, and resumes sending.

## Example Sizes

-   HELLO = 2 (HEADER) + 1 (TYPE) + 2 (LENGTH) + 16 (PAYLOAD) = 21 bytes
-   HELLO_REPLY = 2 + 1 + 2 + 32 = 37 bytes
-   CONFIRM = 2 + 1 + 2 + 16 = 21 bytes
-   NORMAL_MSG = 2 + 1 + 2 + (12 + ciphertext) + 16
