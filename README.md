# chat-cabin
可以工作在nginx tcp负载均衡环境中的集群聊天服务器和客户端源码 基于muduo库实现
 1. Muduo 网络库 —— Reactor 多线程模型

  - chatserver.h:24 —— _server.setThreadNum(4) 配置 1 个 I/O 线程 + 3 个 Worker 线程
  - 网络层与业务层完全解耦：ChatServer 只负责连接/消息回调，通过 ChatService::getInstance() 单例路由到具体业务处理函数
  - chatserver.cpp:43-44 —— 根据 msgid 动态分发到对应 handler，典型命令模式

  auto handler = ChatService::getInstance()->getmsghandler(js["msgid"].get<int>());
  handler(conn, js, time);

  2. Redis 发布/订阅 —— 跨服务器消息路由

  - redis.h —— 封装 hiredis C 库，维护独立的 _publish_redis 和 _subscribe_redis 上下文（发布和订阅必须分开连接）
  - 用户登录时 subscribe(id) 订阅自己的消息通道
  - 当用户在线但不在此服务器实例上（跨服务器场景），通过 publish(channel, message) 转发消息
  - 独立线程 observer_channel_message() 阻塞接收订阅消息，通过回调 _notify_message_Handler 上报业务层

  // chatservice.cpp:173-191 onechat 的三级路由策略：
  // 1. 本服务器直发 (_userconnmap 命中)
  // 2. Redis publish 跨服务器转发 (DB 状态为 online 但不在本地)
  // 3. 离线消息落库 (对方不在线)

  3. MySQL 数据持久化 —— 6 张业务表

  ┌────────────────┬───────────────────────┬──────────────────────┐
  │       表       │      对应 Model       │         功能         │
  ├────────────────┼───────────────────────┼──────────────────────┤
  │ User           │ usermodel.h           │ 注册、登录、状态更新 │
  ├────────────────┼───────────────────────┼──────────────────────┤
  │ Friend         │ friendmodel.h         │ 好友关系维护         │
  ├────────────────┼───────────────────────┼──────────────────────┤
  │ OfflineMessage │ offlinemessagemodel.h │ 离线消息暂存与清理   │
  ├────────────────┼───────────────────────┼──────────────────────┤
  │ AllGroup       │ groupmodel.h          │ 创建群组             │
  ├────────────────┼───────────────────────┼──────────────────────┤
  │ GroupUser      │ groupmodel.h          │ 群成员管理           │
  └────────────────┴───────────────────────┴──────────────────────┘

  - db.h —— 基于 MySQL C API 封装，RAII 管理连接生命周期

  4. 单例模式 + 线程安全

  - chatservice.h:21 —— ChatService 饿汉式单例，getInstance() 返回静态局部对象
  - std::mutex 保护 _userconnmap 的并发读写（登录、退出、异常断开、消息转发均需加锁）
  - 信号处理 SIGINT → reset() → 所有在线用户批量置为 offline

  5. nlohmann/json 序列化

  - 客户端与服务端全部以 JSON 文本通信，通过 public.h 中的 EnMsgType 枚举标记消息类型（LOGIN_MSG=1、ONE_CHAT_MSG=5 等）
  - json::parse() / json::dump() 完成序列化与反序列化

  6. 原生 Socket 客户端

  - client/main.cpp 直接用 POSIX socket API（socket() / connect() / send() / recv()），不依赖任何上层网络库
  - 读线程通过 sem_t 信号量与主线程同步登录/注册应答
  - 命令解析采用 command:arg1:arg2 格式，std::unordered_map 映射到处理函数

  7. CMake 自动化构建

  - 服务端链路：muduo_net + muduo_base + mysqlclient + hiredis + pthread
  - 客户端仅链接 pthread
  - 可执行文件统一输出到 bin/

  ---
  消息流转示例（单对单聊天）

  ClientA 发送 chat:1002:hello
    │
    ▼
  ChatServer::onMessage 解析 JSON → msgid=ONE_CHAT_MSG
    │
    ▼
  ChatService::onechat
    ├─ 目标在本服务器 → 直接通过 _userconnmap 发送
    ├─ 目标在其他服务器但 online → Redis publish(channel=1002, msg)
    └─ 目标 offline → INSERT 到 OfflineMessage 表
