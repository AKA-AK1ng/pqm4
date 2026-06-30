/*
 * toom_static.c — static-workspace Toom-Cook-4 multiplication over Z
 *
 * Drop-in experimental replacement for dynamic toom.c.
 * Main goal: remove malloc/calloc/free from the recursive Toom path.
 *
 * WARNING:
 *   - This is still a reference-style int64_t Toom implementation.
 *   - For n=1024 it uses about 19422 int64_t words of workspace (~155 KB).
 *   - For n=2048 it uses about 49096 int64_t words of workspace (~393 KB).
 *     Keep the workspace in .bss/global memory, not stack.
 *   - This is not yet the final M4-friendly polynomial multiplication core.
 */

#include "toom_static.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define TOOM4_CUTOFF 48
#define TOOM4_MAX_N 2048

/* Workspace words for n=1024 with TOOM4_CUTOFF=48:
 * W(1024) = (58*256-22) + (58*64-22) + (58*16-22) = 19422
 * W(2048) = (58*512-22) + W(1024) = 49096
 */
#define TOOM4_WS_WORDS_1024 19422
#define TOOM4_WS_WORDS_2048 49096
#define TOOM4_WS_WORDS_MAX TOOM4_WS_WORDS_2048

static int64_t toom4_ws_global[TOOM4_WS_WORDS_MAX];

static void i64_add(int64_t *r, const int64_t *a, const int64_t *b, int n)
{ for (int i = 0; i < n; i++) r[i] = a[i] + b[i]; }

static void i64_sub(int64_t *r, const int64_t *a, const int64_t *b, int n)
{ for (int i = 0; i < n; i++) r[i] = a[i] - b[i]; }

static void i64_subfrom(int64_t *r, const int64_t *a, int n)
{ for (int i = 0; i < n; i++) r[i] -= a[i]; }

static void i64_shr(int64_t *r, int n, int shift)
{ for (int i = 0; i < n; i++) r[i] >>= shift; }

static void i64_mul_scalar(int64_t *r, int n, int64_t s)
{ for (int i = 0; i < n; i++) r[i] *= s; }

static void i64_copy(int64_t *r, const int64_t *a, int n)
{ memcpy(r, a, (size_t)n * sizeof(int64_t)); }

static void schoolbook_mul(int64_t *r, const int64_t *a, const int64_t *b, int n)
{
  memset(r, 0, (size_t)(2 * n - 1) * sizeof(int64_t));
  for (int i = 0; i < n; i++) {
    const int64_t ai = a[i];
    for (int j = 0; j < n; j++) {
      r[i + j] += ai * b[j];
    }
  }
}

static int toom4_workspace_words_needed(int n)
{
  int total = 0;
  while (n > TOOM4_CUTOFF) {
    const int k = (n + 3) / 4;
    const int lp = 2 * k - 1;
    total += 14 * k + 22 * lp;
    n = k;
  }
  return total;
}

static int64_t *ws_take(int64_t **ws, int n)
{
  int64_t *r = *ws;
  *ws += n;
  memset(r, 0, (size_t)n * sizeof(int64_t));
  return r;
}

