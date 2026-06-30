

/********************************************************************************************
* FrodoKEM: Learning with Errors Key Encapsulation
*
* Abstract: additional functions for FrodoKEM
*********************************************************************************************/

#include <stdint.h>
#include <string.h>

#include "api.h"
#include "common.h"
#include "params.h"
#include "../frost_e8.h"

#define min(x, y) (((x) < (y)) ? (x) : (y))

uint16_t LE_TO_UINT16(uint16_t n) {     // config.h
    return (((uint8_t *) &n)[0] | (((uint8_t *) &n)[1] << 8));
}

uint16_t UINT16_TO_LE(uint16_t n) {
    uint16_t y;
    uint8_t *z = (uint8_t *) &y;
    z[0] = n & 0xFF;
    z[1] = (n & 0xFF00) >> 8;
    return y;
}


uint16_t xs(const uint16_t *s, const uint16_t *a_row);

void frodo_mul_bs(uint16_t *out, const uint16_t *b, const uint16_t *s) {
    /* b is NBAR_S x N, s is NBAR_R x N; output is NBAR_S x NBAR_R. */
    for (size_t row = 0; row < PARAMS_NBAR_S; row++) {
        for (size_t col = 0; col < PARAMS_NBAR_R; col++) {
            out[row * PARAMS_NBAR_R + col] =
                xs(b + row * PARAMS_N, s + col * PARAMS_N);
        }
    }
}

void frodo_mul_add_sb_plus_e(uint16_t *out, const uint16_t *b, const uint16_t *s) {
    /* b is N x NBAR_R, s is NBAR_S x N; output is NBAR_S x NBAR_R. */
    for (size_t row = 0; row < PARAMS_NBAR_S; row++) {
        for (size_t col = 0; col < PARAMS_NBAR_R; col++) {
            int32_t acc = (int32_t)out[row * PARAMS_NBAR_R + col];
            for (size_t j = 0; j < PARAMS_N; j++) {
                acc += (int32_t)b[j * PARAMS_NBAR_R + col] *
                       (int32_t)(int16_t)s[row * PARAMS_N + j];
            }
            out[row * PARAMS_NBAR_R + col] = (uint16_t)acc;
        }
    }
}

void frodo_add(uint16_t *out, const uint16_t *a, const uint16_t *b) {
    for (size_t i = 0; i < PARAMS_NBAR_R * PARAMS_NBAR_S; i++) {
        out[i] = (uint16_t)((a[i] + b[i]) & ((1u << PARAMS_LOGQ) - 1u));
    }
}

void frodo_sub(uint16_t *out, const uint16_t *a, const uint16_t *b) {
    for (size_t i = 0; i < PARAMS_NBAR_R * PARAMS_NBAR_S; i++) {
        out[i] = (uint16_t)((a[i] - b[i]) & ((1u << PARAMS_LOGQ) - 1u));
    }
}

void frodo_key_encode(uint16_t *out, const uint16_t *in) {
    frost_e8_encode_u16(out, (const uint8_t *)in);
}

void frodo_key_decode(uint16_t *out, const uint16_t *in) {
    frost_e8_decode_u16((uint8_t *)out, in);
}


void frodo_pack(uint8_t *out, size_t outlen, const uint16_t *in, size_t inlen, uint8_t lsb) { // util.c
    // Pack the input uint16 vector into a char output vector, copying lsb bits from each input element.
    // If inlen * lsb / 8 > outlen, only outlen * 8 bits are copied.
    memset(out, 0, outlen);

    size_t i = 0;            // whole bytes already filled in
    size_t j = 0;            // whole uint16_t already copied
    uint16_t w = 0;          // the leftover, not yet copied
    uint8_t bits = 0;        // the number of lsb in w

    while (i < outlen && (j < inlen || ((j == inlen) && (bits > 0)))) {
        /*
        in: |        |        |********|********|
                              ^
                              j
        w : |   ****|
                ^
               bits
        out:|**|**|**|**|**|**|**|**|* |
                                    ^^
                                    ib
        */
        uint8_t b = 0;  // bits in out[i] already filled in
        while (b < 8) {
            int nbits = min(8 - b, bits);
            uint16_t mask = (1 << nbits) - 1;
            uint8_t t = (uint8_t) ((w >> (bits - nbits)) & mask);  // the bits to copy from w to out
            out[i] = out[i] + (t << (8 - b - nbits));
            b += (uint8_t) nbits;
            bits -= (uint8_t) nbits;
            w &= ~(mask << bits);  // not strictly necessary; mostly for debugging

            if (bits == 0) {
                if (j < inlen) {
                    w = in[j];
                    bits = lsb;
                    j++;
                } else {
                    break;  // the input vector is exhausted
                }
            }
        }
        if (b == 8) {  // out[i] is filled in
            i++;
        }
    }
}


void frodo_unpack(uint16_t *out, size_t outlen, const uint8_t *in, size_t inlen, uint8_t lsb) {
    // Unpack the input char vector into a uint16_t output vector, copying lsb bits
    // for each output element from input. outlen must be at least ceil(inlen * 8 / lsb).
    memset(out, 0, outlen * sizeof(uint16_t));

    size_t i = 0;            // whole uint16_t already filled in
    size_t j = 0;            // whole bytes already copied
    uint8_t w = 0;           // the leftover, not yet copied
    uint8_t bits = 0;        // the number of lsb bits of w

    while (i < outlen && (j < inlen || ((j == inlen) && (bits > 0)))) {
        /*
        in: |  |  |  |  |  |  |**|**|...
                              ^
                              j
        w : | *|
              ^
              bits
        out:|   *****|   *****|   ***  |        |...
                              ^   ^
                              i   b
        */
        uint8_t b = 0;  // bits in out[i] already filled in
        while (b < lsb) {
            int nbits = min(lsb - b, bits);
            uint16_t mask = (1 << nbits) - 1;
            uint8_t t = (w >> (bits - nbits)) & mask;  // the bits to copy from w to out
            out[i] = out[i] + (t << (lsb - b - nbits));
            b += (uint8_t) nbits;
            bits -= (uint8_t) nbits;
            w &= ~(mask << bits);  // not strictly necessary; mostly for debugging

            if (bits == 0) {
                if (j < inlen) {
                    w = in[j];
                    bits = 8;
                    j++;
                } else {
                    break;  // the input vector is exhausted
                }
            }
        }
        if (b == lsb) {  // out[i] is filled in
            i++;
        }
    }
}


int8_t ct_verify(const uint16_t *a, const uint16_t *b, size_t len) {
    // Compare two arrays in constant time.
    // Returns 0 if the byte arrays are equal, -1 otherwise.
    uint16_t r = 0;

    for (size_t i = 0; i < len; i++) {
        r |= a[i] ^ b[i];
    }

    r = (-(int16_t)(r >> 1) | -(int16_t)(r & 1)) >> (8 * sizeof(uint16_t) -1);
    return (int8_t)r;
}


void ct_select(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len, int8_t selector) {
    // Select one of the two input arrays to be moved to r
    // If (selector == 0) then load r with a, else if (selector == -1) load r with b

    for (size_t i = 0; i < len; i++) {
        r[i] = (~selector & a[i]) | (selector & b[i]);
    }
}



void clear_bytes(uint8_t *mem, size_t n) {
    // Clear 8-bit bytes from memory. "n" indicates the number of bytes to be zeroed.
    // This function uses the volatile type qualifier to inform the compiler not to optimize out the memory clearing.
    volatile uint8_t *v = mem;

    for (size_t i = 0; i < n; i++) {
        v[i] = 0;
    }
}
