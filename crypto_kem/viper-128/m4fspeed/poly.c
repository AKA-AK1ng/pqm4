#include "poly.h"

#include "cbd.h"
#include "ntt.h"
#include "params.h"
#include "symmetric.h"

#include <stddef.h>
#include <stdint.h>

static void pack_bits(unsigned char *out, const poly *a, int bits) {
    uint32_t acc = 0;
    int acc_bits = 0;
    size_t idx = 0;
    const uint32_t mask = (1u << bits) - 1u;

    for (int i = 0; i < MLWQ_N; i++) {
        uint32_t v = (uint16_t)(a->coeffs[i] + ((a->coeffs[i] >> 15) & MLWQ_Q));
        acc |= (v & mask) << acc_bits;
        acc_bits += bits;
        while (acc_bits >= 8) {
            out[idx++] = (uint8_t)acc;
            acc >>= 8;
            acc_bits -= 8;
        }
    }
}

static void unpack_bits(poly *r, const unsigned char *in, int bits) {
    uint32_t acc = 0;
    int acc_bits = 0;
    size_t idx = 0;
    const uint32_t mask = (1u << bits) - 1u;

    for (int i = 0; i < MLWQ_N; i++) {
        while (acc_bits < bits) {
            acc |= ((uint32_t)in[idx++]) << acc_bits;
            acc_bits += 8;
        }
        r->coeffs[i] = (int16_t)(acc & mask);
        acc >>= bits;
        acc_bits -= bits;
    }
}

static int cmp_pack_bits(const unsigned char *r, const poly *a, int bits) {
    unsigned char buf[MLWQ_POLYBYTES];
    unsigned char rc = 0;
    size_t len = (MLWQ_N * bits) / 8;

    pack_bits(buf, a, bits);
    for (size_t i = 0; i < len; i++)
        rc |= r[i] ^ buf[i];

    return rc;
}

/*************************************************
* Name:        poly_compress
*
* Description: Serialization of a polynomial and subsequent compression of a polynomial;
*
* Arguments:   - unsigned char *r: pointer to output byte array (of length KYBER_POLYCOMPRESSEDBYTES)
*              - const poly *a:    pointer to input polynomial to be serialized
*************************************************/
void poly_compress(unsigned char *r, const poly *a)
{
  pack_bits(r, a, BIT_V);
}

/*************************************************
* Name:        poly_decompress
*
* Description: De-serialization and subsequent decompression of a polynomial;
*              approximate inverse of poly_compress
*
* Arguments:   - poly *r:                pointer to output polynomial
*              - const unsigned char *a: pointer to input byte array (of length KYBER_POLYCOMPRESSEDBYTES bytes)
**************************************************/
void poly_decompress(poly *r, const unsigned char *a)
{
  unpack_bits(r, a, BIT_V);
}

/*************************************************
* Name:        poly_packcompress
*
* Description: Serialization and subsequent compression of a polynomial of a polyvec,
*              writes to a byte string representation of the whole polyvec.
*              Used to compress a polyvec one poly at a time in a loop.
*
* Arguments:   - unsigned char *r:  pointer to output byte string representation of a polyvec (of length KYBER_POLYVECCOMPRESSEDBYTES)
*              - const poly *a:     pointer to input polynomial
*              - int i:             index of to be serialized polynomial in serialized polyec
**************************************************/
void poly_packcompress(unsigned char *r, const poly *a, int i) {
    pack_bits(r + i * MLWQ_POLY_U_BYTES, a, BIT_U);
}

/*************************************************
* Name:        poly_unpackdecompress
*
* Description: Deserialization and subsequent compression of a polynomial of a polyvec,
*              Used to uncompress a polyvec one poly at a time in a loop.
*
* Arguments:   - const poly *r:     pointer to output polynomial
*              - unsigned char *a:  pointer to input byte string representation of a polyvec (of length KYBER_POLYVECCOMPRESSEDBYTES)
*              - int i:             index of poly in polyvec to decompress
**************************************************/
void poly_unpackdecompress(poly *r, const unsigned char *a, int i) {
  unpack_bits(r, a + i * MLWQ_POLY_U_BYTES, BIT_U);
}


