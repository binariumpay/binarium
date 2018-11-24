// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_HASH_H
#define BITCOIN_HASH_H

#include "crypto/ripemd160.h"
#include "crypto/sha256.h"
#include "crypto/sha512.h"
#include "prevector.h"
#include "serialize.h"
#include "uint256.h"
#include "version.h"
//#include "validation.h"

#include "crypto/sph_blake.h"
#include "crypto/sph_bmw.h"
#include "crypto/sph_groestl.h"
#include "crypto/sph_jh.h"
#include "crypto/sph_keccak.h"
#include "crypto/sph_skein.h"
#include "crypto/sph_luffa.h"
#include "crypto/sph_cubehash.h"
#include "crypto/sph_shavite.h"
#include "crypto/sph_simd.h"
#include "crypto/sph_echo.h"

//#include "crypto/cppcrypto/streebog.h"
//#include "crypto/cppcrypto/whirlpool.h"
//#include "crypto/cppcrypto/sm3.h"
#include "crypto/hashing/streebog/stribog.h"
#include "crypto/hashing/whirlpool/whirlpool.h"
//#include "crypto/hashing/swifft/swifft.h"

//#include "crypto/cppcrypto/kuznyechik.h"
//#include "crypto/cppcrypto/threefish.h"
//#include "crypto/cppcrypto/sm4.h"
//#include "crypto/cppcrypto/serpent.h"
//#include "crypto/cppcrypto/aria.h"
//#include "crypto/encryption/camellia/camellia.h"

#include "crypto/encryption/gost2015_kuznechik/libgost15/libgost15.h"
#include "crypto/encryption/three_fish/libskein_skein.h"
#include "openssl/camellia.h"



#include <vector>



typedef uint256 ChainCode;

#ifdef GLOBALDEFINED
#define GLOBAL
#else
#define GLOBAL extern
#endif

#define I_HASH_SIZE 512



GLOBAL sph_blake512_context     z_blake;
GLOBAL sph_bmw512_context       z_bmw;
GLOBAL sph_groestl512_context   z_groestl;
GLOBAL sph_jh512_context        z_jh;
GLOBAL sph_keccak512_context    z_keccak;
GLOBAL sph_skein512_context     z_skein;
GLOBAL sph_luffa512_context     z_luffa;
GLOBAL sph_cubehash512_context  z_cubehash;
GLOBAL sph_shavite512_context   z_shavite;
GLOBAL sph_simd512_context      z_simd;
GLOBAL sph_echo512_context      z_echo;

#define fillz() do { \
    sph_blake512_init(&z_blake); \
    sph_bmw512_init(&z_bmw); \
    sph_groestl512_init(&z_groestl); \
    sph_jh512_init(&z_jh); \
    sph_keccak512_init(&z_keccak); \
    sph_skein512_init(&z_skein); \
    sph_luffa512_init(&z_luffa); \
    sph_cubehash512_init(&z_cubehash); \
    sph_shavite512_init(&z_shavite); \
    sph_simd512_init(&z_simd); \
    sph_echo512_init(&z_echo); \
} while (0)

#define ZBLAKE (memcpy(&ctx_blake, &z_blake, sizeof(z_blake)))
#define ZBMW (memcpy(&ctx_bmw, &z_bmw, sizeof(z_bmw)))
#define ZGROESTL (memcpy(&ctx_groestl, &z_groestl, sizeof(z_groestl)))
#define ZJH (memcpy(&ctx_jh, &z_jh, sizeof(z_jh)))
#define ZKECCAK (memcpy(&ctx_keccak, &z_keccak, sizeof(z_keccak)))
#define ZSKEIN (memcpy(&ctx_skein, &z_skein, sizeof(z_skein)))



/* ----------- Bitcoin Hash ------------------------------------------------- */
/** A hasher class for Bitcoin's 512-bit hash (double SHA-256). */
class CHash512 {
private:
    CSHA512 sha;
public:
    static const size_t OUTPUT_SIZE = CSHA256::OUTPUT_SIZE;

    void Finalize(unsigned char hash[OUTPUT_SIZE]) {
        unsigned char buf[sha.OUTPUT_SIZE];
        sha.Finalize(buf);
        sha.Reset().Write(buf, sha.OUTPUT_SIZE).Finalize(hash);
    }

    CHash512& Write(const unsigned char *data, size_t len) {
        sha.Write(data, len);
        return *this;
    }

    CHash512& Reset() {
        sha.Reset();
        return *this;
    }
};

