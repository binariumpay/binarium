// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "coins.h"
#include "consensus/consensus.h"
#include "consensus/merkle.h"
#include "consensus/validation.h"
#include "hash.h"
#include "validation.h"
#include "net.h"
#include "policy/policy.h"
#include "pow.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "timedata.h"
#include "txmempool.h"
#include "util.h"
#include "utilmoneystr.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "validationinterface.h"
#include "utiltime.h"

//#include "bitcoingui.h"

#include <inttypes.h>

#if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
    #include <dlfcn.h>
#elif defined(_WIN32) || defined(WIN32)
    //#include "cpu-miner.h"
#else

#endif

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>
//#ifdef 
//#include <boost/atomic.hpp>
//#elif
//#include <atomic>
//#endif
#include <queue>

//#include <QApplication>
//#include <QString>
//#include <QtCore/QSysInfo>
//#include <QTimer>

using namespace std;



//extern bool g_bIsQtInterfaceEnabled;
//#ifdef BITCOIN_QT_BITCOINGUI_H
//extern void Notify_MiningIsEnabled ( bool _bIsEnabled );
//#endif
//extern bool g_bNotifyIsMiningEnabled;
bool g_bNotifyIsMiningEnabled = false;
//std :: atomic_bool g_bNotifyIsMiningEnabled = false;

//std :: atomic < THashRateCounter > aHashRateCounters [ I_MAX_GENERATE_THREADS * 2 ];
boost::atomic < THashRateCounter > aHashRateCounters [ I_MAX_GENERATE_THREADS * 2 ];

int g_iAmountOfMiningThreads = -2;
//extern int g_iAmountOfMiningThreads;
int g_iPreviousAmountOfMiningThreads;



char g_sErrorMessage [ 512 ];

bool g_bIsPoolMiningEnabled = false;
#if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
    void * g_pCPUMinerSharedLibrary = nullptr;
#elif defined(_WIN32) || defined(WIN32)
    HMODULE g_hModule_CPUMinerSharedLibrary = 0;
#else

#endif
int ( * g_pfPoolMinerMain ) ( int, char **, char *,
    const char *,
    const char *,
    const char *,
    const char *,
    int, int, int );
void ( * g_pfPoolMiner_StopMining ) ();
double ( * g_pfPoolMiner_GetHashesRate ) ();
bool ( * g_pfPoolMiner_IsReadyForNewStart ) ();

char g_sCPUMinerProgram [ 64 ];
char g_sPoolURL [ 256 ];
char g_sUserLogin [ 256 ];
char g_sUserPassword [ 256 ];
char g_sAlgorithm [ 256 ];
char g_sAmountOfthreads [ 32 ];
char g_sCPUPriority [ 32 ];
char g_sCPUAffinity [ 32 ];
char * g_pArgV [ 8 ] = { g_sCPUMinerProgram, g_sPoolURL, g_sUserLogin, g_sUserPassword, g_sAlgorithm, g_sAmountOfthreads, g_sCPUPriority, g_sCPUAffinity };



//////////////////////////////////////////////////////////////////////////////
//
// BinariumMiner
//

//
// Unconfirmed transactions in the memory pool often depend on other
// transactions in the memory pool. When we select transactions from the
// pool, we select by highest priority or fee rate, so we might consider
// transactions that depend on transactions that aren't yet in the block.

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;

class ScoreCompare
{
public:
    ScoreCompare() {}

    bool operator()(const CTxMemPool::txiter a, const CTxMemPool::txiter b)
    {
        return CompareTxMemPoolEntryByScore()(*b,*a); // Convert to less than
    }
};

int64_t UpdateTime(CBlockHeader* pblock, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{
    int64_t nOldTime = pblock->nTime;
    int64_t nNewTime = std::max(pindexPrev->GetMedianTimePast()+1, GetAdjustedTime());

    if (nOldTime < nNewTime)
        pblock->nTime = nNewTime;

    // Updating time can change work required on testnet:
    if (consensusParams.fPowAllowMinDifficultyBlocks)
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock, consensusParams);

    return nNewTime - nOldTime;
}

