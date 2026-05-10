/*
 * kem.c — Viper-512 KEM (FO 变换封装层)
 *
 * 密文字节布局（MLWQ_CIPHERTEXTBYTES = 3456 字节）：
 *   u 向量压缩  (MLWQ_POLYVECCOMPRESSEDBYTES = 3168 字节，11 位/系数)
 *   v 标量压缩  (MLWQ_POLYCOMPRESSEDBYTES    =  224 字节，7 位/系数)
 *   c = m XOR kr[0:MU_BYTES]   (MU_BYTES = 64 字节，OTP 加密明文 m)
 *
 * 公钥字节布局（MLWQ_PUBLICKEYBYTES = 3200 字节）：
 *   b_q 向量打包 (MLWQ_POLYVECBYTES = 3168 字节，11 位/系数)
 *   seed_A       (SEEDBYTES = 32 字节)
 *
 * KEM 私钥字节布局（MLWQ_KEM_SECRETKEYBYTES = 6432 字节）：
 *   s 向量打包   (MLWQ_POLYVECBYTES = 3168 字节，11 位/系数，负数以补码表示)
 *   公钥字节     (MLWQ_PUBLICKEYBYTES = 3200 字节)
 *   H(pk)        (HASHBYTES = 32 字节)
 *   z            (SEEDBYTES = 32 字节)
 */

#include <string.h>
#include "api.h"
#include "params.h"
#include "structs.h"
#include "mlwq.h"
#include "poly.h"
#include "random.h"
#include "fips202.h"

/* 11 位掩码与有符号转换常量 */
#define SK_PACK_MASK  0x7FFu   /* (1 << BIT_PK) - 1 = 2047 */
#define SK_SIGN_HALF  1024u    /* (1 << (BIT_PK-1))        */
#define SK_SIGN_MOD   2048     /* (1 << BIT_PK)             */

/* -------------------------------------------------------------------------
 * 公钥打包 / 解包
 * 格式：b_q[0..K-1] packed (11 位/系数) || seed_A
 * ------------------------------------------------------------------------- */

static void pack_pk(unsigned char *out, const mlwq_pk *pk)
{
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_tobytes(out + i * MLWQ_POLYBYTES, &pk->b_q.vec[i]);
    memcpy(out + MLWQ_POLYVECBYTES, pk->seed_A, SEEDBYTES);
}

static void unpack_pk(mlwq_pk *pk, const unsigned char *in)
{
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_frombytes(&pk->b_q.vec[i], in + i * MLWQ_POLYBYTES);
    memcpy(pk->seed_A, in + MLWQ_POLYVECBYTES, SEEDBYTES);
    /* seed_d 不在公钥中序列化，置零即可（encrypt 不使用它） */
    memset(pk->seed_d, 0, SEEDBYTES);
}

/* -------------------------------------------------------------------------
 * 私钥打包 / 解包
 * 格式：s packed || pk_bytes || H(pk) || z
 * s 的系数 ∈ {-1, 0, 1}，打包为 11 位无符号（-1 → 2047）
 * ------------------------------------------------------------------------- */

static void pack_sk(unsigned char *out, const mlwq_kem_sk *sk,
                    const unsigned char *pk_bytes)
{
    for (int i = 0; i < MLWQ_K; i++) {
        poly temp = sk->pke_sk.s.vec[i];
        for (int j = 0; j < MLWQ_N; j++)
            temp.coeffs[j] = (int16_t)((uint16_t)temp.coeffs[j] & SK_PACK_MASK);
        ref_poly_tobytes(out + i * MLWQ_POLYBYTES, &temp);
    }
    memcpy(out + MLWQ_POLYVECBYTES, pk_bytes, MLWQ_PUBLICKEYBYTES);
    memcpy(out + MLWQ_POLYVECBYTES + MLWQ_PUBLICKEYBYTES, sk->h_pk, HASHBYTES);
    memcpy(out + MLWQ_POLYVECBYTES + MLWQ_PUBLICKEYBYTES + HASHBYTES, sk->z, SEEDBYTES);
}

