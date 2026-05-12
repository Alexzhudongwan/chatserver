#include <hiredis/hiredis.h>
#include <functional>
#include <thread>
using namespace std;
class Redis {
    public:
    Redis() ;
    ~Redis() ;
    //链接redis
    bool connect() ;
    //订阅
    bool subscribe(int channel) ;
    bool unsubscribe(int channel) ;
    //发布
    bool publish(int channel,string message) ;
    //在独立线程中接收订阅通道的消息
    void observer_channel_message();
    //初始化业务层，上报消息的回调对象
    void init_notify_Handler(function<void(int,string)> fn);


private:
    redisContext *_subscribe_redis;
    redisContext *_publishe_redis;
    function<void(int,string)> _notify_message_Handler;

};
