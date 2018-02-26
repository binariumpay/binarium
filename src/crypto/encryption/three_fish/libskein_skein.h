/*
** Copyright (c) Alexis Megas.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from skein without specific prior written permission.
**
** LIBSKEIN IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** LIBSKEIN, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBSKEIN_SKEIN_H
#define LIBSKEIN_SKEIN_H

extern "C"
{
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
}

#include <algorithm>
#include <iostream>
#include <new>

#ifdef __cplusplus
extern "C"
{
#endif

void libskein_threefish_decrypt
(char *D, /*
	  ** Output storage. The size of the
	  ** container must be identical to
	  ** the size of C.
	  */
 const char *K, // Must be 32, 64, or 128 bytes.
 const char *T, // Must be 16 bytes.
 const char *C, // The ciphertext.
 const size_t C_size, // The size of the ciphertext.
 const size_t block_size); /*
			   ** The block size in
			   ** bits. Must be 256,
			   ** 512, or 1024.
			   */
void libskein_threefish_encrypt
(char *E, /*
	  ** Output storage. The size of the
	  ** container must be identical to
	  ** the size of P.
	  */
 const char *K, // Must be 32, 64, or 128 bytes.
 const char *T, // Must be 16 bytes.
 const char *P, // The plaintext.
 const size_t P_size, // The size of the plaintext.
 const size_t block_size); /*
			   ** The block size in
			   ** bits. Must be 256,
			   ** 512, or 1024.
			   */

#ifdef __cplusplus
}
#endif
#endif
