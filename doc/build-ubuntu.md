UBUNTU BUILD NOTES
====================

Some notes on how to build Binarium Core for Ubuntu.
Using depend system and cross-compilation to native platform is the best way. 

Cross-compilation
-------------------

These steps can be performed on, for example, an Ubuntu VM. The depends system
will also work on other Linux distributions, however the commands for
installing the toolchain will be different.

Make sure you install the build requirements mentioned in
[build-unix.md](/doc/build-unix.md).
Then, install the toolchains and curl:

    sudo apt-get install g++-mingw-w64-i686 mingw-w64-i686-dev g++-mingw-w64-x86-64 mingw-w64-x86-64-dev curl

Freetype2 config libraries required

    wget https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.xz
    tar -xf ./freetype-2.10.4.tar.xz
    cd ./freetype-2.10.4
    ./configure --prefix=/usr --enable-freetype-config --disable-static
    make
    make install

To build executables for Ubuntu 64-bit:

    cd depends
    make HOST=x86_64-linux-gnu -j4
    cd ..
    ./configure --prefix=`pwd`/depends/x86_64-linux-gnu
    make

For further documentation on the depends system see [README.md](../depends/README.md) in the depends directory.
