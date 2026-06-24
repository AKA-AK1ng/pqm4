#ifndef PARAMS_H
#define PARAMS_H

#define PARAMS_N 512
#define PARAMS_NBAR 8
#define PARAMS_NBAR_R 8
#define PARAMS_NBAR_S 8
#define PARAMS_LOGQ 15
#define PARAMS_Q (1 << PARAMS_LOGQ)
#define PARAMS_EXTRACTED_BITS 2
#define PARAMS_STRIPE_STEP 8
#define PARAMS_PARALLEL 4
#define BYTES_SEED_A 32
#define BYTES_MU (PARAMS_EXTRACTED_BITS * PARAMS_NBAR_R * PARAMS_NBAR_S / 8)
#define BYTES_PKHASH CRYPTO_BYTES
#define BYTES_SALT 32
#define BYTES_SEED_SE (2 * CRYPTO_BYTES)
#define PARAMS_PK_LOGP 10
#define PARAMS_U_LOGP 10
#define PARAMS_V_LOGP 5
#define PARAMS_ETA_S 2
#define PARAMS_ETA_R 2
#define PARAMS_ETA PARAMS_ETA_S

#if (PARAMS_NBAR_R != 8 || PARAMS_NBAR_S != 8)
#error Cortex-M4 Frost E8 implementation currently requires an 8x8 message matrix.
#endif

#define shake shake128
#define shakeincctx shake128incctx
#define shake_inc_init shake128_inc_init
#define shake_inc_absorb shake128_inc_absorb
#define shake_inc_finalize shake128_inc_finalize
#define shake_inc_squeeze shake128_inc_squeeze

#endif
