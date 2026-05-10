#ifndef API_H
#define API_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES  MLWQ_KEM_SECRETKEYBYTES  /* 5024 */
#define CRYPTO_PUBLICKEYBYTES  MLWQ_PUBLICKEYBYTES      /* 2496 */
#define CRYPTO_CIPHERTEXTBYTES MLWQ_CIPHERTEXTBYTES     /* 2704 */
#define CRYPTO_BYTES           MLWQ_SSBYTES             /* 48   */

#define CRYPTO_ALGNAME "Viper-384"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

#endif /* API_H */