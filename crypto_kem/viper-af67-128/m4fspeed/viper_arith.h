#ifndef VIPER_ARITH_H
#define VIPER_ARITH_H

/* MAMBA-Viper implementation and implementation support layer where applicable. */

#include "viper.h"
#include <stdio.h>

void poly_mul_m4ntt(vpoly c, const vpoly a, const vpoly b);
void matvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s);
void matTvec_m4ntt(vpolyvec out, vpoly A[VIPER_K][VIPER_K], const vpolyvec s);
void matTvec_dot_m4ntt(vpolyvec out, vpoly dot, vpoly A[VIPER_K][VIPER_K], const vpolyvec a, const vpolyvec s);
typedef void (*viper_expand_matrix_centered_fn)(int16_t out[VIPER_N], const void *ctx, size_t i, size_t j);
typedef void (*viper_expand_vector_centered_fn)(int16_t out[VIPER_N], const void *ctx, size_t i);
typedef void (*viper_emit_poly_fn)(const uint16_t poly[VIPER_N], void *ctx, size_t i);
void matvec_stream_m4ntt(const vpolyvec s, viper_expand_matrix_centered_fn expand_A, const void *expand_ctx, viper_emit_poly_fn emit, void *emit_ctx, int transpose);
void matTvec_dot_stream_m4ntt(vpoly dot, const vpolyvec s, viper_expand_matrix_centered_fn expand_A, const void *expand_ctx, viper_expand_vector_centered_fn expand_dot, const void *dot_ctx, viper_emit_poly_fn emit, void *emit_ctx);
void dot_m4ntt(vpoly out, const vpolyvec a, const vpolyvec b);
void dot_m4shortdense(vpoly out, const vpolyvec s, const vpolyvec a);

#define viper_poly_mul poly_mul_m4ntt
#define viper_matvec matvec_m4ntt
#define viper_matTvec matTvec_m4ntt
#define viper_dot dot_m4ntt

#define VIPER_POLY_MUL_ROUTE "poly_mul_m4ntt"
#define VIPER_MATVEC_ROUTE "matvec_m4ntt"
#define VIPER_MATTVEC_ROUTE "matTvec_m4ntt"
#define VIPER_DOT_ROUTE "dot_m4ntt"

static inline void viper_backend_report(FILE *out)
{
  fprintf(out, "REPORT,VIPER_LEVEL,%d\n", VIPER_LEVEL);
  fprintf(out, "REPORT,VIPER_ARITH_AVX,0\n");
  fprintf(out, "REPORT,VIPER_ARITH_M4_NTT,1\n");
  fprintf(out, "REPORT,VIPER_EXPERIMENTAL_TCHES2021_NTT,0\n");
  fprintf(out, "REPORT,VIPER_EXPERIMENTAL_TCHES2021_HYBRID,0\n");
  fprintf(out, "REPORT,VIPER_EXPERIMENTAL_TCHES2021_HYBRID_L3L5,0\n");
  fprintf(out, "REPORT,resolved_poly_mul_route,%s\n", VIPER_POLY_MUL_ROUTE);
  fprintf(out, "REPORT,resolved_matvec_route,%s\n", VIPER_MATVEC_ROUTE);
  fprintf(out, "REPORT,resolved_matTvec_route,%s\n", VIPER_MATTVEC_ROUTE);
  fprintf(out, "REPORT,resolved_dot_route,%s\n", VIPER_DOT_ROUTE);
}

#endif