/*************************************************
* Name:        cmp_poly_compress
*
* Description: Serializes and consequently compares polynomial to a serialized polynomial
*
* Arguments:   - const unsigned char *r:    pointer to serialized polynomial to compare with
*              - poly *a:                   pointer to input polynomial to serialize and compare
* Returns:                                  boolean indicating whether the polynomials are equal
**************************************************/
int cmp_poly_compress(const unsigned char *r, const poly *a) {
    return cmp_pack_bits(r, a, BIT_V);
}

/*************************************************
* Name:        cmp_poly_packcompress
*
* Description: Serializes and consequently compares poly of polyvec to a serialized polyvec
*              Should be called in a loop over all poly's of a polyvec.
*
* Arguments:   - const unsigned char *r:    pointer to serialized polyvec to compare with
*              - poly *a:                   pointer to input polynomial of polyvec to serialize and compare
*              - int i:                     index of poly in polyvec to compare with
* Returns:                                  boolean indicating whether the polyvecs are equal
**************************************************/
int cmp_poly_packcompress(const unsigned char *r, const poly *a, int i) {
    return cmp_pack_bits(r + i * MLWQ_POLY_U_BYTES, a, BIT_U);
}

/*************************************************
* Name:        poly_tobytes
*
* Description: Serialization of a polynomial
*
* Arguments:   - unsigned char *r: pointer to output byte array (needs space for KYBER_POLYBYTES bytes)
*              - const poly *a:    pointer to input polynomial
**************************************************/
void poly_tobytes(unsigned char *r, poly *a) {
    pack_bits(r, a, BIT_PK);
}

/*************************************************
* Name:        poly_frombytes
*
* Description: De-serialization of a polynomial;
*              inverse of poly_tobytes
*
* Arguments:   - poly *r:                pointer to output polynomial
*              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
**************************************************/
void poly_frombytes(poly *r, const unsigned char *a) {
    unpack_bits(r, a, BIT_PK);
}

/*************************************************
* Name:        poly_frombytes_mul_16_32
*
* Description: Multiplication of a polynomial with a de-serialization of another polynomial
*              Using strategy of better accumulation.
* Arguments:   - const poly *b:          pointer to input polynomial
*              - int32_t *r_tmp:         array for accumulating unreduced results
*              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
**************************************************/
extern void frombytes_mul_asm_16_32(int32_t *r_tmp, const int16_t *b, const unsigned char *c, const int32_t zetas[64]);
void poly_frombytes_mul_16_32(int32_t *r_tmp, const poly *b, const unsigned char *a) {
    frombytes_mul_asm_16_32(r_tmp, b->coeffs, a, zetas);
}

/*************************************************
* Name:        poly_frombytes_mul_32_32
*
* Description: Multiplication of a polynomial with a de-serialization of another polynomial
*              Using strategy of better accumulation.
* Arguments:   - const poly *b:          pointer to input polynomial
*              - int32_t *r_tmp:         array for accumulating unreduced results
*              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
**************************************************/
extern void frombytes_mul_asm_acc_32_32(int32_t *r_tmp, const int16_t *b, const unsigned char *c, const int32_t zetas[64]);
void poly_frombytes_mul_32_32(int32_t *r_tmp, const poly *b, const unsigned char *a) {
    frombytes_mul_asm_acc_32_32(r_tmp, b->coeffs, a, zetas);
}

/*************************************************
* Name:        poly_frombytes_mul_32_16
*
* Description: Multiplication of a polynomial with a de-serialization of another polynomial
*              Using strategy of better accumulation.
* Arguments:   - poly *r:                pointer to output polynomial
*              - const poly *b:          pointer to input polynomial
*              - const int32_t *r_tmp:   array containing unreduced results
*              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
**************************************************/
extern void frombytes_mul_asm_acc_32_16(int16_t *r, const int16_t *b, const unsigned char *c, const int32_t zetas[64], const int32_t *r_tmp);
void poly_frombytes_mul_32_16(poly *r, const poly* b, const unsigned char *a, const int32_t *r_tmp) {
    frombytes_mul_asm_acc_32_16(r->coeffs, b->coeffs, a, zetas, r_tmp);
}

