#ifndef VIPER_PROFILE_H
#define VIPER_PROFILE_H

#include <stdint.h>
#include "viper_params.h"

#if VIPER_ENABLE_PROFILE_API
void viper_profile_u_pack_c(unsigned char *out, const uint16_t *poly, const unsigned char *dither);
unsigned viper_profile_u_cmp_c(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither);
void viper_profile_pk_pack_c(unsigned char *out, const uint16_t *poly, const unsigned char *dpk);
void viper_profile_pk_reconstruct_c(int16_t *out, const unsigned char *in, const unsigned char *dpk);
void viper_profile_v_pack_c(unsigned char *out, const uint16_t *poly, const unsigned char *dither);
unsigned viper_profile_v_cmp_c(const unsigned char *bytes, const uint16_t *poly, const unsigned char *dither);
void viper_profile_v_reconstruct_c(uint16_t *out, const unsigned char *in, const unsigned char *dither);
#endif

#endif
