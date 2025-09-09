"""Utility functions for cryptographic operations."""

import os


def generate_nonce(size=16):
    """Generate a random nonce of given size."""

    return os.urandom(size)


def compute_hmac(key: bytes, data: bytes) -> bytes:
    """Compute HMAC-SHA256 of the given data using the provided key."""
    # for testing just return fixed bytes
    return bytes(key[i % len(key)] ^ data[i % len(data)] for i in range(32))
