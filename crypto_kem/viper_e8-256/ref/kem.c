/* MAMBA-Viper implementation and implementation support layer where applicable.
 * FO KEM using SHAKE128 hashes/KDF and deterministic re-encryption.
 */

#include "api.h"
#include "fips202.h"
#include "randombytes.h"
#include "verify.h"
#include "viper.h"
#include <string.h>

static void hbytes(unsigned char *out, unsigned long long outlen,
                   const unsigned char *in, unsigned long long inlen) {
  shake128(out, outlen, in, inlen);
}

static void h32(unsigned char out[VIPER_HBYTES], const unsigned char *in,
                unsigned long long inlen) {
  hbytes(out, VIPER_HBYTES, in, inlen);
}

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk) {
  unsigned char rho[VIPER_SEEDBYTES], sseed[VIPER_SEEDBYTES];
  unsigned char *sk_pk = sk + VIPER_SECRETKEY_PKE_BYTES;
  unsigned char *sk_hpk = sk_pk + VIPER_PUBLICKEYBYTES;
  unsigned char *sk_z = sk_hpk + VIPER_HBYTES;

  randombytes(rho, sizeof(rho));
  randombytes(sseed, sizeof(sseed));
  viper_pke_keypair(pk, sk, rho, sseed);
  memcpy(sk_pk, pk, VIPER_PUBLICKEYBYTES);
  h32(sk_hpk, pk, VIPER_PUBLICKEYBYTES);
  randombytes(sk_z, VIPER_FALLBACK_KEY_BYTES);
  return 0;
}

int crypto_kem_enc(unsigned char *ct, unsigned char *ss,
                   const unsigned char *pk) {
  unsigned char m[VIPER_MSGBYTES], mh[VIPER_MSGBYTES];
  unsigned char hpk[VIPER_HBYTES];
  unsigned char buf[VIPER_MSGBYTES + VIPER_HBYTES];
  unsigned char kr[2 * VIPER_FALLBACK_KEY_BYTES];
  unsigned char hct[VIPER_HBYTES];
  unsigned char omega[VIPER_FALLBACK_KEY_BYTES + VIPER_MU_BYTES];
  unsigned char kdfin[VIPER_FALLBACK_KEY_BYTES + VIPER_HBYTES];

  randombytes(m, sizeof(m));
  hbytes(mh, sizeof(mh), m, sizeof(m));
  memcpy(m, mh, sizeof(m));
  h32(hpk, pk, VIPER_PUBLICKEYBYTES);
  memcpy(buf, m, sizeof(m));
  memcpy(buf + sizeof(m), hpk, sizeof(hpk));
  hbytes(kr, sizeof(kr), buf, sizeof(buf));

  memcpy(omega, kr + VIPER_FALLBACK_KEY_BYTES, VIPER_FALLBACK_KEY_BYTES);
  h32(omega + VIPER_FALLBACK_KEY_BYTES, kr, VIPER_FALLBACK_KEY_BYTES);
  viper_pke_enc(ct, pk, m, omega);

  h32(hct, ct, VIPER_CIPHERTEXTBYTES);
  memcpy(kdfin, kr, VIPER_FALLBACK_KEY_BYTES);
  memcpy(kdfin + VIPER_FALLBACK_KEY_BYTES, hct, VIPER_HBYTES);
  hbytes(ss, VIPER_SSBYTES, kdfin, sizeof(kdfin));
  return 0;
}

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct,
                   const unsigned char *sk) {
  unsigned char m[VIPER_MSGBYTES];
  unsigned char buf[VIPER_MSGBYTES + VIPER_HBYTES];
  unsigned char kr[2 * VIPER_FALLBACK_KEY_BYTES];
  unsigned char hct[VIPER_HBYTES];
  unsigned char kdfin[VIPER_FALLBACK_KEY_BYTES + VIPER_HBYTES];
  const unsigned char *pk = sk + VIPER_SECRETKEY_PKE_BYTES;
  const unsigned char *hpk = pk + VIPER_PUBLICKEYBYTES;
  const unsigned char *z = hpk + VIPER_HBYTES;
  unsigned char reject;

  viper_pke_dec(m, sk, ct);
  memcpy(buf, m, sizeof(m));
  memcpy(buf + sizeof(m), hpk, VIPER_HBYTES);
  hbytes(kr, sizeof(kr), buf, sizeof(buf));

  reject = (unsigned char)!viper_reencrypt_check(
      ct, pk, m, kr + VIPER_FALLBACK_KEY_BYTES);
  h32(hct, ct, VIPER_CIPHERTEXTBYTES);
  memcpy(kdfin, kr, VIPER_FALLBACK_KEY_BYTES);
  cmov(kdfin, z, VIPER_FALLBACK_KEY_BYTES, reject);
  memcpy(kdfin + VIPER_FALLBACK_KEY_BYTES, hct, VIPER_HBYTES);
  hbytes(ss, VIPER_SSBYTES, kdfin, sizeof(kdfin));
  return 0;
}
