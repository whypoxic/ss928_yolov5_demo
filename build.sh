####!/bin/sh

mkdir -p build/
pushd build/
cmake ..
make -j8
popd