/** A hasher class for Bitcoin's 256-bit hash (double SHA-256). */
class CHash256 {
private:
    CSHA256 sha;
public:
    static const size_t OUTPUT_SIZE = CSHA256::OUTPUT_SIZE;

    void Finalize(unsigned char hash[OUTPUT_SIZE]) {
        unsigned char buf[sha.OUTPUT_SIZE];
        sha.Finalize(buf);
        sha.Reset().Write(buf, sha.OUTPUT_SIZE).Finalize(hash);
    }

    CHash256& Write(const unsigned char *data, size_t len) {
        sha.Write(data, len);
        return *this;
    }

    CHash256& Reset() {
        sha.Reset();
        return *this;
    }
};

/** A hasher class for Bitcoin's 160-bit hash (SHA-256 + RIPEMD-160). */
class CHash160 {
private:
    CSHA256 sha;
public:
    static const size_t OUTPUT_SIZE = CRIPEMD160::OUTPUT_SIZE;

    void Finalize(unsigned char hash[OUTPUT_SIZE]) {
        unsigned char buf[sha.OUTPUT_SIZE];
        sha.Finalize(buf);
        CRIPEMD160().Write(buf, sha.OUTPUT_SIZE).Finalize(hash);
    }

    CHash160& Write(const unsigned char *data, size_t len) {
        sha.Write(data, len);
        return *this;
    }

    CHash160& Reset() {
        sha.Reset();
        return *this;
    }
};



