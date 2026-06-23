/* MAMBA-Viper implementation and implementation support layer where applicable.
 * Viper PKE support with the current polynomial-arithmetic backend, public dithered
 * quantization, and an isolated schoolbook oracle for tests/debug only.
 */

#include "viper.h"
#include "viper_arith.h"
#include "fips202.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint16_t modq_int(int64_t x) { return (uint16_t)x & VIPER_Q_MASK; }

void viper_pack_bits(unsigned char *out, const uint16_t *in, size_t n, unsigned bits) {
  size_t outlen = (n * bits + 7) / 8;
  uint32_t acc = 0;
  unsigned accbits = 0;
  size_t j = 0;
  uint16_t mask = (uint16_t)((1u << bits) - 1u);
  memset(out, 0, outlen);
  for (size_t i = 0; i < n; i++) {
    acc |= ((uint32_t)in[i] & mask) << accbits;
    accbits += bits;
    while (accbits >= 8) {
      out[j++] = (unsigned char)(acc & 0xffu);
      acc >>= 8;
      accbits -= 8;
    }
  }
  if (accbits) out[j] = (unsigned char)(acc & 0xffu);
}

void viper_unpack_bits(uint16_t *out, const unsigned char *in, size_t n, unsigned bits) {
  uint32_t acc = 0;
  unsigned accbits = 0;
  size_t j = 0;
  uint16_t mask = (uint16_t)((1u << bits) - 1u);
  for (size_t i = 0; i < n; i++) {
    while (accbits < bits) {
      acc |= ((uint32_t)in[j++]) << accbits;
      accbits += 8;
    }
    out[i] = (uint16_t)(acc & mask);
    acc >>= bits;
    accbits -= bits;
  }
}

uint16_t viper_quantize(uint16_t x, uint16_t d, unsigned t) {
  const unsigned shift = 12u - t;
  const uint16_t mask = (uint16_t)((1u << t) - 1u);
  uint16_t y = (uint16_t)((x + d) & VIPER_Q_MASK);
  return (uint16_t)(((y + (1u << (shift - 1u))) >> shift) & mask);
}

uint16_t viper_reconstruct(uint16_t b, uint16_t d, unsigned t) {
  const unsigned shift = 12u - t;
  return (uint16_t)(((b << shift) - d) & VIPER_Q_MASK);
}

static uint16_t quantize_shift(uint16_t x, uint16_t d, unsigned shift, uint16_t mask) {
  uint16_t y = (uint16_t)((x + d) & VIPER_Q_MASK);
  return (uint16_t)(((y + (1u << (shift - 1u))) >> shift) & mask);
}

static uint16_t reconstruct_shift(uint16_t b, uint16_t d, unsigned shift) {
  return (uint16_t)(((uint16_t)(b << shift) - d) & VIPER_Q_MASK);
}

static int16_t center_q(uint16_t x) {
  x &= VIPER_Q_MASK;
  return (int16_t)(x < (VIPER_Q >> 1) ? x : (int32_t)x - VIPER_Q);
}

static void viper_pack_secret12(unsigned char out[VIPER_POLYBYTES_12], const uint16_t in[VIPER_N]) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 2, j += 3) {
    uint16_t x0 = in[i + 0] & VIPER_Q_MASK;
    uint16_t x1 = in[i + 1] & VIPER_Q_MASK;
    out[j + 0] = (unsigned char)x0;
    out[j + 1] = (unsigned char)((x0 >> 8) | (x1 << 4));
    out[j + 2] = (unsigned char)(x1 >> 4);
  }
}

static void viper_unpack_secret12_centered(int16_t out[VIPER_N], const unsigned char in[VIPER_POLYBYTES_12]) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 2, j += 3) {
    uint16_t x0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x0fu) << 8));
    uint16_t x1 = (uint16_t)((in[j + 1] >> 4) | ((uint16_t)in[j + 2] << 4));
    out[i + 0] = center_q(x0);
    out[i + 1] = center_q(x1);
  }
}

void viper_quantize_pack_t10_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 4, j += 5) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 2u, 0x03ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 2u, 0x03ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 2u, 0x03ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 2u, 0x03ffu);
    out[j + 0] = (unsigned char)q0;
    out[j + 1] = (unsigned char)((q0 >> 8) | (q1 << 2));
    out[j + 2] = (unsigned char)((q1 >> 6) | (q2 << 4));
    out[j + 3] = (unsigned char)((q2 >> 4) | (q3 << 6));
    out[j + 4] = (unsigned char)(q3 >> 2);
  }
}

void viper_quantize_pack_t9_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 9) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 3u, 0x01ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 3u, 0x01ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 3u, 0x01ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 3u, 0x01ffu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 3u, 0x01ffu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 3u, 0x01ffu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 3u, 0x01ffu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 3u, 0x01ffu);
    out[j + 0] = (unsigned char)q0;
    out[j + 1] = (unsigned char)((q0 >> 8) | (q1 << 1));
    out[j + 2] = (unsigned char)((q1 >> 7) | (q2 << 2));
    out[j + 3] = (unsigned char)((q2 >> 6) | (q3 << 3));
    out[j + 4] = (unsigned char)((q3 >> 5) | (q4 << 4));
    out[j + 5] = (unsigned char)((q4 >> 4) | (q5 << 5));
    out[j + 6] = (unsigned char)((q5 >> 3) | (q6 << 6));
    out[j + 7] = (unsigned char)((q6 >> 2) | (q7 << 7));
    out[j + 8] = (unsigned char)(q7 >> 1);
  }
}

void viper_quantize_pack_t4_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 4) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 8u, 0x000fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 8u, 0x000fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 8u, 0x000fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 8u, 0x000fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 8u, 0x000fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 8u, 0x000fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 8u, 0x000fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 8u, 0x000fu);
    out[j + 0] = (unsigned char)(q0 | (q1 << 4));
    out[j + 1] = (unsigned char)(q2 | (q3 << 4));
    out[j + 2] = (unsigned char)(q4 | (q5 << 4));
    out[j + 3] = (unsigned char)(q6 | (q7 << 4));
  }
}

void viper_quantize_pack_t3_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 3) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 9u, 0x0007u);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 9u, 0x0007u);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 9u, 0x0007u);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 9u, 0x0007u);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 9u, 0x0007u);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 9u, 0x0007u);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 9u, 0x0007u);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 9u, 0x0007u);
    out[j + 0] = (unsigned char)(q0 | (q1 << 3) | (q2 << 6));
    out[j + 1] = (unsigned char)((q2 >> 2) | (q3 << 1) | (q4 << 4) | (q5 << 7));
    out[j + 2] = (unsigned char)((q5 >> 1) | (q6 << 2) | (q7 << 5));
  }
}

static void viper_quantize_pack_t5_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 5) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 7u, 0x001fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 7u, 0x001fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 7u, 0x001fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 7u, 0x001fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 7u, 0x001fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 7u, 0x001fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 7u, 0x001fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 7u, 0x001fu);
    out[j + 0] = (unsigned char)(q0 | (q1 << 5));
    out[j + 1] = (unsigned char)((q1 >> 3) | (q2 << 2) | (q3 << 7));
    out[j + 2] = (unsigned char)((q3 >> 1) | (q4 << 4));
    out[j + 3] = (unsigned char)((q4 >> 4) | (q5 << 1) | (q6 << 6));
    out[j + 4] = (unsigned char)((q6 >> 2) | (q7 << 3));
  }
}

