#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H

#include "inttypes.h"
#include "params.h"
#include "crypto_stream_chacha20.h"
#include "poly.h"

void helprec(poly *c, const poly *v, const unsigned char *seed, unsigned char nonce);
void rec(unsigned char *key, const poly *v, const poly *c);

#endif
