#!/usr/bin/env bash

##### linux for aarch64-mix210-linux- toolchain
mkdir -p build-aarch64-mix210-linux-
pushd build-aarch64-mix210-linux-
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/aarch64-mix210-linux.toolchain.cmake -DNCNN_SIMPLEOCV=ON -DNCNN_BUILD_EXAMPLES=ON  -DNCNN_OPENMP=OFF ..
make -j4
make install
popd
