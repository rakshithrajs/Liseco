#ifndef LISECO_CRYPTO_H
#define LISECO_CRYPTO_H

#include <Arduino.h>
#include <vector>

namespace LisecoCrypto {
    std::vector<uint8_t> generateNonce(size_t size = 16);
    std::vector<uint8_t> computeHMAC(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data);
}

#endif
