/* MAMBA-Viper implementation and implementation support layer where applicable.
 * First Cortex-M4 arithmetic backend: reuse the LightSaber CRT/NTT kernels for
 * one negacyclic polynomial product, then keep Viper's higher-level loops.
 */
#include "viper_arith.h"
#include "NTT.h"
#include <stdint.h>
#include <string.h>

extern void __asm_poly_add_32(uint32_t *des, uint32_t *src1, uint32_t *src2);

static int ntt_bound_add_ok(uint64_t *bound, uint16_t max_a, uint16_t max_b)
{
  *bound += (uint64_t)max_a * max_b * VIPER_N;
  return *bound < (uint64_t)(Q1Q2 / 2);
}

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

static int32_t center_coeff(uint16_t x)
{
  return (int32_t)(((uint32_t)(x & VIPER_Q_MASK) + (VIPER_Q >> 1)) & VIPER_Q_MASK) - (VIPER_Q >> 1);
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
  uint64_t bound = 0;
  uint32_t a_ntt[VIPER_N];
  uint32_t b_ntt[VIPER_N];
  uint16_t prod[VIPER_N];

  center_poly(a_center, a);
  center_poly(b_center, b);

  if (!ntt_bound_add_ok(&bound, max_abs_poly(a_center), max_abs_poly(b_center))) {
    poly_mul_schoolbook_centered(c, a_center, b_center);
    return;
  }

  NTT_forward_32(a_ntt, (uint16_t *)a_center);
  NTT_forward_32(b_ntt, (uint16_t *)b_center);
  NTT_mul_32(a_ntt, a_ntt, b_ntt);
  NTT_inv_32(prod, a_ntt);

  for (size_t i = 0; i < VIPER_N; i++) {
    c[i] = (uint16_t)(prod[i] & VIPER_Q_MASK);
  }
}

void poly_mul_m4ntt(vpoly c, const vpoly a, const vpoly b)
{
  poly_mul_m4ntt_core(c, a, b);
}

static void add_product_ntt(uint32_t acc[VIPER_N], uint32_t a_ntt[VIPER_N], const uint32_t b_ntt[VIPER_N], int first)
{
  if (first) {
    NTT_mul_32(acc, a_ntt, (uint32_t *)b_ntt);
  } else {
    NTT_mul_32(a_ntt, a_ntt, (uint32_t *)b_ntt);
    __asm_poly_add_32(acc, acc, a_ntt);
  }
}

static void finish_acc_ntt(vpoly out, uint32_t acc_ntt[VIPER_N])
{
  NTT_inv_32(out, acc_ntt);
  for (size_t i = 0; i < VIPER_N; i++) {
    out[i] &= VIPER_Q_MASK;
  }
}

static void finish_acc_ntt_emit(uint32_t acc_ntt[VIPER_N], viper_emit_poly_fn emit, void *ctx, size_t i)
{
  uint16_t acc[VIPER_N];

  NTT_inv_32(acc, acc_ntt);
  emit(acc, ctx, i);
}

static void add_product_poly_ntt(uint32_t acc[VIPER_N], const vpoly a, const vpoly b, int first)
{
  int16_t a_center[VIPER_N];
  int16_t b_center[VIPER_N];
  uint32_t a_ntt[VIPER_N];
  uint32_t b_ntt[VIPER_N];

  center_poly(a_center, a);
  center_poly(b_center, b);
  NTT_forward_32(a_ntt, (uint16_t *)a_center);
  NTT_forward_32(b_ntt, (uint16_t *)b_center);
  add_product_ntt(acc, a_ntt, b_ntt, first);
}

static void add_product_centered_poly_ntt(uint32_t acc[VIPER_N], int16_t a_center[VIPER_N], const vpoly b, int first)
{
  int16_t b_center[VIPER_N];
  uint32_t a_ntt[VIPER_N];
  uint32_t b_ntt[VIPER_N];

  center_poly(b_center, b);
  NTT_forward_32(a_ntt, (uint16_t *)a_center);
  NTT_forward_32(b_ntt, (uint16_t *)b_center);
  add_product_ntt(acc, a_ntt, b_ntt, first);
}

static void matvec_acc_ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s, int transpose)
{
  for (size_t i = 0; i < VIPER_K; i++) {
    uint32_t acc_ntt[VIPER_N];

    for (size_t j = 0; j < VIPER_K; j++) {
      add_product_poly_ntt(acc_ntt, transpose ? A[j][i] : A[i][j], s[j], j == 0);
    }

    finish_acc_ntt(out[i], acc_ntt);
  }
}

void matvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s)
{
  matvec_acc_ntt(out, A, s, 0);
}

void matTvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s)
{
  matvec_acc_ntt(out, A, s, 1);
}

static void dot_fallback(vpoly out, const vpolyvec a, const vpolyvec b)
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