static unsigned viper_quantize_pack_cmp_t10_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0; i < VIPER_N; i += 4, j += 5) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 2u, 0x03ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 2u, 0x03ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 2u, 0x03ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 2u, 0x03ffu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)q0);
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q0 >> 8) | (q1 << 2)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q1 >> 6) | (q2 << 4)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q2 >> 4) | (q3 << 6)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)(q3 >> 2));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t9_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 9) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 3u, 0x01ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 3u, 0x01ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 3u, 0x01ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 3u, 0x01ffu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 3u, 0x01ffu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 3u, 0x01ffu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 3u, 0x01ffu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 3u, 0x01ffu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)q0);
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q0 >> 8) | (q1 << 1)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q1 >> 7) | (q2 << 2)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q2 >> 6) | (q3 << 3)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)((q3 >> 5) | (q4 << 4)));
    diff |= (unsigned)(bytes[j + 5] ^ (unsigned char)((q4 >> 4) | (q5 << 5)));
    diff |= (unsigned)(bytes[j + 6] ^ (unsigned char)((q5 >> 3) | (q6 << 6)));
    diff |= (unsigned)(bytes[j + 7] ^ (unsigned char)((q6 >> 2) | (q7 << 7)));
    diff |= (unsigned)(bytes[j + 8] ^ (unsigned char)(q7 >> 1));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t4_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 4) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 8u, 0x000fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 8u, 0x000fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 8u, 0x000fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 8u, 0x000fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 8u, 0x000fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 8u, 0x000fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 8u, 0x000fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 8u, 0x000fu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)(q0 | (q1 << 4)));
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)(q2 | (q3 << 4)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)(q4 | (q5 << 4)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)(q6 | (q7 << 4)));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t3_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 3) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 9u, 0x0007u);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 9u, 0x0007u);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 9u, 0x0007u);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 9u, 0x0007u);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 9u, 0x0007u);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 9u, 0x0007u);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 9u, 0x0007u);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 9u, 0x0007u);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)(q0 | (q1 << 3) | (q2 << 6)));
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q2 >> 2) | (q3 << 1) | (q4 << 4) | (q5 << 7)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q5 >> 1) | (q6 << 2) | (q7 << 5)));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t5_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 5) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither[i + 0], 7u, 0x001fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither[i + 1], 7u, 0x001fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither[i + 2], 7u, 0x001fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither[i + 3], 7u, 0x001fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither[i + 4], 7u, 0x001fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither[i + 5], 7u, 0x001fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither[i + 6], 7u, 0x001fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither[i + 7], 7u, 0x001fu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)(q0 | (q1 << 5)));
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q1 >> 3) | (q2 << 2) | (q3 << 7)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q3 >> 1) | (q4 << 4)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q4 >> 4) | (q5 << 1) | (q6 << 6)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)((q6 >> 2) | (q7 << 3)));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_bits(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither, unsigned bits) {
  uint32_t acc = 0;
  unsigned accbits = 0;
  unsigned diff = 0;
  size_t j = 0;
  uint16_t mask = (uint16_t)((1u << bits) - 1u);
  for (size_t i = 0; i < VIPER_N; i++) {
    acc |= ((uint32_t)quantize_shift(poly[i], dither[i], 12u - bits, mask)) << accbits;
    accbits += bits;
    while (accbits >= 8) {
      diff |= (unsigned)(bytes[j++] ^ (unsigned char)(acc & 0xffu));
      acc >>= 8;
      accbits -= 8;
    }
  }
  if (accbits) {
    diff |= (unsigned)(bytes[j] ^ (unsigned char)(acc & 0xffu));
  }
  return diff;
}

static unsigned __attribute__((unused)) viper_quantize_pack_cmp_array(const unsigned char *bytes, const uint16_t *poly, const uint16_t *dither, unsigned bits) {
  switch (bits) {
    case 10: return viper_quantize_pack_cmp_t10_array(bytes, poly, dither);
    case 9: return viper_quantize_pack_cmp_t9_array(bytes, poly, dither);
    case 5: return viper_quantize_pack_cmp_t5_array(bytes, poly, dither);
    case 4: return viper_quantize_pack_cmp_t4_array(bytes, poly, dither);
    case 3: return viper_quantize_pack_cmp_t3_array(bytes, poly, dither);
    default: return viper_quantize_pack_cmp_bits(bytes, poly, dither, bits);
  }
}

void viper_unpack_reconstruct_t10_array(uint16_t *out, const unsigned char *in, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 4, j += 5) {
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x03u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 2) | ((uint16_t)(in[j + 2] & 0x0fu) << 6));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 4) | ((uint16_t)(in[j + 3] & 0x3fu) << 4));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 6) | ((uint16_t)in[j + 4] << 2));
    out[i + 0] = reconstruct_shift(q0, dither[i + 0], 2u);
    out[i + 1] = reconstruct_shift(q1, dither[i + 1], 2u);
    out[i + 2] = reconstruct_shift(q2, dither[i + 2], 2u);
    out[i + 3] = reconstruct_shift(q3, dither[i + 3], 2u);
  }
}

void viper_unpack_reconstruct_t9_array(uint16_t *out, const unsigned char *in, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 9) {
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x01u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 1) | ((uint16_t)(in[j + 2] & 0x03u) << 7));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 2) | ((uint16_t)(in[j + 3] & 0x07u) << 6));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 3) | ((uint16_t)(in[j + 4] & 0x0fu) << 5));
    uint16_t q4 = (uint16_t)((in[j + 4] >> 4) | ((uint16_t)(in[j + 5] & 0x1fu) << 4));
    uint16_t q5 = (uint16_t)((in[j + 5] >> 5) | ((uint16_t)(in[j + 6] & 0x3fu) << 3));
    uint16_t q6 = (uint16_t)((in[j + 6] >> 6) | ((uint16_t)(in[j + 7] & 0x7fu) << 2));
    uint16_t q7 = (uint16_t)((in[j + 7] >> 7) | ((uint16_t)in[j + 8] << 1));
    out[i + 0] = reconstruct_shift(q0, dither[i + 0], 3u);
    out[i + 1] = reconstruct_shift(q1, dither[i + 1], 3u);
    out[i + 2] = reconstruct_shift(q2, dither[i + 2], 3u);
    out[i + 3] = reconstruct_shift(q3, dither[i + 3], 3u);
    out[i + 4] = reconstruct_shift(q4, dither[i + 4], 3u);
    out[i + 5] = reconstruct_shift(q5, dither[i + 5], 3u);
    out[i + 6] = reconstruct_shift(q6, dither[i + 6], 3u);
    out[i + 7] = reconstruct_shift(q7, dither[i + 7], 3u);
  }
}

void viper_unpack_reconstruct_t4_array(uint16_t *out, const unsigned char *in, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 4) {
    out[i + 0] = reconstruct_shift((uint16_t)(in[j + 0] & 0x0fu), dither[i + 0], 8u);
    out[i + 1] = reconstruct_shift((uint16_t)(in[j + 0] >> 4), dither[i + 1], 8u);
    out[i + 2] = reconstruct_shift((uint16_t)(in[j + 1] & 0x0fu), dither[i + 2], 8u);
    out[i + 3] = reconstruct_shift((uint16_t)(in[j + 1] >> 4), dither[i + 3], 8u);
    out[i + 4] = reconstruct_shift((uint16_t)(in[j + 2] & 0x0fu), dither[i + 4], 8u);
    out[i + 5] = reconstruct_shift((uint16_t)(in[j + 2] >> 4), dither[i + 5], 8u);
    out[i + 6] = reconstruct_shift((uint16_t)(in[j + 3] & 0x0fu), dither[i + 6], 8u);
    out[i + 7] = reconstruct_shift((uint16_t)(in[j + 3] >> 4), dither[i + 7], 8u);
  }
}

void viper_unpack_reconstruct_t3_array(uint16_t *out, const unsigned char *in, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 3) {
    uint16_t q0 = (uint16_t)(in[j + 0] & 0x07u);
    uint16_t q1 = (uint16_t)((in[j + 0] >> 3) & 0x07u);
    uint16_t q2 = (uint16_t)((in[j + 0] >> 6) | ((uint16_t)(in[j + 1] & 0x01u) << 2));
    uint16_t q3 = (uint16_t)((in[j + 1] >> 1) & 0x07u);
    uint16_t q4 = (uint16_t)((in[j + 1] >> 4) & 0x07u);
    uint16_t q5 = (uint16_t)((in[j + 1] >> 7) | ((uint16_t)(in[j + 2] & 0x03u) << 1));
    uint16_t q6 = (uint16_t)((in[j + 2] >> 2) & 0x07u);
    uint16_t q7 = (uint16_t)((in[j + 2] >> 5) & 0x07u);
    out[i + 0] = reconstruct_shift(q0, dither[i + 0], 9u);
    out[i + 1] = reconstruct_shift(q1, dither[i + 1], 9u);
    out[i + 2] = reconstruct_shift(q2, dither[i + 2], 9u);
    out[i + 3] = reconstruct_shift(q3, dither[i + 3], 9u);
    out[i + 4] = reconstruct_shift(q4, dither[i + 4], 9u);
    out[i + 5] = reconstruct_shift(q5, dither[i + 5], 9u);
    out[i + 6] = reconstruct_shift(q6, dither[i + 6], 9u);
    out[i + 7] = reconstruct_shift(q7, dither[i + 7], 9u);
  }
}

