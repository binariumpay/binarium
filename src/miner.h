// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include "primitives/block.h"

#include <stdint.h>

#include <boost/atomic.hpp>

class CBlockIndex;
class CChainParams;
class CConnman;
class CReserveKey;
class CScript;
class CWallet;
namespace Consensus { struct Params; };

static const bool DEFAULT_GENERATE = false;
static const int DEFAULT_GENERATE_THREADS = 1;
static const int I_MAX_GENERATE_THREADS = 64;

struct THashRateCounter {
    int64_t iAmountOfHashes;
    int64_t iStartTimeInMicroSeconds;
    int64_t iCompletionTimeInMicroSeconds;
    float fHashRate;
};

///*static*/ bool g_bGenerateBlocks = false;
//bool i = 1;
//std :: unique_ptr < bool > g_p_bGenerateBlocks = std :: make_unique < bool > ( g_bGenerateBlocks );

extern bool g_bNotifyIsMiningEnabled;

extern boost::atomic < THashRateCounter > aHashRateCounters [ I_MAX_GENERATE_THREADS * 2 ];

static const bool DEFAULT_PRINTPRIORITY = false;

extern int g_iAmountOfMiningThreads;

struct CBlockTemplate
{
    CBlock block;
    std::vector<CAmount> vTxFees;
    std::vector<int64_t> vTxSigOps;
};

/** Run the miner threads */
void GenerateBitcoins(bool fGenerate, int nThreads, const CChainParams& chainparams, CConnman& connman);
/** Generate a new block, without valid proof-of-work */
CBlockTemplate* CreateNewBlock(const CChainParams& chainparams, const CScript& scriptPubKeyIn);
/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pblock, const CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
int64_t UpdateTime(CBlockHeader* pblock, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev);

extern bool g_bIsPoolMiningEnabled;
char * StartPoolMining ( bool _bStart,
    std :: string _sPoolURL = "",
    std :: string _sPoolUser = "",
    std :: string _sPoolUserPassword = "",
    std :: string _sPoolMiningAlgorithm = "",
    int _iAmountOfPoolMiningThreads = 0,
    int _iPoolMinerCPUPriority = 0,
    int _iPoolMinerCPUAffinity = -1 );
double Wallet_PoolMiner_GetHashesRate ();

#endif // BITCOIN_MINER_H
