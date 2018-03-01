// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uint256.h"

#include "utilstrencodings.h"

#include <stdio.h>
#include <string.h>

template <unsigned int BITS>
base_blob<BITS>::base_blob(const std::vector<unsigned char>& vch)
{
    assert(vch.size() == sizeof(data));
    memcpy(data, &vch[0], sizeof(data));
}

template <unsigned int BITS>
std::string base_blob<BITS>::GetHex() const
{
    char psz[sizeof(data) * 2 + 1];
    for (unsigned int i = 0; i < sizeof(data); i++)
        sprintf(psz + i * 2, "%02x", data[sizeof(data) - i - 1]);
    return std::string(psz, psz + sizeof(data) * 2);
}

template <unsigned int BITS>
void base_blob<BITS>::SetHex(const char* psz)
{
    memset(data, 0, sizeof(data));

    // skip leading spaces
    while (isspace(*psz))
        psz++;

    // skip 0x
    if (psz[0] == '0' && tolower(psz[1]) == 'x')
        psz += 2;

    // hex string to uint
    const char* pbegin = psz;
    while (::HexDigit(*psz) != -1)
        psz++;
    psz--;
    unsigned char* p1 = (unsigned char*)data;
    unsigned char* pend = p1 + WIDTH;
    while (psz >= pbegin && p1 < pend) {
        *p1 = ::HexDigit(*psz--);
        if (psz >= pbegin) {
            *p1 |= ((unsigned char)::HexDigit(*psz--) << 4);
            p1++;
        }
    }
}

template <unsigned int BITS>
void base_blob<BITS>::SetHex(const std::string& str)
{
    SetHex(str.c_str());
}

template <unsigned int BITS>
std::string base_blob<BITS>::ToString() const
{
    return (GetHex());
}

// Explicit instantiations for base_blob<160>
template base_blob<160>::base_blob(const std::vector<unsigned char>&);
template std::string base_blob<160>::GetHex() const;
template std::string base_blob<160>::ToString() const;
template void base_blob<160>::SetHex(const char*);
template void base_blob<160>::SetHex(const std::string&);

// Explicit instantiations for base_blob<256>
template base_blob<256>::base_blob(const std::vector<unsigned char>&);
template std::string base_blob<256>::GetHex() const;
template std::string base_blob<256>::ToString() const;
template void base_blob<256>::SetHex(const char*);
template void base_blob<256>::SetHex(const std::string&);

// Explicit instantiations for base_blob<512>
template base_blob<512>::base_blob(const std::vector<unsigned char>&);
template std::string base_blob<512>::GetHex() const;
template std::string base_blob<512>::ToString() const;
template void base_blob<512>::SetHex(const char*);
template void base_blob<512>::SetHex(const std::string&);

// Explicit instantiations for base_blob<1024>
template base_blob<1024>::base_blob(const std::vector<unsigned char>&);
template std::string base_blob<1024>::GetHex() const;
template std::string base_blob<1024>::ToString() const;
template void base_blob<1024>::SetHex(const char*);
template void base_blob<1024>::SetHex(const std::string&);

void uint512 :: XOROperator ( const uint512 & _uint512B ) {
    uint32_t i;

    for ( i = 0; i < 512 / 8 / 8; i ++ ) { // 8 bits in byte and 8 bytes in uint64_t.
        //fprintf(stdout, "uint512.XOROperator () : %i .\n", i * 8 );

        * ( ( uint64_t * ) ( begin () + i * 8 ) ) = 
            * ( ( uint64_t * ) ( begin () + i * 8 ) ) ^
            * ( ( uint64_t * ) ( _uint512B.begin () + i * 8 ) );        

    } //-for

}

void uint512 :: XOROperator ( const uint256 & _uint256B, uint32_t _iOffset ) {
    uint32_t i;

    for ( i = 0; i < 256 / 8 / 8; i ++ ) {
        * ( ( uint64_t * ) ( begin () + _iOffset + i * 8 ) ) =
            * ( ( uint64_t * ) ( begin () + _iOffset + i * 8 ) ) ^
            * ( ( uint64_t * ) ( _uint256B.begin () + i * 8 ) );

    } //-for

}

void uint512 :: XOROperator ( const unsigned char * _pData ) {
    uint32_t i;

    for ( i = 0; i < 512 / 8 / 8; i ++ ) { // 8 bits in byte and 8 bytes in uint64_t.
        //fprintf(stdout, "uint512.XOROperator () : %i .\n", i * 8 );

        * ( ( uint64_t * ) ( begin () + i * 8 ) ) = 
            * ( ( uint64_t * ) ( begin () + i * 8 ) ) ^
            * ( ( uint64_t * ) ( _pData + i * 8 ) );        

    } //-for
    
}