static void viper_unpack_reconstruct_t5_array(uint16_t *out, const unsigned char *in, const uint16_t *dither) {
  for (size_t i = 0, j = 0; i < VIPER_N; i += 8, j += 5) {
    uint16_t q0 = (uint16_t)(in[j + 0] & 0x1fu);
    uint16_t q1 = (uint16_t)((in[j + 0] >> 5) | ((uint16_t)(in[j + 1] & 0x03u) << 3));
    uint16_t q2 = (uint16_t)((in[j + 1] >> 2) & 0x1fu);
    uint16_t q3 = (uint16_t)((in[j + 1] >> 7) | ((uint16_t)(in[j + 2] & 0x0fu) << 1));
    uint16_t q4 = (uint16_t)((in[j + 2] >> 4) | ((uint16_t)(in[j + 3] & 0x01u) << 4));
    uint16_t q5 = (uint16_t)((in[j + 3] >> 1) & 0x1fu);
    uint16_t q6 = (uint16_t)((in[j + 3] >> 6) | ((uint16_t)(in[j + 4] & 0x07u) << 2));
    uint16_t q7 = (uint16_t)(in[j + 4] >> 3);
    out[i + 0] = reconstruct_shift(q0, dither[i + 0], 7u);
    out[i + 1] = reconstruct_shift(q1, dither[i + 1], 7u);
    out[i + 2] = reconstruct_shift(q2, dither[i + 2], 7u);
    out[i + 3] = reconstruct_shift(q3, dither[i + 3], 7u);
    out[i + 4] = reconstruct_shift(q4, dither[i + 4], 7u);
    out[i + 5] = reconstruct_shift(q5, dither[i + 5], 7u);
    out[i + 6] = reconstruct_shift(q6, dither[i + 6], 7u);
    out[i + 7] = reconstruct_shift(q7, dither[i + 7], 7u);
  }
}

void viper_quantize_pack_array(unsigned char *out, const uint16_t *poly, const uint16_t *dither, unsigned bits) {
  uint16_t q[VIPER_N];
  switch (bits) {
    case 10: viper_quantize_pack_t10_array(out, poly, dither); return;
    case 9: viper_quantize_pack_t9_array(out, poly, dither); return;
    case 5: viper_quantize_pack_t5_array(out, poly, dither); return;
    case 4: viper_quantize_pack_t4_array(out, poly, dither); return;
    case 3: viper_quantize_pack_t3_array(out, poly, dither); return;
    default:
      for (size_t i = 0; i < VIPER_N; i++) q[i] = viper_quantize(poly[i], dither[i], bits);
      viper_pack_bits(out, q, VIPER_N, bits);
      return;
  }
}

void viper_unpack_reconstruct_array(uint16_t *out, const unsigned char *in, const uint16_t *dither, unsigned bits) {
  uint16_t q[VIPER_N];
  switch (bits) {
    case 10: viper_unpack_reconstruct_t10_array(out, in, dither); return;
    case 9: viper_unpack_reconstruct_t9_array(out, in, dither); return;
    case 5: viper_unpack_reconstruct_t5_array(out, in, dither); return;
    case 4: viper_unpack_reconstruct_t4_array(out, in, dither); return;
    case 3: viper_unpack_reconstruct_t3_array(out, in, dither); return;
    default:
      viper_unpack_bits(q, in, VIPER_N, bits);
      for (size_t i = 0; i < VIPER_N; i++) out[i] = viper_reconstruct(q[i], dither[i], bits);
      return;
  }
}

static void shake128_label(unsigned char *out, unsigned long long outlen, const char *label, const unsigned char seed[32]) {
  unsigned char in[48];
  size_t l = strlen(label);
  memset(in, 0, sizeof(in));
  if (l > 15) l = 15;
  memcpy(in, label, l);
  memcpy(in + 16, seed, 32);
  shake128(out, outlen, in, sizeof(in));
}

typedef struct {
  const unsigned char *buf;
  size_t pos;
  uint32_t acc;
  unsigned accbits;
} viper_bitreader;

static uint16_t bitreader_read(viper_bitreader *br, unsigned bits) {
  while (br->accbits < bits) {
    br->acc |= ((uint32_t)br->buf[br->pos++]) << br->accbits;
    br->accbits += 8;
  }
  uint16_t mask = (uint16_t)((1u << bits) - 1u);
  uint16_t out = (uint16_t)(br->acc & mask);
  br->acc >>= bits;
  br->accbits -= bits;
  return out;
}

static size_t read_dither2(uint16_t *out, size_t n, const unsigned char *buf, size_t off) {
  for (size_t i = 0; i < n; i += 4, off++) {
    unsigned b = buf[off];
    out[i + 0] = (uint16_t)(b & 3u);
    out[i + 1] = (uint16_t)((b >> 2) & 3u);
    out[i + 2] = (uint16_t)((b >> 4) & 3u);
    out[i + 3] = (uint16_t)((b >> 6) & 3u);
  }
  return off;
}

static size_t read_dither3(uint16_t *out, size_t n, const unsigned char *buf, size_t off) {
  for (size_t i = 0; i < n; i += 8, off += 3) {
    uint32_t w = (uint32_t)buf[off] | ((uint32_t)buf[off + 1] << 8) | ((uint32_t)buf[off + 2] << 16);
    out[i + 0] = (uint16_t)(w & 7u);
    out[i + 1] = (uint16_t)((w >> 3) & 7u);
    out[i + 2] = (uint16_t)((w >> 6) & 7u);
    out[i + 3] = (uint16_t)((w >> 9) & 7u);
    out[i + 4] = (uint16_t)((w >> 12) & 7u);
    out[i + 5] = (uint16_t)((w >> 15) & 7u);
    out[i + 6] = (uint16_t)((w >> 18) & 7u);
    out[i + 7] = (uint16_t)((w >> 21) & 7u);
  }
  return off;
}

static size_t read_dither8(uint16_t *out, size_t n, const unsigned char *buf, size_t off) {
  for (size_t i = 0; i < n; i++, off++) out[i] = buf[off];
  return off;
}

static size_t read_dither_bits(uint16_t *out, size_t n, const unsigned char *buf, size_t off, unsigned bits) {
  viper_bitreader br = {buf + off, 0, 0, 0};
  for (size_t i = 0; i < n; i++) out[i] = bitreader_read(&br, bits);
  return off + (n * bits + 7u) / 8u;
}

void viper_gen_dither(uint16_t du[VIPER_K][VIPER_N], uint16_t dv[VIPER_N], const unsigned char mu[32]) {
  const size_t need_bits = VIPER_K * VIPER_N * (12u - VIPER_T_U) + VIPER_N * (12u - VIPER_T_V);
  const size_t need = (need_bits + 7u) / 8u;
  unsigned char buf[(VIPER_K * VIPER_N * (12u - VIPER_T_U) + VIPER_N * (12u - VIPER_T_V) + 7u) / 8u];
  size_t off = 0;
  shake128_label(buf, need, "ViperDither", mu);
  for (size_t i = 0; i < VIPER_K; i++) {
    if ((12u - VIPER_T_U) == 2u) off = read_dither2(du[i], VIPER_N, buf, off);
    else if ((12u - VIPER_T_U) == 3u) off = read_dither3(du[i], VIPER_N, buf, off);
    else off = read_dither_bits(du[i], VIPER_N, buf, off, 12u - VIPER_T_U);
  }
  if ((12u - VIPER_T_V) == 8u) (void)read_dither8(dv, VIPER_N, buf, off);
  else (void)read_dither_bits(dv, VIPER_N, buf, off, 12u - VIPER_T_V);
}

void viper_gen_dither_bytes(unsigned char dither[VIPER_DITHER_BYTES], const unsigned char mu[32]) {
  shake128_label(dither, VIPER_DITHER_BYTES, "ViperDither", mu);
}

static const unsigned char *viper_dither_u_poly(const unsigned char dither[VIPER_DITHER_BYTES], size_t i) {
  return dither + i * VIPER_DITHER_U_POLYBYTES;
}

static const unsigned char *viper_dither_v_poly(const unsigned char dither[VIPER_DITHER_BYTES]) {
  return dither + VIPER_DITHER_U_BYTES;
}

static void viper_quantize_pack_t10_d2(unsigned char *out, const uint16_t *poly, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 4, j += 5, k++) {
    uint32_t d = dither[k];
    uint16_t q0 = quantize_shift(poly[i + 0], (uint16_t)(d & 0x03u), 2u, 0x03ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], (uint16_t)((d >> 2) & 0x03u), 2u, 0x03ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], (uint16_t)((d >> 4) & 0x03u), 2u, 0x03ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], (uint16_t)((d >> 6) & 0x03u), 2u, 0x03ffu);
    out[j + 0] = (unsigned char)q0;
    out[j + 1] = (unsigned char)((q0 >> 8) | (q1 << 2));
    out[j + 2] = (unsigned char)((q1 >> 6) | (q2 << 4));
    out[j + 3] = (unsigned char)((q2 >> 4) | (q3 << 6));
    out[j + 4] = (unsigned char)(q3 >> 2);
  }
}

