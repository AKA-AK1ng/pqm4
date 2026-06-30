#ifndef API_H
#define API_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES POLY_BYTES
#define CRYPTO_PUBLICKEYBYTES NEWHOPE_SENDABYTES
#define CRYPTO_CIPHERTEXTBYTES NEWHOPE_SENDBBYTES
#define CRYPTO_BYTES 32
#define CRYPTO_ALGNAME "NewHope1024-USENIX-KE-pqm4-wrap"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);
int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);
int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

#endif
