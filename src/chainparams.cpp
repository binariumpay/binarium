// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

#include "arith_uint256.h"

//---For testing only.--------------------------------
#include "crypto/hashing/streebog/stribog.h"
#include "crypto/hashing/streebog/test_data.h"
#include "crypto/hashing/whirlpool/whirlpool.h"
//#include "crypto/hashing/swifft/swifft.h"

#include "crypto/encryption/gost2015_kuznechik/libgost15/libgost15.h"
#include "crypto/encryption/three_fish/libskein_skein.h"
//#include "crypto/encryption/camellia/camellia.h"
#include "openssl/camellia.h"

//----------------------------------------------------


//---For testing only.--------------------------------
void TestCryptographicFunctions ();

/*// SWIFFT
HashKey key ALIGN;
HashState state ALIGN;
HashData data ALIGN;*/

//----------------------------------------------------



static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    //const char* pszTimestamp = "Wired 09/Jan/2014 The Grand Experiment Goes Live: Overstock.com Is Now Accepting Bitcoins";
    const char* pszTimestamp = "21.01.2018 : US Unilateral Actions in Syria Provoked Turkey to Fury";
    const CScript genesisOutputScript = CScript() << ParseHex("043a537e59051f93548ac6f5370cb596b7000e81c369a97b46f080b94bf17f462465eb4aa6d8a1e19ae17d27683ef3ca9bb2402d68ab186945e42abe8248cb2817") << OP_CHECKSIG; // 040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210240; // Note: actual number of blocks per calendar year with DGW v3 is ~200700 (for example 449750 - 249050)
        consensus.nMasternodePaymentsStartBlock = 100000; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 158000; // actual historical value
        consensus.nMasternodePaymentsIncreasePeriod = 576*30; // 17280 - actual historical value
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 328008; // actual historical value
        consensus.nBudgetPaymentsCycleBlocks = 16616; // ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nBudgetProposalEstablishingTime = 60*60*24;
        consensus.nSuperblockStartBlock = 614820; // The block at which 12.1 goes live (end of final 12.0 budget cycle)
        consensus.nSuperblockCycle = 16616; // ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x000007d91d1254d60e2dd1ae580383070a4ddffa4c64c2eeb4a2f9ecc0414343");
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Binarium: 1 day
        consensus.nPowTargetSpacing = 2.5 * 60; // Binarium: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowKGWHeight = 15200;
        consensus.nPowDGWHeight = 34140;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1486252800; // Feb 5th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517788800; // Feb 5th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1508025600; // Oct 15th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1539561600; // Oct 15th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 3226; // 80% of 4032

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000110"); //  000000000000000000000000000000000000000000000100a308553b4863b755 782700

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000000000001c172f518793c3b9e83f202284615592f87fe3506ce964dcd4"); // 782700

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xbf;
        pchMessageStart[1] = 0x0c;
        pchMessageStart[2] = 0x6b;
        pchMessageStart[3] = 0xbd;
        vAlertPubKey = ParseHex("04fb42adea2fe25678063abe0ea60e36f130d7fd9312adbf97096fe483d360994cb91f14d823bdb3105faac6e1ccb5b1fa3d37b50eb0de466d4ff12cd272b4f737"); // 048240a8748a80a286b270ba126705ced4f2ce5a7847b3610ea3c06513150dade2a8512ed5ea86320824683fc0818f0ac019214973e677acd1244f6d0571fc5103
        nDefaultPort = 8884;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 100000;

        uint32_t nTime = 1516957200; // 1516595232
        uint32_t nNonce = 0;
        uint32_t nBits = 0x1e0ffff0;

        //genesis = CreateGenesisBlock(1390095618, 28917698, 0x1e0ffff0, 1, 50 * COIN);
        //genesis = CreateGenesisBlock(1516595232, 28917698, 0x1e0ffff0, 1, 50 * COIN);
        genesis = CreateGenesisBlock(nTime, 3670997, nBits, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetGenesisInitializationHash();

        /*bool fNegative;
        bool fOverflow;
        arith_uint256 bnTarget;

        bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

        // Check range
        if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(consensus.powLimit)) {
            fprintf(stdout, "CMainParams.CMainParams () : nBits below minimum work : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        } else fprintf(stdout, "CMainParams.CMainParams () : nBits check successfull : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        while (UintToArith256(genesis.GetHash()) > bnTarget) {
            genesis = CreateGenesisBlock(nTime, nNonce, nBits, 1, 50 * COIN);
            //consensus.hashGenesisBlock = genesis.GetHash();
            nNonce = nNonce + 1;

        } //-while

        consensus.hashGenesisBlock = genesis.GetHash();*/

        //LogPrintf("CMainParams.CMainParams () : %s .\n", "MainNet");
        //LogPrintf("CMainParams.CMainParams () : %s .\n", consensus.hashGenesisBlock.ToString() );
        //LogPrintf("CMainParams.CMainParams () : %s .\n", genesis.hashMerkleRoot.ToString() );
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", "MainNet");  // strUsage.c_str()
        fprintf(stdout, "validation.cpp : CheckTransaction () : %i .\n", genesis.vtx[0].vin[0].scriptSig.size());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", consensus.hashGenesisBlock.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", genesis.hashMerkleRoot.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : %i .\n", nNonce);

        //assert(consensus.hashGenesisBlock == uint256S("0x0000072080541fadf27d225277417ffbc5909dc8def11a491b44253b7309cc0a")); // 00000ffd590b1485b3caadc19b22e6379c733355108f107a430458cdf3407ab6
        //assert(genesis.hashMerkleRoot == uint256S("0x8a0510ff8dc69bc7e9a33597be92c29aefd30acb7b67b77c03c3edd7b3727b7c")); // e0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7

        vSeeds.push_back(CDNSSeedData("binarium.money", "dnsseed.binarium.money")); // dash.org dnsseed.dash.org
        /*vSeeds.push_back(CDNSSeedData("dashdot.io", "dnsseed.dashdot.io"));
        vSeeds.push_back(CDNSSeedData("masternode.io", "dnsseed.masternode.io"));
        vSeeds.push_back(CDNSSeedData("dashpay.io", "dnsseed.dashpay.io"));*/

        // Binarium addresses start with 'X'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,76);
        // Binarium script addresses start with '7'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,16);
        // Binarium private keys start with '7' or 'X'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,204);
        // Binarium BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Binarium BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // Binarium BIP44 coin type is '5'
        nExtCoinType = 5;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "04ec646dbf379c2af1f9a5112e052481da577d68db40ce8ae0d858d585eef8fabca3c415979e14e659bdcfb1c935957f74bbd29c4d0c79bf63a01d9814e333a6fe"; // 04549ac134f694c0243f503e8c8a9a986f5de6610049c40b07816809b0d1d06a21b07be27b9bb555931773f62ba6cf35a25fd52f694d4e1106ccd237a7bb899fdd

        /*checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  1500, uint256S("0x000000aaf0300f59f49bc3e970bad15c11f961fe2347accffff19d96ec9778e3"))
            (  4991, uint256S("0x000000003b01809551952460744d5dbb8fcbd6cbae3c220267bf7fa43f837367"))
            (  9918, uint256S("0x00000000213e229f332c0ffbe34defdaa9e74de87f2d8d1f01af8d121c3c170b"))
            ( 16912, uint256S("0x00000000075c0d10371d55a60634da70f197548dbbfa4123e12abfcbc5738af9"))
            ( 23912, uint256S("0x0000000000335eac6703f3b1732ec8b2f89c3ba3a7889e5767b090556bb9a276"))
            ( 35457, uint256S("0x0000000000b0ae211be59b048df14820475ad0dd53b9ff83b010f71a77342d9f"))
            ( 45479, uint256S("0x000000000063d411655d590590e16960f15ceea4257122ac430c6fbe39fbf02d"))
            ( 55895, uint256S("0x0000000000ae4c53a43639a4ca027282f69da9c67ba951768a20415b6439a2d7"))
            ( 68899, uint256S("0x0000000000194ab4d3d9eeb1f2f792f21bb39ff767cb547fe977640f969d77b7"))
            ( 74619, uint256S("0x000000000011d28f38f05d01650a502cc3f4d0e793fbc26e2a2ca71f07dc3842"))
            ( 75095, uint256S("0x0000000000193d12f6ad352a9996ee58ef8bdc4946818a5fec5ce99c11b87f0d"))
            ( 88805, uint256S("0x00000000001392f1652e9bf45cd8bc79dc60fe935277cd11538565b4a94fa85f"))
            ( 107996, uint256S("0x00000000000a23840ac16115407488267aa3da2b9bc843e301185b7d17e4dc40"))
            ( 137993, uint256S("0x00000000000cf69ce152b1bffdeddc59188d7a80879210d6e5c9503011929c3c"))
            ( 167996, uint256S("0x000000000009486020a80f7f2cc065342b0c2fb59af5e090cd813dba68ab0fed"))
            ( 207992, uint256S("0x00000000000d85c22be098f74576ef00b7aa00c05777e966aff68a270f1e01a5"))
            ( 312645, uint256S("0x0000000000059dcb71ad35a9e40526c44e7aae6c99169a9e7017b7d84b1c2daf"))
            ( 407452, uint256S("0x000000000003c6a87e73623b9d70af7cd908ae22fee466063e4ffc20be1d2dbc"))
            ( 523412, uint256S("0x000000000000e54f036576a10597e0e42cc22a5159ce572f999c33975e121d4d"))
            ( 523930, uint256S("0x0000000000000bccdb11c2b1cfb0ecab452abf267d89b7f46eaf2d54ce6e652c"))
            ( 750000, uint256S("0x00000000000000b4181bbbdddbae464ce11fede5d0292fb63fdede1e7c8ab21c")),
            1507424630, // * UNIX timestamp of last checkpoint block
            3701128,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            5000        // * estimated number of transactions per day after checkpoint
        };*/

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (   0, uint256S("0x0000072080541fadf27d225277417ffbc5909dc8def11a491b44253b7309cc0a")),
            1516595232, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                      //   (the tx=... number in the SetBestChain debug.log lines)
            500        // * estimated number of transactions per day after checkpoint
        };

    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210240;
        consensus.nMasternodePaymentsStartBlock = 4010; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 4030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 4100;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60*20;
        consensus.nSuperblockStartBlock = 4200; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPeymentsStartBlock
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x0000047d24635e347be3aaaeb66c26be94901a2f962feccd4f95090191f208c1");
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Binarium: 1 day
        consensus.nPowTargetSpacing = 2.5 * 60; // Binarium: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nPowKGWHeight = 4001; // nPowKGWHeight >= nPowDGWHeight means "no KGW"
        consensus.nPowDGWHeight = 4001;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1505692800; // Sep 18th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1537228800; // Sep 18th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 50; // 50% of 100

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0x0000000000000000000000000000000000000000000000000000000000000110"); // 000000000000000000000000000000000000000000000000000924e924a21715 37900

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000004f5aef732d572ff514af99a995702c92e4452c7af10858231668b1f"); // 37900

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0xe2;
        pchMessageStart[2] = 0xca;
        pchMessageStart[3] = 0xff;
        vAlertPubKey = ParseHex("043e2504e0c66e62f8e6523d9f6ca6cc6434fa8ca0a576abd07ccd97d4577e89617ac5548e14f0d18a3d4fbb50acce4fc797ce0a169abf57cbe081d598f8582bdd"); // 04517d8a699cb43d3938d7b24faaff7cda448ca4ea267723ba614784de661949bf632d6304316b244646dea079735b9a6fc4af804efb4752075b9fe2245e14e412
        nDefaultPort = 18884;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 1000;

        TestCryptographicFunctions ();

        uint32_t nTime = 1516957201; // 1516595233
        uint32_t nNonce = 0;
        uint32_t nBits = 0x1e0ffff0;

        //genesis = CreateGenesisBlock(1390666206UL, 3861367235UL, 0x1e0ffff0, 1, 50 * COIN);
        //genesis = CreateGenesisBlock(1516595233UL, 1286475UL, 0x1e0ffff0, 1, 50 * COIN);
        genesis = CreateGenesisBlock(nTime, 53186UL, nBits, 1, 50 * COIN); // 53186UL 288307UL
        consensus.hashGenesisBlock = genesis.GetGenesisInitializationHash();

        /*bool fNegative;
        bool fOverflow;
        arith_uint256 bnTarget;

        bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

        // Check range
        if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(consensus.powLimit)) {
            fprintf(stdout, "CMainParams.CMainParams () : nBits below minimum work : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        } else fprintf(stdout, "CMainParams.CMainParams () : nBits check successfull : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        while (UintToArith256(genesis.GetHash()) > bnTarget) {
            genesis = CreateGenesisBlock(nTime, nNonce, nBits, 1, 50 * COIN);
            //consensus.hashGenesisBlock = genesis.GetHash();
            nNonce = nNonce + 1;

        } //-while

        consensus.hashGenesisBlock = genesis.GetHash();*/

        //LogPrintf("CMainParams.CMainParams () : %s .\n", "TestNet");
        //LogPrintf("CMainParams.CMainParams () : %s .\n", consensus.hashGenesisBlock.ToString() );
        //LogPrintf("CMainParams.CMainParams () : %s .\n", genesis.hashMerkleRoot.ToString() );
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", "TestNet");  // strUsage.c_str()
        fprintf(stdout, "CMainParams.CMainParams () : scriptSig.size() : %i .\n", genesis.vtx[0].vin[0].scriptSig.size());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", consensus.hashGenesisBlock.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", genesis.hashMerkleRoot.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : nNonce : %i .\n", nNonce);
        
        assert(consensus.hashGenesisBlock == uint256S("0x00000319550cb678290c72e93d77d98b6d291c02b6dc581d5b03600253dc42b2")); // 00000319550cb678290c72e93d77d98b6d291c02b6dc581d5b03600253dc42b2     0000002117ef893448661469e2b8e131319eee0a08e076fa18e25550cf578298
        assert(genesis.hashMerkleRoot == uint256S("0x8a0510ff8dc69bc7e9a33597be92c29aefd30acb7b67b77c03c3edd7b3727b7c")); // e0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("binarium.money",  "testnet-seed.binarium.money")); // dashdot.io testnet-seed.dashdot.io
        //vSeeds.push_back(CDNSSeedData("masternode.io", "test.dnsseed.masternode.io"));

        // Testnet Binarium addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Testnet Binarium script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Binarium BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Binarium BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Binarium BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes
        strSporkPubKey = "04bd0c9f7a53cae433ad1eaf9bd973e6266685f3bc6279c6ef8be8022b3509d0f6b929be54160a6832103e7b2b9e2cc4a3e4fdf8f6a95c6becbe9cfcf348117479"; // 046f78dcf911fbd61910136f7f0f8d90578f68d0b3ac973b5040fb7afb501b5939f39b108b0569dca71488f5bbf498d92e4d1194f6f941307ffd95f75e76869f0e

        /*checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (    261, uint256S("0x00000c26026d0815a7e2ce4fa270775f61403c040647ff2c3091f99e894a4618"))
            (   1999, uint256S("0x00000052e538d27fa53693efe6fb6892a0c1d26c0235f599171c48a3cce553b1"))
            (   2999, uint256S("0x0000024bc3f4f4cb30d29827c13d921ad77d2c6072e586c7f60d83c2722cdcc5")),

            1462856598, // * UNIX timestamp of last checkpoint block
            3094,       // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            500         // * estimated number of transactions per day after checkpoint
        };*/

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (   0, uint256S("0x00000319550cb678290c72e93d77d98b6d291c02b6dc581d5b03600253dc42b2")),  // 00000319550cb678290c72e93d77d98b6d291c02b6dc581d5b03600253dc42b2      0000002117ef893448661469e2b8e131319eee0a08e076fa18e25550cf578298
            1516595233, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                      //   (the tx=... number in the SetBestChain debug.log lines)
            500        // * estimated number of transactions per day after checkpoint
        };



        //---Polymorphic hash functions.-----------------
        /*THashFunctionsChangeEvent structureHashFunctionsChangeEvent;
        structureHashFunctionsChangeEvent.iBlockTime = 0;
        structureHashFunctionsChangeEvent.iHashFunctionIndex = 0;
        aHashFunctionsChangesHistory.push_back ( structureHashFunctionsChangeEvent );*/

        THashFunctionsAddEvent structureHashFunctionsAddEvent;
        structureHashFunctionsAddEvent.iBlockTime = 0;
        structureHashFunctionsAddEvent.iHashFunctionsAmount = 2;

        aHashFunctionsAdditionsHistory.push_back ( structureHashFunctionsAddEvent );

    }

    uint256 Hash () {
        return uint256S("0x00000319550cb678290c72e93d77d98b6d291c02b6dc581d5b03600253dc42b2");

    }

};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMasternodePaymentsStartBlock = 240;
        consensus.nMasternodePaymentsIncreaseBlock = 350;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60*20;
        consensus.nSuperblockStartBlock = 1500;
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Binarium: 1 day
        consensus.nPowTargetSpacing = 2.5 * 60; // Binarium: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowKGWHeight = 15200; // same as mainnet
        consensus.nPowDGWHeight = 34140; // same as mainnet
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 0; // never delay GETHEADERS in regtests
        nDefaultPort = 17994;
        nPruneAfterHeight = 1000;

        uint32_t nTime = 1516957202; // 1516595234
        uint32_t nNonce = 0;
        uint32_t nBits = 0x207fffff;

        //genesis = CreateGenesisBlock(1417713337, 1096447, 0x207fffff, 1, 50 * COIN);
        //genesis = CreateGenesisBlock(1516595232, 1096447, 0x207fffff, 1, 50 * COIN);
        //genesis = CreateGenesisBlock(1516595234, 0, 0x207fffff, 1, 50 * COIN);
        genesis = CreateGenesisBlock(nTime, 0, nBits, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetGenesisInitializationHash();

        /*bool fNegative;
        bool fOverflow;
        arith_uint256 bnTarget;

        bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

        // Check range
        if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(consensus.powLimit)) {
            fprintf(stdout, "CMainParams.CMainParams () : nBits below minimum work : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        } else fprintf(stdout, "CMainParams.CMainParams () : nBits check successfull : %s, %i, %i, %i, %i .\n", consensus.powLimit.ToString().c_str(),
              nBits, fNegative, fOverflow, bnTarget );

        while (UintToArith256(genesis.GetHash()) > bnTarget) {
            genesis = CreateGenesisBlock(nTime, nNonce, nBits, 1, 50 * COIN);
            //consensus.hashGenesisBlock = genesis.GetHash();
            nNonce = nNonce + 1;

        } //-while

        consensus.hashGenesisBlock = genesis.GetHash();*/

        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", "RegTest");  // strUsage.c_str()
        fprintf(stdout, "CMainParams.CMainParams () : scriptSig.size() : %i .\n", genesis.vtx[0].vin[0].scriptSig.size());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", consensus.hashGenesisBlock.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : %s .\n", genesis.hashMerkleRoot.ToString().c_str());
        fprintf(stdout, "CMainParams.CMainParams () : nNonce : %i .\n", nNonce);

        //assert(consensus.hashGenesisBlock == uint256S("0xcef8c3397abc0fd4bc1cd21b2b0d7aea2a833958faa49a9ea7716881e9e0051c")); // 000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e
        //assert(genesis.hashMerkleRoot == uint256S("0x8a0510ff8dc69bc7e9a33597be92c29aefd30acb7b67b77c03c3edd7b3727b7c")); // e0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7

        //assert (false);

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        /*checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e")),
            0,
            0,
            0
        };*/

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0xcef8c3397abc0fd4bc1cd21b2b0d7aea2a833958faa49a9ea7716881e9e0051c")),
            0,
            0,
            0
        };

        // Regtest Binarium addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest Binarium script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest Binarium BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Binarium BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest Binarium BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
   }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}


