"""Module for handling Liseco protocol packets."""

from enum import IntEnum

LISECO_HEADER = 0xABCD
LISECO_MAC_SIZE = 16


class PacketType(IntEnum):
    """Packet types for Liseco protocol."""

    HELLO = 0x01
    HELLO_REPLY = 0x02
    CONFIRM = 0x03
    VERIFY = 0x04
    NORMAL_MSG = 0x05


class LisecoPacket:
    """Class representing a Liseco protocol packet."""

    def __init__(self, ptype: int, payload: bytes, mac: bytes = b""):
        self.header = LISECO_HEADER
        self.type = ptype
        self.payload = payload
        self.mac = mac

    def serialize(self) -> bytes:
        """Serialize the packet to bytes.

        Returns:
            bytes: The serialized packet.
        """
        length = len(self.payload)
        raw = (
            self.header.to_bytes(2, "big")
            + self.type.to_bytes(1, "big")
            + length.to_bytes(2, "big")
            + self.payload
            + self.mac
        )
        return raw

    @staticmethod
    def parse(raw: bytes):
        """Parse raw bytes into a LisecoPacket.

        Args:
            raw (bytes): Raw bytes to parse.

        Returns:
            LisecoPacket: Parsed LisecoPacket or None if invalid.
        """
        if len(raw) < 5:
            print("[Liseco] Packet too short to parse")
            return None
        header = int.from_bytes(raw[0:2], "big")
        if header != LISECO_HEADER:
            print("[Liseco] Invalid packet header")
            return None
        ptype = raw[2]
        length = int.from_bytes(raw[3:5], "big")
        if len(raw) < 5 + length + LISECO_MAC_SIZE:
            return None
        payload = raw[5 : 5 + length]
        mac = raw[5 + length : 5 + length + LISECO_MAC_SIZE]
        return LisecoPacket(ptype, payload, mac)
