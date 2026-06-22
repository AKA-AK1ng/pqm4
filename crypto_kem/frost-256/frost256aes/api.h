#ifndef API_H
#define API_H

#include <stddef.h>
#include <stdint.h>

#define CRYPTO_SECRETKEYBYTES  21216
#define CRYPTO_PUBLICKEYBYTES  17192
#define CRYPTO_BYTES              32
#define CRYPTO_CIPHERTEXTBYTES 17256

#define CRYPTO_ALGNAME "MAMBA-Frost-256"

int crypto_kem_keypair(uint8_t *pk, uint8_t *sk);
int crypto_kem_enc(uint8_t *ct, uint8_t *ss, const uint8_t *pk);
int crypto_kem_dec(uint8_t *ss, const uint8_t *ct, const uint8_t *sk);

#endif