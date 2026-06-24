#ifndef API_H
#define API_H

#include <stddef.h>
#include <stdint.h>

#define CRYPTO_SECRETKEYBYTES  11536
#define CRYPTO_PUBLICKEYBYTES   9712
#define CRYPTO_BYTES              24
#define CRYPTO_CIPHERTEXTBYTES  9760

#define CRYPTO_ALGNAME "MAMBA-Frost-192"

int crypto_kem_keypair(uint8_t *pk, uint8_t *sk);
int crypto_kem_enc(uint8_t *ct, uint8_t *ss, const uint8_t *pk);
int crypto_kem_dec(uint8_t *ss, const uint8_t *ct, const uint8_t *sk);

#endif
