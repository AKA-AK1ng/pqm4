#ifndef PARAMS_H
#define PARAMS_H

#define PARAMS_N 576
#define PARAMS_NBAR 8
#define PARAMS_LOGQ 16
#define PARAMS_Q (1 << PARAMS_LOGQ)
#define PARAMS_EXTRACTED_BITS 2
#define PARAMS_STRIPE_STEP 8
#define PARAMS_PARALLEL 4
#define BYTES_SEED_A 32
#define BYTES_MU (PARAMS_EXTRACTED_BITS * PARAMS_NBAR * PARAMS_NBAR / 8)
#define BYTES_PKHASH CRYPTO_BYTES
#define BYTES_SALT 32
#define BYTES_SEED_SE (2 * CRYPTO_BYTES)
#define PARAMS_PK_LOGP 10
#define PARAMS_U_LOGP 10
#define PARAMS_V_LOGP 8
#define PARAMS_ETA_S 3
#define PARAMS_ETA_R 3
#define PARAMS_ETA PARAMS_ETA_S

#if (PARAMS_NBAR % 8 != 0)
#error You have modified the cryptographic parameters. MAMBA-Frost requires PARAMS_NBAR to be a multiple of 8.
#endif

#define shake shake128

#define CDF_TABLE_DATA {4643, 13363, 20579, 25843, 29227, 31145, 32103, 32525, 32689, 32745, 32762, 32766, 32767}
#define CDF_TABLE_LEN 13

#endif