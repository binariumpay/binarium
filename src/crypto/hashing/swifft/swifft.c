/* SwiFFT hash function. */

#include "swifft.h"
#include <stdio.h>

#define INLINE inline extern __attribute__((always_inline))
    
INLINE SWIFFT_ZW shift(SWIFFT_ZW x, int s) // x*2^s mod (P=257)
{ return ((x << s) & 255) - (x >> (8-s)); }

// Reduces mm mod P=257 to the range {-127,383}
INLINE SWIFFT_ZW qReduce(SWIFFT_ZW x) // (x mod 256) - floor(x/256) 
{ return (x & 255) - (x >> 8); } 

// Reduces mm mod P=257 to the range {0,..,(P-1)=256}
INLINE SWIFFT_ZW modP(SWIFFT_ZW mm){
  SWIFFT_ZW tmp = qReduce(qReduce(mm)); 
  return tmp ^ ((tmp == -1) & (-257)); 
}

#define AddSub(a, b) { SWIFFT_ZW tmp = b; b = a - b; a = a + tmp; }

INLINE void FFT(const BitsN t, SWIFFT_ZN u) {
  int i;
  SWIFFT_ZN v;
    
  for (i=0; i<SWIFFT_N/SWIFFT_W; i++) v[i] = fftTable[t[i]] * mulTable[i];

  AddSub(v[0],v[1]);
  AddSub(v[2],v[3]);
  AddSub(v[4],v[5]);
  AddSub(v[6],v[7]);
    
  v[2] = qReduce(v[2]);  AddSub(v[0],v[2]);
  v[3] = shift(v[3],4);  AddSub(v[1],v[3]);
  v[6] = qReduce(v[6]);  AddSub(v[4],v[6]);
  v[7] = shift(v[7],4);  AddSub(v[5],v[7]);

  v[4] = qReduce(v[4]);
  v[5] = shift(v[5],2);
  v[6] = shift(v[6],4);
  v[7] = shift(v[7],6);
  
  u[0] = v[0]+v[4];
  u[4] = v[0]-v[4];
  u[1] = v[1]+v[5];
  u[5] = v[1]-v[5];
  u[2] = v[2]+v[6];
  u[6] = v[2]-v[6];
  u[3] = v[3]+v[7];
  u[7] = v[3]-v[7];
}

#ifdef __cplusplus
extern "C" {
#endif

// ZW parameters must be aligned (16 bytes)
void SwiFFT(const HashKey key, HashState hash, const SWIFFT_HashData data) {
  SWIFFT_ZN fft;
  SWIFFT_ZN out;
  for (int j=0; j<SWIFFT_N/SWIFFT_W; j++) out[j] = key.keysum[j];
  for (int i=0; i<SWIFFT_M; i++) {
    if (i < Mstate) FFT(hash[i],  fft);
    else        FFT(data[i-Mstate],fft);
    for (int j=0; j<SWIFFT_N/SWIFFT_W; j++) 
      out[j] += qReduce((qReduce(fft[j]) - ((SWIFFT_P-1)/2)) * key.keyval[i][j]);    
  }
  SWIFFT_ZW overflow;
  overflow = overflow ^ overflow;
  for (int j=0; j<SWIFFT_N/SWIFFT_W; j++) {
    out[j] = modP(out[j]);
    for (int i=0; i<SWIFFT_W; i++)
      hash[j][i] = (BitsW) out[j][i];
    overflow = (overflow << 1) | (out[j] >> 8);
  }
  for (int i=0; i<SWIFFT_W; i++)
    hash[SWIFFT_N/SWIFFT_W][i] = (BitsW) overflow[i];
}



void SwiFFT_initState(HashState st) {
  for (int i=0; i<Mstate; i++)
    for (int j=0; j<SWIFFT_N/SWIFFT_W; j++)
      st[i][j]=0;
}

void SwiFFT_readKey(const unsigned char * _pKeySourceData, HashKey *key) {  
  int k = 0;

  //fprintf(stdout, "swifft.c : SwiFFT_readKey () : Function start.\n" );
  for (int i=0; i<SWIFFT_M; i++)
    for (int j=0; j<SWIFFT_N; j++){
      key->keyval[i][j/SWIFFT_W][j%SWIFFT_W] = _pKeySourceData [ k ];
      //fprintf(stdout, "%i : ", _pKeySourceData [ k ] );
      //fprintf(stdout, "%3d ; ", key->keyval[i][j/SWIFFT_W][j%SWIFFT_W]);
      k = k + 1;
    } //-for
  //fprintf(stdout, "\nswifft.c : SwiFFT_readKey () : %i.\n", k );
  SwiFFT_setupKey(*key);
}

void SwiFFT_readData(SWIFFT_HashData data, const unsigned char * _pDataSource) {
  int k = 0;

  for (int i=0; i<Mdata; i++) 
    for (int j=0; j<SWIFFT_N/SWIFFT_W; j++) {
      data[i][j] = _pDataSource [ k ];
      k = k + 1;
    } //-for
}

//---Printing information.---------------------------
void SwiFFT_printState(HashState st) {
  fprintf(stdout, "SwiFFT : state :\n");

  for (int i=0; i<Mstate; i++) {
    for (int j=0; j<SWIFFT_N/SWIFFT_W; j=j+2) {
      fprintf(stdout, "%02x%02x ", st[i][j], st[i][j+1]);
    }
    fprintf(stdout, "\n");
  }
}

void SwiFFT_printKey(HashKey k) {
  fprintf(stdout, "SwiFFT : key :\n");

  for (int i=0; i<SWIFFT_M; i++) {
    for (int j=0; j<SWIFFT_N; j++){
      fprintf(stdout, "%3d ", k.keyval[i][j/SWIFFT_W][j%SWIFFT_W]);
    }
    fprintf(stdout, "\n");
  }
}

void SwiFFT_printData(SWIFFT_HashData d) {
  fprintf(stdout, "SwiFFT : data :\n");

  for (int i=0; i<Mdata; i++) {
    for (int j=0; j<SWIFFT_N/SWIFFT_W; j=j+2) {
      fprintf(stdout, "%02x%02x ", d[i][j], d[i][j+1]);
    }
    fprintf(stdout, "\n");
  }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */