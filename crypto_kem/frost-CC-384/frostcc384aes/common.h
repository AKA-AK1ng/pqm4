#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

int frodo_mul_add_as_plus_e(uint16_t *out, const uint16_t *s, const uint8_t *seed_A);
int frodo_mul_add_sa_plus_e(uint16_t *out, const uint16_t *s, const uint8_t *seed_A);
void frost_sample_n(uint16_t *s, size_t n);
void frodo_mul_bs(uint16_t *out, const uint16_t *b, const uint16_t *s);
void frodo_mul_add_sb_plus_e(uint16_t *out, const uint16_t *b, const uint16_t *s);
void frodo_add(uint16_t *out, const uint16_t *a, const uint16_t *b);
void frodo_sub(uint16_t *out, const uint16_t *a, const uint16_t *b);
void frodo_key_encode(uint16_t *out, const uint16_t *in);
void frodo_key_decode(uint16_t *out, const uint16_t *in);
void frodo_pack(uint8_t *out, size_t outlen, const uint16_t *in, size_t inlen, uint8_t lsb);
void frodo_unpack(uint16_t *out, size_t outlen, const uint8_t *in, size_t inlen, uint8_t lsb);
int8_t ct_verify(const uint16_t *a, const uint16_t *b, size_t len);
void ct_select(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len, int8_t selector);
void clear_bytes(uint8_t *mem, size_t n);
uint16_t LE_TO_UINT16(uint16_t n);
uint16_t UINT16_TO_LE(uint16_t n);

#endif
