## Binarium 0.12.9.1

Binarium Core 0.12.9.1 is a cumulative update of the Binarium Core 0.12.x series.

1. Contain a updated ZeroMQ to version 4.1.7 with critical CVE-2019-13132 fix.
If you use ZMQ-based message brokers on you production code, updates with you
server/working nodes is strongly recommended. All binaries builded and tested
with ZMQ 4.1.7
2. Applied cumulative patch, contain commits with all minor fixes from
Dash and Bitcoin up to end of 0.12.* branches.
3. Add backport for 0.12.* compatibility. This release does not include any
changes that affect mainnet.

## These commits has been merged with Binarium & Dash code

Merge bitcoin#8272: [R] Make the dummy argument to getaddednodeinfo optional\
Merge bitcoin#8629: [R] C++11 Replace boost::scoped_ptr to std::unique_ptr\
Merge bitcoin#8466: [R] Do not shadow variables in networking code\
Merge bitcoin#8663: [R] Remove unnecessary variable fDaemon\
Merge bitcoin#8606: [F] Fix some locks in net_processing\
Merge bitcoin#8673: [F] Fix obvious assignment/equality error in test\
Merge bitcoin#8683: [D] Fix incorrect file name bitcoin.qrc, update translation_process.md\
Merge bitcoin#8680: [I] Add waitfornewblock/waitforblock/waitforblockheight rpcs\
Merge bitcoin#8563: [B] Add configure check for -latomic\
Merge bitcoin#8677: [R] Do not shadow upper local variable 'send', prevent -Wshadow compiler warning.\
Merge bitcoin#8681: [F] Performance Regression Fix: Pre-Allocate txChanged vector\
Merge bitcoin#8928: [F] Fix init segfault where InitLoadWallet() calls ATMP before genesis\
Merge dash#1807:    [F] Don't delay GETHEADERS when no blocks have arrived yet in devnet\
Merge dash#1844:    [F] Actually honor fMiningRequiresPeers in getblocktemplate\
Merge bitcoin#8638: [F] rest.cpp: change HTTP_INTERNAL_SERVER_ERROR to HTTP_BAD_REQUEST\
Merge bitcoin#8640: [D] Remove old Qt46 package\
Merge bitcoin#8604: [F] BerkeleyDB v6 compatibility fix\
Merge bitcoin#8191: [R] Do not shadow variables in src/wallet\
Merge bitcoin#8611: [F] Reduce default number of blocks to check at startup\
Merge bitcoin#8624: [D] Mention curl\
Merge bitcoin#8564: [R] Remove unused code/conditions in ReadAtCursor\
Merge bitcoin#8545: [D] Update git-subtree-check.sh README\
Merge bitcoin#8109: [R] Do not shadow member variables\
Merge bitcoin#8602: [D] Mention ++i as preferred over i++\
Merge bitcoin#8590: [R] Remove unused variables\
Merge bitcoin#8581: [R] Wallet RPC: Drop misleading option\
Merge bitcoin#8583: [I] Show XTHIN in GUI\
Merge bitcoin#8561: [F] Show "end" instead of many zeros when getheaders\
Merge bitcoin#8462: [I] Move AdvertiseLocal debug output to net category\
Merge bitcoin#7579: [I] Show network/chain errors in the GUI\
Merge bitcoin#8226: [D] contributing.md: Fix formatting (line lengths and other)\
Merge bitcoin#8578: [R] Remove unused code\
Merge bitcoin#8567: [D] Add default port numbers to REST doc\
Merge bitcoin#8450: [R] Replace rpc_wallet_tests.cpp with python RPC\
Merge bitcoin#8560: [F] Fix two VarInt examples in serialize.h\
Merge bitcoin#8445: [R] Move CWallet::setKeyPool to private section of CWalletDB\
Merge bitcoin#8554: [R] Remove unused variable\
Merge dash#1839:    [I] Include p2pk into addressindex\
Merge dash#1840:    [F] Avoid reference leakage in CKeyHolderStorage::AddKey\
Merge dash#1834:    [F] Protect CKeyHolderStorage via mutex\
Merge dash#1833:    [F] Skip existing masternode conections on mixing\
Merge dash#1824:    [F] Return EXIT_SUCCESS instead of 0 in main()\
Merge dash#1824:    [R]Update code to use new SyncTransaction notifications interface\
Merge bitcoin#8551: [R] Remove unused code\
Merge bitcoin#8548: [R] Use __func__ to get function name for output printing\
Merge bitcoin#8461: [F] Document return value of networkhashps\
Merge bitcoin#8540: [F] Fix random segfault when closing "Choose data directory"\
Merge bitcoin#8453: [R] Bring secp256k1 subtree up to date with master\
Merge bitcoin#8531: [F] Abandonconflict: Use assert_equal\
Merge bitcoin#8504: [B] Remove java comparison tool\
Merge bitcoin#8505: [F] Fix typos in various files\
Merge bitcoin#8512: [F] Corrected JSON typo on setban of net.cpp\
Merge bitcoin#8521: [R] Remove duplicate `hash160` implementation\
Merge bitcoin#8520: [B] Remove check for `openssl/ec.h`\
Merge bitcoin#8516: [R] Remove no-longer-used InitError logic\
Merge bitcoin#8058: [D] Add issue template\
Merge bitcoin#8270: [F] Tests: Use portable #! in python scripts\
Merge bitcoin#8192: [F] Remove URLs from About dialog translations\
Merge dash#1824:    [I] Add SYNC_TRANSACTION_NOT_IN_BLOCK constant and fix callers of SyncTransaction\
Merge bitcoin#7946: [R] Reduce cs_main locks during ConnectTip/SyncWithWallets\
Merge bitcoin#8353: [R] Tiny c++11 refactors\
Merge bitcoin#8293: [F] Allow building libbitcoinconsensus without any univalue\
Merge bitcoin#8492: [B] Allow building bench_bitcoin by itself\
Merge bitcoin#8503: [D] Fix typos, text errors\
Merge bitcoin#8481: [F] Fix minimize and close window bugs\
Merge bitcoin#8444: [F] Fix p2p-feefilter.py for changed tx relay behavior\
Merge bitcoin#8392: [F] Fix several node initialization issues\
Merge bitcoin#8189: [R] rename mapAddrCount to mapNetGroupNodes\
Merge bitcoin#8428: [D] Update README.md inside of src/test/\
Merge bitcoin#8432: [R] Make CWallet::fFileBacked private.\
Merge bitcoin#8152: [R] Remove CWalletDB* parameter from CWallet\
Merge bitcoin#8413: [F] Pass Consensus::Params& instead of CChainParams\
Merge bitcoin#8346: [R] Mempool: Use Consensus::CheckTxInputs direclty override\
Merge bitcoin#8400: [I] Enable rpcbind_test\
Merge bitcoin#8238: [B] ZeroMQ 4.1.5 && ZMQ on Windows\
Merge bitcoin#8427: [F] Ignore `notfound` P2P messages\
Merge bitcoin#8274: [I] Update tinyformat\
Merge bitcoin#8421: [R] httpserver: drop boost (bitcoin#8023 dependency)\
Merge bitcoin#8412: [I] libconsensus: Expose a flag for BIP112\
Merge bitcoin#8321: [R] Switched constants to sizeof()\
Merge bitcoin#8407: [I] Add dbcache migration path\
Merge bitcoin#8414: [D] Prepend license statement to indirectmap.h\
Merge bitcoin#8358: [R] Set memory explicitly (default is too low\
Merge bitcoin#8396: [R] Remove outdated legacy code from key.h\
Merge bitcoin#8347: [R] Make CBlockIndex param const in ContextualCheckBlockHeader and ContextualCheckBlock\
Merge bitcoin#8342: [R] Transform BOOST_FOREACH into for\
Merge dash#1847:    [R] Make boost::multi_index comparators const\
Merge dash#1826:    [F] Vote on IS only if it was accepted to mempool\
Merge dash#1829:    [F] Fix -liquidityprovider option\
Merge dash#1830:    [D] RPC Typos / markdown formatting\
Merge dash#8256:    [F] Fix bitcoin-qt crash\
Merge dash#1828:    [R] Swap iterations and fUseInstantSend parameters in ApproximateBestSubset\
Merge dash#1810:    [F] Fix crash on exit when -createwalletbackups=0\
