#ifndef API_H
#define API_H

#if !defined(_API_Frodo640_H_) && !defined(_API_Frodo976_H_) && !defined(_API_Frodo1344_H_)
#include "api_venom1.h"
#endif

#if defined(_API_Frodo640_H_)
#define crypto_kem_keypair crypto_kem_keypair_Frodo640
#define crypto_kem_enc crypto_kem_enc_Frodo640
#define crypto_kem_dec crypto_kem_dec_Frodo640
#elif defined(_API_Frodo976_H_)
#define crypto_kem_keypair crypto_kem_keypair_Frodo976
#define crypto_kem_enc crypto_kem_enc_Frodo976
#define crypto_kem_dec crypto_kem_dec_Frodo976
#elif defined(_API_Frodo1344_H_)
#define crypto_kem_keypair crypto_kem_keypair_Frodo1344
#define crypto_kem_enc crypto_kem_enc_Frodo1344
#define crypto_kem_dec crypto_kem_dec_Frodo1344
#else
#error "Unknown Venom parameter set; include api_venom1.h, api_venom3.h, or api_venom5.h first."
#endif

#endif