/** Compute the 256-bit hash of an uint512. */
inline uint512 Hash(const uint512 _uint512Value)
{    
    uint512 result;
    CHash512().Write((const unsigned char*) & _uint512Value, sizeof(_uint512Value))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of an uint256. */
inline uint256 Hash(const uint256 _uint256Value)
{    
    uint256 result;
    CHash256().Write((const unsigned char*) & _uint256Value, sizeof(_uint256Value))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of an object. */
template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(pbegin == pend ? pblank : (const unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of two objects. */
template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of three objects. */
template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of three objects. */
template<typename T1, typename T2, typename T3, typename T4>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of three objects. */
template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end,
                    const T5 p5begin, const T5 p5end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Write(p5begin == p5end ? pblank : (const unsigned char*)&p5begin[0], (p5end - p5begin) * sizeof(p5begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of three objects. */
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end,
                    const T5 p5begin, const T5 p5end,
                    const T6 p6begin, const T6 p6end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Write(p5begin == p5end ? pblank : (const unsigned char*)&p5begin[0], (p5end - p5begin) * sizeof(p5begin[0]))
              .Write(p6begin == p6end ? pblank : (const unsigned char*)&p6begin[0], (p6end - p6begin) * sizeof(p6begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 160-bit hash an object. */
template<typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1] = {};
    uint160 result;
    CHash160().Write(pbegin == pend ? pblank : (const unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/** Compute the 160-bit hash of a vector. */
inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
    return Hash160(vch.begin(), vch.end());
}

/** Compute the 160-bit hash of a vector. */
template<unsigned int N>
inline uint160 Hash160(const prevector<N, unsigned char>& vch)
{
    return Hash160(vch.begin(), vch.end());
}

/** A writer stream (for serialization) that computes a 256-bit hash. */
class CHashWriter
{
private:
    CHash256 ctx;

public:
    int nType;
    int nVersion;

    CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {}

    CHashWriter& write(const char *pch, size_t size) {
        ctx.Write((const unsigned char*)pch, size);
        return (*this);
    }

    // invalidates the object
    uint256 GetHash() {
        uint256 result;
        ctx.Finalize((unsigned char*)&result);
        return result;
    }

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        // Serialize to this stream
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

/** Reads data from an underlying stream, while hashing the read data. */
template<typename Source>
class CHashVerifier : public CHashWriter
{
private:
    Source* source;

public:
    CHashVerifier(Source* source_) : CHashWriter(source_->GetType(), source_->GetVersion()), source(source_) {}

    void read(char* pch, size_t nSize)
    {
        source->read(pch, nSize);
        this->write(pch, nSize);
    }

    void ignore(size_t nSize)
    {
        char data[1024];
        while (nSize > 0) {
            size_t now = std::min<size_t>(nSize, 1024);
            read(data, now);
            nSize -= now;
        }
    }

    template<typename T>
    CHashVerifier<Source>& operator>>(T& obj)
    {
        // Unserialize from this stream
        ::Unserialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

/** Compute the 256-bit hash of an object's serialization. */
template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

void BIP32Hash(const ChainCode &chainCode, unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64]);

/** SipHash-2-4, using a uint64_t-based (rather than byte-based) interface */
class CSipHasher
{
private:
    uint64_t v[4];
    int count;

public:
    CSipHasher(uint64_t k0, uint64_t k1);
    CSipHasher& Write(uint64_t data);
    uint64_t Finalize() const;
};

uint64_t SipHashUint256(uint64_t k0, uint64_t k1, const uint256& val);
uint64_t SipHashUint256Extra(uint64_t k0, uint64_t k1, const uint256& val, uint32_t extra);

/* ----------- Binarium Hash ------------------------------------------------ */
template<typename T1>
inline uint256 HashX11(const T1 pbegin, const T1 pend)

{
    sph_blake512_context     ctx_blake;
    sph_bmw512_context       ctx_bmw;
    sph_groestl512_context   ctx_groestl;
    sph_jh512_context        ctx_jh;
    sph_keccak512_context    ctx_keccak;
    sph_skein512_context     ctx_skein;
    sph_luffa512_context     ctx_luffa;
    sph_cubehash512_context  ctx_cubehash;
    sph_shavite512_context   ctx_shavite;
    sph_simd512_context      ctx_simd;
    sph_echo512_context      ctx_echo;
    static unsigned char pblank[1];

    uint512 hash[11];

    sph_blake512_init(&ctx_blake);
    sph_blake512 (&ctx_blake, (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]));
    sph_blake512_close(&ctx_blake, static_cast<void*>(&hash[0]));

    sph_bmw512_init(&ctx_bmw);
    sph_bmw512 (&ctx_bmw, static_cast<const void*>(&hash[0]), 64);
    sph_bmw512_close(&ctx_bmw, static_cast<void*>(&hash[1]));

    sph_groestl512_init(&ctx_groestl);
    sph_groestl512 (&ctx_groestl, static_cast<const void*>(&hash[1]), 64);
    sph_groestl512_close(&ctx_groestl, static_cast<void*>(&hash[2]));

    sph_skein512_init(&ctx_skein);
    sph_skein512 (&ctx_skein, static_cast<const void*>(&hash[2]), 64);
    sph_skein512_close(&ctx_skein, static_cast<void*>(&hash[3]));

    sph_jh512_init(&ctx_jh);
    sph_jh512 (&ctx_jh, static_cast<const void*>(&hash[3]), 64);
    sph_jh512_close(&ctx_jh, static_cast<void*>(&hash[4]));

    sph_keccak512_init(&ctx_keccak);
    sph_keccak512 (&ctx_keccak, static_cast<const void*>(&hash[4]), 64);
    sph_keccak512_close(&ctx_keccak, static_cast<void*>(&hash[5]));

    sph_luffa512_init(&ctx_luffa);
    sph_luffa512 (&ctx_luffa, static_cast<void*>(&hash[5]), 64);
    sph_luffa512_close(&ctx_luffa, static_cast<void*>(&hash[6]));

    sph_cubehash512_init(&ctx_cubehash);
    sph_cubehash512 (&ctx_cubehash, static_cast<const void*>(&hash[6]), 64);
    sph_cubehash512_close(&ctx_cubehash, static_cast<void*>(&hash[7]));

    sph_shavite512_init(&ctx_shavite);
    sph_shavite512(&ctx_shavite, static_cast<const void*>(&hash[7]), 64);
    sph_shavite512_close(&ctx_shavite, static_cast<void*>(&hash[8]));

    sph_simd512_init(&ctx_simd);
    sph_simd512 (&ctx_simd, static_cast<const void*>(&hash[8]), 64);
    sph_simd512_close(&ctx_simd, static_cast<void*>(&hash[9]));

    sph_echo512_init(&ctx_echo);
    sph_echo512 (&ctx_echo, static_cast<const void*>(&hash[9]), 64);
    sph_echo512_close(&ctx_echo, static_cast<void*>(&hash[10]));

    return hash[10].trim256();
}



/*//template<typename T1>
//inline uint256 (*pHashFunction)(const T1, const T1);
//template<class T1>
//using pHashFunction = uint256(*)(const T1, const T1);

//template<class T1>
//using aHashFunctions = uint256(*)(const T1, const T1) [ 2 ];

typedef char*& pCharValueParameter;
typedef uint256(*pHashFunction) ( const pCharValueParameter, const pCharValueParameter );
pHashFunction aHashFunctions [ 2 ];
//std::vector <pHashFunction> aHashFunctions;
pHashFunction pFunctionPointer = & HashX11;

aHashFunctions [ 0 ] = pFunctionPointer;
//aHashFunctions [ 1 ] = & Hash;
//aHashFunctions.push_back ( pFunctionPointer );

//template<class T1>
//std::function<void(const T1 _a1, const T1 _a2)> aHashFunctions [ 2 ];

//aHashFunctions [ 0 ] = & HashX11 ( int32_t &, int32_t & );
//aHashFunctions [ 1 ] = Hash;*/



#ifndef HASH_GENERATOR
#define HASH_GENERATOR
typedef void ( * TCryptographyFunction ) ( const void *, const uint32_t, const void *, void * ); // uint512

#define I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS 14
#define I_AMOUNT_OF_INTERMEDIATE_ENCRYPTION_FUNCTIONS 3

// Data, key, result
//uint512 ( * aIntermediateHashFunctions [ I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS ] ) ( const void *, const uint32_t, const void *, void * );
//uint512 ( * aIntermediateEncryptionFunctions [ I_AMOUNT_OF_INTERMEDIATE_ENCRYPTION_FUNCTIONS ] ) ( const void *, const uint32_t, const void *, void * );

extern TCryptographyFunction aIntermediateHashFunctions [ I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS ];
extern TCryptographyFunction aIntermediateEncryptionFunctions [ I_AMOUNT_OF_INTERMEDIATE_ENCRYPTION_FUNCTIONS ];

//inline TCryptographyFunction GetHashingFunction ( int _iIndex );
//inline TCryptographyFunction GetEcryptionFunction ( int _iIndex );

//extern CChain chainActive;

/*//---Utility functions.---------------------------------------------------
inline uint64_t GetUint64IndexFrom512BitsKey ( const void * _pKey, int pos ) {
    const uint8_t* ptr = ( const uint8_t * ) _pKey + pos * 8;
    return ((uint64_t)ptr[0]) | \
           ((uint64_t)ptr[1]) << 8 | \
           ((uint64_t)ptr[2]) << 16 | \
           ((uint64_t)ptr[3]) << 24 | \
           ((uint64_t)ptr[4]) << 32 | \
           ((uint64_t)ptr[5]) << 40 | \
           ((uint64_t)ptr[6]) << 48 | \
           ((uint64_t)ptr[7]) << 56;
}*/

void bin2hex(char *s, const unsigned char *p, size_t len);



//---Hashing.-------------------------------------------------------------
inline void IntermediateHashFunction_Blake ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_blake512_context     ctx_blake;

    sph_blake512_init(&ctx_blake);
    sph_blake512 (&ctx_blake, _pData, _iDataSize);
    sph_blake512_close(&ctx_blake, _pResult);
}

inline void IntermediateHashFunction_BMW ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_bmw512_context       ctx_bmw;

    sph_bmw512_init(&ctx_bmw);
    sph_bmw512 (&ctx_bmw, _pData, _iDataSize);
    sph_bmw512_close(&ctx_bmw, _pResult);
}

inline void IntermediateHashFunction_Groestl ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_groestl512_context   ctx_groestl;

    sph_groestl512_init(&ctx_groestl);
    sph_groestl512 (&ctx_groestl, _pData, _iDataSize);
    sph_groestl512_close(&ctx_groestl, _pResult);
}

inline void IntermediateHashFunction_JH ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_jh512_context        ctx_jh;

    sph_jh512_init(&ctx_jh);
    sph_jh512 (&ctx_jh, _pData, _iDataSize);
    sph_jh512_close(&ctx_jh, _pResult);
}

inline void IntermediateHashFunction_Keccak ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_keccak512_context    ctx_keccak;

    sph_keccak512_init(&ctx_keccak);
    sph_keccak512 (&ctx_keccak, _pData, _iDataSize);
    sph_keccak512_close(&ctx_keccak, _pResult);
}

inline void IntermediateHashFunction_Skein ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_skein512_context     ctx_skein;

    sph_skein512_init(&ctx_skein);
    sph_skein512 (&ctx_skein, _pData, _iDataSize);
    sph_skein512_close(&ctx_skein, _pResult);
}

inline void IntermediateHashFunction_Luffa ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_luffa512_context     ctx_luffa;

    sph_luffa512_init(&ctx_luffa);
    sph_luffa512 (&ctx_luffa, _pData, _iDataSize);
    sph_luffa512_close(&ctx_luffa, _pResult);
}

inline void IntermediateHashFunction_Cubehash ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_cubehash512_context  ctx_cubehash;

    sph_cubehash512_init(&ctx_cubehash);
    sph_cubehash512 (&ctx_cubehash, _pData, _iDataSize);
    sph_cubehash512_close(&ctx_cubehash, _pResult);
}

inline void IntermediateHashFunction_Shavite ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_shavite512_context   ctx_shavite;

    sph_shavite512_init(&ctx_shavite);
    sph_shavite512(&ctx_shavite, _pData, _iDataSize);
    sph_shavite512_close(&ctx_shavite, _pResult);
}

