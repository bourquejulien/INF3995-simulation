#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

export MY_INSTALL_DIR=$PWD/build_grpc
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"
sudo apt install -y build-essential autoconf libtool pkg-ldconfig

git clone --recurse-submodules -b v1.48.1 --depth 1 --jobs 8 --shallow-submodules https://github.com/grpc/grpc

cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ../..
make -j $(($(nproc)-$(nproc)/2))
make install
popd

cd ..
rm -rf grpc/
