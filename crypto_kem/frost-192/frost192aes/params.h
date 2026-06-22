#ifndef PARAMS_H
#define PARAMS_H

#define PARAMS_N 968
#define PARAMS_NBAR 8
#define PARAMS_LOGQ 16
#define PARAMS_Q (1 << PARAMS_LOGQ)
#define PARAMS_EXTRACTED_BITS 3
#define PARAMS_STRIPE_STEP 8
#define PARAMS_PARALLEL 4
#define BYTES_SEED_A 32
#define BYTES_MU (PARAMS_EXTRACTED_BITS * PARAMS_NBAR * PARAMS_NBAR / 8)
#define BYTES_PKHASH CRYPTO_BYTES
#define BYTES_SALT 32
#define BYTES_SEED_SE (2 * CRYPTO_BYTES)
#define PARAMS_PK_LOGP 11
#define PARAMS_U_LOGP 11
#define PARAMS_V_LOGP 7
#define PARAMS_ETA_S 1
#define PARAMS_ETA_R 1
#define PARAMS_ETA PARAMS_ETA_S

#if (PARAMS_NBAR % 8 != 0)
#error You have modified the cryptographic parameters. MAMBA-Frost requires PARAMS_NBAR to be a multiple of 8.
#endif

#define shake shake256

#define CDF_TABLE_DATA {5638, 15915, 23689, 28571, 31116, 32217, 32613, 32731, 32760, 32766, 32767}
#define CDF_TABLE_LEN 11

#endif