inline void IntermediateHashFunction_Simd ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_simd512_context      ctx_simd;

    sph_simd512_init(&ctx_simd);
    sph_simd512 (&ctx_simd, _pData, _iDataSize);
    sph_simd512_close(&ctx_simd, _pResult);
}

inline void IntermediateHashFunction_Echo ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    sph_echo512_context      ctx_echo;

    sph_echo512_init(&ctx_echo);
    sph_echo512 (&ctx_echo, _pData, _iDataSize);
    sph_echo512_close(&ctx_echo, _pResult);
}

inline void IntermediateHashFunction_GOST_2012_Streebog ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    hash_512 ( ( const unsigned char * ) _pData, _iDataSize, ( unsigned char * ) _pResult );
}

inline void IntermediateHashFunction_Whirlpool ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    whirlpool_ctx            structureWhirlpoolContext;

    rhash_whirlpool_init ( & structureWhirlpoolContext );
    rhash_whirlpool_update ( & structureWhirlpoolContext, ( const unsigned char * ) _pData, _iDataSize); 
    rhash_whirlpool_final ( & structureWhirlpoolContext, ( unsigned char * ) _pResult);
}

/*inline uint512 IntermediateHashFunction_SWIFFT ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    HashKey key ALIGN;
    HashState state ALIGN;
    SWIFFT_HashData data ALIGN;

    SwiFFT_initState ( state );
    SwiFFT_readKey ( ( const unsigned char * ) _pKey, & key );    
    SwiFFT_readData ( data, ( const unsigned char * ) _pData );
    SwiFFT ( key, state, data );

    memcpy ( _pResult, state, 64 );

    //SwiFFT_printState ( state );
    //SwiFFT_printKey ( key );
    //SwiFFT_printData ( data );
}*/

