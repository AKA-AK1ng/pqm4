
/********************************************************************************************
* FrodoKEM: Learning with Errors Key Encapsulation
*
* Abstract: Key Encapsulation Mechanism (KEM) based on Frodo
*********************************************************************************************/

#include <stdint.h>
#include <string.h>

#include "fips202.h"
#include "randombytes.h"

#include "api.h"
#include "common.h"
#include "params.h"

#define DITHER_DOMAIN_PK 0xA1
#define DITHER_DOMAIN_U  0xB1
#define DITHER_DOMAIN_V  0xC1
#define DITHER_BLOCK_WORDS 128

#define PK_PACKED_BYTES ((PARAMS_PK_LOGP * PARAMS_N * PARAMS_NBAR_R) / 8)
#define CT_C1_PACKED_BYTES ((PARAMS_U_LOGP * PARAMS_N * PARAMS_NBAR_S) / 8)
#define CT_C2_PACKED_BYTES ((PARAMS_V_LOGP * PARAMS_NBAR_R * PARAMS_NBAR_S) / 8)

#define SK_OFFSET_S 0
#define SK_OFFSET_PK (SK_OFFSET_S + CRYPTO_BYTES)
#define SK_OFFSET_SEEDSE (SK_OFFSET_PK + CRYPTO_PUBLICKEYBYTES)
#define SK_OFFSET_PKH (SK_OFFSET_SEEDSE + BYTES_SEED_SE)

static inline uint16_t frodo_q_mask_local(void)
{
    return (uint16_t)((1u << PARAMS_LOGQ) - 1u);
}

static inline uint16_t frodo_p_mask_local(unsigned int logp)
{
    return (uint16_t)((1u << logp) - 1u);
}

static inline uint16_t frodo_reconstruct_local(uint16_t x, unsigned int logp)
{
    return (uint16_t)((x & frodo_p_mask_local(logp)) << (PARAMS_LOGQ - logp));
}

static inline uint16_t frodo_quantize_local(uint16_t x, uint16_t d, unsigned int logp)
{
    const unsigned int shift = PARAMS_LOGQ - logp;
    uint32_t z = ((uint32_t)x & frodo_q_mask_local()) + ((uint32_t)d & ((1u << shift) - 1u));
    z = (z + (1u << (shift - 1))) >> shift;
    return (uint16_t)(z & frodo_p_mask_local(logp));
}

static void frodo_dither_init_stream(shake128incctx *state, const uint8_t *seed, size_t seedlen, uint8_t domain)
{
    uint8_t in[1 + BYTES_SEED_A + BYTES_SALT] = {0};

    in[0] = domain;
    memcpy(&in[1], seed, seedlen);
    shake128_inc_init(state);
    shake128_inc_absorb(state, in, 1 + seedlen);
    shake128_inc_finalize(state);
    clear_bytes(in, sizeof(in));
}

static void frodo_quantize_dithered_local(uint16_t *out, const uint16_t *in, size_t n, const uint8_t *seed, size_t seedlen, uint8_t domain, unsigned int logp)
{
    const unsigned int shift = PARAMS_LOGQ - logp;
    const uint16_t mask = (uint16_t)((1u << shift) - 1u);
    uint16_t d[DITHER_BLOCK_WORDS] = {0};
    shake128incctx state;
    size_t offset = 0;

    frodo_dither_init_stream(&state, seed, seedlen, domain);
    while (offset < n) {
        size_t take = n - offset;
        if (take > DITHER_BLOCK_WORDS) {
            take = DITHER_BLOCK_WORDS;
        }

        shake128_inc_squeeze((uint8_t *)d, take * sizeof(uint16_t), &state);
        for (size_t i = 0; i < take; i++) {
            uint16_t di = LE_TO_UINT16(d[i]) & mask;
            out[offset + i] = frodo_quantize_local(in[offset + i], di, logp);
        }
        offset += take;
    }

    clear_bytes((uint8_t *)d, sizeof(d));
    clear_bytes((uint8_t *)&state, sizeof(state));
}

