#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "coder.h"

EncodingEnvironmentPtr arienco_create_encoding_environment()
{
  EncodingEnvironmentPtr eep;
  if ((eep = (EncodingEnvironmentPtr) calloc(1, sizeof(EncodingEnvironment))) == NULL)
    printf("arienco_create_encoding_environment: eep");
  return eep;
}

void arienco_delete_encoding_environment(EncodingEnvironmentPtr eep)
{
  if (eep != NULL) free(eep);
}

void arienco_start_encoding(EncodingEnvironmentPtr eep,
                            unsigned char *code_buffer,
                            int *code_len)
{
  eep->Elow = 0;
  eep->Erange = HALF - 2;
  eep->Ebits_to_follow = 0;
  eep->Ecodestrm = code_buffer;
  eep->Ecodestrm_len = (unsigned int*)code_len;
}

void arienco_done_encoding(EncodingEnvironmentPtr eep)
{
  eep->Ebits_to_follow++;
  if (eep->Elow < QUARTER)
  {
    put_one_bit_0_plus_outstanding;
  }
  else
  {
    put_one_bit_1_plus_outstanding;
  }
  *eep->Ecodestrm_len = (*eep->Ecodestrm_len + 7) & ~7;
}

static void ac_encode_range(EncodingEnvironmentPtr eep,
                            uint64_t cum_lo,
                            uint64_t cum_hi,
                            uint64_t total)
{
  uint64_t range = eep->Erange;
  uint64_t low   = eep->Elow;

  uint64_t r_lo = (range * cum_lo) / total;
  uint64_t r_hi = (range * cum_hi) / total;
  low  += r_lo;
  range = r_hi - r_lo;
  if (range == 0) range = 1;

  while (range < QUARTER)
  {
    if (low >= HALF)
    {
      put_one_bit_1_plus_outstanding;
      low -= HALF;
    }
    else if (low < QUARTER)
    {
      put_one_bit_0_plus_outstanding;
    }
    else
    {
      eep->Ebits_to_follow++;
      low -= QUARTER;
    }
    low   <<= 1;
    range <<= 1;
  }

  eep->Erange = (unsigned int)range;
  eep->Elow   = (unsigned int)low;
}


/*
 * Оценка вероятности D:
 *   p(esc)             = M_t / (2t)               (вероятность escape-символа)
 *   p(a) seen          = (2 * tau(a) - 1) / (2t)  (вероятность символа a, если он уже встречался)
 *   p(a) unseen, t>0   = M_t / (2t * (M - M_t))   (вероятность символа a, если он не встречался и t > 0)
 *   p(a) unseen, t=0   = 1 / M                    (вероятность символа a, когда t = 0 - самое начало)
 */

// Кумулятивная сумма уже встреченных символов до символа s в алфавите
static uint64_t seen_cum_below(BiContextTypePtr ctx, int s) {
  uint64_t cum = 0;
  for (int a = 0; a < s; a++) {
    if (ctx->tau[a] > 0) {
      cum += 2 * (uint64_t)ctx->tau[a] - 1;
    }
  }
  return cum;
}

// Количество не встреченных символов до символа s в алфавите
static uint64_t unseen_count_below(BiContextTypePtr ctx, int s) {
  uint64_t count = 0;
  for (int a = 0; a < s; a++) {
    if (ctx->tau[a] == 0) {
      count++;
    }
  }
  return count;
}

void biari_encode_symbol(EncodingEnvironmentPtr eep, int symbol, BiContextTypePtr ctx) {
  if (ctx->t == 0) {
    // t = 0
    ac_encode_range(eep, (uint64_t)symbol, (uint64_t)symbol + 1, ALPHABET_SIZE);
  } else if (ctx->tau[symbol] > 0) {
    // Символ уже встречался
    uint64_t total = 2ULL * ctx->t;
    uint64_t cum_lo = seen_cum_below(ctx, symbol);
    uint64_t f = 2ULL * ctx->tau[symbol] - 1;
    ac_encode_range(eep, cum_lo, cum_lo + f, total);
  }else {
    // Символ не встречался
    // esc
    uint64_t total = 2ULL * ctx->t;
    uint64_t esc_lo = total - ctx->M_t;
    ac_encode_range(eep, esc_lo, total, total);

    // Сам символ
    uint64_t unseen = ALPHABET_SIZE - ctx->M_t;
    uint64_t count_lo = unseen_count_below(ctx, symbol);
    ac_encode_range(eep, count_lo, count_lo + 1, unseen);
  }

  // Обновление контекста
  if (ctx->tau[symbol] == 0) {
    ctx->M_t++;
  }
  ctx->tau[symbol]++;
  ctx->t++;
}


unsigned int ac_encode_buffer(const unsigned char* input,
                              unsigned int input_len,
                              unsigned char* code_buffer) {
  BiContextType ctx;
  biari_init_context(&ctx);

  int code_len = 0;
  EncodingEnvironmentPtr eep = arienco_create_encoding_environment();
  arienco_start_encoding(eep, code_buffer, &code_len);

  for (unsigned int i = 0; i < input_len; i++) {
    biari_encode_symbol(eep, input[i], &ctx);
  }

  arienco_done_encoding(eep);
  arienco_delete_encoding_environment(eep);

  return (unsigned int)code_len;
}
