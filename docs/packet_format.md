# Lightweight Secure Comms – Packet Format

All packets follow a **common structure** for easy parsing:

```pgsql
[ HEADER | TYPE | LENGTH | PAYLOAD | MAC ]
```

## Common Fields

| Field   | Size     | Description                             |
| ------- | -------- | --------------------------------------- |
| HEADER  | 2 bytes  | Fixed magic bytes to identify protocol packets. |
| TYPE    | 1 byte   | Packet type (HELLO, REPLY, etc.).               |
| LENGTH  | 2 bytes  | Length of PAYLOAD (big-endian).                 |
| PAYLOAD | variable | Data specific to packet type.                   |
| MAC     | 32 bytes | HMAC-SHA256 over |

## Packet Types

### 1. HELLO (client → server)

```ini
TYPE = 0x01
PAYLOAD = [DeviceID (4B) | Nonce1 (16B) | ESP_PublicKey (64B)]
```

### 2. HELLO_REPLY (server → client)

```ini
TYPE = 0x02
PAYLOAD = [Nonce1 (16B) | Nonce2 (16B) | Laptop_PublicKey (64B)]
```

### 3. CONFIRM (client → server)

```ini
TYPE = 0x03
PAYLOAD = [Nonce2 (16B)]
MAC = HMAC(SessionKey, [Nonce2])
```

### 4. VERIFY (server → client)

```ini
TYPE = 0x04
PAYLOAD = [Nonce1 (16B)]
MAC = HMAC(SessionKey, [Nonce1])
```

### 5. NORMAL_MSG (client ↔ server)

```ini
TYPE = 0x05
PAYLOAD = [IV (12B) | Ciphertext (variable)]
MAC = HMAC(SessionKey, [IV | Ciphertext])
```

* Encryption: AES-128-GCM or ChaCha20 (depending on ESP library support).
* IV (Initialization Vector) must be unique per message.

## Example Sizes

* HELLO = 2 (HEADER) + 1 (TYPE) + 2 (LENGTH) + 84 (PAYLOAD) = 89 bytes
* NORMAL_MSG = overhead of \~47 bytes + ciphertext

## Notes

* **DeviceID**: Helps identify which ESP is talking in multi-device scenarios.
* **Nonces**: 16 bytes = 128-bit random challenge.
* **Public Keys**: Assuming `secp256r1` curve (ECC), public key = 64 bytes (uncompressed x||y).
* **MAC**: HMAC-SHA256 → 32 bytes.