void matTvec_dot_m4ntt(vpolyvec out, vpoly dot, vpoly A[VIPER_K][VIPER_K], const vpolyvec a, const vpolyvec s)
{
  for (size_t i = 0; i < VIPER_K; i++) {
    uint32_t acc_ntt[VIPER_N];

    for (size_t j = 0; j < VIPER_K; j++) {
      add_product_poly_ntt(acc_ntt, A[j][i], s[j], j == 0);
    }

    finish_acc_ntt(out[i], acc_ntt);
  }

  {
    uint32_t acc_ntt[VIPER_N];

    for (size_t i = 0; i < VIPER_K; i++) {
      add_product_poly_ntt(acc_ntt, a[i], s[i], i == 0);
    }

    finish_acc_ntt(dot, acc_ntt);
  }
}

void matvec_stream_m4ntt(const vpolyvec s, viper_expand_matrix_centered_fn expand_A, const void *expand_ctx, viper_emit_poly_fn emit, void *emit_ctx, int transpose)
{
  for (size_t i = 0; i < VIPER_K; i++) {
    uint32_t acc_ntt[VIPER_N];

    for (size_t j = 0; j < VIPER_K; j++) {
      int16_t a_center[VIPER_N];

      expand_A(a_center, expand_ctx, transpose ? j : i, transpose ? i : j);
      add_product_centered_poly_ntt(acc_ntt, a_center, s[j], j == 0);
    }

    finish_acc_ntt_emit(acc_ntt, emit, emit_ctx, i);
  }
}

void matTvec_dot_stream_m4ntt(vpoly dot, const vpolyvec s, viper_expand_matrix_centered_fn expand_A, const void *expand_ctx, viper_expand_vector_centered_fn expand_dot, const void *dot_ctx, viper_emit_poly_fn emit, void *emit_ctx)
{
  for (size_t i = 0; i < VIPER_K; i++) {
    uint32_t acc_ntt[VIPER_N];

    for (size_t j = 0; j < VIPER_K; j++) {
      int16_t a_center[VIPER_N];

      expand_A(a_center, expand_ctx, j, i);
      add_product_centered_poly_ntt(acc_ntt, a_center, s[j], j == 0);
    }

    finish_acc_ntt_emit(acc_ntt, emit, emit_ctx, i);
  }

  {
    uint32_t acc_ntt[VIPER_N];

    for (size_t i = 0; i < VIPER_K; i++) {
      int16_t dot_center[VIPER_N];

      expand_dot(dot_center, dot_ctx, i);
      add_product_centered_poly_ntt(acc_ntt, dot_center, s[i], i == 0);
    }

    finish_acc_ntt(dot, acc_ntt);
  }
}

void dot_dec_stream_m4ntt(vpoly out, viper_expand_vector_centered_fn expand_a, const void *a_ctx, viper_expand_vector_centered_fn expand_b, const void *b_ctx)
{
  uint32_t acc_ntt[VIPER_N];

  for (size_t i = 0; i < VIPER_K; i++) {
    int16_t centered[VIPER_N];
    uint32_t a_ntt[VIPER_N];
    uint32_t b_ntt[VIPER_N];

    expand_a(centered, a_ctx, i);
    NTT_forward_32(a_ntt, (uint16_t *)centered);
    expand_b(centered, b_ctx, i);
    NTT_forward_32(b_ntt, (uint16_t *)centered);
    add_product_ntt(acc_ntt, a_ntt, b_ntt, i == 0);
  }

  finish_acc_ntt(out, acc_ntt);
}

void dot_m4shortdense(vpoly out, const vpolyvec s, const vpolyvec a)
{
  int32_t acc[VIPER_N];
  memset(acc, 0, sizeof(acc));

  for (size_t v = 0; v < VIPER_K; v++) {
    for (size_t j = 0; j < VIPER_N; j++) {
      int32_t sj = center_coeff(s[v][j]);
      size_t limit = VIPER_N - j;

      for (size_t k = 0; k < limit; k++) {
        acc[j + k] += sj * (int32_t)(a[v][k] & VIPER_Q_MASK);
      }
      for (size_t k = limit; k < VIPER_N; k++) {
        acc[j + k - VIPER_N] -= sj * (int32_t)(a[v][k] & VIPER_Q_MASK);
      }
    }
  }

  for (size_t i = 0; i < VIPER_N; i++) {
    out[i] = (uint16_t)acc[i] & VIPER_Q_MASK;
  }
}

void dot_m4ntt(vpoly out, const vpolyvec a, const vpolyvec b)
{
  uint32_t acc_ntt[VIPER_N];
  uint64_t bound = 0;

  for (size_t i = 0; i < VIPER_K; i++) {
    int16_t a_center[VIPER_N];
    int16_t b_center[VIPER_N];
    uint32_t a_ntt[VIPER_N];
    uint32_t b_ntt[VIPER_N];

    center_poly(a_center, a[i]);
    center_poly(b_center, b[i]);
    if (!ntt_bound_add_ok(&bound, max_abs_poly(a_center), max_abs_poly(b_center))) {
      dot_fallback(out, a, b);
      return;
    }
    NTT_forward_32(a_ntt, (uint16_t *)a_center);
    NTT_forward_32(b_ntt, (uint16_t *)b_center);
    add_product_ntt(acc_ntt, a_ntt, b_ntt, i == 0);
  }

  finish_acc_ntt(out, acc_ntt);
}
