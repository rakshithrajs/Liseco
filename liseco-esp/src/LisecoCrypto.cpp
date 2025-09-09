#include "LisecoCrypto.h"

namespace LisecoCrypto {
    std::vector<uint8_t> generateNonce(size_t size) {
        std::vector<uint8_t> nonce(size);
        for (size_t i = 0; i < size; i++) {
            nonce[i] = random(0, 256);
        }
        return nonce;
    }

    std::vector<uint8_t> computeHMAC(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
        std::vector<uint8_t> mac(32, 0xAA); 
        return mac;
    }
}