static void frodo_reconstruct_dithered_local(uint16_t *normal, const uint16_t *split, size_t n, const uint8_t *seed, size_t seedlen, uint8_t domain, unsigned int logp)
{
    const unsigned int shift = PARAMS_LOGQ - logp;
    const uint16_t mask = (uint16_t)((1u << shift) - 1u);
    uint16_t d[DITHER_BLOCK_WORDS] = {0};
    shake128incctx state;
    size_t offset = 0;

    frodo_dither_init_stream(&state, seed, seedlen, domain);
    while (offset < n) {
        size_t take = n - offset;
        if (take > DITHER_BLOCK_WORDS) {
            take = DITHER_BLOCK_WORDS;
        }

        shake128_inc_squeeze((uint8_t *)d, take * sizeof(uint16_t), &state);
        for (size_t i = 0; i < take; i++) {
            uint16_t di = LE_TO_UINT16(d[i]) & mask;
            normal[offset + i] = (uint16_t)((frodo_reconstruct_local(split[offset + i], logp) - di) & frodo_q_mask_local());
        }
        offset += take;
    }

    clear_bytes((uint8_t *)d, sizeof(d));
    clear_bytes((uint8_t *)&state, sizeof(state));
}

static void frost_hash_ciphertext_key(uint8_t *ss, const uint8_t *ct, const uint8_t *key)
{
    shake128incctx state;
    shake128_inc_init(&state);
    shake128_inc_absorb(&state, ct, CRYPTO_CIPHERTEXTBYTES);
    shake128_inc_absorb(&state, key, CRYPTO_BYTES);
    shake128_inc_finalize(&state);
    shake128_inc_squeeze(ss, CRYPTO_BYTES, &state);
    clear_bytes((uint8_t *)&state, sizeof(state));
}

int crypto_kem_keypair(uint8_t *pk, uint8_t *sk)
{
    uint8_t *pk_seedA = &pk[0];
    uint8_t *pk_b = &pk[BYTES_SEED_A];
    uint8_t *sk_s = &sk[SK_OFFSET_S];
    uint8_t *sk_pk = &sk[SK_OFFSET_PK];
    uint8_t *sk_seedSE = &sk[SK_OFFSET_SEEDSE];
    uint8_t *sk_pkh = &sk[SK_OFFSET_PKH];
    uint16_t B[PARAMS_N * PARAMS_NBAR_R] = {0};
    uint16_t S[PARAMS_N * PARAMS_NBAR_R] = {0};
    uint8_t randomness[CRYPTO_BYTES + BYTES_SEED_SE + BYTES_SEED_A];
    uint8_t *randomness_s = &randomness[0];
    uint8_t *randomness_seedSE = &randomness[CRYPTO_BYTES];
    uint8_t *randomness_z = &randomness[CRYPTO_BYTES + BYTES_SEED_SE];
    uint8_t shake_input_seedSE[1 + BYTES_SEED_SE];
    shake128incctx state;

    if (randombytes(randomness, sizeof(randomness)) != 0) {
        return 1;
    }
    shake(pk_seedA, BYTES_SEED_A, randomness_z, BYTES_SEED_A);

    shake_input_seedSE[0] = 0x5F;
    memcpy(&shake_input_seedSE[1], randomness_seedSE, BYTES_SEED_SE);

    shake128_inc_init(&state);
    shake128_inc_absorb(&state, shake_input_seedSE, 1 + BYTES_SEED_SE);
    shake128_inc_finalize(&state);
    shake128_inc_squeeze((uint8_t *)S, PARAMS_N * PARAMS_NBAR_R * sizeof(uint16_t), &state);

    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_R; i++) {
        S[i] = LE_TO_UINT16(S[i]);
    }

    frost_sample_n(S, PARAMS_N * PARAMS_NBAR_R);
    memset(B, 0, sizeof(B));
    frodo_mul_add_as_plus_e(B, S, pk_seedA);
    frodo_quantize_dithered_local(B, B, PARAMS_N * PARAMS_NBAR_R, pk_seedA, BYTES_SEED_A, DITHER_DOMAIN_PK, PARAMS_PK_LOGP);

    frodo_pack(pk_b, PK_PACKED_BYTES, B, PARAMS_N * PARAMS_NBAR_R, PARAMS_PK_LOGP);

    memset(sk, 0, CRYPTO_SECRETKEYBYTES);
    memcpy(sk_s, randomness_s, CRYPTO_BYTES);
    memcpy(sk_pk, pk, CRYPTO_PUBLICKEYBYTES);
    memcpy(sk_seedSE, randomness_seedSE, BYTES_SEED_SE);

    shake(sk_pkh, BYTES_PKHASH, pk, CRYPTO_PUBLICKEYBYTES);

    clear_bytes((uint8_t *)B, sizeof(B));
    clear_bytes((uint8_t *)S, sizeof(S));
    clear_bytes(randomness, CRYPTO_BYTES + BYTES_SEED_SE);
    clear_bytes(shake_input_seedSE, sizeof(shake_input_seedSE));
    clear_bytes((uint8_t *)&state, sizeof(state));
    return 0;
}