static void viper_quantize_pack_t9_d3(unsigned char *out, const uint16_t *poly, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 3) {
    uint32_t d = (uint32_t)dither[k] | ((uint32_t)dither[k + 1] << 8) | ((uint32_t)dither[k + 2] << 16);
    uint16_t q0 = quantize_shift(poly[i + 0], (uint16_t)(d & 0x07u), 3u, 0x01ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], (uint16_t)((d >> 3) & 0x07u), 3u, 0x01ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], (uint16_t)((d >> 6) & 0x07u), 3u, 0x01ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], (uint16_t)((d >> 9) & 0x07u), 3u, 0x01ffu);
    uint16_t q4 = quantize_shift(poly[i + 4], (uint16_t)((d >> 12) & 0x07u), 3u, 0x01ffu);
    uint16_t q5 = quantize_shift(poly[i + 5], (uint16_t)((d >> 15) & 0x07u), 3u, 0x01ffu);
    uint16_t q6 = quantize_shift(poly[i + 6], (uint16_t)((d >> 18) & 0x07u), 3u, 0x01ffu);
    uint16_t q7 = quantize_shift(poly[i + 7], (uint16_t)((d >> 21) & 0x07u), 3u, 0x01ffu);
    out[j + 0] = (unsigned char)q0;
    out[j + 1] = (unsigned char)((q0 >> 8) | (q1 << 1));
    out[j + 2] = (unsigned char)((q1 >> 7) | (q2 << 2));
    out[j + 3] = (unsigned char)((q2 >> 6) | (q3 << 3));
    out[j + 4] = (unsigned char)((q3 >> 5) | (q4 << 4));
    out[j + 5] = (unsigned char)((q4 >> 4) | (q5 << 5));
    out[j + 6] = (unsigned char)((q5 >> 3) | (q6 << 6));
    out[j + 7] = (unsigned char)((q6 >> 2) | (q7 << 7));
    out[j + 8] = (unsigned char)(q7 >> 1);
  }
}

static unsigned viper_quantize_pack_cmp_t10_d2(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 4, j += 5, k++) {
    uint32_t d = dither[k];
    uint16_t q0 = quantize_shift(poly[i + 0], (uint16_t)(d & 0x03u), 2u, 0x03ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], (uint16_t)((d >> 2) & 0x03u), 2u, 0x03ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], (uint16_t)((d >> 4) & 0x03u), 2u, 0x03ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], (uint16_t)((d >> 6) & 0x03u), 2u, 0x03ffu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)q0);
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q0 >> 8) | (q1 << 2)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q1 >> 6) | (q2 << 4)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q2 >> 4) | (q3 << 6)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)(q3 >> 2));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t9_d3(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 3) {
    uint32_t d = (uint32_t)dither[k] | ((uint32_t)dither[k + 1] << 8) | ((uint32_t)dither[k + 2] << 16);
    uint16_t q0 = quantize_shift(poly[i + 0], (uint16_t)(d & 0x07u), 3u, 0x01ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], (uint16_t)((d >> 3) & 0x07u), 3u, 0x01ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], (uint16_t)((d >> 6) & 0x07u), 3u, 0x01ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], (uint16_t)((d >> 9) & 0x07u), 3u, 0x01ffu);
    uint16_t q4 = quantize_shift(poly[i + 4], (uint16_t)((d >> 12) & 0x07u), 3u, 0x01ffu);
    uint16_t q5 = quantize_shift(poly[i + 5], (uint16_t)((d >> 15) & 0x07u), 3u, 0x01ffu);
    uint16_t q6 = quantize_shift(poly[i + 6], (uint16_t)((d >> 18) & 0x07u), 3u, 0x01ffu);
    uint16_t q7 = quantize_shift(poly[i + 7], (uint16_t)((d >> 21) & 0x07u), 3u, 0x01ffu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)q0);
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q0 >> 8) | (q1 << 1)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q1 >> 7) | (q2 << 2)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q2 >> 6) | (q3 << 3)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)((q3 >> 5) | (q4 << 4)));
    diff |= (unsigned)(bytes[j + 5] ^ (unsigned char)((q4 >> 4) | (q5 << 5)));
    diff |= (unsigned)(bytes[j + 6] ^ (unsigned char)((q5 >> 3) | (q6 << 6)));
    diff |= (unsigned)(bytes[j + 7] ^ (unsigned char)((q6 >> 2) | (q7 << 7)));
    diff |= (unsigned)(bytes[j + 8] ^ (unsigned char)(q7 >> 1));
  }
  return diff;
}

static void viper_unpack_reconstruct_t10_d2(uint16_t *out, const unsigned char *in, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 4, j += 5, k++) {
    uint32_t d = dither[k];
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x03u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 2) | ((uint16_t)(in[j + 2] & 0x0fu) << 6));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 4) | ((uint16_t)(in[j + 3] & 0x3fu) << 4));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 6) | ((uint16_t)in[j + 4] << 2));
    out[i + 0] = reconstruct_shift(q0, (uint16_t)(d & 0x03u), 2u);
    out[i + 1] = reconstruct_shift(q1, (uint16_t)((d >> 2) & 0x03u), 2u);
    out[i + 2] = reconstruct_shift(q2, (uint16_t)((d >> 4) & 0x03u), 2u);
    out[i + 3] = reconstruct_shift(q3, (uint16_t)((d >> 6) & 0x03u), 2u);
  }
}

static void viper_unpack_reconstruct_t9_d3(uint16_t *out, const unsigned char *in, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 3) {
    uint32_t d = (uint32_t)dither[k] | ((uint32_t)dither[k + 1] << 8) | ((uint32_t)dither[k + 2] << 16);
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x01u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 1) | ((uint16_t)(in[j + 2] & 0x03u) << 7));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 2) | ((uint16_t)(in[j + 3] & 0x07u) << 6));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 3) | ((uint16_t)(in[j + 4] & 0x0fu) << 5));
    uint16_t q4 = (uint16_t)((in[j + 4] >> 4) | ((uint16_t)(in[j + 5] & 0x1fu) << 4));
    uint16_t q5 = (uint16_t)((in[j + 5] >> 5) | ((uint16_t)(in[j + 6] & 0x3fu) << 3));
    uint16_t q6 = (uint16_t)((in[j + 6] >> 6) | ((uint16_t)(in[j + 7] & 0x7fu) << 2));
    uint16_t q7 = (uint16_t)((in[j + 7] >> 7) | ((uint16_t)in[j + 8] << 1));
    out[i + 0] = reconstruct_shift(q0, (uint16_t)(d & 0x07u), 3u);
    out[i + 1] = reconstruct_shift(q1, (uint16_t)((d >> 3) & 0x07u), 3u);
    out[i + 2] = reconstruct_shift(q2, (uint16_t)((d >> 6) & 0x07u), 3u);
    out[i + 3] = reconstruct_shift(q3, (uint16_t)((d >> 9) & 0x07u), 3u);
    out[i + 4] = reconstruct_shift(q4, (uint16_t)((d >> 12) & 0x07u), 3u);
    out[i + 5] = reconstruct_shift(q5, (uint16_t)((d >> 15) & 0x07u), 3u);
    out[i + 6] = reconstruct_shift(q6, (uint16_t)((d >> 18) & 0x07u), 3u);
    out[i + 7] = reconstruct_shift(q7, (uint16_t)((d >> 21) & 0x07u), 3u);
  }
}

