#ifndef TOOM_STATIC_H
#define TOOM_STATIC_H

#include <stdint.h>

#define TOOM4_WS_WORDS_1024 19422
#define TOOM4_WS_WORDS_2048 49096
#define TOOM4_WS_WORDS_MAX TOOM4_WS_WORDS_2048

void toom4_mul_static(int64_t *r, const int64_t *a, const int64_t *b, int n);
void toom4_mul_ws(int64_t *r,
                  const int64_t *a,
                  const int64_t *b,
                  int n,
                  int64_t *workspace,
                  int workspace_words);

#endif
