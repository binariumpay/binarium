DEBIAN BUILD NOTES
====================

Prepare build environment and install dependencies
------------------

	apt-get install gcc autoconf build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils libboost-all-dev libminiupnpc-dev libzmq3-dev

Use only absolute paths for autogen.sh, configure and their parameters for prevent some issues

	/full/path/to/binarium/autogen.sh
	
We recommend build the berkeley db yourself and use it for compilation. For
instructions see https://github.com/binariumpay/binarium/blob/master/doc/build-unix.md#berkeley-db

Or run the

	./berkeley.sh

After these steps, BDB libraries will be here /full/path/to/binarium/db4

BDB required for wallet functionality.

Configuration & Build
-------------------

Example configure string for server variant of binaries (No Qt GUI)

Use flag CFLAGS='-std=c11' for successfully build some components (kuznetchik).

	./configure --enable-cxx --disable-shared \
	--without-gui \
	--prefix='/usr/bin/' \
	CFLAGS='-std=c11' \
	CPPFLAGS='-I/full/path/to/binarium/db4/include/ -O2' \
	LDFLAGS='-L/full/path/to/binarium/db4/lib/'
	
	make
	make install
	
Tested on Debian GNU/Linux 8.10 (jessie) x86_64 (gcc 4.9.2)
