"""Liseco Server Implementation"""

import socket
from liseco.packet import LisecoPacket, PacketType
from liseco.crypto import LisecoCrypto
import cryptography.exceptions

PSK = b"shared key"

class LisecoServer:
    """Liseco protocol server."""

    def __init__(self, host="0.0.0.0", port=8765):
        self.host = host
        self.port = port
        self.last_nonce2 = None
        self.last_nonce1 = None
        self.session_key = None
        self.secure = False

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
                        print(f"[Liseco]: Type={pkt.type}, Payload={pkt.payload.hex()}")
                        if pkt.type == PacketType.HELLO:
                            nonce1 = pkt.payload[:16]
                            nonce2 = LisecoCrypto.generate_nonce(16)
                            self.last_nonce2 = nonce2
                            self.last_nonce1 = nonce1
                            reply_payload = nonce1 + nonce2
                            mac = LisecoCrypto.compute_hmac(PSK, reply_payload)
                            reply_pkt = LisecoPacket(
                                PacketType.HELLO_REPLY, reply_payload, mac
                            )
                            conn.sendall(reply_pkt.serialize())
                            print("[Liseco]: Sent HELLO_REPLY")
                        elif pkt.type == PacketType.CONFIRM:
                            recv_nonce2 = pkt.payload
                            print(
                                f"[Liseco]: CONFIRM received, payload={recv_nonce2.hex()}"
                            )
                            if self.last_nonce2 is None:
                                print("[Liseco] No nonce2 stored - protocol error")
                                conn.close()
                                return

                            if recv_nonce2 == self.last_nonce2:
                                self.session_key = LisecoCrypto.derive_session_key(
                                    PSK,
                                    nonce1=self.last_nonce1,
                                    nonce2=self.last_nonce2,
                                )
                                self.secure = True
                                print(
                                    "[Liseco] Handshake COMPLETE, Secure session established!"
                                )
                            else:
                                print("[Liseco] Handshake FAILED,  Nonce2 mismatch")
                                conn.close()
                                return
                        elif pkt.type == PacketType.NORMAL_MSG:
                            if not self.secure:
                                print(
                                    "[Liseco] Received NORMAL_MSG before session secured"
                                )
                                continue
                            iv = pkt.payload[:12]
                            ciphertext = pkt.payload[12:]
                            tag = pkt.mac
                            aesgcm = LisecoCrypto(self.session_key)
                            try:
                                plaintext = aesgcm.decrypt_message(iv, ciphertext + tag)
                            except cryptography.exceptions.InvalidTag:
                                print("[Liseco] InvalidTag during decrypt!")
                                print("[Liseco] iv:", iv.hex())
                                print("[Liseco] ciphertext:", ciphertext.hex())
                                print("[Liseco] tag:", tag.hex())
                                print("[Liseco] session_key:", self.session_key.hex())
                                conn.close()
                                return
                            print(
                                f"[Liseco] Decrypted message: {plaintext.decode()}"
                            )
                    else:
                        print("[Liseco] Received invalid packet")