/*************************************************
* Name:        poly_getnoise_eta1
*
* Description: Sample a polynomial deterministically from a seed and a nonce,
*              with output polynomial close to centered binomial distribution
*              with parameter KYBER_ETA1
*
* Arguments:   - poly *r:                   pointer to output polynomial
*              - const unsigned char *seed: pointer to input seed (pointing to array of length KYBER_SYMBYTES bytes)
*              - unsigned char nonce:       one-byte input nonce
*              - int add:                   boolean to indicate to accumulate into r
**************************************************/
void poly_noise_eta1(poly *r, const unsigned char *seed, unsigned char nonce, int add) {
    unsigned char buf[KYBER_ETA1 * KYBER_N / 4];

    prf(buf, KYBER_ETA1 * KYBER_N / 4, seed, nonce);
    cbd_eta1(r, buf, add);
}

/*************************************************
* Name:        poly_getnoise_eta2
*
* Description: Sample a polynomial deterministically from a seed and a nonce,
*              with output polynomial close to centered binomial distribution
*              with parameter KYBER_ETA2
*
* Arguments:   - poly *r:                   pointer to output polynomial
*              - const unsigned char *seed: pointer to input seed (pointing to array of length KYBER_SYMBYTES bytes)
*              - unsigned char nonce:       one-byte input nonce
*              - int add:                   boolean to indicate to accumulate into r
**************************************************/
void poly_noise_eta2(poly *r, const unsigned char *seed, unsigned char nonce, int add) {
    unsigned char buf[KYBER_ETA2 * KYBER_N / 4];

    prf(buf, KYBER_ETA2 * KYBER_N / 4, seed, nonce);
    cbd_eta2(r, buf, add);
}

/*************************************************
* Name:        poly_basemul_opt_16_32
*
* Description: Multiplication of two polynomials using asymmetric multiplication.
*              Cached values are generated during matrix-vector product.
*              Using strategy of better accumulation (initial step).
* Arguments:   - const poly *a:       pointer to input polynomial
*              - const poly *b:       pointer to input polynomial
*              - const poly *a_prime: pointer to a pre-multiplied by zetas 
*              - int32_t *r_tmp:      array for accumulating unreduced results
**************************************************/
extern void basemul_asm_opt_16_32(int32_t *, const int16_t *, const int16_t *, const int16_t *);
void poly_basemul_opt_16_32(int32_t *r_tmp, const poly *a, const poly *b, const poly *a_prime) {
    basemul_asm_opt_16_32(r_tmp, a->coeffs, b->coeffs, a_prime->coeffs);
}

/*************************************************
* Name:        poly_basemul_acc_opt_32_32
*
* Description: Multiplication of two polynomials using asymmetric multiplication.
*              Cached values are generated during matrix-vector product.
*              Using strategy of better accumulation.
* Arguments:   - const poly *a:       pointer to input polynomial
*              - const poly *b:       pointer to input polynomial
*              - const poly *a_prime: pointer to a pre-multiplied by zetas 
*              - int32_t *r_tmp:      array for accumulating unreduced results
**************************************************/
extern void basemul_asm_acc_opt_32_32(int32_t *, const int16_t *, const int16_t *, const int16_t *);
void poly_basemul_acc_opt_32_32(int32_t *r, const poly *a, const poly *b, const poly *a_prime) {
    basemul_asm_acc_opt_32_32(r, a->coeffs, b->coeffs, a_prime->coeffs);
}

/*************************************************
* Name:        poly_basemul_acc_opt_32_16
*
* Description: Multiplication of two polynomials using asymmetric multiplication.
*              Cached values are generated during matrix-vector product.
*              Using strategy of better accumulation (final step).
* Arguments:   - const poly *a:        pointer to input polynomial
*              - const poly *b:        pointer to input polynomial
*              - const poly *a_prime:  pointer to a pre-multiplied by zetas 
*              - poly *r:              pointer to output polynomial
*              - const int32_t *r_tmp: array containing unreduced results
**************************************************/
extern void basemul_asm_acc_opt_32_16(int16_t *, const int16_t *, const int16_t *, const int16_t *, const int32_t *);
void poly_basemul_acc_opt_32_16(poly *r, const poly *a, const poly *b, const poly *a_prime, const int32_t * r_tmp) {
    basemul_asm_acc_opt_32_16(r->coeffs, a->coeffs, b->coeffs, a_prime->coeffs, r_tmp);
}

