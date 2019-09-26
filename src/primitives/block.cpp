// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
#include "crypto/encryption/salsa20/ecrypt-sync.h"
#include "chain.h"
#include "chainparams.h"
#include "validation.h"

#include <inttypes.h>



// Divide by CBlockIndexSize + 16 bites for phashBlock.
#define I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY_CPP 3 * 1024 * 1024 / ( 136 + 16 )

//#define I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_MINUTES 10080
#define I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_SECONDS 604800    // 600

#define I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING 3571

#define I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION 32 * 1024
#define I_MEMORY_BLOCK_SIZE_TO_ENCRYPTION_COMPUTATIONS_RATIO 1



extern CChain chainActive;



CBlockIndex * pWeekChangeBlock = nullptr;



//---Utility functions.---------------------------------------------------
inline uint64_t GetUint64IndexFrom512BitsKey ( const unsigned char * _pKey, int pos ) {
    //const uint8_t* ptr = ( const uint8_t * ) _pKey + pos * 8;
    //return ((uint64_t)ptr[0]) | \
           ((uint64_t)ptr[1]) << 8 | \
           ((uint64_t)ptr[2]) << 16 | \
           ((uint64_t)ptr[3]) << 24 | \
           ((uint64_t)ptr[4]) << 32 | \
           ((uint64_t)ptr[5]) << 40 | \
           ((uint64_t)ptr[6]) << 48 | \
           ((uint64_t)ptr[7]) << 56;

    uint64_t * pAdress = ( uint64_t * ) ( _pKey + pos );
    return pAdress [ 0 ] ^ pAdress [ 1 ] ^ pAdress [ 2 ] ^ pAdress [ 3 ] ^
           pAdress [ 4 ] ^ pAdress [ 5 ] ^ pAdress [ 6 ] ^ pAdress [ 7 ];

}

uint512 GetBlockData ( uint64_t _iIndex, uint64_t _iHeightOfBlockchain ) {
    uint512 uint512Result;    
    //fprintf(stdout, "block.cpp : GetBlockData () : %" PRIu64 " , %i ,%" PRIu64 ", %i .\n", _iIndex, chainActive.Height (), _iIndex % chainActive.Height (), sizeof ( chainActive.Tip () ) );
    //fprintf(stdout, "block.cpp : GetBlockData () : %" PRIu64 " , %" PRIu64 " .\n", _iHeightOfBlockchain, _iIndex );
    //uint64_t iIndex = iIndex % chainActive.Height ();

    //aIntermediateEncryptionFunctions [ 0 ] ( & chainActive [ chainActive.Height () - iIndex ] -> nVersion
    memcpy ( uint512Result.begin (), & chainActive [ _iHeightOfBlockchain - _iIndex - 1 ] -> nVersion, 48 );   // chainActive.Height ()
    memcpy ( uint512Result.begin () + 48, chainActive [ _iHeightOfBlockchain - _iIndex - 1 ] -> phashBlock -> begin (), 64 - 48 );

    return uint512Result;

}

void PrintMemoryArea ( uint64_t * _pMemoryArea, uint64_t _iAmount ) {
    int i;

    fprintf ( stdout, "block.cpp  : PrintMemoryArea () : " );
    for ( i = 0; i < _iAmount; i ++ ) {
        fprintf ( stdout, "%" PRIu64 " ", _pMemoryArea [ i ] );

    } //-for

    fprintf ( stdout, ".\n" );

}



