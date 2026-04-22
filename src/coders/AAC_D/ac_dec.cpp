#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "coder.h"

DecodingEnvironmentPtr arideco_create_decoding_environment()
{
  DecodingEnvironmentPtr dep;
  if ((dep = (DecodingEnvironmentPtr) calloc(1, sizeof(DecodingEnvironment))) == NULL)
    printf("arideco_create_decoding_environment: dep");
  return dep;
}

void arideco_delete_decoding_environment(DecodingEnvironmentPtr dep)
{
  free(dep);
}

void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *cpixcode,
                            int /*firstbyte*/, int *cpixcode_len)
{
  unsigned int bbit = 0;

  dep->Dcodestrm = cpixcode;
  dep->Dcodestrm_len = (unsigned int*)cpixcode_len;
  dep->Dvalue = 0;
  for (int i = 1; i <= BITS_IN_REGISTER; i++)
  {
    Get1Bit(dep->Dcodestrm, *dep->Dcodestrm_len, bbit);
    dep->Dvalue = 2 * dep->Dvalue + bbit;
  }
  dep->Dlow = 0;
  dep->Drange = HALF - 2;
}

static inline uint64_t ac_decode_target(DecodingEnvironmentPtr dep, uint64_t total)
{
  uint64_t value = dep->Dvalue;
  uint64_t low   = dep->Dlow;
  uint64_t range = dep->Drange;
  return ((value - low + 1) * total - 1) / range;
}

static void ac_decode_range(DecodingEnvironmentPtr dep,
                            uint64_t cum_lo,
                            uint64_t cum_hi,
                            uint64_t total)
{
  uint64_t low   = dep->Dlow;
  uint64_t value = dep->Dvalue;
  uint64_t range = dep->Drange;

  uint64_t r_lo = (range * cum_lo) / total;
  uint64_t r_hi = (range * cum_hi) / total;
  low  += r_lo;
  range = r_hi - r_lo;
  if (range == 0) range = 1;

  unsigned int bbit;
  while (range < QUARTER)
  {
    if (low >= HALF)
    {
      low   -= HALF;
      value -= HALF;
    }
    else if (low < QUARTER)
    {
      // Верхняя четверть не определена; ничего не вычитать
    }
    else
    {
      low   -= QUARTER;
      value -= QUARTER;
    }
    range <<= 1;
    low   <<= 1;
    Get1Bit(dep->Dcodestrm, *dep->Dcodestrm_len, bbit);
    value = (value << 1) | bbit;
  }

  dep->Dlow   = (unsigned int)low;
  dep->Dvalue = (unsigned int)value;
  dep->Drange = (unsigned int)range;
}


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

unsigned int biari_decode_symbol(DecodingEnvironmentPtr dep, BiContextTypePtr ctx) {
  unsigned int symbol = 0;

  if (ctx->t == 0) {
    // t = 0
    uint64_t target = ac_decode_target(dep, ALPHABET_SIZE);
    if (target >= ALPHABET_SIZE) target = ALPHABET_SIZE - 1;
    symbol = (unsigned int)target;
    ac_decode_range(dep, symbol, symbol + 1, ALPHABET_SIZE);
  } else {
    // t > 0
    uint64_t total  = 2ULL * ctx->t;
    uint64_t target = ac_decode_target(dep, total);
    if (target >= total) {
      target = total - 1;
    }
    uint64_t seen_total = total - ctx->M_t;

    if (target < seen_total) {
      // Символ уже встречался
      uint64_t cum = 0;
      int s = 0;
      for (int a = 0; a < ALPHABET_SIZE; a++) {
        if (ctx->tau[a] == 0) {
          continue;
        }
        uint64_t f = 2ULL * ctx->tau[a] - 1;
        if (target < cum + f) {
          s = a;
          break;
        }
        cum += f;
      }
      symbol = (unsigned int)s;
      uint64_t f_s = 2ULL * ctx->tau[s] - 1;
      ac_decode_range(dep, cum, cum + f_s, total);
    } else {
      // Символ не встречался
      // esc
      ac_decode_range(dep, seen_total, total, total);

      // Сам символ
      uint64_t unseen = ALPHABET_SIZE - ctx->M_t;
      uint64_t target2 = ac_decode_target(dep, unseen);
      if (target2 >= unseen) {
        target2 = unseen - 1;
      }

      uint64_t r = 0;
      int s = 0;
      for (int a = 0; a < ALPHABET_SIZE; a++)
      {
        if (ctx->tau[a] != 0) {
          continue;
        }
        if (target2 == r) {
          s = a;
          break;
        }
        r++;
      }
      symbol = (unsigned int)s;
      ac_decode_range(dep, r, r + 1, unseen);
    }
  }

  // Обновление контекста
  if (ctx->tau[symbol] == 0) {
    ctx->M_t++;
  }
  ctx->tau[symbol]++;
  ctx->t++;

  return symbol;
}


void ac_decode_buffer(const unsigned char* code_buffer,
                      unsigned int output_len,
                      unsigned char* output) {
  BiContextType ctx;
  biari_init_context(&ctx);

  int code_len = 0;
  DecodingEnvironmentPtr dep = arideco_create_decoding_environment();
  arideco_start_decoding(dep, (unsigned char*)code_buffer, 0, &code_len);

  for (unsigned int i = 0; i < output_len; i++) {
    output[i] = (unsigned char)biari_decode_symbol(dep, &ctx);
  }

  arideco_delete_decoding_environment(dep);
}
