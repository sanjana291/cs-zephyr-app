# Keys

This directory holds signing keys for MCUboot / secure boot.

**Never commit real private keys to this repository.**

- `sample_key.pem` — placeholder; replace with your actual key before signing images.
- Generate a new key pair:  `imgtool keygen -k signing_key.pem -t rsa-2048`