void TestCryptographicFunctions () {
    /*uint512 uint512TestHash;
    uint512 uint512Hash;
    uint512 aHashes [ 11 ];

    //---Streebog.-----------------------------------------------------
    // M1 = 0x323130393837363534333231303938373635343332313039383736353433323130393837363534333231303938373635343332313039383736353433323130
    // M2 = 0xfbe2e5f0eee3c820fbeafaebef20fffbf0e1e0f0f520e0ed20e8ece0ebe5f0f2f120fff0eeec20f120faf2fee5e2202ce8f6f3ede220e8e6eee1e8f0f2d1202ce8f0f2e5e220e5d1
    // H1_512 = 0x486f64c1917879417fef082b3381a4e211c324f074654c38823a7b76f830ad00fa1fbae42b1285c0352f227524bc9ab16254288dd6863dccd5b9f54a1ad0541b
    // H2_512 = 0x28fbc9bada033b1460642bdcddb90c3fb3e56c497ccd0f62b8a2ad4935e85f037613966de4ee00531ae60f3b5a47f8dae06915d5f2f194996fcabf2622e6881e

    unsigned char h512[64]={};

    const unsigned char * sTestString_Streebog = ( unsigned char * ) ( "0x32, 0x31, ... 30393837363534333231303938373635343332313039383736353433323130393837363534333231303938373635343332313039383736353433323130" );
    hash_512 ( Message [0], MessageLength [0], uint512TestHash.begin () );
    hash_512 ( Message [0], MessageLength [0], h512 );
    //hash_512 ( static_cast<const unsigned char *>(static_cast<const void*>(sTestString_Streebog)), strlen ( ( const char * ) sTestString_Streebog ) * 8, uint512TestHash.begin () );
    uint512Hash.SetHex ( "0x486f64c1917879417fef082b3381a4e211c324f074654c38823a7b76f830ad00fa1fbae42b1285c0352f227524bc9ab16254288dd6863dccd5b9f54a1ad0541b" );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Streebog : %s, %i.\n", uint512TestHash.ToString ().c_str (), int ( strlen ( ( const char * ) sTestString_Streebog ) ) );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Streebog : %s.\n", uint512Hash.ToString ().c_str () );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Streebog : %s.\n", h512 );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Streebog : %s.\n", Hash_512 [ 0 ] );
    //assert ( uint512TestHash == uint512Hash );
    //assert ( memcmp(h512,Hash_512[0],sizeof(unsigned char)*64) == 0 );
    assert ( memcmp(uint512TestHash.begin (),Hash_512[0],sizeof(unsigned char)*64) == 0 );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Successfully passes checking.\n" );
    aHashes [ 0 ] = uint512TestHash;

    //---Whirlpool.----------------------------------------------------
    whirlpool_ctx            structureWhirlpoolContext;

    rhash_whirlpool_init ( & structureWhirlpoolContext );
    //fprintf(stdout, "hash.h : HashX11_Generator () : Whirlpool = %s.\n", hash[5].ToString ().c_str () );  // strUsage.c_str()
    //LogPrintf("debug turned off: thread %s\n", GetThreadName());*/

    /*"", "19FA61D75522A4669B44E39C1D2E1726C530232130D407F89AFEE0964997F7A73E83BE698B288FEBCF88E3E03C4F0757EA8964E59B63D93708B138CC42A66EB3",
    "a", "8ACA2602792AEC6F11A67206531FB7D7F0DFF59413145E6973C45001D0087B42D11BC645413AEFF63A42391A39145A591A92200D560195E53B478584FDAE231A",
    "abc", "4E2448A4C6F486BB16B6562C73B4020BF3043E3A731BCE721AE1B303D97E6D4C7181EEBDB6C57E277D0E34957114CBD6C797FC9D95D8B582D225292076D4EEF5",
    "message digest", "378C84A4126E2DC6E56DCC7458377AAC838D00032230F53CE1F5700C0FFB4D3B8421557659EF55C106B4B52AC5A4AAA692ED920052838F3362E86DBD37A8903E",
    "abcdefghijklmnopqrstuvwxyz", "F1D754662636FFE92C82EBB9212A484A8D38631EAD4238F5442EE13B8054E41B08BF2A9251C30B6A0B8AAE86177AB4A6F68F673E7207865D5D9819A3DBA4EB3B",
    "The quick brown fox jumps over the lazy dog", "B97DE512E91E3828B40D2B0FDCE9CEB3C4A71F9BEA8D88E75C4FA854DF36725FD2B52EB6544EDCACD6F8BEDDFEA403CB55AE31F03AD62A5EF54E42EE82C3FB35",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "DC37E008CF9EE69BF11F00ED9ABA26901DD7C28CDEC066CC6AF42E40F82F3A1E08EBA26629129D8FB7CB57211B9281A65517CC879D7B962142C65F5A7AF01467",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
    "466EF18BABB0154D25B9D38A6414F5C08784372BCCB204D6549C4AFADB6014294D5BD8DF2A6C44E538CD047B2681A51A2C60481E88C5A20B2C2A80CF3A9A083B",*/
    
    /*//rhash_whirlpool_update ( & structureWhirlpoolContext, "abcdbcdecdefdefgefghfghighijhijk", 64);
    //"2A987EA40F917061F5D6F0A0E4644F488A7A5A52DEEE656207C562F988E95C6916BDC8031BC5BE1B7B947639FE050B56939BAAA0ADFF9AE6745B7B181C3BE3FD",

    //std::string sTestString = "The quick brown fox jumps over the lazy dog";
    //rhash_whirlpool_update ( & structureWhirlpoolContext, sTestString.c_str, sTestString.length ());
    const unsigned char * sTestString_Whirlpool = ( unsigned char * ) ( "The quick brown fox jumps over the lazy dog" );
    rhash_whirlpool_update ( & structureWhirlpoolContext, sTestString_Whirlpool, strlen ( ( const char * ) sTestString_Whirlpool ) );
    rhash_whirlpool_final ( & structureWhirlpoolContext, static_cast<unsigned char *>(static_cast<void*> ( & uint512TestHash ) ) );    
    uint512Hash.SetHex ( "0xB97DE512E91E3828B40D2B0FDCE9CEB3C4A71F9BEA8D88E75C4FA854DF36725FD2B52EB6544EDCACD6F8BEDDFEA403CB55AE31F03AD62A5EF54E42EE82C3FB35" );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Whirlpool : %s.", uint512TestHash.ToString ().c_str () );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Whirlpool : %s.", uint512Hash.ToString ().c_str () );
    //assert ( uint512TestHash == uint512Hash );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Successfully passes checking.\n" );
    aHashes [ 1 ] = uint512TestHash;

    //---SWIFFT.----------------------------------------------------
    const unsigned char * sTestString_SWIFFT = ( unsigned char * ) ( "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog." );
    SwiFFT_initState ( state );
    SwiFFT_readKey( ( const unsigned char * ) & aHashes [ 0 ], & key );
    
    SwiFFT_readData ( data, sTestString_SWIFFT );

    SwiFFT ( key, state, data );
    SwiFFT_printState ( state );
    //SwiFFT_printKey ( key );
    //SwiFFT_printData ( data );*/

    //assert ( false );

    //---GOST 2015_Kuznechik---------------------------------------
    /*int numberOfFailedTests_ = 0;
    const uint8_t roundKeys_[NumberOfRounds * BlockLengthInBytes] = {
            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
            0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
            0xdb, 0x31, 0x48, 0x53, 0x15, 0x69, 0x43, 0x43, 0x22, 0x8d, 0x6a, 0xef, 0x8c, 0xc7, 0x8c, 0x44,
            0x3d, 0x45, 0x53, 0xd8, 0xe9, 0xcf, 0xec, 0x68, 0x15, 0xeb, 0xad, 0xc4, 0x0a, 0x9f, 0xfd, 0x04,
            0x57, 0x64, 0x64, 0x68, 0xc4, 0x4a, 0x5e, 0x28, 0xd3, 0xe5, 0x92, 0x46, 0xf4, 0x29, 0xf1, 0xac,
            0xbd, 0x07, 0x94, 0x35, 0x16, 0x5c, 0x64, 0x32, 0xb5, 0x32, 0xe8, 0x28, 0x34, 0xda, 0x58, 0x1b,
            0x51, 0xe6, 0x40, 0x75, 0x7e, 0x87, 0x45, 0xde, 0x70, 0x57, 0x27, 0x26, 0x5a, 0x00, 0x98, 0xb1,
            0x5a, 0x79, 0x25, 0x01, 0x7b, 0x9f, 0xdd, 0x3e, 0xd7, 0x2a, 0x91, 0xa2, 0x22, 0x86, 0xf9, 0x84,
            0xbb, 0x44, 0xe2, 0x53, 0x78, 0xc7, 0x31, 0x23, 0xa5, 0xf3, 0x2f, 0x73, 0xcd, 0xb6, 0xe5, 0x17,
            0x72, 0xe9, 0xdd, 0x74, 0x16, 0xbc, 0xf4, 0x5b, 0x75, 0x5d, 0xba, 0xa8, 0x8e, 0x4a, 0x40, 0x43,
    };
    uint8_t block_[BlockLengthInBytes] = {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
    };
    const uint8_t expectedCiphertext_[BlockLengthInBytes] = {
            0x7f, 0x67, 0x9d, 0x90, 0xbe, 0xbc, 0x24, 0x30, 0x5a, 0x46, 0x8d, 0x42, 0xb9, 0xd4, 0xed, 0xcd,
    };*/

    //encryptBlockWithGost15 ( & aHashes [ 0 ], & aHashes [ 1 ] );
    //assert (memcmp(block_, expectedCiphertext_, BlockLengthInBytes) == 0);

    //assert ( false );

    //---Three Fish.-------------------------------------------
    /*char D[64];
    char E[64];
    char K[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char P[64] = {'!', 'H', 'e', 'l', 'l', '0', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char T[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t i = 0;

    memset(E, 0, sizeof(E));*/
    /*libskein_threefish_encrypt(E, K, T, P, sizeof(P), 512);
    printf("Test #2, 512-bit Threefish Encryption: ");

    for(i = 0; i < sizeof(E) / sizeof(E[0]); i++)
        printf("%02x", E[i] & 0xff);

    printf(".\n");
    printf("Test #2, 512-bit Threefish Decryption: ");*/
    //memset(D, 0, sizeof(D));
    /*libskein_threefish_decrypt(D, K, T, E, sizeof(E), 512);

    for(i = 0; i < sizeof(D) / sizeof(D[0]); i++)
        printf("%c", D[i]);

    printf("\n");*/

    //---Camellia.------------------------------------------
    /*KEY_TABLE_TYPE aKey;

    Camellia_Ekeygen ( 256, aHashes [ 0 ].begin (), aKey );
    Camellia_EncryptBlock ( 256, ( const unsigned char * ) P, aKey, ( unsigned char * ) E );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Camelia : %s.\n", E );
    Camellia_DecryptBlock ( 256, ( const unsigned char * ) E, aKey, ( unsigned char * ) D );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Camelia : %s.\n", D );*/

    /*CAMELLIA_KEY stKey;
    Camellia_set_key ( aHashes [ 0 ].begin (), 256, & stKey ); // userKey
    Camellia_encrypt ( ( const unsigned char * ) P, ( unsigned char * ) E, & stKey );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Camelia : %s.\n", E );
    Camellia_decrypt ( ( const unsigned char * ) E, ( unsigned char * ) D, & stKey );
    fprintf(stdout, "chainparams.cpp : TestCryptographicFunctions () : Camelia : %s.\n", D );

    //assert ( false );*/

}