技术架构

  ┌───────────────┬──────────────────────────────────────────────────┐
  │      层       │                      技术栈                      │
  ├───────────────┼──────────────────────────────────────────────────┤
  │ 网络层        │ Muduo (Reactor 模式，1 I/O 线程 + 3 Worker 线程) │
  ├───────────────┼──────────────────────────────────────────────────┤
  │ 序列化        │ nlohmann/json                                    │
  ├───────────────┼──────────────────────────────────────────────────┤
  │ 持久化        │ MySQL（用户、好友、群组、离线消息）              │
  ├───────────────┼──────────────────────────────────────────────────┤
  │ 缓存/跨服务器 │ Redis（发布/订阅，用于多服务器消息路由）         │
  ├───────────────┼──────────────────────────────────────────────────┤
  │ 构建工具      │ CMake                                            │
  └───────────────┴──────────────────────────────────────────────────┘

  项目结构

  chat-cabin/
  ├── include/
  │   ├── public.h                    # 消息类型枚举(客户端+服务端共用)
  │   └── server/
  │       ├── chatserver.h            # 网络层：封装 Muduo TcpServer
  │       ├── chatservice.h           # 业务层：单例，消息分发+业务逻辑
  │       ├── db/db.h                 # MySQL 数据库封装 (C API)
  │       ├── redis/redis.h           # Redis 发布/订阅封装 (hiredis)
  │       └── model/
  │           ├── user.h              # User 实体类
  │           ├── usermodel.h         # User 表 CRUD
  │           ├── friendmodel.h       # Friend 关系表
  │           ├── group.h / groupuser.h  # Group、GroupUser 实体
  │           ├── groupmodel.h        # 群组表 CRUD
  │           └── offlinemessagemodel.h  # 离线消息表
  ├── src/
  │   ├── server/                     # ChatServer 服务端实现
  │   │   ├── main.cpp                # 入口: ./ChatServer <ip> <port>
  │   │   ├── chatserver.cpp          # 网络层：连接/消息回调
  │   │   ├── chatservice.cpp         # 业务层：8种消息类型处理
  │   │   ├── db/db.cpp               # MySQL 连接池实现
  │   │   ├── redis/redis.cpp         # Redis pub/sub 实现
  │   │   └── model/*.cpp             # 各 Model 实现
  │   └── client/
  │       └── main.cpp                # 命令行客户端: ./ChatClient <ip> <port>
  ├── test/test.cpp
  ├── thirdparty/                     # 第三方依赖
  └── CMakeLists.txt

  已实现的功能

  1. 用户注册/登录 —— MySQL 存储用户信息，支持状态管理（online/offline）
  2. 单对单聊天 —— 根据对方在线状态，选择直发、Redis 跨服务器转发、或离线消息存储
  3. 添加好友 —— 登录时自动获取好友列表及在线状态
  4. 创建群组 / 加入群组 —— 支持 creator 和 normal 角色
  5. 群组聊天 —— 群发消息给群内所有成员（过滤自己）
  6. 离线消息 —— 用户离线期间的消息暂存，登录后批量推送并删除
  7. 跨服务器通信 —— 通过 Redis 发布/订阅机制，支持多服务器实例间的消息路由
  8. 异常断开处理 —— 客户端异常断开或服务器 SIGINT 信号，自动重置用户状态为 offline

  客户端交互

  命令行客户端支持以下命令：

  ┌─────────────┬─────────────────────────────────┐
  │    命令     │              格式               │
  ├─────────────┼─────────────────────────────────┤
  │ help        │ 查看所有命令                    │
  ├─────────────┼─────────────────────────────────┤
  │ chat        │ chat:friendid:message           │
  ├─────────────┼─────────────────────────────────┤
  │ addfriend   │ addfriend:friendid              │
  ├─────────────┼─────────────────────────────────┤
  │ creategroup │ creategroup:groupname:groupdesc │
  ├─────────────┼─────────────────────────────────┤
  │ addgroup    │ addgroup:groupid                │
  ├─────────────┼─────────────────────────────────┤
  │ groupchat   │ groupchat:groupid:message       │
  ├─────────────┼─────────────────────────────────┤
  │ logout      │ 退出登录                        │
  └─────────────┴─────────────────────────────────┘

  总的来说，这是一个学习 Muduo 网络库 + MySQL + Redis 的综合性项目，涵盖了 Reactor 网络编程、数据库 CRUD、Redis 发布/订阅、单例模式、线程安全等技术点。
