//
// Created by 31132 on 2026/4/26.
//

#ifndef CHAT_PUBLIC_H
#define CHAT_PUBLIC_H
//server和client的公共文件
enum EnMsgType{
    LOGIN_MSG=1,//登录
    LOGIN_MSG_ACK,//登录回应
    REG_MSG,//注册
    REG_MSG_ACK,//注册回应
    ONE_CHAT_MSG,//单对单聊天
    ADD_FRIEND_MSG,//添加好友
    CHEATE_GROUP_MSG,//创建群组
    ADD_GROUP_MSG,//加入群组
    GROUP_CHAT_MSG,//群聊天
    LOGOUT_MSG
};
#endif //CHAT_PUBLIC_H
