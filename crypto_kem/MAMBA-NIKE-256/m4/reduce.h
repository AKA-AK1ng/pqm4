#ifndef REDUCE_M4_H
#define REDUCE_M4_H
#include <stdint.h>
#include "params.h"
#define montgomery_reduce(a) ((uint16_t)((uint32_t)(a) & (PARAM_Q - 1)))
#define barrett_reduce(a)    ((uint16_t)((uint32_t)(a) & (PARAM_Q - 1)))
#endif
