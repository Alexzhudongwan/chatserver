set(SERVER_SRCS
        main.cpp
        chatserver.cpp
        chatservice.cpp
        db/db.cpp
        model/usermodel.cpp
        model/offlinemessagemodel.cpp
        model/friendmodel.cpp
        model/groupmodel.cpp
        redis/redis.cpp
        ../../test/test.cpp
)

add_executable(ChatServer ${SERVER_SRCS})

# 头文件（100%覆盖所有路径）
target_include_directories(ChatServer PRIVATE
        ${MUDUO_ROOT}/include
        /home/linxiao/nlohmann-json/include
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/server
        ${PROJECT_SOURCE_DIR}/include/server/db
        ${PROJECT_SOURCE_DIR}/include/server/model
        /usr/include/mysql
        /usr/local/include
)

# 库路径
target_link_directories(ChatServer PRIVATE
        ${MUDUO_ROOT}/lib
        /usr/local/lib
)

# 【锁死链接】直接用绝对路径链接hiredis，绝无报错！
target_link_libraries(ChatServer
        muduo_net
        muduo_base
        mysqlclient
        pthread
        /usr/local/lib/libhiredis.so
)