#ifndef API_H
#define API_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES  MLWQ_KEM_SECRETKEYBYTES  /* 6432 */
#define CRYPTO_PUBLICKEYBYTES  MLWQ_PUBLICKEYBYTES       /* 3200 */
#define CRYPTO_CIPHERTEXTBYTES MLWQ_CIPHERTEXTBYTES      /* 3456 */
#define CRYPTO_BYTES           MLWQ_SSBYTES              /* 64   */

#define CRYPTO_ALGNAME "Viper-512"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

#endif /* API_H */