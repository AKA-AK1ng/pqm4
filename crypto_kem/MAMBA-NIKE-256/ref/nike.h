#ifndef NIKE_H
#define NIKE_H

#include "poly.h"
#include "crypto_stream_chacha20.h"
#include "error_correction.h"

void nike_keygen(unsigned char *send, poly *sk);
void nike_sharedb(unsigned char *sharedkey, unsigned char *send, const unsigned char *received);
void nike_shareda(unsigned char *sharedkey, const poly *ska, const unsigned char *pk, const unsigned char *received);

#endif