static void viper_unpack_reconstruct_t9_d3_centered(int16_t out[VIPER_N], const unsigned char *in, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 3) {
    uint32_t d = (uint32_t)dither[k] | ((uint32_t)dither[k + 1] << 8) | ((uint32_t)dither[k + 2] << 16);
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x01u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 1) | ((uint16_t)(in[j + 2] & 0x03u) << 7));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 2) | ((uint16_t)(in[j + 3] & 0x07u) << 6));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 3) | ((uint16_t)(in[j + 4] & 0x0fu) << 5));
    uint16_t q4 = (uint16_t)((in[j + 4] >> 4) | ((uint16_t)(in[j + 5] & 0x1fu) << 4));
    uint16_t q5 = (uint16_t)((in[j + 5] >> 5) | ((uint16_t)(in[j + 6] & 0x3fu) << 3));
    uint16_t q6 = (uint16_t)((in[j + 6] >> 6) | ((uint16_t)(in[j + 7] & 0x7fu) << 2));
    uint16_t q7 = (uint16_t)((in[j + 7] >> 7) | ((uint16_t)in[j + 8] << 1));
    out[i + 0] = center_q(reconstruct_shift(q0, (uint16_t)(d & 0x07u), 3u));
    out[i + 1] = center_q(reconstruct_shift(q1, (uint16_t)((d >> 3) & 0x07u), 3u));
    out[i + 2] = center_q(reconstruct_shift(q2, (uint16_t)((d >> 6) & 0x07u), 3u));
    out[i + 3] = center_q(reconstruct_shift(q3, (uint16_t)((d >> 9) & 0x07u), 3u));
    out[i + 4] = center_q(reconstruct_shift(q4, (uint16_t)((d >> 12) & 0x07u), 3u));
    out[i + 5] = center_q(reconstruct_shift(q5, (uint16_t)((d >> 15) & 0x07u), 3u));
    out[i + 6] = center_q(reconstruct_shift(q6, (uint16_t)((d >> 18) & 0x07u), 3u));
    out[i + 7] = center_q(reconstruct_shift(q7, (uint16_t)((d >> 21) & 0x07u), 3u));
  }
}

static uint16_t dither9(const unsigned char *d, unsigned idx) {
  unsigned bit = idx * 9u;
  unsigned byte = bit >> 3;
  unsigned shift = bit & 7u;
  uint32_t w = (uint32_t)d[byte] | ((uint32_t)d[byte + 1] << 8);
  if (shift > 7u) {
    w |= (uint32_t)d[byte + 2] << 16;
  }
  return (uint16_t)((w >> shift) & 0x01ffu);
}

static uint16_t dither7(const unsigned char *d, unsigned idx) {
  unsigned bit = idx * 7u;
  unsigned byte = bit >> 3;
  unsigned shift = bit & 7u;
  uint32_t w = d[byte];
  if (byte < 6u) {
    w |= (uint32_t)d[byte + 1] << 8;
  }
  return (uint16_t)((w >> shift) & 0x007fu);
}

static void viper_quantize_pack_t3_d9(unsigned char *out, const uint16_t *poly, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 3, k += 9) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither9(dither + k, 0), 9u, 0x0007u);
    uint16_t q1 = quantize_shift(poly[i + 1], dither9(dither + k, 1), 9u, 0x0007u);
    uint16_t q2 = quantize_shift(poly[i + 2], dither9(dither + k, 2), 9u, 0x0007u);
    uint16_t q3 = quantize_shift(poly[i + 3], dither9(dither + k, 3), 9u, 0x0007u);
    uint16_t q4 = quantize_shift(poly[i + 4], dither9(dither + k, 4), 9u, 0x0007u);
    uint16_t q5 = quantize_shift(poly[i + 5], dither9(dither + k, 5), 9u, 0x0007u);
    uint16_t q6 = quantize_shift(poly[i + 6], dither9(dither + k, 6), 9u, 0x0007u);
    uint16_t q7 = quantize_shift(poly[i + 7], dither9(dither + k, 7), 9u, 0x0007u);
    out[j + 0] = (unsigned char)(q0 | (q1 << 3) | (q2 << 6));
    out[j + 1] = (unsigned char)((q2 >> 2) | (q3 << 1) | (q4 << 4) | (q5 << 7));
    out[j + 2] = (unsigned char)((q5 >> 1) | (q6 << 2) | (q7 << 5));
  }
}

static void viper_quantize_pack_t5_d7(unsigned char *out, const uint16_t *poly, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 5, k += 7) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither7(dither + k, 0), 7u, 0x001fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither7(dither + k, 1), 7u, 0x001fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither7(dither + k, 2), 7u, 0x001fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither7(dither + k, 3), 7u, 0x001fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither7(dither + k, 4), 7u, 0x001fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither7(dither + k, 5), 7u, 0x001fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither7(dither + k, 6), 7u, 0x001fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither7(dither + k, 7), 7u, 0x001fu);
    out[j + 0] = (unsigned char)(q0 | (q1 << 5));
    out[j + 1] = (unsigned char)((q1 >> 3) | (q2 << 2) | (q3 << 7));
    out[j + 2] = (unsigned char)((q3 >> 1) | (q4 << 4));
    out[j + 3] = (unsigned char)((q4 >> 4) | (q5 << 1) | (q6 << 6));
    out[j + 4] = (unsigned char)((q6 >> 2) | (q7 << 3));
  }
}

static unsigned viper_quantize_pack_cmp_t3_d9(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 3, k += 9) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither9(dither + k, 0), 9u, 0x0007u);
    uint16_t q1 = quantize_shift(poly[i + 1], dither9(dither + k, 1), 9u, 0x0007u);
    uint16_t q2 = quantize_shift(poly[i + 2], dither9(dither + k, 2), 9u, 0x0007u);
    uint16_t q3 = quantize_shift(poly[i + 3], dither9(dither + k, 3), 9u, 0x0007u);
    uint16_t q4 = quantize_shift(poly[i + 4], dither9(dither + k, 4), 9u, 0x0007u);
    uint16_t q5 = quantize_shift(poly[i + 5], dither9(dither + k, 5), 9u, 0x0007u);
    uint16_t q6 = quantize_shift(poly[i + 6], dither9(dither + k, 6), 9u, 0x0007u);
    uint16_t q7 = quantize_shift(poly[i + 7], dither9(dither + k, 7), 9u, 0x0007u);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)(q0 | (q1 << 3) | (q2 << 6)));
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q2 >> 2) | (q3 << 1) | (q4 << 4) | (q5 << 7)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q5 >> 1) | (q6 << 2) | (q7 << 5)));
  }
  return diff;
}

static unsigned viper_quantize_pack_cmp_t5_d7(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither) {
  unsigned diff = 0;
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 5, k += 7) {
    uint16_t q0 = quantize_shift(poly[i + 0], dither7(dither + k, 0), 7u, 0x001fu);
    uint16_t q1 = quantize_shift(poly[i + 1], dither7(dither + k, 1), 7u, 0x001fu);
    uint16_t q2 = quantize_shift(poly[i + 2], dither7(dither + k, 2), 7u, 0x001fu);
    uint16_t q3 = quantize_shift(poly[i + 3], dither7(dither + k, 3), 7u, 0x001fu);
    uint16_t q4 = quantize_shift(poly[i + 4], dither7(dither + k, 4), 7u, 0x001fu);
    uint16_t q5 = quantize_shift(poly[i + 5], dither7(dither + k, 5), 7u, 0x001fu);
    uint16_t q6 = quantize_shift(poly[i + 6], dither7(dither + k, 6), 7u, 0x001fu);
    uint16_t q7 = quantize_shift(poly[i + 7], dither7(dither + k, 7), 7u, 0x001fu);
    diff |= (unsigned)(bytes[j + 0] ^ (unsigned char)(q0 | (q1 << 5)));
    diff |= (unsigned)(bytes[j + 1] ^ (unsigned char)((q1 >> 3) | (q2 << 2) | (q3 << 7)));
    diff |= (unsigned)(bytes[j + 2] ^ (unsigned char)((q3 >> 1) | (q4 << 4)));
    diff |= (unsigned)(bytes[j + 3] ^ (unsigned char)((q4 >> 4) | (q5 << 1) | (q6 << 6)));
    diff |= (unsigned)(bytes[j + 4] ^ (unsigned char)((q6 >> 2) | (q7 << 3)));
  }
  return diff;
}

