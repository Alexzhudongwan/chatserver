#!/bin/bash
set -x
# 清理构建目录
rm -rf ./build/*
# 进入build文件夹（核心！）
cd build
# 编译项目
cmake .. && make
