//
// Created by 31132 on 2026/5/10.
//

#ifndef CHAT_REDIS_H
#define CHAT_REDIS_H

#include "/usr/local/include/hiredis/hiredis.h"
#include <functional>
#include <thread>
#include <string>  // 新增：必须加string头文件

// 【删除】头文件禁止 using namespace std;

class Redis {
public:
    Redis();
    ~Redis();

    // 连接redis
    bool connect();
    // 订阅
    bool subscribe(int channel);
    // 取消订阅
    bool unsubscribe(int channel);
    // 发布
    bool publish(int channel, std::string message);  // 补全std::
    // 在独立线程中接收订阅通道的消息
    void observer_channel_message();
    // 初始化业务层上报消息的回调对象
    void init_notify_Handler(std::function<void(int, std::string)> fn);

private:
    // 修复拼写错误 publishe → publish
    redisContext* _publish_redis;   // 发布上下文
    redisContext* _subscribe_redis; // 订阅上下文

    std::function<void(int, std::string)> _notify_message_Handler;
};

#endif //CHAT_REDIS_H