//---Encryption.----------------------------------------------------------------------
inline void IntermediateEncryptionFunction_GOST_2015_Kuznechik ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    //char roundkeys_str [ 401 ];

    //uint64_t iIndex = GetUint64IndexFrom512BitsKey ( _pKey, 0 );
    //iIndex = iIndex % chainActive.Height ();

    //bin2hex(roundkeys_str, (unsigned char *) _pData, 200 );
    //fprintf(stdout, "IntermediateEncryptionFunction_GOST_2015_Kuznechik () : roundkeys_str : %s.\n", roundkeys_str );

    encryptBlockWithGost15 ( _pKey, ( unsigned char * ) _pResult );           // _pKey & chainActive [ chainActive.Height () - iIndex ] -> nVersion
    encryptBlockWithGost15 ( _pData, ( unsigned char * ) _pResult + 16 );     // _pData
    encryptBlockWithGost15 ( _pKey, ( unsigned char * ) _pResult + 32 );      // _pKey
    encryptBlockWithGost15 ( _pData, ( unsigned char * ) _pResult + 48 );     // _pData
}

inline void IntermediateEncryptionFunction_ThreeFish ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    char T[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        
    // Result, Key, Text.
    libskein_threefish_encrypt( ( char * ) _pResult, ( const char * ) _pKey, T, ( const char * ) _pData, 64, 512);
}

inline void IntermediateEncryptionFunction_Camellia ( const void * _pData, const uint32_t _iDataSize, const void * _pKey, void * _pResult ) {
    CAMELLIA_KEY stKey;
    Camellia_set_key ( ( const unsigned char * ) _pKey, 256, & stKey ); // userKey    
    Camellia_encrypt ( ( const unsigned char * ) _pData, ( unsigned char * ) _pResult, & stKey );        // in, out, key 
}



