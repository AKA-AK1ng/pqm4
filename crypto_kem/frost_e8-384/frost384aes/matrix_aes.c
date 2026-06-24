#include <stdint.h>
#include <string.h>

#include "aes-publicinputs.h"
#include "api.h"
#include "common.h"
#include "params.h"

extern uint16_t xs(const uint16_t *s, const uint16_t *a_row);

static void frost_aes_expand_row(uint16_t *row, uint16_t *input,
                                 uint16_t row_index,
                                 const aes128ctx_publicinputs *ctx)
{
    memset(input, 0, PARAMS_N * sizeof(uint16_t));
    for (size_t j = 0; j < PARAMS_N; j += PARAMS_STRIPE_STEP) {
        input[j] = UINT16_TO_LE(row_index);
        input[j + 1] = UINT16_TO_LE((uint16_t)j);
    }
    aes128_ecb_publicinputs((uint8_t *)row, (const uint8_t *)input,
                            PARAMS_N * sizeof(uint16_t) / AES_BLOCKBYTES, ctx);
    for (size_t j = 0; j < PARAMS_N; j++) row[j] = LE_TO_UINT16(row[j]);
}

int frodo_mul_add_as_plus_e(uint16_t *out, const uint16_t *s,
                            const uint8_t *seed_A)
{
    uint16_t a_row[PARAMS_N];
    uint16_t a_input[PARAMS_N];
    aes128ctx_publicinputs ctx;
    aes128_ecb_keyexp_publicinputs(&ctx, seed_A);

    for (size_t i = 0; i < PARAMS_N; i++) {
        frost_aes_expand_row(a_row, a_input, (uint16_t)i, &ctx);
        for (size_t k = 0; k < PARAMS_NBAR_R; k++) {
            out[i * PARAMS_NBAR_R + k] =
                (uint16_t)(out[i * PARAMS_NBAR_R + k] +
                           xs(s + k * PARAMS_N, a_row));
        }
    }
    return 1;
}

int frodo_mul_add_sa_plus_e(uint16_t *out, const uint16_t *s,
                            const uint8_t *seed_A)
{
    uint16_t a_row[PARAMS_N];
    uint16_t a_input[PARAMS_N];
    aes128ctx_publicinputs ctx;
    aes128_ecb_keyexp_publicinputs(&ctx, seed_A);

    for (size_t j = 0; j < PARAMS_N; j++) {
        frost_aes_expand_row(a_row, a_input, (uint16_t)j, &ctx);
        for (size_t k = 0; k < PARAMS_NBAR_S; k++) {
            int32_t sp = (int32_t)(int16_t)s[k * PARAMS_N + j];
            for (size_t i = 0; i < PARAMS_N; i++) {
                int32_t v = (int32_t)out[k * PARAMS_N + i] +
                            (int32_t)a_row[i] * sp;
                out[k * PARAMS_N + i] = (uint16_t)v;
            }
        }
    }
    return 1;
}