//---Member functions.----------------------------------------------------
uint256 CBlockHeader::GetHash() const
{
    //CBlockIndex* pGenesisBlock = Params ().GenesisBlock(); // chainActive.Genesis()
    uint32_t iTimeFromGenesisBlock;
    uint32_t iAlgorithmSelector;
    uint32_t iHashFunctionsAmount;

    //assert ( pGenesisBlock != nullptr );

    CBlockIndex * pPrevBlockIndex = nullptr;
    /*BlockMap::const_iterator t = mapBlockIndex.find(hashPrevBlock);
    if (t != mapBlockIndex.end()) {
        pPrevBlockIndex = t->second;
        //m_pPreviousBlockIndex = t->second;
    }*/

    iTimeFromGenesisBlock = nTime - Params ().GenesisBlock().nTime; // Params ().GenesisBlock() chainActive.Genesis ()
    //iHashFunctionsAmount = nTime > Params ().aHashFunctionsAdditionsHistory [ 0 ].iBlockTime ? 2 : 2;
    iHashFunctionsAmount = 2;
    //iAlgorithmSelector = ( iTimeFromGenesisBlock / Params ().iHashAlgorithmChangeInterval ) % iHashFunctionsAmount;
    //iAlgorithmSelector = iTimeFromGenesisBlock < 20 || pPrevBlockIndex -> nHeight == 0 ? 0 : 1; // chainActive == nullptr ||  nHeightOfPreviousBlock <= 1 ;  chainActive.Height () <= 1    pPrevBlockIndex == nullptr
    iAlgorithmSelector = iTimeFromGenesisBlock < 3528000 ? 0 : 1; // chainActive == nullptr ||  nHeightOfPreviousBlock <= 1 ;  chainActive.Height () <= 1    pPrevBlockIndex == nullptr  // 20
    //iAlgorithmSelector = 0;

    //fprintf(stdout, "CBlockHeader.GetHash () : iAlgorithmSelector = %i : %i .\n", iTimeFromGenesisBlock, iAlgorithmSelector);  // strUsage.c_str()
    /*if ( iAlgorithmSelector == 0 )
        return HashX11(BEGIN(nVersion), END(nNonce));
    else
        return Hash(BEGIN(nVersion), END(nNonce));*/

    //if ( ( pWeekChangeBlock == nullptr ) || (  ) ) 

    //return ( this -> * aHashFunctions [ iAlgorithmSelector ] ) ();
    return (this->*(aHashFunctions[iAlgorithmSelector]))( pPrevBlockIndex, iTimeFromGenesisBlock );

}

uint256 CBlockHeader::GetGenesisInitializationHash() const
{
    HashGenerator_Init ();

    return HashX11(BEGIN(nVersion), END(nNonce));
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i].ToString() << "\n";
    }
    return s.str();
}


