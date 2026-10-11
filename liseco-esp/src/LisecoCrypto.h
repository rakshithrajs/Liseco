#ifndef LISECO_CRYPTO_H
#define LISECO_CRYPTO_H

#include <vector>
#include <cstdint>
#include <stddef.h>

namespace LisecoCrypto {
    std::vector<uint8_t> generateNonce(size_t size);

    bool hmac_sha256(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data, std::vector<uint8_t>& out);

    bool derive_session_key(const std::vector<uint8_t>& psk,
                            const std::vector<uint8_t>& nonce1,
                            const std::vector<uint8_t>& nonce2,
                            std::vector<uint8_t>& outSessionKey);

    bool aes_gcm_encrypt(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        const std::vector<uint8_t>& plaintext,
                        const std::vector<uint8_t>& aad,
                        std::vector<uint8_t>& ciphertext,
                        std::vector<uint8_t>& tag);

    bool aes_gcm_decrypt(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        const std::vector<uint8_t>& ciphertext,
                        const std::vector<uint8_t>& aad,
                        const std::vector<uint8_t>& tag,
                        std::vector<uint8_t>& plaintext);
}

#endif
