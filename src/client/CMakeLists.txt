# 客户端源文件（只有自己的main.cpp，绝对不能包含服务端文件！）
set(CLIENT_SRCS
        main.cpp
)

# 生成ChatClient
add_executable(ChatClient ${CLIENT_SRCS})

# 头文件路径（客户端只需要json和公共头文件）
# 客户端头文件路径（修正后）
target_include_directories(ChatClient PRIVATE
        /home/linxiao/nlohmann-json/include
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/server/model # 新增：user.h所在目录
)

# 客户端只需要链接pthread
target_link_libraries(ChatClient pthread)