/*************************************************
* Name:        poly_ntt
*
* Description: Computes negacyclic number-theoretic transform (NTT) of
*              a polynomial in place;
*              inputs assumed to be in normal order, output in bitreversed order
*
* Arguments:   - uint16_t *r: pointer to in/output polynomial
**************************************************/
void poly_ntt(poly *r) {
    ntt(r->coeffs);
}

/*************************************************
* Name:        poly_invntt
*
* Description: Computes inverse of negacyclic number-theoretic transform (NTT) of
*              a polynomial in place;
*              inputs assumed to be in bitreversed order, output in normal order
*
* Arguments:   - uint16_t *a: pointer to in/output polynomial
**************************************************/
void poly_invntt(poly *r) {
    invntt(r->coeffs);
}

extern void asm_fromplant(int16_t *r);
/*************************************************
* Name:        poly_fromplantt
*
* Description: Inplace conversion of all coefficients of a polynomial
*              from Montgomery domain to normal domain
*
* Arguments:   - poly *r:       pointer to input/output polynomial
**************************************************/
void poly_fromplant(poly *r) {
  asm_fromplant(r->coeffs);
}

extern void asm_barrett_reduce(int16_t *r);
/*************************************************
* Name:        poly_reduce
*
* Description: Applies Barrett reduction to all coefficients of a polynomial
*              for details of the Barrett reduction see comments in reduce.c
*
* Arguments:   - poly *r:       pointer to input/output polynomial
**************************************************/
void poly_reduce(poly *r) {
  asm_barrett_reduce(r->coeffs);
}

extern void pointwise_add(int16_t *, const int16_t *, const int16_t *);
/*************************************************
* Name:        poly_add
*
* Description: Add two polynomials
*
* Arguments: - poly *r:       pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void poly_add(poly *r, const poly *a, const poly *b) {
    pointwise_add(r->coeffs,a->coeffs,b->coeffs);
}


extern void pointwise_sub(int16_t *, const int16_t *, const int16_t *);
/*************************************************
* Name:        poly_sub
*
* Description: Subtract two polynomials
*
* Arguments: - poly *r:       pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void poly_sub(poly *r, const poly *a, const poly *b) {
    pointwise_sub(r->coeffs,a->coeffs,b->coeffs);
}

void cmov_int16(int16_t *r, int16_t v, uint16_t b);

/*************************************************
* Name:        poly_frommsg
*
* Description: Convert 32-byte message to polynomial
*
* Arguments:   - poly *r:                  pointer to output polynomial
*              - const unsigned char *msg: pointer to input message
**************************************************/
void poly_frommsg(poly *r, const uint8_t msg[KYBER_INDCPA_MSGBYTES])
{
  unsigned int i,j;

#if (KYBER_INDCPA_MSGBYTES != KYBER_N/8)
#error "KYBER_INDCPA_MSGBYTES must be equal to KYBER_N/8 bytes!"
#endif

  for(i=0;i<KYBER_N/8;i++) {
    for(j=0;j<8;j++) {
      r->coeffs[8*i+j] = 0;
      cmov_int16(r->coeffs+8*i+j, ((KYBER_Q+1)/2), (msg[i] >> j)&1);
    }
  }
}

/*************************************************
* Name:        poly_tomsg
*
* Description: Convert polynomial to 32-byte message
*
* Arguments:   - unsigned char *msg: pointer to output message
*              - const poly *a:      pointer to input polynomial
**************************************************/
void poly_tomsg(unsigned char msg[KYBER_SYMBYTES], poly *a) {
    uint32_t t;
    int i, j;

    for (i = 0; i < KYBER_SYMBYTES; i++) {
        msg[i] = 0;
        for (j = 0; j < 8; j++) {
            t  = a->coeffs[8*i+j];
            t <<= 1;
            t += 1665;
            t *= 80635;
            t >>= 28;
            t &= 1;
            msg[i] |= t << j;
        }
    }
}

/*************************************************
* Name:        poly_zeroize
*
* Description: Zeros a polynomial
*
* Arguments:   - poly *p: pointer to polynomial
**************************************************/
void poly_zeroize(poly *p) {
  int i;
  for(i = 0; i < KYBER_N; i++)
   p->coeffs[i] = 0;
}
