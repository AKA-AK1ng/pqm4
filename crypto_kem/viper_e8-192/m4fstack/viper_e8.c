#include "viper_e8.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint16_t modq_i32(int32_t x) { return (uint16_t)(x & VIPER_QMASK); }
static int32_t centered_u16(uint16_t x) {
  int32_t v = (int32_t)(x & VIPER_QMASK);
  if (v >= VIPER_Q / 2) v -= VIPER_Q;
  return v;
}

static unsigned e8_modulus(void) { return 2u * (unsigned)(VIPER_Q / VIPER_E8_ALPHA); }

#if VIPER_E8_RATE != 1
static unsigned e8_count_cache[9][2][4];
static int e8_count_ready = 0;

static void e8_init_counts(void) {
  if (e8_count_ready) return;
  unsigned mod = e8_modulus();
  for (unsigned parity = 0; parity < 2; parity++) e8_count_cache[0][parity][0] = 1;
  for (unsigned rem = 1; rem <= 8; rem++) {
    for (unsigned parity = 0; parity < 2; parity++) {
      for (unsigned sum = 0; sum < 4; sum++) {
        unsigned count = 0;
        for (unsigned v = parity; v < mod; v += 2) count += e8_count_cache[rem - 1][parity][(sum + v) & 3u];
        e8_count_cache[rem][parity][sum] = count;
      }
    }
  }
  e8_count_ready = 1;
}

static unsigned e8_count_suffix(unsigned remaining, unsigned parity, unsigned sum_mod4) {
  e8_init_counts();
  return e8_count_cache[remaining][parity & 1u][sum_mod4 & 3u];
}
#endif

#if VIPER_E8_RATE != 1
static int e8_valid_residue_vector(const unsigned d[8]) {
  unsigned parity = d[0] & 1u, sum = 0;
  for (unsigned i = 0; i < 8; i++) {
    if ((d[i] & 1u) != parity) return 0;
    sum += d[i];
  }
  return (sum & 3u) == 0;
}
#endif

static void e8_unrank(uint8_t out[8], unsigned label) {
#if VIPER_E8_RATE == 1
  /* For modulus 4, lexicographic E8 residues split into four groups selected
   * by d[0].  Six payload bits select d[1..6], and d[7] is the unique value
   * that makes the number of high residue bits even. */
  unsigned d0 = (label >> 6) & 3u;
  unsigned payload = label & 0x3fu;
  unsigned parity = d0 & 1u;
  unsigned high_parity = d0 >> 1;
  out[0] = (uint8_t)d0;
  for (unsigned pos = 1; pos < 7; pos++) {
    unsigned high = (payload >> (6u - pos)) & 1u;
    out[pos] = (uint8_t)(parity + 2u * high);
    high_parity ^= high;
  }
  out[7] = (uint8_t)(parity + 2u * high_parity);
#else
  unsigned mod = e8_modulus();
  unsigned labels = 1u << VIPER_E8_BITS_PER_BLOCK;
  unsigned rank = label & (labels - 1u);
  unsigned sum = 0;
  int parity = -1;
  for (unsigned pos = 0; pos < 8; pos++) {
    for (unsigned v = 0; v < mod; v++) {
      if (parity >= 0 && (int)(v & 1u) != parity) continue;
      unsigned next_parity = (parity >= 0) ? (unsigned)parity : (v & 1u);
      unsigned cnt = e8_count_suffix(7 - pos, next_parity, (sum + v) & 3u);
      if (rank >= cnt) {
        rank -= cnt;
      } else {
        out[pos] = (uint8_t)v;
        sum = (sum + v) & 3u;
        parity = (int)next_parity;
        break;
      }
    }
  }
#endif
}

static unsigned e8_rank(const unsigned d[8]) {
#if VIPER_E8_RATE == 1
  unsigned rank = (d[0] & 3u) << 6;
  for (unsigned pos = 1; pos < 7; pos++) {
    rank |= ((d[pos] >> 1) & 1u) << (6u - pos);
  }
  return rank;
#else
  unsigned mod = e8_modulus();
  if (!e8_valid_residue_vector(d)) return 0;
  unsigned rank = 0, sum = 0;
  int parity = -1;
  for (unsigned pos = 0; pos < 8; pos++) {
    for (unsigned v = 0; v < d[pos]; v++) {
      if (parity >= 0 && (int)(v & 1u) != parity) continue;
      unsigned next_parity = (parity >= 0) ? (unsigned)parity : (v & 1u);
      rank += e8_count_suffix(7 - pos, next_parity, (sum + v) & 3u);
    }
    if (d[pos] >= mod) return 0;
    if (parity < 0) parity = (int)(d[pos] & 1u);
    if ((int)(d[pos] & 1u) != parity) return 0;
    sum = (sum + d[pos]) & 3u;
  }
  return rank;
#endif
}

uint16_t viper_e8_label_to_coeff(unsigned label, unsigned coord) {
  uint8_t d[8];
  e8_unrank(d, label);
  coord &= 7u;
  return modq_i32((int32_t)d[coord] * (int32_t)(VIPER_E8_ALPHA / 2));
}

unsigned viper_e8_coeffs_to_label(const uint16_t coeffs[8]) {
  unsigned d[8];
  for (unsigned i = 0; i < 8; i++) {
    int32_t c = centered_u16(coeffs[i]);
    int32_t half = VIPER_E8_ALPHA / 2;
    int32_t q = c / half;
    int32_t r = c % half;
    if (r < 0) { r += half; q--; }
    if (2 * r >= half) q++;
    d[i] = (unsigned)q & (e8_modulus() - 1u);
  }
  return e8_rank(d);
}

