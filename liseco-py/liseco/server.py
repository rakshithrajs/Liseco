"""Liseco Server Implementation"""

import socket
from liseco.packet import LisecoPacket, PacketType
from liseco.crypto import generate_nonce, compute_hmac


class LisecoServer:
    """Liseco protocol server."""

    def __init__(self, host="0.0.0.0", port=8765):
        self.host = host
        self.port = port

    def run(self):
        """Run the Liseco server."""
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind((self.host, self.port))
            server.listen(1)
            print(f"[Liseco] Listening on {self.host}:{self.port}...")

            conn, addr = server.accept()
            with conn:
                print(f"[Liseco] Connection from {addr}")
                while True:
                    data = conn.recv(1024)
                    if not data:
                        print("[Liseco] Client disconnected.")
                        break

                    pkt = LisecoPacket.parse(data)
                    if pkt:
                        print(
                            f"[ESP → Laptop]: Type={pkt.type}, Payload={pkt.payload.hex()}"
                        )
                        if pkt.type == PacketType.HELLO:
                            nonce1 = pkt.payload[:16]
                            nonce2 = generate_nonce(16)
                            reply_payload = nonce1 + nonce2
                            mac = compute_hmac(b"shared_secret", reply_payload)
                            reply_pkt = LisecoPacket(
                                PacketType.HELLO_REPLY, reply_payload, mac
                            )
                            conn.sendall(reply_pkt.serialize())
                            print("[Laptop → ESP]: Sent HELLO_REPLY")
                    else:
                        print("[Liseco] Received invalid packet")
