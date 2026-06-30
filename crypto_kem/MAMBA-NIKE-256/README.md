# MAMBA-NIKE-256

pqm4-style KEM wrapper for MAMBA-NIKE 2.0.

Copy this scheme directory to:

    pqm4/crypto_kem/MAMBA-NIKE-256/ref/

The implementation exports crypto_kem_keypair, crypto_kem_enc, and crypto_kem_dec.
The secret key stores sk_poly || pk, which is required by NIKE2.0's transcript-bound KDF.