CBlockTemplate* CreateNewBlock(const CChainParams& chainparams, const CScript& scriptPubKeyIn)
{
    // Create new block
    std::unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if(!pblocktemplate.get())
        return NULL;
    CBlock *pblock = &pblocktemplate->block; // pointer for convenience

    // Create coinbase tx
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKeyIn;

    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    // Limit to between 1K and MAX_BLOCK_SIZE-1K for sanity:
    nBlockMaxSize = std::max((unsigned int)1000, std::min((unsigned int)(MaxBlockSize(fDIP0001ActiveAtTip)-1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    // Collect memory pool transactions into the block
    CTxMemPool::setEntries inBlock;
    CTxMemPool::setEntries waitSet;

    // This vector will be sorted into a priority queue:
    vector<TxCoinAgePriority> vecPriority;
    TxCoinAgePriorityCompare pricomparer;
    std::map<CTxMemPool::txiter, double, CTxMemPool::CompareIteratorByHash> waitPriMap;
    typedef std::map<CTxMemPool::txiter, double, CTxMemPool::CompareIteratorByHash>::iterator waitPriIter;
    double actualPriority = -1;

    std::priority_queue<CTxMemPool::txiter, std::vector<CTxMemPool::txiter>, ScoreCompare> clearedTxs;
    bool fPrintPriority = GetBoolArg("-printpriority", DEFAULT_PRINTPRIORITY);
    uint64_t nBlockSize = 1000;
    uint64_t nBlockTx = 0;
    unsigned int nBlockSigOps = 100;
    int lastFewTxs = 0;
    CAmount nFees = 0;

    {
        LOCK(cs_main);

        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        pblock->nTime = GetAdjustedTime();
        const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();

        // Add our coinbase tx as first transaction
        pblock->vtx.push_back(txNew);
        pblocktemplate->vTxFees.push_back(-1); // updated at end
        pblocktemplate->vTxSigOps.push_back(-1); // updated at end
        pblock->nVersion = ComputeBlockVersion(pindexPrev, chainparams.GetConsensus());
        // -regtest only: allow overriding block.nVersion with
        // -blockversion=N to test forking scenarios
        if (chainparams.MineBlocksOnDemand())
            pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

        int64_t nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST)
                                ? nMedianTimePast
                                : pblock->GetBlockTime();

        {
            LOCK(mempool.cs);

            bool fPriorityBlock = nBlockPrioritySize > 0;
            if (fPriorityBlock) {
                vecPriority.reserve(mempool.mapTx.size());
                for (CTxMemPool::indexed_transaction_set::iterator mi = mempool.mapTx.begin();
                     mi != mempool.mapTx.end(); ++mi)
                {
                    double dPriority = mi->GetPriority(nHeight);
                    CAmount dummy;
                    mempool.ApplyDeltas(mi->GetTx().GetHash(), dPriority, dummy);
                    vecPriority.push_back(TxCoinAgePriority(dPriority, mi));
                }
                std::make_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
            }

            CTxMemPool::indexed_transaction_set::nth_index<3>::type::iterator mi = mempool.mapTx.get<3>().begin();
            CTxMemPool::txiter iter;

            while (mi != mempool.mapTx.get<3>().end() || !clearedTxs.empty())
            {
                bool priorityTx = false;
                if (fPriorityBlock && !vecPriority.empty()) { // add a tx from priority queue to fill the blockprioritysize
                    priorityTx = true;
                    iter = vecPriority.front().second;
                    actualPriority = vecPriority.front().first;
                    std::pop_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
                    vecPriority.pop_back();
                }
                else if (clearedTxs.empty()) { // add tx with next highest score
                    iter = mempool.mapTx.project<0>(mi);
                    mi++;
                }
                else {  // try to add a previously postponed child tx
                    iter = clearedTxs.top();
                    clearedTxs.pop();
                }

                if (inBlock.count(iter))
                    continue; // could have been added to the priorityBlock

                const CTransaction& tx = iter->GetTx();

                bool fOrphan = false;
                BOOST_FOREACH(CTxMemPool::txiter parent, mempool.GetMemPoolParents(iter))
                {
                    if (!inBlock.count(parent)) {
                        fOrphan = true;
                        break;
                    }
                }
                if (fOrphan) {
                    if (priorityTx)
                        waitPriMap.insert(std::make_pair(iter,actualPriority));
                    else
                        waitSet.insert(iter);
                    continue;
                }

                unsigned int nTxSize = iter->GetTxSize();
                if (fPriorityBlock &&
                    (nBlockSize + nTxSize >= nBlockPrioritySize || !AllowFree(actualPriority))) {
                    fPriorityBlock = false;
                    waitPriMap.clear();
                }
                if (!priorityTx &&
                    (iter->GetModifiedFee() < ::minRelayTxFee.GetFee(nTxSize) && nBlockSize >= nBlockMinSize)) {
                    break;
                }
                if (nBlockSize + nTxSize >= nBlockMaxSize) {
                    if (nBlockSize >  nBlockMaxSize - 100 || lastFewTxs > 50) {
                        break;
                    }
                    // Once we're within 1000 bytes of a full block, only look at 50 more txs
                    // to try to fill the remaining space.
                    if (nBlockSize > nBlockMaxSize - 1000) {
                        lastFewTxs++;
                    }
                    continue;
                }

                if (!IsFinalTx(tx, nHeight, nLockTimeCutoff))
                    continue;

                unsigned int nTxSigOps = iter->GetSigOpCount();
                unsigned int nMaxBlockSigOps = MaxBlockSigOps(fDIP0001ActiveAtTip);
                if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps) {
                    if (nBlockSigOps > nMaxBlockSigOps - 2) {
                        break;
                    }
                    continue;
                }

                CAmount nTxFees = iter->GetFee();
                // Added
                pblock->vtx.push_back(tx);
                pblocktemplate->vTxFees.push_back(nTxFees);
                pblocktemplate->vTxSigOps.push_back(nTxSigOps);
                nBlockSize += nTxSize;
                ++nBlockTx;
                nBlockSigOps += nTxSigOps;
                nFees += nTxFees;

                if (fPrintPriority)
                {
                    double dPriority = iter->GetPriority(nHeight);
                    CAmount dummy;
                    mempool.ApplyDeltas(tx.GetHash(), dPriority, dummy);
                    LogPrintf("priority %.1f fee %s txid %s\n",
                              dPriority , CFeeRate(iter->GetModifiedFee(), nTxSize).ToString(), tx.GetHash().ToString());
                }

                inBlock.insert(iter);

                // Add transactions that depend on this one to the priority queue
                BOOST_FOREACH(CTxMemPool::txiter child, mempool.GetMemPoolChildren(iter))
                {
                    if (fPriorityBlock) {
                        waitPriIter wpiter = waitPriMap.find(child);
                        if (wpiter != waitPriMap.end()) {
                            vecPriority.push_back(TxCoinAgePriority(wpiter->second,child));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
                            waitPriMap.erase(wpiter);
                        }
                    }
                    else {
                        if (waitSet.count(child)) {
                            clearedTxs.push(child);
                            waitSet.erase(child);
                        }
                    }
                }
            }

        }

        // NOTE: unlike in bitcoin, we need to pass PREVIOUS block height here
        CAmount blockReward = nFees + GetBlockSubsidy(pindexPrev->nBits, pindexPrev->nHeight, Params().GetConsensus());

        // Compute regular coinbase transaction.
        txNew.vout[0].nValue = blockReward;
        txNew.vin[0].scriptSig = CScript() << nHeight << OP_0;

        // Update coinbase transaction with additional info about masternode and governance payments,
        // get some info back to pass to getblocktemplate
        FillBlockPayments(txNew, nHeight, blockReward, pblock->txoutMasternode, pblock->voutSuperblock);
        // LogPrintf("CreateNewBlock -- nBlockHeight %d blockReward %lld txoutMasternode %s txNew %s",
        //             nHeight, blockReward, pblock->txoutMasternode.ToString(), txNew.ToString());

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        LogPrintf("CreateNewBlock(): total size %u txs: %u fees: %ld sigops %d\n", nBlockSize, nBlockTx, nFees, nBlockSigOps);

        // Update block coinbase
        pblock->vtx[0] = txNew;
        pblocktemplate->vTxFees[0] = -nFees;

        // Fill in header
        pblock->hashPrevBlock  = pindexPrev->GetBlockHash();
        UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev);
        pblock->nBits          = GetNextWorkRequired(pindexPrev, pblock, chainparams.GetConsensus());
        //pblock->nNonce         = 0;
        //fprintf(stdout, "miner.cpp : CreateNewBlock () : machineHostName = %s.\n", QSysInfo :: machineHostName ().toUtf8 ().data () );
        //fprintf(stdout, "miner.cpp : CreateNewBlock () : kernelType = %s.\n", QSysInfo :: kernelType ().toUtf8 ().data () );
        //fprintf(stdout, "miner.cpp : CreateNewBlock () : kernelVersion = %s.\n", QSysInfo :: kernelVersion ().toUtf8 ().data () );
        //fprintf(stdout, "miner.cpp : CreateNewBlock () : prettyProductName = %s.\n", QSysInfo :: prettyProductName ().toUtf8 ().data () );
        //boost :: filesystem :: path full_path ( boost::filesystem::current_path() );
        //fprintf(stdout, "miner.cpp : CreateNewBlock () : prettyProductName = %s.\n", full_path.string () );
        //pblock->nNonce         = ( get_uptime () + GetTimeMicros () % 100000000 ) % 100000000;
        pblock->nNonce         = ( get_uptime () % 86400 ) * 29 + ( GetTimeMicros () % 1000 ) * 131071;
        fprintf(stdout, "miner.cpp : CreateNewBlock () : nNonce = %i.\n", pblock->nNonce );
        //pblock->nHeightOfPreviousBlock = pindexPrev->nHeightOfPreviousBlock + 1;
        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (!TestBlockValidity(state, chainparams, *pblock, pindexPrev, false, false)) {
            throw std::runtime_error(strprintf("%s: TestBlockValidity failed: %s", __func__, FormatStateMessage(state)));
        }
    }

    return pblocktemplate.release();
}

