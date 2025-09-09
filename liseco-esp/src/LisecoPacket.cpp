#include "LisecoPacket.h"
using namespace std;

LisecoPacket buildPacket(uint8_t type, const vector<uint8_t>& payload, const vector<uint8_t>& mac) {
    LisecoPacket packet;
    packet.header = LISECO_HEADER;
    packet.type = type;
    packet.length = payload.size();
    packet.payload = payload;
    packet.mac = mac;
    return packet;
}

bool parsePacket(const vector<uint8_t>& raw, LisecoPacket& packet) {
    if (raw.size() < 5) return false;

    packet.header = (raw[0] << 8) | raw[1];
    if (packet.header != LISECO_HEADER) return false;

    packet.type = raw[2];
    packet.length = (raw[3] << 8) | raw[4];

    if (raw.size() < 5 + packet.length + LISECO_MAC_SIZE) return false;

    packet.payload.assign(raw.begin() + 5, raw.begin() + 5 + packet.length);
    packet.mac.assign(raw.begin() + 5 + packet.length, raw.end());

    return true;
}

std::vector<uint8_t> serializePacket(const LisecoPacket& packet) {
    std::vector<uint8_t> raw;

    raw.push_back((packet.header >> 8) & 0xFF);
    raw.push_back(packet.header & 0xFF);

    raw.push_back(packet.type);

    raw.push_back((packet.length >> 8) & 0xFF);
    raw.push_back(packet.length & 0xFF);

    raw.insert(raw.end(), packet.payload.begin(), packet.payload.end());

    raw.insert(raw.end(), packet.mac.begin(), packet.mac.end());

    return raw;
}