static void viper_unpack_reconstruct_t3_d9(uint16_t *out, const unsigned char *in, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 3, k += 9) {
    uint16_t q0 = (uint16_t)(in[j + 0] & 0x07u);
    uint16_t q1 = (uint16_t)((in[j + 0] >> 3) & 0x07u);
    uint16_t q2 = (uint16_t)((in[j + 0] >> 6) | ((uint16_t)(in[j + 1] & 0x01u) << 2));
    uint16_t q3 = (uint16_t)((in[j + 1] >> 1) & 0x07u);
    uint16_t q4 = (uint16_t)((in[j + 1] >> 4) & 0x07u);
    uint16_t q5 = (uint16_t)((in[j + 1] >> 7) | ((uint16_t)(in[j + 2] & 0x03u) << 1));
    uint16_t q6 = (uint16_t)((in[j + 2] >> 2) & 0x07u);
    uint16_t q7 = (uint16_t)((in[j + 2] >> 5) & 0x07u);
    out[i + 0] = reconstruct_shift(q0, dither9(dither + k, 0), 9u);
    out[i + 1] = reconstruct_shift(q1, dither9(dither + k, 1), 9u);
    out[i + 2] = reconstruct_shift(q2, dither9(dither + k, 2), 9u);
    out[i + 3] = reconstruct_shift(q3, dither9(dither + k, 3), 9u);
    out[i + 4] = reconstruct_shift(q4, dither9(dither + k, 4), 9u);
    out[i + 5] = reconstruct_shift(q5, dither9(dither + k, 5), 9u);
    out[i + 6] = reconstruct_shift(q6, dither9(dither + k, 6), 9u);
    out[i + 7] = reconstruct_shift(q7, dither9(dither + k, 7), 9u);
  }
}

static void viper_unpack_reconstruct_t5_d7(uint16_t *out, const unsigned char *in, const unsigned char *dither) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 5, k += 7) {
    uint16_t q0 = (uint16_t)(in[j + 0] & 0x1fu);
    uint16_t q1 = (uint16_t)((in[j + 0] >> 5) | ((uint16_t)(in[j + 1] & 0x03u) << 3));
    uint16_t q2 = (uint16_t)((in[j + 1] >> 2) & 0x1fu);
    uint16_t q3 = (uint16_t)((in[j + 1] >> 7) | ((uint16_t)(in[j + 2] & 0x0fu) << 1));
    uint16_t q4 = (uint16_t)((in[j + 2] >> 4) | ((uint16_t)(in[j + 3] & 0x01u) << 4));
    uint16_t q5 = (uint16_t)((in[j + 3] >> 1) & 0x1fu);
    uint16_t q6 = (uint16_t)((in[j + 3] >> 6) | ((uint16_t)(in[j + 4] & 0x07u) << 2));
    uint16_t q7 = (uint16_t)(in[j + 4] >> 3);
    out[i + 0] = reconstruct_shift(q0, dither7(dither + k, 0), 7u);
    out[i + 1] = reconstruct_shift(q1, dither7(dither + k, 1), 7u);
    out[i + 2] = reconstruct_shift(q2, dither7(dither + k, 2), 7u);
    out[i + 3] = reconstruct_shift(q3, dither7(dither + k, 3), 7u);
    out[i + 4] = reconstruct_shift(q4, dither7(dither + k, 4), 7u);
    out[i + 5] = reconstruct_shift(q5, dither7(dither + k, 5), 7u);
    out[i + 6] = reconstruct_shift(q6, dither7(dither + k, 6), 7u);
    out[i + 7] = reconstruct_shift(q7, dither7(dither + k, 7), 7u);
  }
}

static void viper_quantize_pack_dither_array(unsigned char *out, const uint16_t *poly, const unsigned char *dither, unsigned bits) {
  switch (bits) {
    case 10: viper_quantize_pack_t10_d2(out, poly, dither); return;
    case 9: viper_quantize_pack_t9_d3(out, poly, dither); return;
    case 5: viper_quantize_pack_t5_d7(out, poly, dither); return;
    case 3: viper_quantize_pack_t3_d9(out, poly, dither); return;
    default: return;
  }
}

static unsigned viper_quantize_pack_cmp_dither_array(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither, unsigned bits) {
  switch (bits) {
    case 10: return viper_quantize_pack_cmp_t10_d2(bytes, poly, dither);
    case 9: return viper_quantize_pack_cmp_t9_d3(bytes, poly, dither);
    case 5: return viper_quantize_pack_cmp_t5_d7(bytes, poly, dither);
    case 3: return viper_quantize_pack_cmp_t3_d9(bytes, poly, dither);
    default: return 0;
  }
}

static void viper_unpack_reconstruct_dither_array(uint16_t *out, const unsigned char *in, const unsigned char *dither, unsigned bits) {
  switch (bits) {
    case 10: viper_unpack_reconstruct_t10_d2(out, in, dither); return;
    case 9: viper_unpack_reconstruct_t9_d3(out, in, dither); return;
    case 5: viper_unpack_reconstruct_t5_d7(out, in, dither); return;
    case 3: viper_unpack_reconstruct_t3_d9(out, in, dither); return;
    default: return;
  }
}

#define VIPER_PUBLIC_A_POLYBYTES (VIPER_N * 12u / 8u)
#define VIPER_PUBLIC_DPK_POLYBYTES (VIPER_N * 2u / 8u)
#define VIPER_PUBLIC_A_BYTES (VIPER_K * VIPER_K * VIPER_PUBLIC_A_POLYBYTES)
#define VIPER_PUBLIC_DPK_BYTES (VIPER_K * VIPER_PUBLIC_DPK_POLYBYTES)
#define VIPER_PUBLIC_STREAM_BYTES (VIPER_PUBLIC_A_BYTES + VIPER_PUBLIC_DPK_BYTES)
#define VIPER_GENPUBLIC_INPUT_BYTES 36u
#define VIPER_DOMAIN_A 0x41u
#define VIPER_DOMAIN_DPK 0x44u

static void genpublic_input(unsigned char in[VIPER_GENPUBLIC_INPUT_BYTES], const unsigned char rho[32], unsigned domain, size_t i, size_t j) {
  memcpy(in, rho, 32);
  in[32] = (unsigned char)domain;
  in[33] = (unsigned char)i;
  in[34] = (unsigned char)j;
  in[35] = 0;
}

static size_t genpublic_a_offset(size_t i, size_t j) {
  return (i * VIPER_K + j) * VIPER_PUBLIC_A_POLYBYTES;
}

static size_t genpublic_dpk_offset(size_t i) {
  return VIPER_PUBLIC_A_BYTES + i * VIPER_PUBLIC_DPK_POLYBYTES;
}

void viper_gen_public_shake(unsigned char *buf, const unsigned char rho[32]) {
  unsigned char in[VIPER_GENPUBLIC_INPUT_BYTES];
  for (size_t i = 0; i < VIPER_K; i++) {
    for (size_t j = 0; j < VIPER_K; j++) {
      genpublic_input(in, rho, VIPER_DOMAIN_A, i, j);
      shake128(buf + genpublic_a_offset(i, j), VIPER_PUBLIC_A_POLYBYTES, in, sizeof(in));
    }
  }
  for (size_t i = 0; i < VIPER_K; i++) {
    genpublic_input(in, rho, VIPER_DOMAIN_DPK, i, 0);
    shake128(buf + genpublic_dpk_offset(i), VIPER_PUBLIC_DPK_POLYBYTES, in, sizeof(in));
  }
}

static void parse_A_poly(vpoly out, const unsigned char *buf) {
  for (size_t l = 0, off = 0; l < VIPER_N; l += 2, off += 3) {
    uint32_t w = (uint32_t)buf[off] | ((uint32_t)buf[off + 1] << 8) | ((uint32_t)buf[off + 2] << 16);
    out[l + 0] = (uint16_t)(w & VIPER_Q_MASK);
    out[l + 1] = (uint16_t)((w >> 12) & VIPER_Q_MASK);
  }
}

static void parse_A_poly_centered(int16_t out[VIPER_N], const unsigned char *buf) {
  for (size_t l = 0, off = 0; l < VIPER_N; l += 2, off += 3) {
    uint32_t w = (uint32_t)buf[off] | ((uint32_t)buf[off + 1] << 8) | ((uint32_t)buf[off + 2] << 16);
    out[l + 0] = center_q((uint16_t)(w & VIPER_Q_MASK));
    out[l + 1] = center_q((uint16_t)((w >> 12) & VIPER_Q_MASK));
  }
}

void viper_gen_public_parse_A(vpoly A[VIPER_K][VIPER_K], const unsigned char *buf) {
  for (size_t i = 0; i < VIPER_K; i++) {
    for (size_t j = 0; j < VIPER_K; j++) parse_A_poly(A[i][j], buf + genpublic_a_offset(i, j));
  }
}

static void parse_dpk_poly(uint16_t out[VIPER_N], const unsigned char *buf) {
  for (size_t l = 0; l < VIPER_N; l += 4, buf++) {
    unsigned b = *buf;
    out[l + 0] = (uint16_t)(b & 3u);
    out[l + 1] = (uint16_t)((b >> 2) & 3u);
    out[l + 2] = (uint16_t)((b >> 4) & 3u);
    out[l + 3] = (uint16_t)((b >> 6) & 3u);
  }
}