inline void HashGenerator_Init () {
    aIntermediateHashFunctions [ 0 ]        = & IntermediateHashFunction_Blake;
    aIntermediateHashFunctions [ 1 ]        = & IntermediateHashFunction_BMW;
    aIntermediateHashFunctions [ 2 ]        = & IntermediateHashFunction_Groestl;
    aIntermediateHashFunctions [ 3 ]        = & IntermediateHashFunction_JH;
    aIntermediateHashFunctions [ 4 ]        = & IntermediateHashFunction_Keccak;
    aIntermediateHashFunctions [ 5 ]        = & IntermediateHashFunction_Skein;
    aIntermediateHashFunctions [ 6 ]        = & IntermediateHashFunction_Luffa;
    aIntermediateHashFunctions [ 7 ]        = & IntermediateHashFunction_Cubehash;
    aIntermediateHashFunctions [ 8 ]        = & IntermediateHashFunction_Shavite;
    aIntermediateHashFunctions [ 9 ]        = & IntermediateHashFunction_Simd;
    aIntermediateHashFunctions [ 10 ]       = & IntermediateHashFunction_Echo;
    aIntermediateHashFunctions [ 11 ]       = & IntermediateHashFunction_GOST_2012_Streebog;
    aIntermediateHashFunctions [ 12 ]       = & IntermediateHashFunction_Whirlpool;
    //aIntermediateHashFunctions [ 13 ]       = & IntermediateHashFunction_SWIFFT;
    aIntermediateHashFunctions [ 13 ]       = & IntermediateHashFunction_GOST_2012_Streebog;

    aIntermediateEncryptionFunctions [ 0 ]         = & IntermediateEncryptionFunction_GOST_2015_Kuznechik;
    aIntermediateEncryptionFunctions [ 1 ]         = & IntermediateEncryptionFunction_ThreeFish;
    aIntermediateEncryptionFunctions [ 2 ]         = & IntermediateEncryptionFunction_Camellia;
}

#endif // HASH_GENERATOR


template<typename T1>
inline uint256 HashX11_Generator(const T1 pbegin, const T1 pend) // , uint32_t _iAlgorithmDefiningValue

