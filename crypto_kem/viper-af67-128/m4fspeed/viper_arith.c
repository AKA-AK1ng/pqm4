/* MAMBA-Viper implementation and implementation support layer where applicable.
 * First Cortex-M4 arithmetic backend: reuse the LightSaber CRT/NTT kernels for
 * one negacyclic polynomial product, then keep Viper's higher-level loops.
 */
#include "viper_arith.h"
#include "NTT.h"
#include <stdint.h>
#include <string.h>

static void center_poly(int16_t out[VIPER_N], const vpoly in)
{
  for (size_t i = 0; i < VIPER_N; i++) {
    uint16_t x = (uint16_t)(in[i] & VIPER_Q_MASK);
    out[i] = (int16_t)(x < (VIPER_Q >> 1) ? x : (int32_t)x - VIPER_Q);
  }
}

static uint16_t max_abs_poly(const int16_t a[VIPER_N])
{
  uint16_t m = 0;
  for (size_t i = 0; i < VIPER_N; i++) {
    int16_t x = a[i];
    uint16_t ax = (uint16_t)(x < 0 ? -x : x);
    if (ax > m) {
      m = ax;
    }
  }
  return m;
}

static void poly_mul_schoolbook_centered(vpoly c, const int16_t a[VIPER_N], const int16_t b[VIPER_N])
{
  int32_t tmp[VIPER_N];
  memset(tmp, 0, sizeof(tmp));

  for (size_t i = 0; i < VIPER_N; i++) {
    for (size_t j = 0; j < VIPER_N; j++) {
      int32_t prod = (int32_t)a[i] * b[j];
      size_t idx = i + j;
      if (idx >= VIPER_N) {
        tmp[idx - VIPER_N] -= prod;
      } else {
        tmp[idx] += prod;
      }
    }
  }

  for (size_t i = 0; i < VIPER_N; i++) {
    c[i] = (uint16_t)tmp[i] & VIPER_Q_MASK;
  }
}

static void poly_mul_m4ntt_core(vpoly c, const vpoly a, const vpoly b)
{
  int16_t a_center[VIPER_N];
  int16_t b_center[VIPER_N];
  uint32_t a_ntt[VIPER_N];
  uint32_t b_ntt[VIPER_N];
  uint32_t prod_ntt[VIPER_N];
  uint16_t prod[VIPER_N];

  center_poly(a_center, a);
  center_poly(b_center, b);

  if ((uint32_t)max_abs_poly(a_center) * max_abs_poly(b_center) * VIPER_N >= (uint32_t)(Q1Q2 / 2)) {
    poly_mul_schoolbook_centered(c, a_center, b_center);
    return;
  }

  NTT_forward_32(a_ntt, (uint16_t *)a_center);
  NTT_forward_32(b_ntt, (uint16_t *)b_center);
  NTT_mul_32(prod_ntt, a_ntt, b_ntt);
  NTT_inv_32(prod, prod_ntt);

  for (size_t i = 0; i < VIPER_N; i++) {
    c[i] = (uint16_t)(prod[i] & VIPER_Q_MASK);
  }
}

void poly_mul_m4ntt(vpoly c, const vpoly a, const vpoly b)
{
  poly_mul_m4ntt_core(c, a, b);
}

void matvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s)
{
  vpoly t;
  for (size_t i = 0; i < VIPER_K; i++) {
    memset(out[i], 0, sizeof(vpoly));
    for (size_t j = 0; j < VIPER_K; j++) {
      poly_mul_m4ntt(t, A[i][j], s[j]);
      for (size_t k = 0; k < VIPER_N; k++) {
        out[i][k] = (uint16_t)((out[i][k] + t[k]) & VIPER_Q_MASK);
      }
    }
  }
}

void matTvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s)
{
  vpoly t;
  for (size_t i = 0; i < VIPER_K; i++) {
    memset(out[i], 0, sizeof(vpoly));
    for (size_t j = 0; j < VIPER_K; j++) {
      poly_mul_m4ntt(t, A[j][i], s[j]);
      for (size_t k = 0; k < VIPER_N; k++) {
        out[i][k] = (uint16_t)((out[i][k] + t[k]) & VIPER_Q_MASK);
      }
    }
  }
}

void dot_m4ntt(vpoly out, const vpolyvec a, const vpolyvec b)
{
  vpoly t;
  memset(out, 0, sizeof(vpoly));
  for (size_t i = 0; i < VIPER_K; i++) {
    poly_mul_m4ntt(t, a[i], b[i]);
    for (size_t k = 0; k < VIPER_N; k++) {
      out[k] = (uint16_t)((out[k] + t[k]) & VIPER_Q_MASK);
    }
  }
}