static unsigned e8_label_from_doubled(const int d[8]) {
  unsigned u[8];
  unsigned mod = e8_modulus();
  for (unsigned i = 0; i < 8; i++) u[i] = (unsigned)d[i] & (mod - 1u);
  return e8_rank(u);
}

static int32_t round_alpha_i32(int32_t a) {
  _Static_assert(VIPER_E8_ALPHA == 2048,
                 "specialized E8 rounding requires alpha=2048");
  /* Match the original nearest-integer rule, including ties toward the
   * smaller integer, while exposing a constant power-of-two divisor. */
  return a >= 0 ? (a + 1023) / 2048 : (a - 1024) / 2048;
}

/* For all five active parameter sets alpha is 2048.  The nearest-grid
 * candidate and either adjacent point therefore stay within a few alpha of a
 * centered coefficient, so every square and their eight-coordinate sum fit
 * in uint32_t.  Keeping this hot decoder path 32-bit avoids software 64-bit
 * multiply/compare sequences on Cortex-M4. */
static uint32_t d8_candidate(int out_d[8], const int32_t c[8], int half_shift) {
  int z[8];
  int sum = 0;
  uint32_t dist = 0;
  for (unsigned i = 0; i < 8; i++) {
    int32_t target = c[i] - (half_shift ? (int32_t)(VIPER_E8_ALPHA / 2) : 0);
    z[i] = (int)round_alpha_i32(target);
    sum += z[i];
  }
  if (sum & 1) {
    uint32_t best_penalty = UINT32_MAX;
    unsigned best_i = 0;
    int best_delta = 1;
    for (unsigned i = 0; i < 8; i++) {
      int32_t base = ((2 * z[i]) + half_shift) * (int32_t)(VIPER_E8_ALPHA / 2);
      int32_t plus = base + VIPER_E8_ALPHA;
      int32_t minus = base - VIPER_E8_ALPHA;
      int32_t cur_diff = c[i] - base;
      int32_t plus_diff = c[i] - plus;
      int32_t minus_diff = c[i] - minus;
      uint32_t cur = (uint32_t)(cur_diff * cur_diff);
      uint32_t ppen = (uint32_t)(plus_diff * plus_diff) - cur;
      uint32_t mpen = (uint32_t)(minus_diff * minus_diff) - cur;
      if (ppen < best_penalty || (ppen == best_penalty && i < best_i)) { best_penalty = ppen; best_i = i; best_delta = 1; }
      if (mpen < best_penalty || (mpen == best_penalty && i < best_i)) { best_penalty = mpen; best_i = i; best_delta = -1; }
    }
    z[best_i] += best_delta;
  }
  for (unsigned i = 0; i < 8; i++) {
    out_d[i] = 2 * z[i] + half_shift;
    int32_t val = out_d[i] * (int32_t)(VIPER_E8_ALPHA / 2);
    int32_t diff = c[i] - val;
    dist += (uint32_t)(diff * diff);
  }
  return dist;
}

static void nearest_e8_doubled(int out_d[8], const uint16_t coeffs[8]) {
  int32_t c[8];
  int d0[8], d1[8];
  for (unsigned i = 0; i < 8; i++) c[i] = centered_u16(coeffs[i]);
  uint32_t dist0 = d8_candidate(d0, c, 0);
  uint32_t dist1 = d8_candidate(d1, c, 1);
  const int *src = (dist0 <= dist1) ? d0 : d1;
  for (unsigned i = 0; i < 8; i++) out_d[i] = src[i];
}

void viper_e8_encode(vpoly out, const unsigned char m[VIPER_MSGBYTES]) {
  memset(out, 0, sizeof(vpoly));
  const unsigned mask = (1u << VIPER_E8_BITS_PER_BLOCK) - 1u;
  for (unsigned block = 0; block < VIPER_E8_ACTIVE_BLOCKS; block++) {
    uint8_t d[8];
    unsigned label = 0;
    for (unsigned b = 0; b < VIPER_E8_BITS_PER_BLOCK; b++) {
      unsigned bit = block * VIPER_E8_BITS_PER_BLOCK + b;
      label |= (unsigned)((m[bit >> 3] >> (bit & 7)) & 1u) << b;
    }
    label &= mask;
    /* Unranking is the expensive part of encoding.  The old implementation
     * repeated it once per coordinate even though all eight coordinates use
     * the same label. */
    e8_unrank(d, label);
    for (unsigned j = 0; j < 8; j++) {
      out[8 * block + j] =
          modq_i32((int32_t)d[j] * (int32_t)(VIPER_E8_ALPHA / 2));
    }
  }
}

void viper_e8_decode(unsigned char m[VIPER_MSGBYTES], const vpoly in) {
  memset(m, 0, VIPER_MSGBYTES);
  for (unsigned block = 0; block < VIPER_E8_ACTIVE_BLOCKS; block++) {
    uint16_t coeffs[8];
    int d[8];
    for (unsigned j = 0; j < 8; j++) coeffs[j] = in[8 * block + j];
    nearest_e8_doubled(d, coeffs);
    unsigned label = e8_label_from_doubled(d);
    for (unsigned b = 0; b < VIPER_E8_BITS_PER_BLOCK; b++) {
      unsigned bit = block * VIPER_E8_BITS_PER_BLOCK + b;
      if (bit < VIPER_MSGBITS) m[bit >> 3] |= (unsigned char)(((label >> b) & 1u) << (bit & 7));
    }
  }
}

const char *viper_e8_labeling_name(void) { return "lexicographic E8/(q/alpha)Z^8 doubled-coordinate residues"; }
