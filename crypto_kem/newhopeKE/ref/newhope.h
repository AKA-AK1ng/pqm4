#ifndef NEWHOPE_H
#define NEWHOPE_H

#include "poly.h"

void newhope_keygen(unsigned char *send, poly *sk);
void newhope_sharedb(unsigned char *sharedkey, unsigned char *send, const unsigned char *received);
void newhope_shareda(unsigned char *sharedkey, const poly *ska, const unsigned char *received);

#endif
