DEBIAN BUILD NOTES
====================

Use only absolute paths for autogen.sh, configure and their parameters for prevent some issues

	/full/path/to/binarium/autogen.sh
	
We recommend build the berkeley db yourself and use it for compilation. For
instructions see https://github.com/binariumpay/binarium/blob/master/doc/build-unix.md#berkeley-db

After these steps, BDB libraries will be here /full/path/to/binarium/db4

BDB required for wallet functionality.

Configuration
-------------------

Example configure string for server variant of binaries (No Qt GUI)

Use flag CFLAGS='-std=c11' for successfully build some components (kuznetchik).

	/full/path/to/binarium/configure --enable-cxx --disable-shared \
	--prefix=/full/path/to/binarium/db4 \
	--without-gui \
	CFLAGS='-std=c11' \
	CPPFLAGS='-I/full/path/to/binarium/db4/include/ -O2' \
	LDFLAGS='-L/full/path/to/binarium/db4/lib/'
	
Tested on Debian GNU/Linux 8.10 (jessie) x86_64