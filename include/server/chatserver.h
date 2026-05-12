//
// Created by 31132 on 2026/4/26.
//

#ifndef MYMUDUOPROJECT_CHATSERVER_H
#define MYMUDUOPROJECT_CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
class ChatServer
{
public:
    ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//IP+PORT
            const string& nameArg);
    void start() ;
private:
    //专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr & conn) ;
    void onMessage(const TcpConnectionPtr &conn,//连接
        Buffer *buffer,//缓冲区
        Timestamp time//接受到数据的时候的时间信息
        ) ;
    TcpServer _server;
    EventLoop *_loop;
};


#endif //MYMUDUOPROJECT_CHATSERVER_H