static void unpack_sk(mlwq_kem_sk *sk, const unsigned char *in)
{
    for (int i = 0; i < MLWQ_K; i++) {
        ref_poly_frombytes(&sk->pke_sk.s.vec[i], in + i * MLWQ_POLYBYTES);
        /* 将 11 位无符号恢复为有符号：v > 1024 时为负数 */
        for (int j = 0; j < MLWQ_N; j++) {
            uint16_t v = (uint16_t)sk->pke_sk.s.vec[i].coeffs[j];
            if (v > SK_SIGN_HALF)
                sk->pke_sk.s.vec[i].coeffs[j] = (int16_t)(v - SK_SIGN_MOD);
            else
                sk->pke_sk.s.vec[i].coeffs[j] = (int16_t)v;
        }
    }
    unpack_pk(&sk->pk, in + MLWQ_POLYVECBYTES);
    memcpy(sk->h_pk,
           in + MLWQ_POLYVECBYTES + MLWQ_PUBLICKEYBYTES,
           HASHBYTES);
    memcpy(sk->z,
           in + MLWQ_POLYVECBYTES + MLWQ_PUBLICKEYBYTES + HASHBYTES,
           SEEDBYTES);
}

/* -------------------------------------------------------------------------
 * 密文打包 / 解包
 * 格式：u packed (11 位) || v packed (7 位) || c (MU_BYTES，OTP 加密 m)
 * ------------------------------------------------------------------------- */

static void pack_ct(unsigned char *out, const mlwq_ciphertext *ct,
                    const uint8_t *c)
{
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_compress_u(out + i * MLWQ_POLY_U_BYTES, &ct->u.vec[i]);
    ref_poly_compress_v(out + MLWQ_POLYVECCOMPRESSEDBYTES, &ct->v);
    memcpy(out + MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES,
           c, MU_BYTES);
}

static void unpack_ct(mlwq_ciphertext *ct, uint8_t *c,
                      const unsigned char *in)
{
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_decompress_u(&ct->u.vec[i], in + i * MLWQ_POLY_U_BYTES);
    ref_poly_decompress_v(&ct->v, in + MLWQ_POLYVECCOMPRESSEDBYTES);
    memcpy(c,
           in + MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES,
           MU_BYTES);
}

/* -------------------------------------------------------------------------
 * 辅助：将 mlwq_ciphertext 结构体打包为字节（仅 LWE 部分，不含 c）
 * 输出长度：MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES
 * ------------------------------------------------------------------------- */

static void pack_ct_lwe(unsigned char *out, const mlwq_ciphertext *ct)
{
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_compress_u(out + i * MLWQ_POLY_U_BYTES, &ct->u.vec[i]);
    ref_poly_compress_v(out + MLWQ_POLYVECCOMPRESSEDBYTES, &ct->v);
}

/* -------------------------------------------------------------------------
 * crypto_kem_keypair
 *
 * 调用 ref_mlwq_kem_keygen 生成密钥对，然后序列化为字节。
 * ------------------------------------------------------------------------- */

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
    mlwq_pk     pk_s;
    mlwq_kem_sk sk_s;

    ref_mlwq_kem_keygen(&pk_s, &sk_s);

    pack_pk(pk, &pk_s);
    pack_sk(sk, &sk_s, pk);

    return 0;
}

/* -------------------------------------------------------------------------
 * crypto_kem_enc
 *
 * FO 变换封装：
 *   1. 随机采样 m ∈ {0,1}^MU_BYTES
 *   2. 计算 H(pk)（MU_BYTES 字节输出）
 *   3. 派生 kr = SHAKE128(m || H(pk))，长度 MLWQ_SSBYTES + SEEDBYTES
 *   4. ss = kr[0 : MLWQ_SSBYTES]
 *   5. LWE 加密：(u, v) = Enc(pk, m; kr[MLWQ_SSBYTES:])
 *   6. c = m XOR kr[0 : MU_BYTES]（OTP 加密明文，供解封装额外验证）
 *   7. 打包 ct = u || v || c
 * ------------------------------------------------------------------------- */

int crypto_kem_enc(unsigned char *ct, unsigned char *ss,
                   const unsigned char *pk)
{
    mlwq_pk         pk_s;
    mlwq_ciphertext ct_s;

    unpack_pk(&pk_s, pk);

    /* Step 1: 随机明文 m */
    uint8_t m[MU_BYTES];
    random_bytes(m, MU_BYTES);

    /* Step 2–3: buf = m || H(pk)，然后 kr = SHAKE128(buf) */
    uint8_t buf[2 * MU_BYTES];
    memcpy(buf, m, MU_BYTES);
    /* H(pk) 使用与 keygen 一致的规范公钥字节（pk 参数本身已是打包格式） */
    shake128(buf + MU_BYTES, MU_BYTES, pk, MLWQ_PUBLICKEYBYTES);

    uint8_t kr[MLWQ_SSBYTES + SEEDBYTES];
    shake128(kr, sizeof(kr), buf, sizeof(buf));

    /* Step 4: 共享密钥 */
    memcpy(ss, kr, MLWQ_SSBYTES);

    /* Step 5: LWE 加密，seed_ct = kr[MLWQ_SSBYTES:] */
    ref_mlwq_encrypt(&ct_s, &pk_s, m, kr + MLWQ_SSBYTES);

    /* Step 6: c = m XOR ss（kr[0:MU_BYTES] == ss，因为 MU_BYTES == MLWQ_SSBYTES） */
    uint8_t c[MU_BYTES];
    for (int i = 0; i < MU_BYTES; i++)
        c[i] = m[i] ^ kr[i];

    /* Step 7: 打包密文 */
    pack_ct(ct, &ct_s, c);
    return 0;
}

