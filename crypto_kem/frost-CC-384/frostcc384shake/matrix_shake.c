#include <stdint.h>
#include <string.h>

#include "fips202.h"
#include "api.h"
#include "common.h"
#include "params.h"

extern uint16_t xs(const uint16_t *s, const uint16_t *a_row);

static void frost_shake_expand_row(uint16_t *row, uint16_t row_index,
                                   const uint8_t *seed_A)
{
    uint8_t input[2 + BYTES_SEED_A];
    uint16_t le_index = UINT16_TO_LE(row_index);
    memcpy(input, &le_index, sizeof(le_index));
    memcpy(input + 2, seed_A, BYTES_SEED_A);
    shake128((uint8_t *)row, 2u * PARAMS_N, input, sizeof(input));
    for (size_t j = 0; j < PARAMS_N; j++) row[j] = LE_TO_UINT16(row[j]);
}

int frodo_mul_add_as_plus_e(uint16_t *out, const uint16_t *s,
                            const uint8_t *seed_A)
{
    uint16_t a_row[PARAMS_N];
    for (size_t i = 0; i < PARAMS_N; i++) {
        frost_shake_expand_row(a_row, (uint16_t)i, seed_A);
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
    for (size_t j = 0; j < PARAMS_N; j++) {
        frost_shake_expand_row(a_row, (uint16_t)j, seed_A);
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
