//
// Created by 31132 on 2026/4/26.
//
#include "chatserver.h"
#include <functional>
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"
#include "chatservice.h"
using namespace std;
using namespace placeholders;
using json=nlohmann::json;
ChatServer::ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//IP+PORT
            const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop) {

    //给服务器用户连接的创建和断开设置回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));

    //给服务器注册用户的读写事件设置回调
    _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数量 1个I/O线程，3个worker线程
    _server.setThreadNum(4);
}
void ChatServer::start() {
    _server.start();
}
void ChatServer::onConnection(const TcpConnectionPtr & conn) {
        if (!conn->connected()) {
            ChatService::getInstance()->ClientCloseException(conn);
            conn->shutdown();
        }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn,//连接
        Buffer *buffer,//缓冲区
        Timestamp time//接受到数据的时候的时间信息
        ) {
    string buf=buffer->retrieveAllAsString();
    json js=json::parse(buf);
    //完全解耦网络模块和业务模块的代码
    //通过js["msgid"]来获得=》业务handler=》conn js time
    auto handler=ChatService::getInstance()->getmsghandler(js["msgid"].get<int>());
    handler(conn,js,time);
}

