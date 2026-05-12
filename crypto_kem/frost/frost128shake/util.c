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

#define min(x, y) (((x) < (y)) ? (x) : (y))

uint16_t frodo_le_to_uint16(uint16_t n)
{
    return (((uint8_t *)&n)[0] | (((uint8_t *)&n)[1] << 8));
}

uint16_t frodo_uint16_to_le(uint16_t n)
{
    uint16_t y;
    uint8_t *z = (uint8_t *)&y;
    z[0] = n & 0xFF;
    z[1] = (n & 0xFF00) >> 8;
    return y;
}

uint16_t xs(const uint16_t *s, const uint16_t *a_row);
void frodo_mul_bs(uint16_t *out, const uint16_t *b, const uint16_t *s)
{
    int i;

    for (i = 0; i < PARAMS_NBAR; i++) {
        out[i * PARAMS_NBAR + 0] = xs(b + i * PARAMS_N, s + 0 * PARAMS_N);
        out[i * PARAMS_NBAR + 1] = xs(b + i * PARAMS_N, s + 1 * PARAMS_N);
        out[i * PARAMS_NBAR + 2] = xs(b + i * PARAMS_N, s + 2 * PARAMS_N);
        out[i * PARAMS_NBAR + 3] = xs(b + i * PARAMS_N, s + 3 * PARAMS_N);
        out[i * PARAMS_NBAR + 4] = xs(b + i * PARAMS_N, s + 4 * PARAMS_N);
        out[i * PARAMS_NBAR + 5] = xs(b + i * PARAMS_N, s + 5 * PARAMS_N);
        out[i * PARAMS_NBAR + 6] = xs(b + i * PARAMS_N, s + 6 * PARAMS_N);
        out[i * PARAMS_NBAR + 7] = xs(b + i * PARAMS_N, s + 7 * PARAMS_N);
    }
}

void sb(uint16_t *out, const uint16_t *s, const uint16_t *a);
void frodo_mul_add_sb_plus_e(uint16_t *out, const uint16_t *b, const uint16_t *s)
{
    for (size_t k = 0; k < PARAMS_N; k += 8) {
        sb(out + 0, s + k, b + k * PARAMS_NBAR + 0);
        sb(out + 1, s + k, b + k * PARAMS_NBAR + 1);
        sb(out + 2, s + k, b + k * PARAMS_NBAR + 2);
        sb(out + 3, s + k, b + k * PARAMS_NBAR + 3);
        sb(out + 4, s + k, b + k * PARAMS_NBAR + 4);
        sb(out + 5, s + k, b + k * PARAMS_NBAR + 5);
        sb(out + 6, s + k, b + k * PARAMS_NBAR + 6);
        sb(out + 7, s + k, b + k * PARAMS_NBAR + 7);
    }
}

void frodo_add(uint16_t *out, const uint16_t *a, const uint16_t *b)
{
    for (size_t i = 0; i < (PARAMS_NBAR * PARAMS_NBAR); i++) {
        out[i] = (a[i] + b[i]) & ((1 << PARAMS_LOGQ) - 1);
    }
}

void frodo_sub(uint16_t *out, const uint16_t *a, const uint16_t *b)
{
    for (size_t i = 0; i < (PARAMS_NBAR * PARAMS_NBAR); i++) {
        out[i] = (a[i] - b[i]) & ((1 << PARAMS_LOGQ) - 1);
    }
}

void frodo_key_encode(uint16_t *out, const uint16_t *in)
{
    unsigned int i, j, npieces_word = 8;
    unsigned int nwords = (PARAMS_NBAR * PARAMS_NBAR) / 8;
    uint64_t temp, mask = ((uint64_t)1 << PARAMS_EXTRACTED_BITS) - 1;
    uint16_t *pos = out;

    for (i = 0; i < nwords; i++) {
        temp = 0;
        for (j = 0; j < PARAMS_EXTRACTED_BITS; j++) {
            temp |= ((uint64_t)((uint8_t *)in)[i * PARAMS_EXTRACTED_BITS + j]) << (8 * j);
        }
        for (j = 0; j < npieces_word; j++) {
            *pos = (uint16_t)((temp & mask) << (PARAMS_LOGQ - PARAMS_EXTRACTED_BITS));
            temp >>= PARAMS_EXTRACTED_BITS;
            pos++;
        }
    }
}

