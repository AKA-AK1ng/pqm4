#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include "params.h"

/* 单个多项式（N=256 个 int16_t 系数，32 字节对齐） */
typedef struct {
    int16_t coeffs[MLWQ_N];
} __attribute__((aligned(32))) poly;

/* K 维多项式向量 */
typedef struct {
    poly vec[MLWQ_K];
} __attribute__((aligned(32))) poly_vec;

/* K×K 多项式矩阵 */
typedef struct {
    poly_vec row[MLWQ_K];
} __attribute__((aligned(32))) poly_matrix;

/* 公钥：包含量化后的 b_q、矩阵种子 seed_A 与抖动种子 seed_d */
typedef struct {
    uint8_t  seed_A[SEEDBYTES];
    uint8_t  seed_d[SEEDBYTES];
    poly_vec b_q;
} mlwq_pk;

/* PKE 私钥：仅包含秘密向量 s */
typedef struct {
    poly_vec s;
} mlwq_sk;

/* KEM 私钥（FO 变换扩展）：PKE 私钥 + 公钥 + H(pk) + 拒绝随机数 z */
typedef struct {
    mlwq_sk pke_sk;
    mlwq_pk pk;
    uint8_t h_pk[HASHBYTES];
    uint8_t z[SEEDBYTES];
} mlwq_kem_sk;

/* 密文：包含向量 u（KEM 随机性部分）与标量 v（消息承载项） */
typedef struct {
    poly_vec u;
    poly     v;
} mlwq_ciphertext;

#endif /* STRUCTS_H */