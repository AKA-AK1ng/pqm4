#ifndef VIPER_ASM_H
#define VIPER_ASM_H

#include <stdint.h>
#include "viper_params.h"

/* Each switch can be overridden with -D...=0 to build the C reference path. */
#ifndef VIPER_USE_ASM
#define VIPER_USE_ASM 1
#endif
#ifndef VIPER_USE_ASM_UNPACK12_CENTERED
#define VIPER_USE_ASM_UNPACK12_CENTERED VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_U_RECONSTRUCT
#define VIPER_USE_ASM_U_RECONSTRUCT VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_U_PACK
#define VIPER_USE_ASM_U_PACK VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_U_COMPARE
#define VIPER_USE_ASM_U_COMPARE VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_PK_PACK
#define VIPER_USE_ASM_PK_PACK VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_PK_RECONSTRUCT
#define VIPER_USE_ASM_PK_RECONSTRUCT VIPER_USE_ASM
#endif
#ifndef VIPER_USE_ASM_ETA1_DECODE
/* Viper-E8-128 uses eta=2.  Keep the legacy symbol for differential tests,
 * but never select it from the production sampler. */
#define VIPER_USE_ASM_ETA1_DECODE 0
#endif
#ifndef VIPER_USE_ASM_SECRET_PACK12
#define VIPER_USE_ASM_SECRET_PACK12 VIPER_USE_ASM
#endif

void viper_asm_unpack12_centered(int16_t out[VIPER_N],
                                 const unsigned char in[VIPER_SECRET_POLYBYTES]);
void viper_ref_unpack12_centered(int16_t out[VIPER_N],
                                 const unsigned char in[VIPER_SECRET_POLYBYTES]);
void viper_unpack12_centered(int16_t out[VIPER_N],
                             const unsigned char in[VIPER_SECRET_POLYBYTES]);
void viper_asm_unpack_reconstruct_t9_d3_centered(int16_t out[VIPER_N],
                                                 const unsigned char in[VIPER_PACKED_U_POLYBYTES],
                                                 const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
void viper_ref_unpack_reconstruct_t9_d3_centered(int16_t out[VIPER_N],
                                                 const unsigned char in[VIPER_PACKED_U_POLYBYTES],
                                                 const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
void viper_unpack_reconstruct_t9_d3_centered(int16_t out[VIPER_N],
                                             const unsigned char in[VIPER_PACKED_U_POLYBYTES],
                                             const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
void viper_asm_quantize_pack_t9_d3(unsigned char out[VIPER_PACKED_U_POLYBYTES],
                                   const uint16_t poly[VIPER_N],
                                   const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
unsigned viper_asm_quantize_pack_cmp_t9_d3(const unsigned char bytes[VIPER_PACKED_U_POLYBYTES],
                                           const uint16_t poly[VIPER_N],
                                           const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
void viper_ref_quantize_pack_t9_d3(unsigned char out[VIPER_PACKED_U_POLYBYTES],
                                   const uint16_t poly[VIPER_N],
                                   const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
unsigned viper_ref_quantize_pack_cmp_t9_d3(const unsigned char bytes[VIPER_PACKED_U_POLYBYTES],
                                           const uint16_t poly[VIPER_N],
                                           const unsigned char dither[VIPER_DITHER_U_POLYBYTES]);
void viper_asm_quantize_pack_pk_t9_d2(unsigned char out[VIPER_PACKED_PK_POLYBYTES],
                                      const uint16_t poly[VIPER_N], const unsigned char dpk[VIPER_N / 4]);
void viper_ref_quantize_pack_pk_t9_d2(unsigned char out[VIPER_PACKED_PK_POLYBYTES],
                                      const uint16_t poly[VIPER_N], const unsigned char dpk[VIPER_N / 4]);
void viper_asm_unpack_reconstruct_pk_centered_t9_d2(int16_t out[VIPER_N],
                                                    const unsigned char in[VIPER_PACKED_PK_POLYBYTES],
                                                    const unsigned char dpk[VIPER_N / 4]);
void viper_ref_unpack_reconstruct_pk_centered_t9_d2(int16_t out[VIPER_N],
                                                    const unsigned char in[VIPER_PACKED_PK_POLYBYTES],
                                                    const unsigned char dpk[VIPER_N / 4]);
void viper_asm_eta1_decode_centered(int16_t out[VIPER_N], const unsigned char in[VIPER_N]);
void viper_asm_eta1_decode_modq(uint16_t out[VIPER_N], const unsigned char in[VIPER_N]);
void viper_ref_eta1_decode_centered(int16_t out[VIPER_N], const unsigned char in[VIPER_N]);
void viper_ref_eta1_decode_modq(uint16_t out[VIPER_N], const unsigned char in[VIPER_N]);
void viper_asm_pack_secret12(unsigned char out[VIPER_SECRET_POLYBYTES], const uint16_t in[VIPER_N]);
void viper_ref_pack_secret12(unsigned char out[VIPER_SECRET_POLYBYTES], const uint16_t in[VIPER_N]);

#endif
