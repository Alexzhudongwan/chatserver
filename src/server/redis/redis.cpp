//
// Created by 31132 on 2026/5/10.
//
#include "redis/redis.h"
#include <iostream>
// 新增：用于atoi
#include <cstdlib>

using namespace std;

Redis::Redis() : _publish_redis(nullptr), _subscribe_redis(nullptr) {}

Redis::~Redis() {
    if (_publish_redis != nullptr) {
        redisFree(_publish_redis);
    }
    if (_subscribe_redis != nullptr) {
        redisFree(_subscribe_redis);
    }
}

bool Redis::connect() {
    // 1. 连接发布上下文
    _publish_redis = redisConnect("127.0.0.1", 6379);
    if (_publish_redis == nullptr || _publish_redis->err) {
        cerr << "Redis 发布连接失败！" << endl;
        return false;
    }

    // 2. 【修复】订阅连接也用 redisConnect，没有redisSubscribe！
    _subscribe_redis = redisConnect("127.0.0.1", 6379);
    if (_subscribe_redis == nullptr || _subscribe_redis->err) {
        cerr << "Redis 订阅连接失败！" << endl;
        return false;
    }

    // 启动监听线程
    thread t(&Redis::observer_channel_message, this);
    t.detach();

    cout << "Redis 连接成功！" << endl;
    return true;
}

bool Redis::publish(int channel, string message) {
    // 执行publish命令
    redisReply* reply = (redisReply*)redisCommand(_publish_redis, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr) {
        cerr << "Redis 发布消息失败！" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel) {
    // 订阅命令
    if (REDIS_ERR == redisAppendCommand(_subscribe_redis, "SUBSCRIBE %d", channel)) {
        cerr << "Redis 订阅失败！" << endl;
        return false;
    }

    int done = 0;
    while (!done) {
        // 【修复】= 改为 == 判断！！！
        if (REDIS_ERR == redisBufferWrite(_subscribe_redis, &done)) {
            cerr << "Redis 订阅缓冲区写入失败！" << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(_subscribe_redis, "UNSUBSCRIBE %d", channel)) {
        cerr << "Redis 取消订阅失败！" << endl;
        return false;
    }

    int done = 0;
    while (!done) {
        // 【修复】= 改为 == 判断！！！
        if (REDIS_ERR == redisBufferWrite(_subscribe_redis, &done)) {
            cerr << "Redis 取消订阅缓冲区写入失败！" << endl;
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message() {
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(_subscribe_redis, (void**)&reply)) {
        if (reply != nullptr && reply->elements == 3) {
            // 【修复】判断回调函数是否初始化，防止崩溃
            if (_notify_message_Handler&&reply->element[1] != nullptr && reply->element[1]->str != nullptr &&
                reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
                int channel = atoi(reply->element[1]->str);
                string msg = reply->element[2]->str;
                _notify_message_Handler(channel, msg);
            }
        }
        freeReplyObject(reply);
    }
}

void Redis::init_notify_Handler(function<void(int, string)> fn) {
    _notify_message_Handler = fn;
}