void IncrementExtraNonce(CBlock* pblock, const CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock)
    {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight+1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}

//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//

// ***TODO*** ScanHash is not yet used in Binarium
//
// ScanHash scans nonces looking for a hash with at least some zero bits.
// The nonce is usually preserved between calls, but periodically or if the
// nonce is 0xffff0000 or above, the block is rebuilt and nNonce starts over at
// zero.
//
//bool static ScanHash(const CBlockHeader *pblock, uint32_t& nNonce, uint256 *phash)
//{
//    // Write the first 76 bytes of the block header to a double-SHA256 state.
//    CHash256 hasher;
//    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
//    ss << *pblock;
//    assert(ss.size() == 80);
//    hasher.Write((unsigned char*)&ss[0], 76);

//    while (true) {
//        nNonce++;

//        // Write the last 4 bytes of the block header (the nonce) to a copy of
//        // the double-SHA256 state, and compute the result.
//        CHash256(hasher).Write((unsigned char*)&nNonce, 4).Finalize((unsigned char*)phash);

//        // Return the nonce if the hash has at least some zero bits,
//        // caller will check if it has enough to reach the target
//        if (((uint16_t*)phash)[15] == 0)
//            return true;

//        // If nothing found after trying for a while, return -1
//        if ((nNonce & 0xfff) == 0)
//            return false;
//    }
//}

