#include "api.h"
#include "newhope.h"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
  poly skpoly;

  newhope_keygen(pk, &skpoly);
  poly_tobytes(sk, &skpoly);

  return 0;
}

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk)
{
  newhope_sharedb(ss, ct, pk);

  return 0;
}

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk)
{
  poly skpoly;

  poly_frombytes(&skpoly, sk);
  newhope_shareda(ss, &skpoly, ct);

  return 0;
}
