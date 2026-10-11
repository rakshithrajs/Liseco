#ifndef LISECO_PACKET_H
#define LISECO_PACKET_H

#include <Arduino.h>

#define LISECO_HEADER 0xABCD
#define LISECO_MAC_SIZE 16

enum PacketType {
    HELLO       = 0x01,
    HELLO_REPLY = 0x02,
    CONFIRM     = 0x03,
    VERIFY      = 0x04,
    NORMAL_MSG  = 0x05
};

struct LisecoPacket {
    uint16_t header;
    uint8_t type;
    uint16_t length;
    std::vector<uint8_t> payload;
    std::vector<uint8_t> mac;
};

LisecoPacket buildPacket(uint8_t type, const std::vector<uint8_t>& payload, const std::vector<uint8_t>& mac);

bool parsePacket(const std::vector<uint8_t>& raw, LisecoPacket& packet);

std::vector<uint8_t> serializePacket(const LisecoPacket& packet);

#endif