static bool ProcessBlockFound(const CBlock* pblock, const CChainParams& chainparams)
{
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("ProcessBlockFound -- generated block is stale");
    }

    // Inform about the new block
    GetMainSignals().BlockFound(pblock->GetHash());

    // Process this block the same as if we had received it from another node
    if (!ProcessNewBlock(chainparams, pblock, true, NULL, NULL))
        return error("ProcessBlockFound -- ProcessNewBlock() failed, block not accepted");

    return true;
}

// ***TODO*** that part changed in bitcoin, we are using a mix with old one here for now
void static BitcoinMiner(const CChainParams& chainparams, CConnman& connman, const int _iIndex )   // const int & _iIndex
{
    int iIndex = _iIndex;

    fprintf(stdout, "miner.cpp : BitcoinMiner () : Miner thread started : %i.\n", iIndex );

    LogPrintf("BinariumMiner -- started\n");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("binarium-miner");

    unsigned int nExtraNonce = 0;

    boost::shared_ptr<CReserveScript> coinbaseScript;
    GetMainSignals().ScriptForMining(coinbaseScript);

    try {
        // Throw an error if no script was provided.  This can happen
        // due to some internal error but also if the keypool is empty.
        // In the latter case, already the pointer is NULL.
        if (!coinbaseScript || coinbaseScript->reserveScript.empty())
            throw std::runtime_error("No coinbase script available (mining requires a wallet)");

        while (true) {
            LogPrintf("BinariumMiner : Starting miner cycle step.\n");

            THashRateCounter structureHashRateCounter;

            structureHashRateCounter.iAmountOfHashes = 0;
            structureHashRateCounter.iStartTimeInMicroSeconds = GetTimeMicros ();
            structureHashRateCounter.iCompletionTimeInMicroSeconds = 0;
            aHashRateCounters [ iIndex ].store ( structureHashRateCounter );

            if (chainparams.MiningRequiresPeers()) {
                // Busy-wait for the network to come online so we don't waste time mining
                // on an obsolete chain. In regtest mode we expect to fly solo.
                do {                    
                    bool fvNodesEmpty = connman.GetNodeCount(CConnman::CONNECTIONS_ALL) == 0;
                    LogPrintf("BinariumMiner : iterating in \"if (chainparams.MiningRequiresPeers())\" cycle. : %i, %i, %i.\n",
                        !fvNodesEmpty, !IsInitialBlockDownload(), masternodeSync.IsSynced());
                    if (!fvNodesEmpty && !IsInitialBlockDownload() && masternodeSync.IsSynced())
                        break;
                    MilliSleep(1000);
                } while (true);
            }

            //#ifdef BITCOIN_QT_BITCOINGUI_H
            //fprintf(stdout, "miner.cpp : BitcoinMiner () : Notifing, that miner is enabled.\n" );
            //Notify_MiningIsEnabled ( true );
            //#endif
            g_bNotifyIsMiningEnabled = true;

            //
            // Create new block
            //
            unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrev = chainActive.Tip();
            if(!pindexPrev) {
                LogPrintf("BinariumMiner : if(!pindexPrev).\n");
                break;

            } //-if

            std::unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(chainparams, coinbaseScript->reserveScript));
            if (!pblocktemplate.get())
            {
                LogPrintf("BinariumMiner -- Keypool ran out, please call keypoolrefill before restarting the mining thread\n");
                return;
            }
            CBlock *pblock = &pblocktemplate->block;
            IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

            LogPrintf("BinariumMiner -- Running miner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            //
            // Search
            //
            int64_t nStart = GetTime();
            arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);
            unsigned int nHashesDoneInThisMinerCycleStep = 0;
            while (true)
            {
                unsigned int nHashesDone = 0;

                uint256 hash;
                while (true)
                {
                    hash = pblock->GetHash();
                    if (UintToArith256(hash) <= hashTarget)
                    {
                        // Found a solution
                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        LogPrintf("BinariumMiner:\n  proof-of-work found\n  hash: %s\n  target: %s\n", hash.GetHex(), hashTarget.GetHex());
                        ProcessBlockFound(pblock, chainparams);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);
                        coinbaseScript->KeepScript();

                        //assert ( false );

                        // In regression test mode, stop mining after a block is found. This
                        // allows developers to controllably generate a block on demand.
                        if (chainparams.MineBlocksOnDemand())
                            throw boost::thread_interrupted();

                        break;
                    }
                    pblock->nNonce += 1;
                    //pblock->nNonce += GetTimeMicros () % 10 * 19;
                    //pblock->nNonce         = ( get_uptime () + GetTimeMicros () % 100000000 ) % 100000000;
                    //fprintf(stdout, "miner.cpp : BitcoinMiner () : %i : nNonce = %i.\n", nHashesDone, pblock->nNonce );
                    nHashesDone += 1;
                    nHashesDoneInThisMinerCycleStep = nHashesDoneInThisMinerCycleStep + 1;
                    //aHashRateCounters [ iIndex ].iAmountOfHashes = aHashRateCounters [ iIndex ].iAmountOfHashes + 1;
                    //if ((pblock->nNonce & 0xFF) == 0)
                    if ((nHashesDone & 0xFF) == 0)
                        break;

                } //-while

                structureHashRateCounter = aHashRateCounters [ iIndex ].load ();
                structureHashRateCounter.iCompletionTimeInMicroSeconds = GetTimeMicros ();
                structureHashRateCounter.fHashRate = float ( nHashesDoneInThisMinerCycleStep ) /
                    ( float ( structureHashRateCounter.iCompletionTimeInMicroSeconds - structureHashRateCounter.iStartTimeInMicroSeconds ) * 0.000001f );
                aHashRateCounters [ iIndex ].store ( structureHashRateCounter );

                //fprintf(stdout, "miner.cpp : BitcoinMiner () : %i, %i, %" PRIu64 ", %f.\n",
                //    iIndex, nHashesDoneInThisMinerCycleStep, structureHashRateCounter.iCompletionTimeInMicroSeconds - structureHashRateCounter.iStartTimeInMicroSeconds,
                //    structureHashRateCounter.fHashRate );

                // Check for stop or if block needs to be rebuilt
                boost::this_thread::interruption_point();
                // Regtest mode doesn't require peers
                if (connman.GetNodeCount(CConnman::CONNECTIONS_ALL) == 0 && chainparams.MiningRequiresPeers())
                    break;
                //if (pblock->nNonce >= 0xffff0000)
                if ( nHashesDoneInThisMinerCycleStep >= 0xffff0000 )
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;

                // Update nTime every few seconds
                if (UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev) < 0)
                    break; // Recreate the block if the clock has run backwards,
                           // so that we can use the correct time.
                if (chainparams.GetConsensus().fPowAllowMinDifficultyBlocks)
                {
                    // Changing pblock->nTime can change work required on testnet:
                    hashTarget.SetCompact(pblock->nBits);
                }

            } //-while

            //#ifdef BITCOIN_QT_BITCOINGUI_H
            //Notify_MiningIsEnabled ( false );
            //#endif
            //g_bNotifyIsMiningEnabled = false;

            /*structureHashRateCounter = aHashRateCounters [ iIndex ].load ();
            structureHashRateCounter.iCompletionTimeInMicroSeconds = GetTimeMicros ();
            structureHashRateCounter.fHashRate = float ( nHashesDoneInThisMinerCycleStep ) /
                ( float ( structureHashRateCounter.iCompletionTimeInMicroSeconds - structureHashRateCounter.iStartTimeInMicroSeconds ) * 0.000001f );
            aHashRateCounters [ iIndex ].store ( structureHashRateCounter );*/

            LogPrintf("BinariumMiner : Completing mining cycle step.\n\n");

        } //-while

    } //-try

    catch (const boost::thread_interrupted&)
    {
        LogPrintf("BinariumMiner -- terminated\n");
        //#ifdef BITCOIN_QT_BITCOINGUI_H
        //Notify_MiningIsEnabled ( false );
        //#endif
        //g_bNotifyIsMiningEnabled = false;
        throw;
    }
    catch (const std::runtime_error &e)
    {
        //#ifdef BITCOIN_QT_BITCOINGUI_H
        //Notify_MiningIsEnabled ( false );
        //#endif
        //g_bNotifyIsMiningEnabled = false;
        LogPrintf("BinariumMiner -- runtime error: %s\n", e.what());
        return;
    }
}