uint256 CBlockHeader::GetHash_X11( void * _pPreviousBlockIndex, uint32_t _iTimeFromGenesisBlock ) const { return HashX11(BEGIN(nVersion), END(nNonce)); }
uint256 CBlockHeader::GetHash_SHA256AndX11( void * _pPreviousBlockIndex, uint32_t _iTimeFromGenesisBlock ) const {
    //return HashX11_Generator_Blocks (BEGIN(nVersion), END(nNonce));

    uint512 hash[11];
    uint512 uint512AdditionalHash;

    uint512 uint512ChainBlockData;
    //uint512ChainBlockData.SetNull ();
    //CBlockIndex * pPrevBlockIndex = nullptr;   // mapBlockIndex.find(hashPrevBlock);
    //BlockMap::const_iterator t = mapBlockIndex.find(hashPrevBlock);
    //if (t != mapBlockIndex.end()) {
    //    pPrevBlockIndex = t->second;
    //}

    uint1024 uint1024CombinedHashes;

    //uint64_t iIndexOfBlcok;
    uint64_t iWeekNumber;
    //uint64_t iIndexFromWeekChangeBlock;
    uint64_t iIndex;

    uint32_t i = 0;



    // blake512
    aIntermediateHashFunctions [ 0 ] ( & nVersion, ( ( unsigned char * ) & nNonce - ( unsigned char * ) & nVersion ) /* sizeof(pbegin[0])*/ + sizeof ( nNonce ), nullptr, hash[0].begin () );

    // bmw512
    //aIntermediateHashFunctions [ 1 ] ( static_cast<const void*>(&hash[0]), 64, nullptr, static_cast<void*>(&hash[1]) );
    aIntermediateHashFunctions [ 1 ] ( static_cast<const void*>(&hash[0]), 64, nullptr, uint512AdditionalHash.begin () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : bmw512 : %s .\n", uint512AdditionalHash.ToString ().c_str () );

    //iIndexOfBlcok = ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight / ( I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_MINUTES / 2 ) * ( I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_MINUTES / 2 );    
    iWeekNumber = _iTimeFromGenesisBlock / I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_SECONDS * I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_SECONDS;
    //iIndexFromWeekChangeBlock = GetUint64IndexFrom512BitsKey ( chainActive [ iIndexOfBlcok ] -> phashBlock -> begin (), 0 );
    //iIndex = iIndexFromWeekChangeBlock % I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS;
    iIndex = ( iWeekNumber + nBits ) % I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS;
    //memcpy ( hash [ 0 ].begin (), hash [ 1 ].begin (), 64 );
    //aIntermediateHashFunctions [ iIndex ] ( static_cast<const void*>(&hash[0]), 64, nullptr, static_cast<void*>(&hash[1]) );
    aIntermediateHashFunctions [ iIndex ] ( uint512AdditionalHash.begin (), 64, nullptr, static_cast<void*>(&hash[1]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : aIntermediateHashFunctions 1 : %s .\n", hash[1].ToString ().c_str () );

    // groestl512
    aIntermediateHashFunctions [ 2 ] ( static_cast<const void*>(&hash[1]), 64, nullptr, static_cast<void*>(&hash[2]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : groestl512 : %s .\n", hash[2].ToString ().c_str () );

    // skein512
    //aIntermediateHashFunctions [ 5 ] ( static_cast<const void*>(&hash[2]), 64, nullptr, static_cast<void*>(&hash[3]) );
    aIntermediateHashFunctions [ 5 ] ( static_cast<const void*>(&hash[2]), 64, nullptr, uint1024CombinedHashes.begin () + 64 );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : skein512 ( uint1024CombinedHashes in + 64 ) : %s .\n", uint1024CombinedHashes.ToString ().c_str () );

    //-Streebog.--------------------------------------
    // jh512    

    //aIntermediateHashFunctions [ 3 ] ( static_cast<const void*>(&hash[3]), 64, nullptr, static_cast<void*>(&hash[4]) );
    //aIntermediateHashFunctions [ 3 ] ( static_cast<const void*>(&hash[3]), 64, nullptr, uint1024CombinedHashes.begin () );
    //memcpy ( hash[4].begin (), uint1024CombinedHashes.begin (), 64 );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : %s .\n", uint1024CombinedHashes.ToString ().c_str () );
    //* ( ( uint64_t * ) hash[4].begin () ) = 100;
    //aIntermediateHashFunctions [ 3 ] ( static_cast<const void*>(&hash[3]), 64, nullptr, static_cast<void*> ( & uint1024CombinedHashes ) );
    //aIntermediateHashFunctions [ 3 ] ( uint1024CombinedHashes.begin () + 64, 64, nullptr, static_cast<void*>(&hash[4]) );
    aIntermediateHashFunctions [ 3 ] ( uint1024CombinedHashes.begin () + 64, 64, nullptr, uint1024CombinedHashes.begin () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : jh512 ( uint1024CombinedHashes ) : %s .\n", uint1024CombinedHashes.ToString ().c_str () );
    //memcpy ( hash[3].begin (), uint1024CombinedHashes.begin () + 64, 64 );

    //aIntermediateHashFunctions [ 11 ] ( static_cast<const void*>(&hash[3]), 64 * 8, nullptr, static_cast<void*>(&hash[4]) );

    //-Ensuring, that blocks are in memory for memory-hard random-accesses computations.----
    /*// Around 300 blocks are checked pseudo-randomly from 3 MB memory area.
    for ( i = 0; i < I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY_CPP / 100; i ++ ) {
        iIndex = ( iIndexFromWeekChangeBlock + i * I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING ) %
            std :: min ( I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY_CPP, ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight );
        //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %" PRIu64 " , %" PRIu64 " .\n", ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight, iIndex );
        iIndex = ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight - iIndex - 1;
        hash[3].XOROperator ( * chainActive [ iIndex ] -> phashBlock, ( i % 2 ) * 32 );
        memcpy ( hash [ 2 ].begin (), hash [ 3 ].begin (), 64 );
        aIntermediateHashFunctions [ 0 ] ( hash [ 2 ].begin (), 64, nullptr, static_cast<void*>(&hash[3]) );

    } //-for

    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : %s .\n", hash[3].ToString ().c_str () );*/

    ECRYPT_ctx structECRYPT_ctx;

    unsigned char aMemoryArea [ I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION ];
    //unsigned char * aMemoryArea = new unsigned char [ I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION ] ();
    memset ( aMemoryArea, 0, I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION );

    ECRYPT_keysetup ( & structECRYPT_ctx, hash[1].begin (), ECRYPT_MAXKEYSIZE, ECRYPT_MAXIVSIZE );
    ECRYPT_ivsetup ( & structECRYPT_ctx, hash[2].begin () );

    uint64_t iWriteIndex;
    
    //ECRYPT_PrintContext ( & structECRYPT_ctx );

    // Amplifying data and making random write accesses to memory.
    // Block size is 64 bytes. ECRYPT_BLOCKLENGTH .
    // Hash size is 64 bytes.
    for ( i = 0; i < I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION / ( 64 ) / 2; i ++ ) {  // / 64 // I_MEMORY_BLOCK_SIZE_TO_ENCRYPTION_COMPUTATIONS_RATIO  // 8 * 
        //iIndex = ( iIndexFromWeekChangeBlock + i * I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING ) %
        //    std :: min ( I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY_CPP, ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight );
        ////fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %" PRIu64 " , %" PRIu64 " .\n", ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight, iIndex );
        //iIndex = ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight - iIndex - 1;
        //hash[3].XOROperator ( * chainActive [ iIndex ] -> phashBlock, ( i % 2 ) * 32 );
        //memcpy ( hash [ 2 ].begin (), hash [ 3 ].begin (), 64 );
        //aIntermediateHashFunctions [ 0 ] ( hash [ 2 ].begin (), 64, nullptr, static_cast<void*>(&hash[3]) );

        iWriteIndex =
            (
                //  % I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION here is to prevent integer overflow
                // on subsequent addition operation.
                //GetUint64IndexFrom512BitsKey ( hash [ 3 ].begin (), 0 ) % I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION +
                GetUint64IndexFrom512BitsKey ( uint1024CombinedHashes.begin () + 64, 0 ) % I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION +
                //* ( ( uint64_t * ) hash [ 3 ].begin () ) % I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION +
                i * I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING )
            %
            ( I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION - 8 * ECRYPT_BLOCKLENGTH );

        //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %" PRIu64 " , %" PRIu64 " , %" PRIu64 " .\n",
        //    iWriteIndex, GetUint64IndexFrom512BitsKey ( hash [ 3 ].begin (), 0 ), ( GetUint64IndexFrom512BitsKey ( hash [ 3 ].begin (), 0 ) + i * I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING ) );
        //iWriteIndex = 0;

        // From previous encryption result in memory to next encryption result in memory.
        ECRYPT_encrypt_blocks ( & structECRYPT_ctx,
            //hash [ 3 ].begin (), // const u8* plaintext  //  & aMemoryArea [ ( 0 + i * I_PRIME_NUMBER_FOR_MEMORY_HARD_HASHING ) % ( I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION - 64 ) ]
            uint1024CombinedHashes.begin () + 64,
            & ( aMemoryArea [ iWriteIndex ] ), // cyphertext ff
            1 );  // 8

        //hash [ 3 ].XOROperator ( & ( aMemoryArea [ iWriteIndex ] ) );
        uint1024CombinedHashes.XOROperator ( 64, & ( aMemoryArea [ iWriteIndex ] ) );

    } //-for
    
    //PrintMemoryArea ( ( uint64_t * ) & ( aMemoryArea [ 0 ] ), 128 );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : hash [ 3 ] : %s .\n", hash[3].ToString ().c_str () );

    // Veryfying that memory is allocated and making random read accesses to it.    
    //aIntermediateHashFunctions [ 0 ] ( & ( aMemoryArea [ 0 ] ), I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION, nullptr, static_cast<void*>(&hash[3]) );
    //memcpy ( uint1024CombinedHashes.begin () + 64, hash[3].begin (), 64 );
    //memcpy ( hash[3].begin (), uint1024CombinedHashes.begin () + 64, 64 );
    //memcpy ( uint1024CombinedHashes.begin () + 64, hash[3].begin (), 64 );
    for ( i = 0; i < I_AMOUNT_OF_BYTES_FOR_MEMORY_HARD_FUNCTION / 64; i ++ ) {
    //for ( i = 0; i < 4; i ++ ) {
        //hash [ 3 ].XOROperator ( & ( aMemoryArea [ i * 64 ] ) );
        //hash [ 3 ].XOROperator ( hash [ 0 ].begin () );
        uint1024CombinedHashes.XOROperator ( 64, & ( aMemoryArea [ i * 64 ] ) );

    } //-for

    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : hash [ 3 ] : %s .\n", hash[3].ToString ().c_str () );
    
    //uint1024 uint1024CombinedHashes;
    //memcpy ( uint1024CombinedHashes.begin (), hash [ 4 ].begin (), 64 );
    //memcpy ( uint1024CombinedHashes.begin () + 64, hash [ 3 ].begin (), 64 );
    
    //hash[4].XOROperator ( hash[3] );    

    //delete aMemoryArea;

    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : %s .\n", uint1024CombinedHashes.ToString ().c_str () );
    //assert ( false );

    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : Memory hard hashing function : %s .\n", uint1024CombinedHashes.ToString ().c_str () );

    //-Whirlpool--------------------------------------
    // keccak512
    //aIntermediateHashFunctions [ 4 ] ( static_cast<const void*>(&hash[4]), 64, nullptr, static_cast<void*>(&hash[5]) );
    aIntermediateHashFunctions [ 4 ] ( uint1024CombinedHashes.begin (), 128, nullptr, static_cast<void*>(&hash[5]) );
    //aIntermediateHashFunctions [ 12 ] ( static_cast<const void*>(&hash[4]), 64, nullptr, static_cast<void*>(&hash[5]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : %s .\n", hash[5].ToString ().c_str () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : keccak512 : %s .\n", hash[5].ToString ().c_str () );

    //-SWIFFT.----------------------------------------
    // luffa512
    aIntermediateHashFunctions [ 6 ] ( static_cast<const void*>(&hash[5]), 64, nullptr, static_cast<void*>(&hash[6]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : luffa512 : %s .\n", hash[6].ToString ().c_str () );

    /*char pSwiFFTKey [ 1025 ];
    
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
        
    aIntermediateHashFunctions [ 13 ] ( static_cast<const void*>(hash[5].begin ()), 64, pSwiFFTKey, static_cast<void*>(hash[6].begin ()) );*/

    // cubehash512
    aIntermediateHashFunctions [ 7 ] ( static_cast<const void*>(&hash[6]), 64, nullptr, static_cast<void*>(&hash[7]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : cubehash512 : %s .\n", hash[7].ToString ().c_str () );

    //-GOST 2015_Kuznechik.---------------------------
    /*iIndex = GetUint64IndexFrom512BitsKey ( hash[3].begin (), 0 );
    //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %i .\n", chainActive.Height () );
    //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %" PRIu64 " , %" PRIu64 " ,%" PRIu64 ", %i .\n", iIndex, nHeightOfPreviousBlock, iIndex % nHeightOfPreviousBlock, sizeof ( chainActive.Tip () ) );
    //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %i .\n", sizeof ( CBlockIndex ) );
    //if ( chainActive.Height () > 0 )    

    // Here is need to use height of previous block index from map.    
    iIndex = iIndex % std :: min ( I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY_CPP, ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight ); // chainActive.Height ()   ( nHeightOfPreviousBlock - 1 )
    //uint512ChainBlockData = GetBlockData ( iIndex );
    hash[0] = GetBlockData ( iIndex, ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight ); // nHeightOfPreviousBlock
    //uint512ChainBlockData = GetBlockData ( iIndex, pPrevBlockIndex -> nHeight ); // nHeightOfPreviousBlock
    //uint512ChainBlockData.SetNull ();
    //memcpy ( uint512ChainBlockData.begin (), hash [ 0 ].begin (), 64 );
    //aIntermediateEncryptionFunctions [ 0 ] ( uint512ChainBlockData.begin (), 64, uint512ChainBlockData.begin (), static_cast<void*>(&hash[7]) ); // 0 static_cast<const void*>(&hash[3]) & chainActive [ chainActive.Height () - iIndex ] -> nVersion // uint512ChainBlockData.begin ()
    //aIntermediateEncryptionFunctions [ 0 ] ( static_cast<const void*>(&hash[0]), 64, static_cast<const void*>(&hash[3]), static_cast<void*>(&hash[7]) ); // 0 static_cast<const void*>(&hash[3]) & chainActive [ chainActive.Height () - iIndex ] -> nVersion
    // Copying from 7 to 6.*/
    memcpy ( hash [ 6 ].begin (), hash [ 7 ].begin (), 64 );
    // Getting index of encryption function.
    // 10080 minutes in week. Here we take index of block, which corresponds to beginning of current week of blocks algorithm configuration.
    /*iIndexOfBlcok = ( ( CBlockIndex * ) _pPreviousBlockIndex ) -> nHeight / ( I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_MINUTES / 2 ) * ( I_ALGORITHM_RECONFIGURATION_TIME_PERIOD_IN_MINUTES / 2 );
    //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %i .\n", iIndexOfBlcok );
    iIndex = GetUint64IndexFrom512BitsKey ( chainActive [ iIndexOfBlcok ] -> phashBlock -> begin (), 0 );*/
    //iIndex = iIndexFromWeekChangeBlock % I_AMOUNT_OF_INTERMEDIATE_ENCRYPTION_FUNCTIONS;
    iIndex = ( iWeekNumber + nBits ) % I_AMOUNT_OF_INTERMEDIATE_ENCRYPTION_FUNCTIONS;
    //fprintf(stdout, "block.cpp : GetHash_SHA256AndX11 () : %i .\n", iIndex );
    //iIndex = 0;
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : aIntermediateEncryptionFunctions 1 : Data : %s .\n", hash[6].ToString ().c_str () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : aIntermediateEncryptionFunctions 1 : Key : %s .\n", hash[0].ToString ().c_str () );
    aIntermediateEncryptionFunctions [ iIndex ] ( static_cast<const void*>(&hash[6]), 64, static_cast<const void*>(&hash[0]), static_cast<void*>(&hash[7]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : %s , %s .\n", uint512ChainBlockData.ToString ().c_str (), hash [ 7 ].ToString ().c_str () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : aIntermediateEncryptionFunctions 1 : %i , %s .\n", iIndex, hash[7].ToString ().c_str () );

    // shavite512
    aIntermediateHashFunctions [ 8 ] ( static_cast<const void*>(&hash[7]), 64, nullptr, static_cast<void*>(&hash[8]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : shavite512 : %s .\n", hash[8].ToString ().c_str () );

    //---ThreeFish.----------------------------------
    // Copying from 8 to 7.
    //memcpy ( hash [ 7 ].begin (), hash [ 8 ].begin (), 64 );
    //aIntermediateEncryptionFunctions [ 1 ] ( static_cast<const void*>(&hash[7]), 64, static_cast<const void*>(&hash[4]), static_cast<void*>(&hash[8]) );

    // simd512
    //aIntermediateHashFunctions [ 9 ] ( static_cast<const void*>(&hash[8]), 64, nullptr, static_cast<void*>(&hash[9]) );
    aIntermediateHashFunctions [ 9 ] ( static_cast<const void*>(&hash[8]), 64, nullptr, uint512AdditionalHash.begin () );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : simd512 : %s .\n", uint512AdditionalHash.ToString ().c_str () );

    //iIndex = ( iIndexFromWeekChangeBlock + 10 ) % I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS; nBits
    iIndex = ( iWeekNumber + nBits + 10 ) % I_AMOUNT_OF_INTERMEDIATE_HASH_FUNCTIONS;
    //memcpy ( hash [ 8 ].begin (), hash [ 9 ].begin (), 64 );
    //aIntermediateHashFunctions [ iIndex ] ( static_cast<const void*>(&hash[8]), 64, nullptr, static_cast<void*>(&hash[9]) );
    aIntermediateHashFunctions [ iIndex ] ( uint512AdditionalHash.begin (), 64, nullptr, static_cast<void*>(&hash[9]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : aIntermediateHashFunctions 2 : %s .\n", hash[9].ToString ().c_str () );

    //---Camellia.-----------------------------------    
    //memcpy ( hash [ 8 ].begin (), hash [ 9 ].begin (), 64 );    
    //aIntermediateEncryptionFunctions [ 2 ] ( static_cast<const void*>(&hash[8]), 64, static_cast<const void*>(&hash[5]), static_cast<void*>(&hash[9]) );

    // echo512
    aIntermediateHashFunctions [ 10 ] ( static_cast<const void*>(&hash[9]), 64, nullptr, static_cast<void*>(&hash[10]) );
    //fprintf(stdout, "hash.cpp : GetHash_SHA256AndX11 () : echo512 : %s .\n", hash[10].ToString ().c_str () );



    return hash[10].trim256();

} // Hash(HashX11