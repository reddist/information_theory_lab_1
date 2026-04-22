#ifndef AAC_D_CODER_H
#define AAC_D_CODER_H

#include "stream.h"
#include <string.h>

#define BITS_IN_REGISTER 30
#define TOP_VALUE (((long) 1 << BITS_IN_REGISTER) - 1)
#define QUARTER (TOP_VALUE / 4 + 1)
#define FIRST_QTR (TOP_VALUE / 4 + 1)
#define HALF      (2 * FIRST_QTR)
#define THIRD_QTR (3 * FIRST_QTR)

#define ALPHABET_SIZE 256
#define MAX_BITS_IN_SERIE 25
///   25 MAXIMUM
  #define PutZeros(nbits) {\
    unsigned int iii;\
    for(iii = 0; iii<(nbits); iii++ )\
    {\
      Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 0);\
    }\
      }

  #define put_one_bit_1_plus_outstanding { \
                                          Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 1); \
                                          PutZeros(eep->Ebits_to_follow);\
                                          eep->Ebits_to_follow = 0;\
                                         }

  #define PutLongOnes(nbits) {\
      unsigned int i1=0xFFFFFFFF;\
      int bits1;\
      int main = (nbits)/MAX_BITS_IN_SERIE;\
      int tail = (nbits)%MAX_BITS_IN_SERIE;\
      for(bits1 = 0; bits1<main; bits1++ ) {\
        PutBits(eep->Ecodestrm, *eep->Ecodestrm_len,i1,MAX_BITS_IN_SERIE);\
      }\
      PutBits(eep->Ecodestrm, *eep->Ecodestrm_len,i1,tail);\
    }

  #define put_one_bit_0_plus_outstanding { \
                                          Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 0); \
                                          if( eep->Ebits_to_follow > MAX_BITS_IN_SERIE ) \
                                          { \
                                            PutLongOnes(eep->Ebits_to_follow);\
                                          }\
                                          else \
                                          {\
                                            PutBitsOnes(eep->Ecodestrm, *eep->Ecodestrm_len, eep->Ebits_to_follow);\
                                          }\
                                          eep->Ebits_to_follow = 0;\
                                         }



// CONTEXT
typedef struct
{
  // Контекст для оценки вероятности D:
  //   tau[a] = количество вхождений символа a в уже закодированный префикс (τ_t(a))
  //   t      = количество закодированных символов (t)
  //   M_t    = количество уже встреченных различных символов (M_t)
  unsigned long tau[ALPHABET_SIZE];
  unsigned long t;
  unsigned long M_t;
} BiContextType;

typedef BiContextType *BiContextTypePtr;

static inline void biari_init_context(BiContextTypePtr ctx)
{
  memset(ctx->tau, 0, sizeof(ctx->tau));
  ctx->t   = 0;
  ctx->M_t = 0;
}



// ENCODER
typedef struct
{
  unsigned int  Elow, Erange, Ehigh;
  unsigned int  Ebits_to_follow;
  unsigned char          *Ecodestrm;
  unsigned int  *Ecodestrm_len;
} EncodingEnvironment;

typedef EncodingEnvironment *EncodingEnvironmentPtr;

// ac_enc.cpp
EncodingEnvironmentPtr arienco_create_encoding_environment();
void arienco_delete_encoding_environment(EncodingEnvironmentPtr eep);
void arienco_start_encoding(EncodingEnvironmentPtr eep,
                            unsigned char *code_buffer,
                            int *code_len );
void arienco_done_encoding(EncodingEnvironmentPtr eep);

void biari_encode_symbol(EncodingEnvironmentPtr eep, int symbol, BiContextTypePtr bi_ct);

unsigned int ac_encode_buffer(const unsigned char* input,
                              unsigned int input_len,
                              unsigned char* code_buffer);



// DECODER
typedef struct
{
  unsigned int    Dlow, Drange, Dhigh;
  unsigned int    Dvalue;
  unsigned int    Dbuffer;
  int             Dbits_to_go;
  unsigned char   *Dcodestrm;
  unsigned int    *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment *DecodingEnvironmentPtr;

// ac_dec.cpp
DecodingEnvironmentPtr arideco_create_decoding_environment();
void arideco_delete_decoding_environment(DecodingEnvironmentPtr dep);
void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *cpixcode,
                            int firstbyte, int *cpixcode_len );
int arideco_bits_read(DecodingEnvironmentPtr dep);
void arideco_done_decoding(DecodingEnvironmentPtr dep);

unsigned int biari_decode_symbol(DecodingEnvironmentPtr dep, BiContextTypePtr bi_ct);

void ac_decode_buffer(const unsigned char* code_buffer,
                      unsigned int output_len,
                      unsigned char* output);

#endif // !AAC_D_CODER_H
