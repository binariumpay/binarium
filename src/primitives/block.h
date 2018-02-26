// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include "hash.h"
#include "primitives/transaction.h"
//#include "validation.h"
#include "serialize.h"
#include "uint256.h"



#define I_AMOUNT_OF_HASH_FUNCTIONS 2

#define I_MAX_AMOUNT_OF_BLOCKS_IN_MEMORY 1000



//typedef uint256 ( * TPHashFunction ) ();



//extern CChain chainActive;



//---Utility functions.---------------------------------------------------
inline uint64_t GetUint64IndexFrom512BitsKey ( const void * _pKey, int pos );



/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    //uint64_t nHeightOfPreviousBlock;

    uint256 ( CBlockHeader::* aHashFunctions [ I_AMOUNT_OF_HASH_FUNCTIONS ] ) ( void * ) const;
    //void * m_pPreviousBlockIndex = nullptr;

    CBlockHeader()
    {
        SetNull();

        aHashFunctions [ 0 ] = & CBlockHeader::GetHash_X11;
        aHashFunctions [ 1 ] = & CBlockHeader::GetHash_SHA256AndX11;

    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        //READWRITE(nHeightOfPreviousBlock);
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        //nHeightOfPreviousBlock = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;
    uint256 GetGenesisInitializationHash() const;

    uint256 GetHash_X11( void * _pPreviousBlockIndex ) const;
    uint256 GetHash_SHA256AndX11 ( void * _pPreviousBlockIndex ) const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // memory only
    mutable CTxOut txoutMasternode; // masternode payment
    mutable std::vector<CTxOut> voutSuperblock; // superblock payment
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        txoutMasternode = CTxOut();
        voutSuperblock.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        //block.nHeightOfPreviousBlock        = nHeightOfPreviousBlock;
        return block;
    }

    std::string ToString() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};



//uint512 GetBlockData ( uint64_t _iIndex );



/*template<typename T1>
inline uint256 HashX11_Generator_Blocks(const T1 pbegin, const T1 pend) // , uint32_t _iAlgorithmDefiningValue

{
    static unsigned char pblank[1];

    uint512 hash[11];

    uint512 uint512ChainBlockData;

    uint64_t iIndex;



    // blake512
    aIntermediateHashFunctions [ 0 ] ( (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]), nullptr, hash[0].begin () );

    // bmw512
    aIntermediateHashFunctions [ 1 ] ( static_cast<const void*>(&hash[0]), 64, nullptr, static_cast<void*>(&hash[1]) );

    // groestl512
    aIntermediateHashFunctions [ 2 ] ( static_cast<const void*>(&hash[1]), 64, nullptr, static_cast<void*>(&hash[2]) );

    // skein512
    aIntermediateHashFunctions [ 5 ] ( static_cast<const void*>(&hash[2]), 64, nullptr, static_cast<void*>(&hash[3]) );

    //-Streebog.--------------------------------------
    // jh512
    aIntermediateHashFunctions [ 11 ] ( static_cast<const void*>(&hash[3]), 64 * 8, nullptr, static_cast<void*>(&hash[4]) );

    //-Whirlpool--------------------------------------
    // keccak512
    aIntermediateHashFunctions [ 12 ] ( static_cast<const void*>(&hash[4]), 64, nullptr, static_cast<void*>(&hash[5]) );

    //-SWIFFT.----------------------------------------
    // luffa512    
    
    char pSwiFFTKey [ 1025 ];
    
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
        
    aIntermediateHashFunctions [ 13 ] ( static_cast<const void*>(hash[5].begin ()), 64, pSwiFFTKey, static_cast<void*>(hash[6].begin ()) );

    // cubehash512
    aIntermediateHashFunctions [ 7 ] ( static_cast<const void*>(&hash[6]), 64, nullptr, static_cast<void*>(&hash[7]) );

    //-GOST 2015_Kuznechik.---------------------------
    iIndex = GetUint64IndexFrom512BitsKey ( hash[3].begin (), 0 );
    //fprintf(stdout, "block.h : GetHash_SHA256AndX11 () : %" PRIu64 " , %i ,%" PRIu64 ", %i .\n", iIndex, chainActive.Height (), iIndex % chainActive.Height (), sizeof ( chainActive.Tip () ) );
    //iIndex = iIndex % chainActive.Height ();
    uint512ChainBlockData = GetBlockData ( iIndex );
    //aIntermediateEncryptionFunctions [ 0 ] ( uint512ChainBlockData.begin (), 64, uint512ChainBlockData.begin (), static_cast<void*>(&hash[7]) ); // 0 static_cast<const void*>(&hash[3]) & chainActive [ chainActive.Height () - iIndex ] -> nVersion
    aIntermediateEncryptionFunctions [ 0 ] ( static_cast<const void*>(&hash[0]), 64, static_cast<const void*>(&hash[3]), static_cast<void*>(&hash[7]) ); // 0 static_cast<const void*>(&hash[3]) & chainActive [ chainActive.Height () - iIndex ] -> nVersion

    // shavite512
    aIntermediateHashFunctions [ 8 ] ( static_cast<const void*>(&hash[7]), 64, nullptr, static_cast<void*>(&hash[8]) );

    //---ThreeFish.----------------------------------
    // Copying from 8 to 7.
    memcpy ( hash [ 7 ].begin (), hash [ 8 ].begin (), 64 );
    aIntermediateEncryptionFunctions [ 1 ] ( static_cast<const void*>(&hash[7]), 64, static_cast<const void*>(&hash[4]), static_cast<void*>(&hash[8]) );

    // simd512
    aIntermediateHashFunctions [ 9 ] ( static_cast<const void*>(&hash[8]), 64, nullptr, static_cast<void*>(&hash[9]) );

    //---Camellia.-----------------------------------    
    memcpy ( hash [ 8 ].begin (), hash [ 9 ].begin (), 64 );    
    aIntermediateEncryptionFunctions [ 2 ] ( static_cast<const void*>(&hash[8]), 64, static_cast<const void*>(&hash[5]), static_cast<void*>(&hash[9]) );

    // echo512
    aIntermediateHashFunctions [ 10 ] ( static_cast<const void*>(&hash[9]), 64, nullptr, static_cast<void*>(&hash[10]) );

    return hash[10].trim256();
}*/

#endif // BITCOIN_PRIMITIVES_BLOCK_H