void viper_gen_public_parse_dpk(uint16_t dpk[VIPER_K][VIPER_N], const unsigned char *buf) {
  for (size_t i = 0; i < VIPER_K; i++) {
    parse_dpk_poly(dpk[i], buf + genpublic_dpk_offset(i));
  }
}

void viper_gen_public(vpoly A[VIPER_K][VIPER_K], uint16_t dpk[VIPER_K][VIPER_N], const unsigned char rho[32]) {
  unsigned char buf[VIPER_PUBLIC_STREAM_BYTES];
  viper_gen_public_shake(buf, rho);
  viper_gen_public_parse_A(A, buf);
  viper_gen_public_parse_dpk(dpk, buf);
}

static void genpublic_expand_A_poly(vpoly out, const unsigned char rho[32], size_t i, size_t j) {
  unsigned char in[VIPER_GENPUBLIC_INPUT_BYTES];
  unsigned char buf[VIPER_PUBLIC_A_POLYBYTES];
  genpublic_input(in, rho, VIPER_DOMAIN_A, i, j);
  shake128(buf, sizeof(buf), in, sizeof(in));
  parse_A_poly(out, buf);
}

static void genpublic_expand_A_centered_cb(int16_t out[VIPER_N], const void *ctx, size_t i, size_t j) {
  unsigned char in[VIPER_GENPUBLIC_INPUT_BYTES];
  unsigned char buf[VIPER_PUBLIC_A_POLYBYTES];
  genpublic_input(in, (const unsigned char *)ctx, VIPER_DOMAIN_A, i, j);
  shake128(buf, sizeof(buf), in, sizeof(in));
  parse_A_poly_centered(out, buf);
}

static void genpublic_expand_dpk_bytes(unsigned char out[VIPER_PUBLIC_DPK_POLYBYTES], const unsigned char rho[32], size_t i) {
  unsigned char in[VIPER_GENPUBLIC_INPUT_BYTES];
  genpublic_input(in, rho, VIPER_DOMAIN_DPK, i, 0);
  shake128(out, VIPER_PUBLIC_DPK_POLYBYTES, in, sizeof(in));
}

static void genpublic_expand_dpk_poly(uint16_t out[VIPER_N], const unsigned char rho[32], size_t i) {
  unsigned char buf[VIPER_PUBLIC_DPK_POLYBYTES];
  genpublic_expand_dpk_bytes(buf, rho, i);
  parse_dpk_poly(out, buf);
}

static void viper_gen_public_dpk(uint16_t dpk[VIPER_K][VIPER_N], const unsigned char rho[32]) {
  for (size_t i = 0; i < VIPER_K; i++) {
    genpublic_expand_dpk_poly(dpk[i], rho, i);
  }
}

static void viper_quantize_pack_pk_t9_d2(unsigned char *out, const uint16_t poly[VIPER_N], const unsigned char dpk[VIPER_PUBLIC_DPK_POLYBYTES]) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 2) {
    uint32_t d = (uint32_t)dpk[k] | ((uint32_t)dpk[k + 1] << 8);
    uint16_t q0 = quantize_shift(poly[i + 0], (uint16_t)(d & 3u), 3u, 0x01ffu);
    uint16_t q1 = quantize_shift(poly[i + 1], (uint16_t)((d >> 2) & 3u), 3u, 0x01ffu);
    uint16_t q2 = quantize_shift(poly[i + 2], (uint16_t)((d >> 4) & 3u), 3u, 0x01ffu);
    uint16_t q3 = quantize_shift(poly[i + 3], (uint16_t)((d >> 6) & 3u), 3u, 0x01ffu);
    uint16_t q4 = quantize_shift(poly[i + 4], (uint16_t)((d >> 8) & 3u), 3u, 0x01ffu);
    uint16_t q5 = quantize_shift(poly[i + 5], (uint16_t)((d >> 10) & 3u), 3u, 0x01ffu);
    uint16_t q6 = quantize_shift(poly[i + 6], (uint16_t)((d >> 12) & 3u), 3u, 0x01ffu);
    uint16_t q7 = quantize_shift(poly[i + 7], (uint16_t)((d >> 14) & 3u), 3u, 0x01ffu);
    out[j + 0] = (unsigned char)q0;
    out[j + 1] = (unsigned char)((q0 >> 8) | (q1 << 1));
    out[j + 2] = (unsigned char)((q1 >> 7) | (q2 << 2));
    out[j + 3] = (unsigned char)((q2 >> 6) | (q3 << 3));
    out[j + 4] = (unsigned char)((q3 >> 5) | (q4 << 4));
    out[j + 5] = (unsigned char)((q4 >> 4) | (q5 << 5));
    out[j + 6] = (unsigned char)((q5 >> 3) | (q6 << 6));
    out[j + 7] = (unsigned char)((q6 >> 2) | (q7 << 7));
    out[j + 8] = (unsigned char)(q7 >> 1);
  }
}

static void viper_unpack_reconstruct_pk_centered_t9_d2(int16_t out[VIPER_N], const unsigned char *in, const unsigned char dpk[VIPER_PUBLIC_DPK_POLYBYTES]) {
  for (size_t i = 0, j = 0, k = 0; i < VIPER_N; i += 8, j += 9, k += 2) {
    uint32_t d = (uint32_t)dpk[k] | ((uint32_t)dpk[k + 1] << 8);
    uint16_t q0 = (uint16_t)(in[j + 0] | ((uint16_t)(in[j + 1] & 0x01u) << 8));
    uint16_t q1 = (uint16_t)((in[j + 1] >> 1) | ((uint16_t)(in[j + 2] & 0x03u) << 7));
    uint16_t q2 = (uint16_t)((in[j + 2] >> 2) | ((uint16_t)(in[j + 3] & 0x07u) << 6));
    uint16_t q3 = (uint16_t)((in[j + 3] >> 3) | ((uint16_t)(in[j + 4] & 0x0fu) << 5));
    uint16_t q4 = (uint16_t)((in[j + 4] >> 4) | ((uint16_t)(in[j + 5] & 0x1fu) << 4));
    uint16_t q5 = (uint16_t)((in[j + 5] >> 5) | ((uint16_t)(in[j + 6] & 0x3fu) << 3));
    uint16_t q6 = (uint16_t)((in[j + 6] >> 6) | ((uint16_t)(in[j + 7] & 0x7fu) << 2));
    uint16_t q7 = (uint16_t)((in[j + 7] >> 7) | ((uint16_t)in[j + 8] << 1));
    out[i + 0] = center_q(reconstruct_shift(q0, (uint16_t)(d & 3u), 3u));
    out[i + 1] = center_q(reconstruct_shift(q1, (uint16_t)((d >> 2) & 3u), 3u));
    out[i + 2] = center_q(reconstruct_shift(q2, (uint16_t)((d >> 4) & 3u), 3u));
    out[i + 3] = center_q(reconstruct_shift(q3, (uint16_t)((d >> 6) & 3u), 3u));
    out[i + 4] = center_q(reconstruct_shift(q4, (uint16_t)((d >> 8) & 3u), 3u));
    out[i + 5] = center_q(reconstruct_shift(q5, (uint16_t)((d >> 10) & 3u), 3u));
    out[i + 6] = center_q(reconstruct_shift(q6, (uint16_t)((d >> 12) & 3u), 3u));
    out[i + 7] = center_q(reconstruct_shift(q7, (uint16_t)((d >> 14) & 3u), 3u));
  }
}

typedef struct {
  const unsigned char *pk;
  const unsigned char *rho;
} public_poly_ctx;

static void expand_public_poly_centered(int16_t out[VIPER_N], const void *opaque, size_t i) {
  const public_poly_ctx *ctx = (const public_poly_ctx *)opaque;
  unsigned char dpk[VIPER_PUBLIC_DPK_POLYBYTES];
  genpublic_expand_dpk_bytes(dpk, ctx->rho, i);
  viper_unpack_reconstruct_pk_centered_t9_d2(out, ctx->pk + 32 + i * VIPER_PACKED_PK_POLYBYTES, dpk);
}

void viper_genpublic_matvec_fused_experiment(vpolyvec out, uint16_t dpk[VIPER_K][VIPER_N], const unsigned char rho[32], const vpolyvec s, int transpose) {
  vpoly a, t;
  memset(out, 0, sizeof(vpolyvec));
  for (size_t i = 0; i < VIPER_K; i++) {
    for (size_t j = 0; j < VIPER_K; j++) {
      genpublic_expand_A_poly(a, rho, transpose ? j : i, transpose ? i : j);
      viper_poly_mul(t, a, s[j]);
      for (size_t l = 0; l < VIPER_N; l++) out[i][l] = (uint16_t)((out[i][l] + t[l]) & VIPER_Q_MASK);
    }
  }
  viper_gen_public_dpk(dpk, rho);
}