void GenerateBitcoins(bool fGenerate, int nThreads, const CChainParams& chainparams, CConnman& connman)
{
    static boost::thread_group* minerThreads = NULL;
    int i;

    if (nThreads < 0)
        nThreads = GetNumCores();

    if (minerThreads != NULL)
    {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    //g_bNotifyIsMiningEnabled = true;

    THashRateCounter structureHashRateCounter;
    structureHashRateCounter.fHashRate = 0.0f;

    for ( i = 0; i < I_MAX_GENERATE_THREADS * 2; i ++ ) {
        aHashRateCounters [ i ].store ( structureHashRateCounter );

    } //-for

    g_iPreviousAmountOfMiningThreads = g_iAmountOfMiningThreads;
    g_iAmountOfMiningThreads = fGenerate ? nThreads : 0;

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();
    for ( i = 0; i < nThreads; i++) {
        //minerThreads->create_thread(boost::bind(&BitcoinMiner, boost::cref(chainparams), boost::ref(connman), boost::cref(i) ) );
        minerThreads->create_thread(boost::bind(&BitcoinMiner, boost::cref(chainparams), boost::ref(connman), i ) );
    }

}



std::string HelpExampleCli2(const std::string& methodname, const std::string& args)
{
    return "> binarium-cli " + methodname + " " + args + "\n";
}

std::string HelpExampleRpc2(const std::string& methodname, const std::string& args)
{
    return "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", "
        "\"method\": \"" + methodname + "\", \"params\": [" + args + "] }' -H 'content-type: text/plain;' http://127.0.0.1:9998/\n";
}

UniValue GetClientHashesPerSecond () {
    THashRateCounter structureHashRateCounter;
    float fHashRateSum = 0.0f;
    int i = 0;

    for ( i = 0; i < g_iAmountOfMiningThreads; i ++ ) {   // I_MAX_GENERATE_THREADS * 2
        structureHashRateCounter = aHashRateCounters [ i ].load ();
        //if ( structureHashRateCounter.fHashRate > 0.0f ) {
            fHashRateSum = fHashRateSum + structureHashRateCounter.fHashRate;
        //} //-if

    } //-for

    return fHashRateSum + Wallet_PoolMiner_GetHashesRate ();
}

UniValue get_client_hashes_per_second (const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
            "get_client_hashes_per_second ()\n"
            "\nReturns the estimated client hashes per second.\n"
            "\nArguments:\n"
            "\nResult:\n"
            "x             (numeric) Hashes per second estimated\n"
            "\nExamples:\n"
            + HelpExampleCli2("get_client_hashes_per_second", "")
            + HelpExampleRpc2("get_client_hashes_per_second", "")
       );

    LOCK(cs_main);
    return GetClientHashesPerSecond ();
}