int crypto_kem_enc(uint8_t *ct, uint8_t *ss, const uint8_t *pk)
{
    const uint8_t *pk_seedA = &pk[0];
    const uint8_t *pk_b = &pk[BYTES_SEED_A];
    uint8_t *ct_c1 = &ct[0];
    uint8_t *ct_c2 = &ct[CT_C1_PACKED_BYTES];
    uint16_t B_norm[PARAMS_N * PARAMS_NBAR_R] = {0};
    uint16_t Bp_raw[PARAMS_N * PARAMS_NBAR_S] = {0};
    uint16_t V_raw[PARAMS_NBAR_R * PARAMS_NBAR_S] = {0};
    uint16_t C_enc[PARAMS_NBAR_R * PARAMS_NBAR_S] = {0};
    uint16_t Sp[PARAMS_N * PARAMS_NBAR_S] = {0};
    uint8_t G2in[BYTES_PKHASH + BYTES_MU + BYTES_SALT];
    uint8_t *pkh = &G2in[0];
    uint8_t *mu = &G2in[BYTES_PKHASH];
    uint8_t *salt = &G2in[BYTES_PKHASH + BYTES_MU];
    uint8_t G2out[BYTES_SEED_SE + CRYPTO_BYTES];
    uint8_t *seedSE = &G2out[0];
    uint8_t *k = &G2out[BYTES_SEED_SE];
    uint8_t shake_input_seedSE[1 + BYTES_SEED_SE];
    shake128incctx state;

    shake(pkh, BYTES_PKHASH, pk, CRYPTO_PUBLICKEYBYTES);
    if (randombytes(mu, BYTES_MU + BYTES_SALT) != 0) {
        return 1;
    }
    shake(G2out, BYTES_SEED_SE + CRYPTO_BYTES, G2in, BYTES_PKHASH + BYTES_MU + BYTES_SALT);

    shake_input_seedSE[0] = 0x96;
    memcpy(&shake_input_seedSE[1], seedSE, BYTES_SEED_SE);

    shake128_inc_init(&state);
    shake128_inc_absorb(&state, shake_input_seedSE, 1 + BYTES_SEED_SE);
    shake128_inc_finalize(&state);
    shake128_inc_squeeze((uint8_t *)Sp, PARAMS_N * PARAMS_NBAR_S * sizeof(uint16_t), &state);

    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_S; i++) {
        Sp[i] = LE_TO_UINT16(Sp[i]);
    }

    frost_sample_n(Sp, PARAMS_N * PARAMS_NBAR_S);
    memset(Bp_raw, 0, sizeof(Bp_raw));
    frodo_mul_add_sa_plus_e(Bp_raw, Sp, pk_seedA);
    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_S; i++) {
        Bp_raw[i] &= frodo_q_mask_local();
    }
    frodo_quantize_dithered_local(Bp_raw, Bp_raw, PARAMS_N * PARAMS_NBAR_S, salt, BYTES_SALT, DITHER_DOMAIN_U, PARAMS_U_LOGP);
    frodo_pack(ct_c1, CT_C1_PACKED_BYTES, Bp_raw, PARAMS_N * PARAMS_NBAR_S, PARAMS_U_LOGP);

    frodo_unpack(B_norm, PARAMS_N * PARAMS_NBAR_R, pk_b, PK_PACKED_BYTES, PARAMS_PK_LOGP);
    frodo_reconstruct_dithered_local(B_norm, B_norm, PARAMS_N * PARAMS_NBAR_R, pk_seedA, BYTES_SEED_A, DITHER_DOMAIN_PK, PARAMS_PK_LOGP);
    memset(V_raw, 0, sizeof(V_raw));
    frodo_mul_add_sb_plus_e(V_raw, B_norm, Sp);

    frodo_key_encode(C_enc, (uint16_t *)mu);
    frodo_add(C_enc, V_raw, C_enc);
    frodo_quantize_dithered_local(C_enc, C_enc, PARAMS_NBAR_R * PARAMS_NBAR_S, salt, BYTES_SALT, DITHER_DOMAIN_V, PARAMS_V_LOGP);
    frodo_pack(ct_c2, CT_C2_PACKED_BYTES, C_enc, PARAMS_NBAR_R * PARAMS_NBAR_S, PARAMS_V_LOGP);

    // 加入salt到密文中
    memcpy(&ct[CRYPTO_CIPHERTEXTBYTES - BYTES_SALT], salt, BYTES_SALT);

    frost_hash_ciphertext_key(ss, ct, k);

    clear_bytes((uint8_t *)V_raw, sizeof(V_raw));
    clear_bytes((uint8_t *)Sp, sizeof(Sp));
    clear_bytes(mu, BYTES_MU);
    clear_bytes(G2out, sizeof(G2out));
    clear_bytes(shake_input_seedSE, sizeof(shake_input_seedSE));
    clear_bytes((uint8_t *)&state, sizeof(state));
    clear_bytes((uint8_t *)B_norm, sizeof(B_norm));
    clear_bytes((uint8_t *)C_enc, sizeof(C_enc));
    clear_bytes((uint8_t *)Bp_raw, sizeof(Bp_raw));
    return 0;
}

