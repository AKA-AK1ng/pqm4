#include "api.h"
#include "nike.h"
#include <string.h>

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
  poly skpoly;

  nike_keygen(pk, &skpoly);
  poly_tobytes(sk, &skpoly);
  memcpy(sk + POLY_BYTES, pk, NIKE_SENDABYTES);

  return 0;
}

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk)
{
  nike_sharedb(ss, ct, pk);

  return 0;
}

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk)
{
  poly skpoly;
  const unsigned char *pk = sk + POLY_BYTES;

  poly_frombytes(&skpoly, sk);
  nike_shareda(ss, &skpoly, pk, ct);

  return 0;
}
