//
// Created by 31132 on 2026/4/26.
//

#ifndef CHAT_CHATSERVICE_H
#define CHAT_CHATSERVICE_H
#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis/redis.h"
#include<unordered_map>
#include<muduo/net/TcpConnection.h>
#include<mutex>
#include"nlohmann/json.hpp"
using namespace muduo::net;
using namespace muduo;
using json=nlohmann::json;
using MsgHandler=std::function<void(const TcpConnectionPtr& ,json&,Timestamp )>;
using namespace std::placeholders;
class ChatService{
public:
    //获取单例模式
    static ChatService* getInstance();
    //登录业务
    void login(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //注册业务
    void reg(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //获取msgid对应的事件回调函数
    MsgHandler getmsghandler(int msgid);
    //客户端异常断开，处理登录状态
    void ClientCloseException(const TcpConnectionPtr& conn);
    //单对单聊天
    void onechat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //登录状态均改为offline（暂用于服务器异常断开）
    void reset();
    //添加好友
    void addfriend(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //创建群组
    void creategroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //注销登录
    void logout(const TcpConnectionPtr& conn,json& js,Timestamp time);

    void handleredissubscribemessage(int channel,string message);
private:
    ChatService();//绑定msgid 与 事件处理回调函数
    std::unordered_map<int,MsgHandler> _MsgHandlerMap;//msgid与事件回调函数
    std::unordered_map<int,TcpConnectionPtr> _userconnmap;//用户id与客户端套接字
    std::mutex _mutex;
    UserModel _usermodel;//USE表

    OfflineMsgModel _offlinemsgmodel;//OFFLINEMSG表

    FriendModel _friendmodel;//FRIEND表

    GroupModel _groupmodel;//allgroup，groupuser表

    Redis _redis;//redis中间件
};
#endif //CHAT_CHATSERVICE_H
