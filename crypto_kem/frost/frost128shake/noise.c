/********************************************************************************************
* FrodoKEM: Learning with Errors Key Encapsulation
*
* Abstract: noise sampling functions
*********************************************************************************************/

#include <stdint.h>

#include "api.h"
#include "common.h"
#include "params.h"

static uint16_t CDF_TABLE[CDF_TABLE_LEN] = CDF_TABLE_DATA;

void frodo_sample_n(uint16_t *s, size_t n)
{
    size_t i;
    unsigned int j;

    for (i = 0; i < n; ++i) {
        uint16_t sample = 0;
        uint16_t prnd = s[i] >> 1;
        uint16_t sign = s[i] & 0x1;

        for (j = 0; j < (unsigned int)(CDF_TABLE_LEN - 1); j++) {
            sample += (uint16_t)(CDF_TABLE[j] - prnd) >> 15;
        }
        s[i] = ((-sign) ^ sample) + sign;
    }
}