/* -------------------------------------------------------------------------
 * crypto_kem_dec
 *
 * FO 变换解封装：
 *   1. 解包 ct → (ct_struct, c)；解包 sk → sk_struct
 *   2. LWE 解密恢复 m'
 *   3. 重算 H(pk) 与 kr'
 *   4. 重新加密 m' 并打包为字节，与原 ct 的 LWE 部分做字节比较
 *   5. 验证 c XOR kr'[0:MU_BYTES] == m'（OTP 检查）
 *   6. 两项检查均通过：ss = kr'[0:MLWQ_SSBYTES]
 *      否则隐式拒绝：ss = SHAKE128(z || ct)
 * ------------------------------------------------------------------------- */

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct,
                   const unsigned char *sk)
{
    mlwq_kem_sk     sk_s;
    mlwq_ciphertext ct_s;
    uint8_t         c[MU_BYTES];

    unpack_sk(&sk_s, sk);
    unpack_ct(&ct_s, c, ct);

    /* Step 2: LWE 解密 */
    uint8_t m[MU_BYTES];
    ref_mlwq_decrypt(m, &sk_s.pke_sk, &ct_s);

    /* Step 3: 从 sk 内存储的公钥重建规范公钥字节，计算 H(pk) 与 kr' */
    uint8_t pk_bytes[MLWQ_PUBLICKEYBYTES];
    for (int i = 0; i < MLWQ_K; i++)
        ref_poly_tobytes(pk_bytes + i * MLWQ_POLYBYTES, &sk_s.pk.b_q.vec[i]);
    memcpy(pk_bytes + MLWQ_POLYVECBYTES, sk_s.pk.seed_A, SEEDBYTES);

    uint8_t buf[2 * MU_BYTES];
    memcpy(buf, m, MU_BYTES);
    shake128(buf + MU_BYTES, MU_BYTES, pk_bytes, MLWQ_PUBLICKEYBYTES);

    uint8_t kr[MLWQ_SSBYTES + SEEDBYTES];
    shake128(kr, sizeof(kr), buf, sizeof(buf));

    /* Step 4: 重新加密，打包 LWE 部分用于字节比较 */
    mlwq_ciphertext ct_prime;
    memset(&ct_prime, 0, sizeof(ct_prime));
    ref_mlwq_encrypt(&ct_prime, &sk_s.pk, m, kr + MLWQ_SSBYTES);

    uint8_t ct_prime_lwe[MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES];
    pack_ct_lwe(ct_prime_lwe, &ct_prime);

    /* Step 5: OTP 验证：c_check = m XOR kr[0:MU_BYTES] 应等于 c */
    uint8_t c_check[MU_BYTES];
    for (int i = 0; i < MU_BYTES; i++)
        c_check[i] = m[i] ^ kr[i];

    /* Step 6: 两项检查 */
    int lwe_ok = (memcmp(ct, ct_prime_lwe,
                         MLWQ_POLYVECCOMPRESSEDBYTES + MLWQ_POLYCOMPRESSEDBYTES) == 0);
    int c_ok   = (memcmp(c, c_check, MU_BYTES) == 0);

    if (lwe_ok && c_ok) {
        memcpy(ss, kr, MLWQ_SSBYTES);
    } else {
        /* 隐式拒绝：ss = SHAKE128(z || ct) */
        uint8_t input[SEEDBYTES + MLWQ_CIPHERTEXTBYTES];
        memcpy(input, sk_s.z, SEEDBYTES);
        memcpy(input + SEEDBYTES, ct, MLWQ_CIPHERTEXTBYTES);
        shake128(ss, MLWQ_SSBYTES, input, sizeof(input));
    }
    return 0;
}
