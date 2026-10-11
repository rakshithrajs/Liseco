"""Utility functions for cryptographic operations."""

import os
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.primitives.ciphers.aead import AESGCM


class LisecoCrypto:
    """Class encapsulating cryptographic operations for Liseco protocol."""

    def __init__(self, session_key):
        self.aegcm = AESGCM(session_key)
        self.session_key = session_key

    def encrypt_message(self, plaintext: bytes):
        """Encrypt a message using AES-GCM."""
        iv = self.generate_nonce(12)
        ciphertext = self.aegcm.encrypt(iv, plaintext, None)
        return iv, ciphertext

    def decrypt_message(self, iv: bytes, ciphertext: bytes):
        """Decrypt a message using AES-GCM."""
        return self.aegcm.decrypt(iv, ciphertext, None)

    @staticmethod
    def generate_nonce(size=16):
        """Generate a random nonce of given size."""
        return os.urandom(size)

    @staticmethod
    def compute_hmac(key: bytes, data: bytes) -> bytes:
        """Compute HMAC-SHA256 of the given data using the provided key."""
        h = hmac.HMAC(key, hashes.SHA256())
        h.update(data)
        return h.finalize()

    @staticmethod
    def derive_session_key(psk: bytes, nonce1: bytes, nonce2: bytes) -> bytes:
        """Derive a session key from the pre-shared key and two nonces."""
        return LisecoCrypto.compute_hmac(psk, nonce1 + nonce2)
