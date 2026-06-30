#ifndef PARAMS_H
#define PARAMS_H

#define PARAMS_N 1928
#define PARAMS_NBAR 8
#define PARAMS_NBAR_R 12
#define PARAMS_NBAR_S 8
#define PARAMS_LOGQ 16
#define PARAMS_Q (1 << PARAMS_LOGQ)
#define PARAMS_EXTRACTED_BITS 4
#define PARAMS_STRIPE_STEP 8
#define PARAMS_PARALLEL 4
#define BYTES_SEED_A 32
#define BYTES_MU (PARAMS_EXTRACTED_BITS * PARAMS_NBAR_R * PARAMS_NBAR_S / 8)
#define BYTES_PKHASH CRYPTO_BYTES
#define BYTES_SALT 32
#define BYTES_SEED_SE (2 * CRYPTO_BYTES)
#define PARAMS_PK_LOGP 13
#define PARAMS_U_LOGP 13
#define PARAMS_V_LOGP 9
#define PARAMS_ETA_S 1
#define PARAMS_ETA_R 1
#define PARAMS_ETA PARAMS_ETA_S

#if (PARAMS_NBAR_R != 12 || PARAMS_NBAR_S != 8)
#error Cortex-M4 Frost-CC-384 requires a 12x8 message matrix.
#endif

#define shake shake256
#define shake128incctx shake256incctx
#define shake128_inc_init shake256_inc_init
#define shake128_inc_absorb shake256_inc_absorb
#define shake128_inc_finalize shake256_inc_finalize
#define shake128_inc_squeeze shake256_inc_squeeze

#endif
