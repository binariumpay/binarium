/* File swifft.h */
#ifndef SWIFFT_H
#define SWIFFT_H

#include <stdint.h>

#define SWIFFT_N 64       // Vector length. Must be a power of 2.
#define SWIFFT_P 257      // Modulus. Integer arithmetic is performed modulo P. 
#define SWIFFT_M 16       // Number of vectors per block. (= Mstate+Mdata)
#define SWIFFT_W 8        // Number of coordinates in vectorized register
#define Mstate 9       // (N/W)+1: Hash function State size, in blocks of N bits
#define Mdata  7       // (N/W)-1: Hash function Data size,  in blocks of N bits
#define OMEGA 42   // (2N)th primitive root of unity modulo P

#define ALIGN __attribute__ ((aligned (16)))

typedef int16_t SWIFFT_Z; // 16 bit integer

typedef SWIFFT_Z SWIFFT_ZW __attribute__ ((vector_size (SWIFFT_W*sizeof(SWIFFT_Z)))); // Z^W
typedef SWIFFT_ZW SWIFFT_ZN[SWIFFT_N/SWIFFT_W]; // Z^N, in blocks of size W

typedef uint8_t BitsW;    // block of W=8 input bits
typedef BitsW BitsN[SWIFFT_N/SWIFFT_W]; // N input bits, in blocks of size W
typedef BitsN HashState[Mstate];
typedef BitsN SWIFFT_HashData[Mdata]; 
typedef struct {
  SWIFFT_ZN keyval[SWIFFT_M] ALIGN;
  SWIFFT_ZN keysum ALIGN;
} HashKey;

#ifdef __cplusplus
extern "C" {
#endif

void SwiFFT_setupTables();
void SwiFFT_setupKey(HashKey key);
void SwiFFT(const HashKey key, HashState state, const SWIFFT_HashData data);

void SwiFFT_initState(HashState st);
void SwiFFT_readKey(const unsigned char * _pKeySourceData, HashKey *key);
void SwiFFT_readData(SWIFFT_HashData data, const unsigned char * _pDataSource);

//---Printing information.---------------------------
void SwiFFT_printState(HashState st);
void SwiFFT_printKey(HashKey k);
void SwiFFT_printData(SWIFFT_HashData d);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

extern SWIFFT_ZN mulTable       ALIGN;
extern SWIFFT_ZW fftTable[1<<SWIFFT_W] ALIGN;

#endif /* SWIFFT_H */
