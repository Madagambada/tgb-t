#!/bin/bash
echo "x64 dependencies builder 1.0.0"

zlibArchive='https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.xz'
caresArchive='https://github.com/c-ares/c-ares/releases/download/cares-1_19_0/c-ares-1.19.0.tar.gz'
wolfSSLArchive='https://github.com/wolfSSL/wolfssl/archive/refs/tags/v5.5.4-stable.tar.gz'
nghttp2Archive='https://github.com/nghttp2/nghttp2/releases/download/v1.52.0/nghttp2-1.52.0.tar.xz'
curlArchive='https://github.com/curl/curl/releases/download/curl-7_88_1/curl-7.88.1.tar.xz'

echo -n "Install tools... "
apt update && apt install tar xz-utils curl cmake make autoconf libtool -y

echo -n "Create dependencies directory... "
mkdir dependencies
cd dependencies

echo -n "Get latest musl toolchain... "
curl -s -L https://musl.cc/x86_64-linux-musl-native.tgz | tar zx

echo -n "Set toolchain vars... "
export TOOLCHAIN=$(pwd)/x86_64-linux-musl-native
export AR=$TOOLCHAIN/bin/x86_64-linux-musl-gcc-ar
export AS=$TOOLCHAIN/bin/as
export CC=$TOOLCHAIN/bin/x86_64-linux-musl-gcc
export CXX=$TOOLCHAIN/bin/x86_64-linux-musl-g++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/x86_64-linux-musl-gcc-ranlib
export STRIP=$TOOLCHAIN/bin/strip


#zlib
echo -n "Download and Extract zlib... "
curl -s -L $zlibArchive | tar --xz -x

echo -n "Configure zlib... "
cd zlib*
./configure --prefix=$TOOLCHAIN --static

echo -n "Build zlib... "
make -j$(nproc)

echo -n "Install zlib... "
make install 
cd ..


#c-ares
echo -n "Download and Extract c-ares... "
curl -s -L $caresArchive | tar zx

echo -n "Configure c-ares... "
cd c-ares*
./configure --prefix=$TOOLCHAIN --host=x86_64-linux-musl --disable-shared 

echo -n "Build c-ares... "
make -j"$(nproc)" 

echo -n "Install c-ares... "
make install 
cd ..


#wolfSSL
echo -n "Download and Extract wolfSSL... "
curl -s -L $wolfSSLArchive | tar xz

echo -n "Configure wolfSSL... "
cd wolfssl*
./autogen.sh 
./configure --host=x86_64-linux-musl --enable-curl --prefix=$TOOLCHAIN --enable-static --disable-shared --enable-all-crypto --with-libz=$TOOLCHAIN 

echo -n "Build wolfSSL... "
make -j"$(nproc)" 

echo -n "Install wolfSSL... "
make install 
cd ..


#nghttp2
echo -n "Download and Extract nghttp2... "
curl -s -L $nghttp2Archive | tar --xz -x

echo -n "Configure nghttp2... "
cd nghttp2*
./configure --host=x86_64-linux-musl --enable-lib-only --disable-shared --prefix=$TOOLCHAIN

echo -n "Build nghttp2... "
make -j"$(nproc)" 

echo -n "Install nghttp2... "
make install
cd ..


#cURL
echo -n "Download and Extract curl... "
curl -s -L $curlArchive | tar --xz -x

echo -n "Configure curl... "
cd curl*
./configure --host=x86_64-linux-musl --disable-shared --prefix=$TOOLCHAIN --with-wolfssl=$TOOLCHAIN --enable-ares=$TOOLCHAIN --with-nghttp2=$TOOLCHAIN 

echo -n "Build cURL... "
make -j"$(nproc)" 

echo -n "Install cURL... "
make install 

echo -e "\e[32mAll done\e[0m"


exit 0
