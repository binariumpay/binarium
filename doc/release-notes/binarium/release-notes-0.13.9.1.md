[bitcoin#8272] [4d0be42] [R] Make the dummy argument to getaddednodeinfo optional
[bitcoin#8629] [414272a] [R] C++11 Replace boost::scoped_ptr to std::unique_ptr
[bitcoin#8466] [313cfb0] [R] Do not shadow variables in networking code
[bitcoin#8663] [75f920e] [R] Remove unnecessary variable fDaemon
[bitcoin#8606] [f7e9bc6] [F] Fix some locks in net_processing
[bitcoin#8673] [3d26438] [F] Fix obvious assignment/equality error in test
[bitcoin#8683] [6924212] [D] Fix incorrect file name bitcoin.qrc, update translation_process.md
[bitcoin#8680] [c2a601f] [I] Add waitfornewblock/waitforblock/waitforblockheight rpcs
[bitcoin#8563] [b90d761] [B] Add configure check for -latomic
[bitcoin#8677] [0324fa0] [R] Do not shadow upper local variable 'send', prevent -Wshadow compiler warning.
[bitcoin#8681] [3baee7b] [F] Performance Regression Fix: Pre-Allocate txChanged vector
[bitcoin#8928] [fdd0638] [F] Fix init segfault where InitLoadWallet() calls ATMP before genesis
[master#1807]  [81fb931] [F] Don't delay GETHEADERS when no blocks have arrived yet in devnet
[master@1844]  [c2de362] [F] Actually honor fMiningRequiresPeers in getblocktemplate
[bitcoin#8638] [bc91f34] [F] rest.cpp: change HTTP_INTERNAL_SERVER_ERROR to HTTP_BAD_REQUEST
[bitcoin#8640] [a2d18b4] [D] Remove old Qt46 package
[bitcoin#8604] [75f65e9] [F] BerkeleyDB v6 compatibility fix