void viper_sample_secret(vpolyvec s, const unsigned char seed[32]) {
  static const uint16_t eta1_lut[4] = {0, 1, VIPER_Q - 1, 0};
  unsigned char buf[VIPER_K * VIPER_N];
  shake256(buf, sizeof(buf), seed, 32);
  for (size_t i = 0; i < VIPER_K; i++) {
    for (size_t j = 0; j < VIPER_N; j++) {
      s[i][j] = eta1_lut[buf[i * VIPER_N + j] & 3u];
    }
  }
}

void viper_poly_mul_schoolbook_oracle(vpoly c, const vpoly a, const vpoly b) {
  int64_t tmp[VIPER_N];
  memset(tmp, 0, sizeof(tmp));
  for (size_t i = 0; i < VIPER_N; i++) {
    int64_t ai = (int64_t)a[i];
    for (size_t j = 0; j < VIPER_N; j++) {
      int64_t prod = ai * (int64_t)b[j];
      size_t idx = i + j;
      if (idx >= VIPER_N) tmp[idx - VIPER_N] -= prod;
      else tmp[idx] += prod;
    }
  }
  for (size_t i = 0; i < VIPER_N; i++) c[i] = modq_int(tmp[i]);
}

static void viper_encode_add(vpoly out, const unsigned char m[32], const vpoly addend) {
  for (size_t i = 0; i < VIPER_N; i++) {
    uint16_t encoded = (uint16_t)((m[i >> 3] >> (i & 7)) & 1u) << 11;
    out[i] = (uint16_t)(addend[i] + encoded) & VIPER_Q_MASK;
  }
}

static void viper_decode_sub(unsigned char m[32], const vpoly lhs, const vpoly rhs) {
  for (size_t i = 0; i < 32; i++) {
    unsigned byte = 0;
    for (size_t j = 0; j < 8; j++) {
      size_t k = 8 * i + j;
      uint16_t x = (uint16_t)(lhs[k] - rhs[k]) & VIPER_Q_MASK;
      byte |= (unsigned)(x >= 1024u && x < 3072u) << j;
    }
    m[i] = (unsigned char)byte;
  }
}

typedef struct {
  unsigned char *pk;
  unsigned char *skpke;
  const unsigned char *rho;
  const uint16_t *s;
} keypair_emit_ctx;

static void emit_keypair_poly(const uint16_t poly[VIPER_N], void *opaque, size_t i) {
  keypair_emit_ctx *ctx = (keypair_emit_ctx *)opaque;
  unsigned char dpk[VIPER_PUBLIC_DPK_POLYBYTES];
  genpublic_expand_dpk_bytes(dpk, ctx->rho, i);
  viper_quantize_pack_pk_t9_d2(ctx->pk + 32 + i * VIPER_PACKED_PK_POLYBYTES, poly, dpk);
  viper_pack_secret12(ctx->skpke + i * VIPER_POLYBYTES_12, ctx->s + i * VIPER_N);
}

typedef struct {
  unsigned char *ct;
  const unsigned char *dither;
} ciphertext_emit_ctx;

static void emit_ciphertext_u(const uint16_t poly[VIPER_N], void *opaque, size_t i) {
  ciphertext_emit_ctx *ctx = (ciphertext_emit_ctx *)opaque;
  viper_quantize_pack_dither_array(ctx->ct + i * VIPER_PACKED_U_POLYBYTES, poly, viper_dither_u_poly(ctx->dither, i), VIPER_T_U);
}

typedef struct {
  const unsigned char *ct;
  const unsigned char *dither;
  unsigned diff;
} ciphertext_compare_ctx;

static void compare_ciphertext_u(const uint16_t poly[VIPER_N], void *opaque, size_t i) {
  ciphertext_compare_ctx *ctx = (ciphertext_compare_ctx *)opaque;
  ctx->diff |= viper_quantize_pack_cmp_dither_array(ctx->ct + i * VIPER_PACKED_U_POLYBYTES, poly, viper_dither_u_poly(ctx->dither, i), VIPER_T_U);
}

typedef struct {
  const unsigned char *skpke;
} dec_secret_ctx;

static void expand_dec_secret_centered(int16_t out[VIPER_N], const void *opaque, size_t i) {
  const dec_secret_ctx *ctx = (const dec_secret_ctx *)opaque;
  viper_unpack_secret12_centered(out, ctx->skpke + i * VIPER_POLYBYTES_12);
}

typedef struct {
  const unsigned char *ct;
  const unsigned char *dither;
} dec_u_ctx;

static void expand_dec_u_centered(int16_t out[VIPER_N], const void *opaque, size_t i) {
  const dec_u_ctx *ctx = (const dec_u_ctx *)opaque;
  viper_unpack_reconstruct_t9_d3_centered(out, ctx->ct + i * VIPER_PACKED_U_POLYBYTES, viper_dither_u_poly(ctx->dither, i));
}

void viper_pke_keypair(unsigned char *pk, unsigned char *skpke, const unsigned char rho[32], const unsigned char sseed[32]) {
  vpolyvec s;
  keypair_emit_ctx emit_ctx = {pk, skpke, rho, &s[0][0]};
  memcpy(pk, rho, 32);
  viper_sample_secret(s, sseed);
  matvec_stream_m4ntt(s, genpublic_expand_A_centered_cb, rho, emit_keypair_poly, &emit_ctx, 0);
}

void viper_pke_enc(unsigned char *ct, const unsigned char *pk, const unsigned char m[32], const unsigned char omega[64]) {
  vpoly acc, t;
  unsigned char dither[VIPER_DITHER_BYTES];
  vpolyvec r;
  const unsigned char *rho = pk;
  const unsigned char *mu = omega + 32;
  public_poly_ctx public_ctx = {pk, rho};
  ciphertext_emit_ctx emit_ctx = {ct, dither};
  viper_gen_dither_bytes(dither, mu);
  viper_sample_secret(r, omega);
  matTvec_dot_stream_m4ntt(t, r, genpublic_expand_A_centered_cb, rho, expand_public_poly_centered, &public_ctx, emit_ciphertext_u, &emit_ctx);
  viper_encode_add(acc, m, t);
  viper_quantize_pack_dither_array(ct + VIPER_PACKED_U_BYTES, acc, viper_dither_v_poly(dither), VIPER_T_V);
  memcpy(ct + VIPER_PACKED_U_BYTES + VIPER_PACKED_V_BYTES, mu, 32);
}

void viper_pke_dec(unsigned char m[32], const unsigned char *skpke, const unsigned char *ct, const unsigned char dither[VIPER_DITHER_BYTES]) {
  vpoly w, t;
  dec_secret_ctx secret_ctx = {skpke};
  dec_u_ctx u_ctx = {ct, dither};
  dot_dec_stream_m4ntt(t, expand_dec_secret_centered, &secret_ctx, expand_dec_u_centered, &u_ctx);
  viper_unpack_reconstruct_dither_array(w, ct + VIPER_PACKED_U_BYTES, viper_dither_v_poly(dither), VIPER_T_V);
  viper_decode_sub(m, w, t);
}

int viper_reencrypt_check(const unsigned char *ct, const unsigned char *pk, const unsigned char m[32], const unsigned char sigma[32], const unsigned char dither[VIPER_DITHER_BYTES]) {
  vpoly acc, t;
  vpolyvec r;
  unsigned char omega[64];
  const unsigned char *rho = pk;
  const unsigned char *mu = ct + VIPER_PACKED_U_BYTES + VIPER_PACKED_V_BYTES;
  public_poly_ctx public_ctx = {pk, rho};
  ciphertext_compare_ctx compare_ctx = {ct, dither, 0};

  memcpy(omega, sigma, 32);
  memcpy(omega + 32, mu, 32);
  viper_sample_secret(r, omega);
  matTvec_dot_stream_m4ntt(t, r, genpublic_expand_A_centered_cb, rho, expand_public_poly_centered, &public_ctx, compare_ciphertext_u, &compare_ctx);
  viper_encode_add(acc, m, t);
  compare_ctx.diff |= viper_quantize_pack_cmp_dither_array(ct + VIPER_PACKED_U_BYTES, acc, viper_dither_v_poly(dither), VIPER_T_V);
  return compare_ctx.diff == 0;
}
