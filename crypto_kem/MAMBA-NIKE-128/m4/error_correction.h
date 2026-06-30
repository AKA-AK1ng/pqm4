#ifndef ERROR_CORR_M4_H
#define ERROR_CORR_M4_H
#include "poly.h"
void helprec(poly *c, const poly *v, const unsigned char *seed, unsigned char nonce);
void rec(unsigned char *key, const poly *v, const poly *c);
#endif