static void toom4_mul_ws_inner(int64_t *r,
                               const int64_t *a,
                               const int64_t *b,
                               int n,
                               int64_t *ws)
{
  if (n <= TOOM4_CUTOFF) {
    schoolbook_mul(r, a, b, n);
    return;
  }

  const int k = (n + 3) / 4;
  const int lp = 2 * k - 1;

  const int64_t *a0 = a;
  const int64_t *a1 = n > k     ? a + k     : NULL;
  const int64_t *a2 = n > 2 * k ? a + 2 * k : NULL;
  const int64_t *a3 = n > 3 * k ? a + 3 * k : NULL;

  const int l0 = n < k ? n : k;
  const int l1 = (n < 2 * k) ? (n > k     ? n - k     : 0) : k;
  const int l2 = (n < 3 * k) ? (n > 2 * k ? n - 2 * k : 0) : k;
  const int l3 = n > 3 * k ? n - 3 * k : 0;

  const int64_t *b0 = b;
  const int64_t *b1 = n > k     ? b + k     : NULL;
  const int64_t *b2 = n > 2 * k ? b + 2 * k : NULL;
  const int64_t *b3 = n > 3 * k ? b + 3 * k : NULL;

  int64_t *av[7], *bv[7], *p[7], *C[7];
  int64_t *cur = ws;

  for (int i = 0; i < 7; i++) av[i] = ws_take(&cur, k);
  for (int i = 0; i < 7; i++) bv[i] = ws_take(&cur, k);
  for (int i = 0; i < 7; i++) p[i]  = ws_take(&cur, lp);
  for (int i = 0; i < 7; i++) C[i]  = ws_take(&cur, lp);

  int64_t *t1 = ws_take(&cur, lp);
  int64_t *t2 = ws_take(&cur, lp);
  int64_t *t3 = ws_take(&cur, lp);
  int64_t *t4 = ws_take(&cur, lp);
  int64_t *fh = ws_take(&cur, lp);
  int64_t *rd = ws_take(&cur, lp);
  int64_t *re = ws_take(&cur, lp);
  int64_t *tt = ws_take(&cur, lp);

  int64_t *child_ws = cur;

  for (int i = 0; i < k; i++) {
    const int64_t a0v = i < l0 ? a0[i] : 0;
    const int64_t a1v = i < l1 ? a1[i] : 0;
    const int64_t a2v = i < l2 ? a2[i] : 0;
    const int64_t a3v = i < l3 ? a3[i] : 0;
    const int64_t b0v = i < l0 ? b0[i] : 0;
    const int64_t b1v = i < l1 ? b1[i] : 0;
    const int64_t b2v = i < l2 ? b2[i] : 0;
    const int64_t b3v = i < l3 ? b3[i] : 0;

    av[0][i] = a0v;
    av[1][i] = a3v;
    av[2][i] = a0v + a1v + a2v + a3v;
    av[3][i] = a0v - a1v + a2v - a3v;
    av[4][i] = a0v + 2 * a1v + 4 * a2v + 8 * a3v;
    av[5][i] = a0v - 2 * a1v + 4 * a2v - 8 * a3v;
    av[6][i] = 8 * a0v + 4 * a1v + 2 * a2v + a3v;

    bv[0][i] = b0v;
    bv[1][i] = b3v;
    bv[2][i] = b0v + b1v + b2v + b3v;
    bv[3][i] = b0v - b1v + b2v - b3v;
    bv[4][i] = b0v + 2 * b1v + 4 * b2v + 8 * b3v;
    bv[5][i] = b0v - 2 * b1v + 4 * b2v - 8 * b3v;
    bv[6][i] = 8 * b0v + 4 * b1v + 2 * b2v + b3v;
  }

  for (int i = 0; i < 7; i++) {
    toom4_mul_ws_inner(p[i], av[i], bv[i], k, child_ws);
  }

  i64_copy(C[0], p[0], lp);
  i64_copy(C[6], p[1], lp);

  i64_add(t1, p[2], p[3], lp);  i64_shr(t1, lp, 1);
  i64_sub(t2, p[2], p[3], lp);  i64_shr(t2, lp, 1);

  i64_add(t3, p[4], p[5], lp);  i64_shr(t3, lp, 1);
  i64_sub(t4, p[4], p[5], lp);  i64_shr(t4, lp, 2);

  int64_t *g1 = t1;
  i64_subfrom(g1, C[0], lp);
  i64_subfrom(g1, C[6], lp);

  int64_t *h1 = t2;

  int64_t *g2 = t3;
  i64_subfrom(g2, C[0], lp);
  i64_copy(tt, C[6], lp);
  i64_mul_scalar(tt, lp, 64);
  i64_subfrom(g2, tt, lp);

  int64_t *h2 = t4;

  i64_copy(C[4], g2, lp);
  i64_copy(tt, g1, lp);
  i64_mul_scalar(tt, lp, 4);
  i64_subfrom(C[4], tt, lp);
  for (int i = 0; i < lp; i++) C[4][i] /= 12;

  i64_copy(C[2], g1, lp);
  i64_subfrom(C[2], C[4], lp);

  i64_copy(fh, p[6], lp);
  i64_copy(tt, C[0], lp);
  i64_mul_scalar(tt, lp, 64);
  i64_subfrom(fh, tt, lp);
  i64_subfrom(fh, C[6], lp);

  i64_copy(rd, h1, lp);
  i64_mul_scalar(rd, lp, 16);
  i64_subfrom(rd, h2, lp);

  i64_copy(re, fh, lp);
  i64_copy(tt, C[2], lp);
  i64_mul_scalar(tt, lp, 16);
  i64_subfrom(re, tt, lp);
  i64_copy(tt, C[4], lp);
  i64_mul_scalar(tt, lp, 4);
  i64_subfrom(re, tt, lp);
  i64_copy(tt, h1, lp);
  i64_mul_scalar(tt, lp, 2);
  i64_subfrom(re, tt, lp);

  i64_copy(C[3], rd, lp);
  i64_mul_scalar(C[3], lp, 2);
  i64_subfrom(C[3], re, lp);
  for (int i = 0; i < lp; i++) C[3][i] /= 18;

  i64_copy(C[1], rd, lp);
  i64_copy(tt, C[3], lp);
  i64_mul_scalar(tt, lp, 12);
  i64_subfrom(C[1], tt, lp);
  for (int i = 0; i < lp; i++) C[1][i] /= 15;

  i64_copy(C[5], h1, lp);
  i64_subfrom(C[5], C[1], lp);
  i64_subfrom(C[5], C[3], lp);

  const int rlen = 2 * n - 1;
  memset(r, 0, (size_t)rlen * sizeof(int64_t));
  for (int i = 0; i < lp; i++) {
    if (i           < rlen) r[i]         += C[0][i];
    if (i + k       < rlen) r[i + k]     += C[1][i];
    if (i + 2 * k   < rlen) r[i + 2 * k] += C[2][i];
    if (i + 3 * k   < rlen) r[i + 3 * k] += C[3][i];
    if (i + 4 * k   < rlen) r[i + 4 * k] += C[4][i];
    if (i + 5 * k   < rlen) r[i + 5 * k] += C[5][i];
    if (i + 6 * k   < rlen) r[i + 6 * k] += C[6][i];
  }
}

void toom4_mul_ws(int64_t *r,
                  const int64_t *a,
                  const int64_t *b,
                  int n,
                  int64_t *workspace,
                  int workspace_words)
{
  const int needed = toom4_workspace_words_needed(n);
  if (workspace_words < needed) {
    /* Fail closed: deterministic zero output instead of memory corruption. */
    memset(r, 0, (size_t)(2 * n - 1) * sizeof(int64_t));
    return;
  }
  toom4_mul_ws_inner(r, a, b, n, workspace);
}

void toom4_mul_static(int64_t *r, const int64_t *a, const int64_t *b, int n)
{
  int workspace_words = TOOM4_WS_WORDS_MAX;

  if (n <= 1024)
    workspace_words = TOOM4_WS_WORDS_1024;

  toom4_mul_ws(r, a, b, n, toom4_ws_global, workspace_words);
}

/* Optional compatibility alias: enable if you want this file to replace
 * the old dynamic toom.c without changing call sites.
 */
#ifdef TOOM4_STATIC_REPLACE_ORIGINAL
void toom4_mul(int64_t *r, const int64_t *a, const int64_t *b, int n)
{
  toom4_mul_static(r, a, b, n);
}
#endif