int crypto_kem_dec(uint8_t *ss, const uint8_t *ct, const uint8_t *sk)
{
    uint16_t B_norm[PARAMS_N * PARAMS_NBAR_R] = {0};
    uint16_t Bp_split[PARAMS_N * PARAMS_NBAR_S] = {0};
    uint16_t Bp_norm[PARAMS_N * PARAMS_NBAR_S] = {0};
    uint16_t W[PARAMS_NBAR_R * PARAMS_NBAR_S] = {0};
    uint16_t C_split[PARAMS_NBAR_R * PARAMS_NBAR_S] = {0};
    uint16_t C_norm[PARAMS_NBAR_R * PARAMS_NBAR_S] = {0};
    uint16_t S[PARAMS_N * PARAMS_NBAR_R] = {0};
    uint16_t Sp[PARAMS_N * PARAMS_NBAR_S] = {0};
    uint8_t shake_input_seedSE[1 + BYTES_SEED_SE];
    const uint8_t *ct_c1 = &ct[0];
    const uint8_t *ct_c2 = &ct[CT_C1_PACKED_BYTES];
    const uint8_t *ct_salt = &ct[CRYPTO_CIPHERTEXTBYTES - BYTES_SALT];
    const uint8_t *sk_s = &sk[SK_OFFSET_S];
    const uint8_t *sk_pk = &sk[SK_OFFSET_PK];
    const uint8_t *sk_seedSE = &sk[SK_OFFSET_SEEDSE];
    const uint8_t *sk_pkh = &sk[SK_OFFSET_PKH];
    const uint8_t *pk_seedA = &sk_pk[0];
    const uint8_t *pk_b = &sk_pk[BYTES_SEED_A];
    uint8_t G2in[BYTES_PKHASH + BYTES_MU + BYTES_SALT];
    uint8_t *pkh = &G2in[0];
    uint8_t *muprime = &G2in[BYTES_PKHASH];
    uint8_t *G2in_salt = &G2in[BYTES_PKHASH + BYTES_MU];
    uint8_t G2out[BYTES_SEED_SE + CRYPTO_BYTES];
    uint8_t *seedSEprime = &G2out[0];
    uint8_t *kprime = &G2out[BYTES_SEED_SE];
    uint8_t Fin_k[CRYPTO_BYTES];
    uint8_t shake_input_seedSEprime[1 + BYTES_SEED_SE];
    shake128incctx state;

    shake_input_seedSE[0] = 0x5F;
    memcpy(&shake_input_seedSE[1], sk_seedSE, BYTES_SEED_SE);

    frodo_unpack(Bp_split, PARAMS_N * PARAMS_NBAR_S, ct_c1, CT_C1_PACKED_BYTES, PARAMS_U_LOGP);
    frodo_unpack(C_split, PARAMS_NBAR_R * PARAMS_NBAR_S, ct_c2, CT_C2_PACKED_BYTES, PARAMS_V_LOGP);
    frodo_reconstruct_dithered_local(Bp_norm, Bp_split, PARAMS_N * PARAMS_NBAR_S, ct_salt, BYTES_SALT, DITHER_DOMAIN_U, PARAMS_U_LOGP);
    frodo_reconstruct_dithered_local(C_norm, C_split, PARAMS_NBAR_R * PARAMS_NBAR_S, ct_salt, BYTES_SALT, DITHER_DOMAIN_V, PARAMS_V_LOGP);

    shake128_inc_init(&state);
    shake128_inc_absorb(&state, shake_input_seedSE, 1 + BYTES_SEED_SE);
    shake128_inc_finalize(&state);
    shake128_inc_squeeze((uint8_t *)S, PARAMS_N * PARAMS_NBAR_R * sizeof(uint16_t), &state);
    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_R; i++) {
        S[i] = LE_TO_UINT16(S[i]);
    }
    frost_sample_n(S, PARAMS_N * PARAMS_NBAR_R);

    frodo_mul_bs(W, Bp_norm, S);
    frodo_sub(W, C_norm, W);
    frodo_key_decode((uint16_t *)muprime, W);

    memcpy(pkh, sk_pkh, BYTES_PKHASH);
    memcpy(G2in_salt, ct_salt, BYTES_SALT);
    shake(G2out, BYTES_SEED_SE + CRYPTO_BYTES, G2in, BYTES_PKHASH + BYTES_MU + BYTES_SALT);

    shake_input_seedSEprime[0] = 0x96;
    memcpy(&shake_input_seedSEprime[1], seedSEprime, BYTES_SEED_SE);

    shake128_inc_init(&state);
    shake128_inc_absorb(&state, shake_input_seedSEprime, 1 + BYTES_SEED_SE);
    shake128_inc_finalize(&state);
    shake128_inc_squeeze((uint8_t *)Sp, PARAMS_N * PARAMS_NBAR_S * sizeof(uint16_t), &state);
    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_S; i++) {
        Sp[i] = LE_TO_UINT16(Sp[i]);
    }
    frost_sample_n(Sp, PARAMS_N * PARAMS_NBAR_S);

    memset(Bp_norm, 0, sizeof(Bp_norm));
    frodo_mul_add_sa_plus_e(Bp_norm, Sp, pk_seedA);
    for (size_t i = 0; i < PARAMS_N * PARAMS_NBAR_S; i++) {
        Bp_norm[i] &= frodo_q_mask_local();
    }
    frodo_quantize_dithered_local(Bp_norm, Bp_norm, PARAMS_N * PARAMS_NBAR_S, ct_salt, BYTES_SALT, DITHER_DOMAIN_U, PARAMS_U_LOGP);

    memset(C_norm, 0, sizeof(C_norm));
    frodo_unpack(B_norm, PARAMS_N * PARAMS_NBAR_R, pk_b, PK_PACKED_BYTES, PARAMS_PK_LOGP);
    frodo_reconstruct_dithered_local(B_norm, B_norm, PARAMS_N * PARAMS_NBAR_R, pk_seedA, BYTES_SEED_A, DITHER_DOMAIN_PK, PARAMS_PK_LOGP);
    frodo_mul_add_sb_plus_e(C_norm, B_norm, Sp);
    frodo_key_encode(W, (uint16_t *)muprime);
    frodo_add(C_norm, C_norm, W);
    frodo_quantize_dithered_local(C_norm, C_norm, PARAMS_NBAR_R * PARAMS_NBAR_S, ct_salt, BYTES_SALT, DITHER_DOMAIN_V, PARAMS_V_LOGP);

    {
        int8_t selector = ct_verify(Bp_split, Bp_norm, PARAMS_N * PARAMS_NBAR_S) |
                          ct_verify(C_split, C_norm, PARAMS_NBAR_R * PARAMS_NBAR_S);
        ct_select(Fin_k, (uint8_t *)kprime, (uint8_t *)sk_s, CRYPTO_BYTES, selector);
    }
    frost_hash_ciphertext_key(ss, ct, Fin_k);

    clear_bytes((uint8_t *)W, sizeof(W));
    clear_bytes((uint8_t *)S, sizeof(S));
    clear_bytes((uint8_t *)Sp, sizeof(Sp));
    clear_bytes(muprime, BYTES_MU);
    clear_bytes(G2out, sizeof(G2out));
    clear_bytes(Fin_k, CRYPTO_BYTES);
    clear_bytes(shake_input_seedSE, sizeof(shake_input_seedSE));
    clear_bytes(shake_input_seedSEprime, sizeof(shake_input_seedSEprime));
    clear_bytes((uint8_t *)&state, sizeof(state));
    clear_bytes((uint8_t *)B_norm, sizeof(B_norm));
    clear_bytes((uint8_t *)Bp_split, sizeof(Bp_split));
    clear_bytes((uint8_t *)Bp_norm, sizeof(Bp_norm));
    clear_bytes((uint8_t *)C_split, sizeof(C_split));
    clear_bytes((uint8_t *)C_norm, sizeof(C_norm));
    return 0;
}