{
    /*sph_blake512_context     ctx_blake;
    sph_bmw512_context       ctx_bmw;
    sph_groestl512_context   ctx_groestl;
    sph_jh512_context        ctx_jh;
    sph_keccak512_context    ctx_keccak;
    sph_skein512_context     ctx_skein;
    sph_luffa512_context     ctx_luffa;
    sph_cubehash512_context  ctx_cubehash;
    sph_shavite512_context   ctx_shavite;
    sph_simd512_context      ctx_simd;
    sph_echo512_context      ctx_echo;

    //cppcrypto :: streebog *              pHash_Streebog = new cppcrypto :: streebog ( I_HASH_SIZE );
    whirlpool_ctx            structureWhirlpoolContext;*/

    static unsigned char pblank[1];

    uint512 hash[11];



    /*sph_blake512_init(&ctx_blake);
    sph_blake512 (&ctx_blake, (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]));
    sph_blake512_close(&ctx_blake, static_cast<void*>(&hash[0]));*/
    aIntermediateHashFunctions [ 0 ] ( (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]), nullptr, hash[0].begin () );
    //GetHashingFunction ( 0 ) ( (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]), nullptr, hash[0].begin () );

    //sph_bmw512_init(&ctx_bmw);
    //sph_bmw512 (&ctx_bmw, static_cast<const void*>(&hash[0]), 64);
    //sph_bmw512_close(&ctx_bmw, static_cast<void*>(&hash[1]));
    aIntermediateHashFunctions [ 1 ] ( static_cast<const void*>(&hash[0]), 64, nullptr, static_cast<void*>(&hash[1]) );

    //sph_groestl512_init(&ctx_groestl);
    //sph_groestl512 (&ctx_groestl, static_cast<const void*>(&hash[1]), 64);
    //sph_groestl512_close(&ctx_groestl, static_cast<void*>(&hash[2]));
    aIntermediateHashFunctions [ 2 ] ( static_cast<const void*>(&hash[1]), 64, nullptr, static_cast<void*>(&hash[2]) );

    //sph_skein512_init(&ctx_skein);
    //sph_skein512 (&ctx_skein, static_cast<const void*>(&hash[2]), 64);
    //sph_skein512_close(&ctx_skein, static_cast<void*>(&hash[3]));
    aIntermediateHashFunctions [ 5 ] ( static_cast<const void*>(&hash[2]), 64, nullptr, static_cast<void*>(&hash[3]) );

    //-Streebog.--------------------------------------
    //sph_jh512_init(&ctx_jh);
    //sph_jh512 (&ctx_jh, static_cast<const void*>(&hash[3]), 64);
    //sph_jh512_close(&ctx_jh, static_cast<void*>(&hash[4]));    

    //pHash_Streebog -> init ();
    //pHash_Streebog -> update(static_cast<const unsigned char *>(static_cast<const void*>(&hash[3])), 64);
    //pHash_Streebog -> final (static_cast<unsigned char *>(static_cast<void*>(&hash[4])));
    
    //hash_512 ( static_cast<const unsigned char *>(static_cast<const void*>(&hash[3])), 64 * 8, static_cast<unsigned char *>(static_cast<void*>(&hash[4])) );
    aIntermediateHashFunctions [ 11 ] ( static_cast<const void*>(&hash[3]), 64 * 8, nullptr, static_cast<void*>(&hash[4]) );

    //-Whirlpool--------------------------------------
    //sph_keccak512_init(&ctx_keccak);
    //sph_keccak512 (&ctx_keccak, static_cast<const void*>(&hash[4]), 64);
    //sph_keccak512_close(&ctx_keccak, static_cast<void*>(&hash[5]));
        
    //rhash_whirlpool_init ( & structureWhirlpoolContext );
    //rhash_whirlpool_update ( & structureWhirlpoolContext, static_cast<const unsigned char *>(static_cast<const void*>(&hash[4])), 64); 
    //rhash_whirlpool_final ( & structureWhirlpoolContext, static_cast<unsigned char *>(static_cast<void*>(&hash[5])));
    aIntermediateHashFunctions [ 12 ] ( static_cast<const void*>(&hash[4]), 64, nullptr, static_cast<void*>(&hash[5]) );

    //-SWIFFT.----------------------------------------
    //sph_luffa512_init(&ctx_luffa);
    //sph_luffa512 (&ctx_luffa, static_cast<void*>(&hash[5]), 64);
    //sph_luffa512_close(&ctx_luffa, static_cast<void*>(&hash[6]));
    
    char pSwiFFTKey [ 1025 ];
    //uint1024 uint1024SiFFTKey;

    /*HashKey key ALIGN;
    HashState state ALIGN;
    HashData data ALIGN;

    SwiFFT_initState ( state );
    SwiFFT_readKey ( ( const unsigned char * ) & hash[1], & key );    
    SwiFFT_readData ( data, ( const unsigned char * ) & hash[5] );
    SwiFFT ( key, state, data );
    memcpy ( hash [ 6 ].begin (), state, 64 );
    //memcpy ( hash [ 7 ].begin (), & state, 64 );
    //SwiFFT_printState ( state );
    //SwiFFT_printKey ( key );
    //SwiFFT_printData ( data );*/
    memset(pSwiFFTKey, 0, sizeof(pSwiFFTKey));
    memcpy ( pSwiFFTKey, hash[1].begin (), 64 );
    memcpy ( pSwiFFTKey + 64, hash[2].begin (), 64 );
    memcpy ( pSwiFFTKey + 128, hash[0].begin (), 64 );
    memcpy ( pSwiFFTKey + 192, hash[3].begin (), 64 );
    memcpy ( pSwiFFTKey + 256, hash[4].begin (), 64 );
    memcpy ( pSwiFFTKey + 320, hash[5].begin (), 64 );
    memcpy ( pSwiFFTKey + 384, hash[0].begin (), 64 );
    memcpy ( pSwiFFTKey + 448, hash[2].begin (), 64 );
    memcpy ( pSwiFFTKey + 512, hash[4].begin (), 64 );
    memcpy ( pSwiFFTKey + 576, hash[5].begin (), 64 );
    memcpy ( pSwiFFTKey + 640, hash[3].begin (), 64 );
    memcpy ( pSwiFFTKey + 704, hash[0].begin (), 64 );
    memcpy ( pSwiFFTKey + 768, hash[1].begin (), 64 );
    memcpy ( pSwiFFTKey + 832, hash[5].begin (), 64 );
    memcpy ( pSwiFFTKey + 896, hash[2].begin (), 64 );
    memcpy ( pSwiFFTKey + 960, hash[0].begin (), 64 );
    //memcpy ( pSwiFFTKey + 1024, hash[4].begin (), 64 );
    
    //pSwiFFTKey [ 1024 ] = 0;
    //fprintf(stdout, "hash.h : HashX11_Generator () : SWIFFT Key : %s.\n", pSwiFFTKey );
    aIntermediateHashFunctions [ 13 ] ( static_cast<const void*>(hash[5].begin ()), 64, pSwiFFTKey, static_cast<void*>(hash[6].begin ()) );
    //fprintf(stdout, "hash.h : HashX11_Generator () : SWIFFT 1 : %s.\n", hash [ 5 ].ToString ().c_str () );
    //fprintf(stdout, "hash.h : HashX11_Generator () : SWIFFT 2 : %s.\n", hash [ 6 ].ToString ().c_str () );

    //sph_cubehash512_init(&ctx_cubehash);
    //sph_cubehash512 (&ctx_cubehash, static_cast<const void*>(&hash[6]), 64);
    //sph_cubehash512_close(&ctx_cubehash, static_cast<void*>(&hash[7]));
    aIntermediateHashFunctions [ 7 ] ( static_cast<const void*>(&hash[6]), 64, nullptr, static_cast<void*>(&hash[7]) );

    //-GOST 2015_Kuznechik.---------------------------
    /*//fprintf(stdout, "hash.h : HashX11_Generator () : %s .\n", hash [ 7 ].ToString ().c_str () );
    //memcpy ( hash [ 7 ].begin (), hash [ 6 ].begin (), 64 );
    encryptBlockWithGost15 ( & hash [ 3 ], hash [ 7 ].begin () );
    encryptBlockWithGost15 ( & hash [ 0 ], hash [ 7 ].begin () + 16 );
    encryptBlockWithGost15 ( & hash [ 3 ], hash [ 7 ].begin () + 32 );
    encryptBlockWithGost15 ( & hash [ 0 ], hash [ 7 ].begin () + 48 );

    //fprintf(stdout, "hash.h : HashX11_Generator () : %s .\n", hash [ 7 ].ToString ().c_str () );*/
    aIntermediateEncryptionFunctions [ 0 ] ( static_cast<const void*>(&hash[0]), 64, static_cast<const void*>(&hash[3]), static_cast<void*>(&hash[7]) );

    //sph_shavite512_init(&ctx_shavite);
    //sph_shavite512(&ctx_shavite, static_cast<const void*>(&hash[7]), 64);
    //sph_shavite512_close(&ctx_shavite, static_cast<void*>(&hash[8]));
    aIntermediateHashFunctions [ 8 ] ( static_cast<const void*>(&hash[7]), 64, nullptr, static_cast<void*>(&hash[8]) );

    //---ThreeFish.----------------------------------            
    //char T[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Copying from 8 to 7.
    memcpy ( hash [ 7 ].begin (), hash [ 8 ].begin (), 64 );
    // Copying from 7 to 8.
    //memcpy ( hash [ 8 ].begin (), hash [ 7 ].begin (), 64 );
    /*//fprintf(stdout, "hash.h : HashX11_Generator () : %s .\n", hash [ 8 ].ToString ().c_str () );
    // Result, Key, Text.
    libskein_threefish_encrypt( ( char * ) ( hash[8].begin () ), ( const char * ) ( hash[4].begin () ), T, ( const char * ) ( hash [ 7 ].begin () ), 64, 512);
    //fprintf(stdout, "hash.h : HashX11_Generator () : %s .\n", hash [ 8 ].ToString ().c_str () );*/
    aIntermediateEncryptionFunctions [ 1 ] ( static_cast<const void*>(&hash[7]), 64, static_cast<const void*>(&hash[4]), static_cast<void*>(&hash[8]) );

    //sph_simd512_init(&ctx_simd);
    //sph_simd512 (&ctx_simd, static_cast<const void*>(&hash[8]), 64);
    //sph_simd512_close(&ctx_simd, static_cast<void*>(&hash[9]));
    aIntermediateHashFunctions [ 9 ] ( static_cast<const void*>(&hash[8]), 64, nullptr, static_cast<void*>(&hash[9]) );

    //---Camellia.-----------------------------------
    //CAMELLIA_KEY stKey;
    //Camellia_set_key ( hash [ 5 ].begin (), 256, & stKey ); // userKey
    // Copying from 9 to 8.
    memcpy ( hash [ 8 ].begin (), hash [ 9 ].begin (), 64 );
    // Copying from 8 to 9.
    //memcpy ( hash [ 9 ].begin (), hash [ 8 ].begin (), 64 );
    //Camellia_encrypt ( hash [ 8 ].begin (), hash [ 9 ].begin (), & stKey );        // in, out, key
    aIntermediateEncryptionFunctions [ 2 ] ( static_cast<const void*>(&hash[8]), 64, static_cast<const void*>(&hash[5]), static_cast<void*>(&hash[9]) );

    //sph_echo512_init(&ctx_echo);
    //sph_echo512 (&ctx_echo, static_cast<const void*>(&hash[9]), 64);
    //sph_echo512_close(&ctx_echo, static_cast<void*>(&hash[10]));
    aIntermediateHashFunctions [ 10 ] ( static_cast<const void*>(&hash[9]), 64, nullptr, static_cast<void*>(&hash[10]) );



    //delete pHash_Streebog;



    return hash[10].trim256();
}

#endif // BITCOIN_HASH_H
