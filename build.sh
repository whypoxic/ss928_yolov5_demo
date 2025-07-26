####!/bin/sh
# 关闭输出日志
export ASCEND_GLOBAL_LOG_LEVEL=3
export ASCEND_SLOG_PRINT_TO_STDOUT=0
export MS_PROFILE_MODE=0

# 环境变量
export LD_LIBRARY_PATH=/opt/linux/x86-arm/aarch64-mix210-linux/lib:$LD_LIBRARY_PATH

# 指定交叉编译器
export CC=/opt/linux/x86-arm/aarch64-mix210-linux/bin/aarch64-mix210-linux-gcc
export CXX=/opt/linux/x86-arm/aarch64-mix210-linux/bin/aarch64-mix210-linux-g++

# 清除主机可能带入的编译参数
unset CFLAGS
unset CXXFLAGS

rm -rf build/

mkdir -p build/
pushd build/
# cmake ..
cmake .. \
  -DCMAKE_C_COMPILER=${CC} \
  -DCMAKE_CXX_COMPILER=${CXX} \
  -DCMAKE_C_FLAGS="" \
  -DCMAKE_CXX_FLAGS=""
  
make -j8
popd