void frodo_key_decode(uint16_t *out, const uint16_t *in)
{
    unsigned int i, j, index = 0, npieces_word = 8;
    unsigned int nwords = (PARAMS_NBAR * PARAMS_NBAR) / 8;
    uint16_t temp, maskex = ((uint16_t)1 << PARAMS_EXTRACTED_BITS) - 1, maskq = ((uint16_t)1 << PARAMS_LOGQ) - 1;
    uint8_t *pos = (uint8_t *)out;
    uint64_t templong;

    for (i = 0; i < nwords; i++) {
        templong = 0;
        for (j = 0; j < npieces_word; j++) {
            temp = ((in[index] & maskq) + (1 << (PARAMS_LOGQ - PARAMS_EXTRACTED_BITS - 1))) >> (PARAMS_LOGQ - PARAMS_EXTRACTED_BITS);
            templong |= ((uint64_t)(temp & maskex)) << (PARAMS_EXTRACTED_BITS * j);
            index++;
        }
        for (j = 0; j < PARAMS_EXTRACTED_BITS; j++) {
            pos[i * PARAMS_EXTRACTED_BITS + j] = (templong >> (8 * j)) & 0xFF;
        }
    }
}

void frodo_pack(uint8_t *out, size_t outlen, const uint16_t *in, size_t inlen, uint8_t lsb)
{
    memset(out, 0, outlen);

    size_t i = 0;
    size_t j = 0;
    uint16_t w = 0;
    uint8_t bits = 0;

    while (i < outlen && (j < inlen || ((j == inlen) && (bits > 0)))) {
        uint8_t b = 0;
        while (b < 8) {
            int nbits = min(8 - b, bits);
            uint16_t mask = (1 << nbits) - 1;
            uint8_t t = (uint8_t)((w >> (bits - nbits)) & mask);
            out[i] = out[i] + (t << (8 - b - nbits));
            b += (uint8_t)nbits;
            bits -= (uint8_t)nbits;
            w &= ~(mask << bits);

            if (bits == 0) {
                if (j < inlen) {
                    w = in[j];
                    bits = lsb;
                    j++;
                } else {
                    break;
                }
            }
        }
        if (b == 8) {
            i++;
        }
    }
}

void frodo_unpack(uint16_t *out, size_t outlen, const uint8_t *in, size_t inlen, uint8_t lsb)
{
    memset(out, 0, outlen * sizeof(uint16_t));

    size_t i = 0;
    size_t j = 0;
    uint8_t w = 0;
    uint8_t bits = 0;

    while (i < outlen && (j < inlen || ((j == inlen) && (bits > 0)))) {
        uint8_t b = 0;
        while (b < lsb) {
            int nbits = min(lsb - b, bits);
            uint16_t mask = (1 << nbits) - 1;
            uint8_t t = (w >> (bits - nbits)) & mask;
            out[i] = out[i] + (t << (lsb - b - nbits));
            b += (uint8_t)nbits;
            bits -= (uint8_t)nbits;
            w &= ~(mask << bits);

            if (bits == 0) {
                if (j < inlen) {
                    w = in[j];
                    bits = 8;
                    j++;
                } else {
                    break;
                }
            }
        }
        if (b == lsb) {
            i++;
        }
    }
}

int8_t frodo_ct_verify(const uint16_t *a, const uint16_t *b, size_t len)
{
    uint16_t r = 0;

    for (size_t i = 0; i < len; i++) {
        r |= a[i] ^ b[i];
    }

    r = (-(int16_t)(r >> 1) | -(int16_t)(r & 1)) >> (8 * sizeof(uint16_t) - 1);
    return (int8_t)r;
}

void frodo_ct_select(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len, int8_t selector)
{
    for (size_t i = 0; i < len; i++) {
        r[i] = (~selector & a[i]) | (selector & b[i]);
    }
}

void frodo_clear_bytes(uint8_t *mem, size_t n)
{
    volatile uint8_t *v = mem;

    for (size_t i = 0; i < n; i++) {
        v[i] = 0;
    }
}
