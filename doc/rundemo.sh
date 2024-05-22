#!/bin/sh

# 设置 LD_LIBRARY_PATH
export LD_LIBRARY_PATH=${PWD}/lib:${PWD}/lib/npu:${PWD}/lib/svp_npu:$LD_LIBRARY_PATH

# 杀死进程
killall -9 ss928_yolov5

# 执行命令，传入参数
./ss928_yolov5 "$@"

