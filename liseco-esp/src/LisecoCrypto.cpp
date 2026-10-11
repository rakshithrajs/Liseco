#include "LisecoCrypto.h"
#include <mbedtls/md.h>
#include <mbedtls/gcm.h>
#include <mbedtls/platform.h> 
#include <Arduino.h> 
#include <string.h>

namespace LisecoCrypto {
    std::vector<uint8_t> generateNonce(size_t size) {
        std::vector<uint8_t> nonce(size);
        for (size_t i = 0; i < size; ++i) {
            nonce[i] = (uint8_t)random(0, 256);
        }
        return nonce;
    }

    bool hmac_sha256(const std::vector<uint8_t>& key,
                    const std::vector<uint8_t>& data,
                    std::vector<uint8_t>& out) {
        const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        if (!info) return false;

        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        if (mbedtls_md_setup(&ctx, info, 1) != 0) { mbedtls_md_free(&ctx); return false; }

        if (mbedtls_md_hmac_starts(&ctx, key.data(), key.size()) != 0) { mbedtls_md_free(&ctx); return false; }
        if (mbedtls_md_hmac_update(&ctx, data.data(), data.size()) != 0) { mbedtls_md_free(&ctx); return false; }

        unsigned char buf[32];
        if (mbedtls_md_hmac_finish(&ctx, buf) != 0) { mbedtls_md_free(&ctx); return false; }
        out.assign(buf, buf + 32);
        mbedtls_md_free(&ctx);
        return true;
    }

    bool derive_session_key(const std::vector<uint8_t>& psk,
                            const std::vector<uint8_t>& nonce1,
                            const std::vector<uint8_t>& nonce2,
                            std::vector<uint8_t>& outSessionKey) {
        std::vector<uint8_t> data;
        data.insert(data.end(), nonce1.begin(), nonce1.end());
        data.insert(data.end(), nonce2.begin(), nonce2.end());
        return hmac_sha256(psk, data, outSessionKey);
    }

    bool aes_gcm_encrypt(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        const std::vector<uint8_t>& plaintext,
                        const std::vector<uint8_t>& aad,
                        std::vector<uint8_t>& ciphertext,
                        std::vector<uint8_t>& tag) {

        if (key.size() != 32 || iv.size() != 12) return false;
        ciphertext.resize(plaintext.size());
        tag.resize(16);

        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key.data(), 256) != 0) { mbedtls_gcm_free(&gcm); return false; }

        int ret = mbedtls_gcm_crypt_and_tag(&gcm,
                                            MBEDTLS_GCM_ENCRYPT,
                                            plaintext.size(),
                                            iv.data(), iv.size(),
                                            aad.empty() ? NULL : aad.data(), aad.size(),
                                            plaintext.data(),
                                            ciphertext.data(),
                                            tag.size(), tag.data());
        mbedtls_gcm_free(&gcm);
        return ret == 0;
    }

    bool aes_gcm_decrypt(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv,
                        const std::vector<uint8_t>& ciphertext,
                        const std::vector<uint8_t>& aad,
                        const std::vector<uint8_t>& tag,
                        std::vector<uint8_t>& plaintext) {

        if (key.size() != 32 || iv.size() != 12 || tag.size() != 16) return false;
        plaintext.resize(ciphertext.size());

        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key.data(), 256) != 0) { mbedtls_gcm_free(&gcm); return false; }

        int ret = mbedtls_gcm_auth_decrypt(&gcm,
                                            ciphertext.size(),
                                            iv.data(), iv.size(),
                                            aad.empty() ? NULL : aad.data(), aad.size(),
                                            tag.data(), tag.size(),
                                            ciphertext.data(),
                                            plaintext.data());
        mbedtls_gcm_free(&gcm);
        return ret == 0;
    }
}