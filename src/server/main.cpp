//
// Created by 31132 on 2026/4/26.
//
#include "chatserver.h"
#include "chatservice.h"
#include <signal.h>
#include <iostream>
using namespace std;
void sigintHandler(int) {
    ChatService::getInstance()->reset();
    exit(0);
}
int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Please use the following command line" << endl<< "example :" << " ./ChatServer 127.0.0.1 6000 "<< endl;
        exit(-1);
    }
    // 规范类型：const char*
    const char *ip = argv[1];
    // 端口安全转换
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    signal(SIGINT,sigintHandler);
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop,addr,"chatserver");
    server.start();
    loop.loop();
    return 0;
}