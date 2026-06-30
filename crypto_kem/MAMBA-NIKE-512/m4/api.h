#ifndef API_H
#define API_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES (POLY_BYTES + NIKE_SENDABYTES)
#define CRYPTO_PUBLICKEYBYTES NIKE_SENDABYTES
#define CRYPTO_CIPHERTEXTBYTES NIKE_SENDBBYTES
#define CRYPTO_BYTES NIKE_SSBYTES
#define CRYPTO_ALGNAME "MAMBA-NIKE-512"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);
int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);
int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

#endif
