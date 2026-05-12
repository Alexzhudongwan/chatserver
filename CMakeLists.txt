cmake_minimum_required(VERSION 3.16)
project(chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Muduo 安装路径
set(MUDUO_ROOT "/home/linxiao/build/release-install-cpp11")

# 可执行文件输出到根目录bin
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 加载src子目录
add_subdirectory(src)