char * StartPoolMining ( bool _bStart,
    std :: string _sPoolURL,
    std :: string _sPoolUser,
    std :: string _sPoolUserPassword,
    std :: string _sPoolMiningAlgorithm,
    int _iAmountOfPoolMiningThreads,
    int _iPoolMinerCPUPriority,
    int _iPoolMinerCPUAffinity ) {
    //char * pcErrorMessage;

    #if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
    if ( g_pCPUMinerSharedLibrary == nullptr ) {
        g_pCPUMinerSharedLibrary = dlopen ( "./cpuminer.so", RTLD_NOW );
        if ( g_pCPUMinerSharedLibrary == nullptr ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : cpuminer shared library was not found.\n" );
            return nullptr;
        }
        g_pfPoolMinerMain = ( int (*)(int, char**, char*, const char*, const char*, const char*, const char*, int, int, int) ) dlsym ( g_pCPUMinerSharedLibrary, "PoolMinerMain" );
        if ( g_pfPoolMinerMain == nullptr ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMinerMain function was not found in cpuminer shared library.\n" );
            return nullptr;
        }
        g_pfPoolMiner_StopMining = ( void (*)() ) dlsym ( g_pCPUMinerSharedLibrary, "PoolMiner_StopMining" );
        if ( g_pfPoolMiner_StopMining == nullptr ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_StopMining function was not found in cpuminer shared library.\n" );
            return nullptr;
        }
        g_pfPoolMiner_GetHashesRate = ( double (*)() ) dlsym ( g_pCPUMinerSharedLibrary, "PoolMiner_GetHashesRate" );
        if ( g_pfPoolMiner_GetHashesRate == nullptr ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_GetHashesRate function was not found in cpuminer shared library.\n" );
            return nullptr;
        }
        g_pfPoolMiner_IsReadyForNewStart = ( bool (*)() ) dlsym ( g_pCPUMinerSharedLibrary, "PoolMiner_IsReadyForNewStart" );
        if ( g_pfPoolMiner_IsReadyForNewStart == nullptr ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_IsReadyForNewStart function was not found in cpuminer shared library.\n" );
            return nullptr;
        }

    }

    #elif defined(_WIN32) || defined(WIN32)
    if ( g_hModule_CPUMinerSharedLibrary == nullptr ) {
            g_hModule_CPUMinerSharedLibrary = LoadLibrary ( "cpuminer.dll" ); // dll exe
            if ( g_hModule_CPUMinerSharedLibrary == nullptr ) {
                fprintf ( stdout, "miner.cpp : StartPoolMining () : cpuminer shared library was not found.\n" );
                return nullptr;
            }
            g_pfPoolMinerMain = ( int (*)(int, char**, char*, const char*, const char*, const char*, const char*, int, int, int) ) GetProcAddress ( g_hModule_CPUMinerSharedLibrary, "PoolMinerMain" );
            if ( g_pfPoolMinerMain == nullptr ) {
                fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMinerMain function was not found in cpuminer shared library.\n" );
                return nullptr;
            }
            g_pfPoolMiner_StopMining = ( void (*)() ) GetProcAddress ( g_hModule_CPUMinerSharedLibrary, "PoolMiner_StopMining" );
            if ( g_pfPoolMiner_StopMining == nullptr ) {
                fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_StopMining function was not found in cpuminer shared library.\n" );
                return nullptr;
            }
            g_pfPoolMiner_GetHashesRate = ( double (*)() ) GetProcAddress ( g_hModule_CPUMinerSharedLibrary, "PoolMiner_GetHashesRate" );
            if ( g_pfPoolMiner_GetHashesRate == nullptr ) {
                fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_GetHashesRate function was not found in cpuminer shared library.\n" );
                return nullptr;
            }
            g_pfPoolMiner_IsReadyForNewStart = ( bool (*)() ) GetProcAddress ( g_hModule_CPUMinerSharedLibrary, "PoolMiner_IsReadyForNewStart" );
            if ( g_pfPoolMiner_IsReadyForNewStart == nullptr ) {
                fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner_IsReadyForNewStart function was not found in cpuminer shared library.\n" );
                return nullptr;
            }

    }

    #else

    #endif

    fprintf ( stdout, "miner.cpp : StartPoolMining () : %i , %i , %s , %s , %s , %s ,%i , %i .\n", _bStart, _iAmountOfPoolMiningThreads,
        _sPoolURL.c_str (), _sPoolUser.c_str (), _sPoolUserPassword.c_str (), _sPoolMiningAlgorithm.c_str (), _iPoolMinerCPUPriority, _iPoolMinerCPUAffinity );
    if ( _bStart ) {
        //GenerateBitcoins(fGenerate, nGenProcLimit, Params(), *g_connman);
        //PoolMinerMain ( 0, nullptr );

        if ( ! g_pfPoolMiner_IsReadyForNewStart () ) {
            fprintf ( stdout, "miner.cpp : StartPoolMining () : PoolMiner is not ready for new start from previous run.\n" );
            strcpy ( g_sErrorMessage, "PoolMiner is not ready for new start from previous run. Please, try again in 20 - 30 seconds.\0" );
            return g_sErrorMessage;
        }

        if ( ( _sPoolURL != "" ) &&
         ( _sPoolUser != "" ) &&
         ( _sPoolUserPassword != "" ) &&
         ( _sPoolMiningAlgorithm != "" ) &&
         ( _iAmountOfPoolMiningThreads > 0 ) ) {

        g_pfPoolMiner_StopMining ();

        // setgenerate_in_pool true 1 \"stratum+tcp://pool.binarium.money:3001\" \"XbCiEW3RpLyvuTBxf2Kn99bv6PrPB9Azy8\" \"password\" \"Binarium_hash_v1\" 0 -1
        strcpy ( g_sCPUMinerProgram, "./cpuminer" );
        /*strcpy ( g_sPoolURL, "--url=stratum+tcp://pool.binarium.money:3001" );
        strcpy ( g_sUserLogin, "--user=XbCiEW3RpLyvuTBxf2Kn99bv6PrPB9Azy8" );
        strcpy ( g_sUserPassword, "--pass=password" );
        strcpy ( g_sAlgorithm, "--algo=Binarium_hash_v1" );
        strcpy ( g_sAmountOfthreads, "--threads=2" );
        strcpy ( g_sCPUPriority, "--cpu-priority=0" );
        strcpy ( g_sCPUAffinity, "--cpu-affinity=-1" );*/
        strcpy ( g_sPoolURL, ( "--url=" + _sPoolURL ).c_str () );
        strcpy ( g_sUserLogin, ( "--user=" + _sPoolUser ).c_str () );
        strcpy ( g_sUserPassword, ( "--pass=" + _sPoolUserPassword ).c_str () );
        strcpy ( g_sAlgorithm, ( "--algo=" + _sPoolMiningAlgorithm ).c_str () );
        strcpy ( g_sAmountOfthreads, ( "--threads=" + std :: to_string ( _iAmountOfPoolMiningThreads ) ).c_str () );
        strcpy ( g_sCPUPriority, ( "--cpu-priority=" + std :: to_string ( _iPoolMinerCPUPriority ) ).c_str () );
        strcpy ( g_sCPUAffinity, ( "--cpu-affinity=" + std :: to_string ( _iPoolMinerCPUAffinity ) ).c_str () );
        memset ( g_sErrorMessage, 0, sizeof ( g_sErrorMessage ) );

        //#if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
        g_pfPoolMinerMain ( 8, g_pArgV, g_sErrorMessage,
            _sPoolURL.c_str (),
            _sPoolUser.c_str (),
            _sPoolUserPassword.c_str (),
            _sPoolMiningAlgorithm.c_str (),
            _iAmountOfPoolMiningThreads,
            _iPoolMinerCPUPriority,
            _iPoolMinerCPUAffinity );

        /*#elif defined(_WIN32) || defined(WIN32)
        PoolMinerMain ( 8, g_pArgV, g_sErrorMessage,
            _sPoolURL.c_str (),
            _sPoolUser.c_str (),
            _sPoolUserPassword.c_str (),
            _sPoolMiningAlgorithm.c_str (),
            _iAmountOfPoolMiningThreads,
            _iPoolMinerCPUPriority,
            _iPoolMinerCPUAffinity );

        #else

        #endif*/

        if ( g_sErrorMessage [ 0 ] != 0 ) {
            return g_sErrorMessage;
        }
        g_bIsPoolMiningEnabled = true;

        }

    } else {
        //#if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
        g_pfPoolMiner_StopMining ();

        /*#elif defined(_WIN32) || defined(WIN32)
        PoolMiner_StopMining ();

        #else

        #endif*/

        g_bIsPoolMiningEnabled = false;

        #if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
    if ( g_pCPUMinerSharedLibrary != nullptr ) {
        //dlclose ( g_pCPUMinerSharedLibrary );

    }

        #elif defined(_WIN32) || defined(WIN32)
    if ( g_hModule_CPUMinerSharedLibrary != nullptr ) {
        //FreeLibrary ( g_hModule_CPUMinerSharedLibrary );
    }

        #endif

    }

    return nullptr;
}

double Wallet_PoolMiner_GetHashesRate () {
    if ( g_bIsPoolMiningEnabled )
        //#if defined(__linux__) || defined(__unix__) || defined(__linux) || defined(linux)
        return g_pfPoolMiner_GetHashesRate ();
        //return 2.0;

        /*#elif defined(_WIN32) || defined(WIN32)
        return PoolMiner_GetHashesRate ();

        #else

        #endif*/

    else
        return 0.0;
}