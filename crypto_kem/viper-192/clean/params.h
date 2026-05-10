#ifndef PARAMS_H
#define PARAMS_H

/* -------------------------------------------------------------------------
 * Viper-512 参数集（从 params_ref.h VIPER_PARAM=512 分支提取）
 * 对应 NIST 安全级别 5
 * ------------------------------------------------------------------------- */

#define PARAM_NAME "Viper-512"

/* 基础参数 */
#define MLWQ_N    256
#define MLWQ_Q    3329

#define SEEDBYTES 32
#define HASHBYTES 32

/* 维度 */
#define MLWQ_K 9

/* 噪声参数 */
#define MLWQ_ETA_S 1
#define MLWQ_ETA_R 1
#define MLWQ_ETA1  MLWQ_ETA_S

/* 位宽 / 压缩参数 */
#define BIT_PK 11
#define BIT_U  11
#define BIT_V   7

/* 量化模数 */
#define P_PK (1 << BIT_PK)   /* 2048 */
#define P_U  (1 << BIT_U)    /* 2048 */
#define P_V  (1 << BIT_V)    /* 128  */

/* 多比特消息参数（Viper-512 每个系数编码 2 bit，256 系数 * 2 = 512 bit = 64 字节） */
#define MSG_BITS  2
#define MU_BYTES  64
#define MU_BITS   (MU_BYTES * 8)   /* 512 */

/* 共享密钥长度 */
#define MLWQ_SSBYTES 64

/* -------------------------------------------------------------------------
 * 自动推导字节大小
 * ------------------------------------------------------------------------- */

/* 公钥多项式向量打包（BIT_PK=11 位/系数） */
#define MLWQ_POLYBYTES           ((MLWQ_N * BIT_PK) / 8)           /* 256*11/8 = 352  */
#define MLWQ_POLYVECBYTES        (MLWQ_K * MLWQ_POLYBYTES)          /* 9*352   = 3168 */

/* 密文 u 压缩（BIT_U=11 位/系数） */
#define MLWQ_POLY_U_BYTES        ((MLWQ_N * BIT_U) / 8)            /* 256*11/8 = 352  */
#define MLWQ_POLYVECCOMPRESSEDBYTES (MLWQ_K * MLWQ_POLY_U_BYTES)   /* 9*352   = 3168 */

/* 密文 v 压缩（BIT_V=7 位/系数） */
#define MLWQ_POLYCOMPRESSEDBYTES ((MLWQ_N * BIT_V) / 8)            /* 256*7/8  = 224  */

/* 总大小
 *
 * 密文 = u_packed(3168) || v_packed(224) || c(64)
 * 其中 c = m XOR ss 为消息的 OTP 加密（参见 kem.c）
 */
#define MLWQ_CIPHERTEXTBYTES  (MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES + MU_BYTES)
                               /* 3168 + 224 + 64 = 3456 */

/* 公钥 = b_q packed(3168) || seed_A(32) */
#define MLWQ_PUBLICKEYBYTES   (MLWQ_POLYVECBYTES + SEEDBYTES)       /* 3168 + 32  = 3200 */

/* PKE 私钥（仅 s） */
#define MLWQ_SECRETKEYBYTES    MLWQ_POLYVECBYTES                    /* 3168 */

/* KEM 私钥 = s_packed(3168) || pk_packed(3200) || H(pk)(32) || z(32) */
#define MLWQ_KEM_SECRETKEYBYTES \
    (MLWQ_POLYVECBYTES + MLWQ_PUBLICKEYBYTES + HASHBYTES + SEEDBYTES)
    /* 3168 + 3200 + 32 + 32 = 6432 */

#endif /* PARAMS_H */