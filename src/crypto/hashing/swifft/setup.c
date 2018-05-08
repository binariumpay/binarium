#include "swifft.h"

SWIFFT_ZN mulTable ALIGN;
SWIFFT_ZW fftTable[1<<SWIFFT_W] ALIGN;

long center(long x) { 
  // center(x) = x mod P, |center(x)| <= P/2
  long y = ((x % SWIFFT_P)+SWIFFT_P)%SWIFFT_P; 
  if (y > SWIFFT_P/2) y -= SWIFFT_P;
  return y; 
}

int rev(int i, int bound) { // bit reversal permutation
  int irev=0;
  for(i |= bound; i>1; i>>=1)
    irev = (irev<<1) | (i&1);
  return irev;
}

int bit(int x, int b) // extract b-th bit of x
{ return (x >> b) & 1; }

#ifdef __cplusplus
extern "C" {
#endif

void SwiFFT_setupTables() {
  long omega_pow[2*SWIFFT_N]; // the powers of omega
  omega_pow[0] = 1;
  for(int i = 1; i < 2*SWIFFT_N; i++) 
    omega_pow[i] = center(omega_pow[i-1] * OMEGA);  

  for (int i=0; i<SWIFFT_N/SWIFFT_W; i++)
    for (int j=0; j<SWIFFT_W; j++)
      mulTable[i][j] = omega_pow[rev(i,SWIFFT_N/SWIFFT_W)*(2*j+1)];

  for (int x=0; x<(1<<SWIFFT_W); x++)
    for (int j=0; j<SWIFFT_W; j++) {
      long tmp = 0;
      for (int k=0; k<SWIFFT_W; k++)
	tmp += omega_pow[((SWIFFT_N/SWIFFT_W)*(2*j+1)*rev(k,SWIFFT_W))%(2*SWIFFT_N)]*bit(x,k);
      fftTable[x][j] = center(tmp);
    }
}

void SwiFFT_setupKey(HashKey key) { // Reduce key mod P and initialize keysum 
  for (int j=0; j<SWIFFT_N; j++) {
    long sum = 0;
    for (int i=0; i<SWIFFT_M; i++) {
      key.keyval[i][j/SWIFFT_W][j%SWIFFT_W] = center(key.keyval[i][j/SWIFFT_W][j%SWIFFT_W]);
      sum += key.keyval[i][j/SWIFFT_W][j%SWIFFT_W];
    }
    key.keysum[j/SWIFFT_W][j%SWIFFT_W] = center(sum * ((SWIFFT_P-1)/2));
  }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */