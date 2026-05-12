//
// Created by 31132 on 2026/4/26.
//
#include "chatservice.h"

#include <user.h>

#include "public.h"
#include <vector>
#include "muduo/base/Logging.h"
using namespace std;
ChatService *ChatService::getInstance() {
    static  ChatService instance;
    return &instance;
}
ChatService::ChatService() {
    _MsgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _MsgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _MsgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::onechat,this,_1,_2,_3)});
    _MsgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addfriend,this,_1,_2,_3)});
    _MsgHandlerMap.insert({CHEATE_GROUP_MSG,std::bind(&ChatService::creategroup,this,_1,_2,_3)});
    _MsgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _MsgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _MsgHandlerMap.insert({LOGOUT_MSG,std::bind(&ChatService::logout,this,_1,_2,_3)});
    if (_redis.connect()) {
        _redis.init_notify_Handler(bind(&ChatService::handleredissubscribemessage,this,_1,_2));
    }
}
//登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int id=js["id"].get<int>();
    string password=js["password"];
    User user=_usermodel.query(id);
    if (user.getId()==id&&user.getPassword()==password) {
        if (user.getState()=="online") {
            json response;
            response["errno"]=2;
            response["errmsg"]="This account is using ,intput another,please!";
            response["msgid"]=LOGIN_MSG_ACK;
            conn->send(response.dump());
        }else {
            {
                lock_guard<mutex> _lock(_mutex);
                _userconnmap.insert({id,conn});

            }
            //订阅通道

            _redis.subscribe(id);

            user.setState("online");
            _usermodel.updateState(user);
            json response;
            response["id"]=user.getId();
            response["name"]=user.getName();
            response["errno"]=0;
            response["msgid"]=LOGIN_MSG_ACK;
            vector<string> vec1=_offlinemsgmodel.query(id);;
            if (!vec1.empty()) {
                response["offlinemsg"]=vec1;
                _offlinemsgmodel.remove(id);
            }
            vector<string> vec2;
            vector<User> msg=_friendmodel.queryfriend(id);
            for (auto it=msg.begin();it!=msg.end();++it) {
                json fr;
                fr["id"]=it->getId();
                fr["name"]=it->getName();
                fr["state"]=it->getState();
                vec2.push_back(fr.dump());
            }
            response["friend"]=vec2;
            //该用户所在的群组信息
            vector<Group> grpvec=_groupmodel.queryGroups(id);
            if (!grpvec.empty()) {
                vector<string> vec3;
                for (auto it=grpvec.begin();it!=grpvec.end();++it) {
                    json grp;
                    grp["id"]=it->getId();
                    grp["groupname"]=it->getName();
                    grp["groupdesc"]=it->getDescription();
                    vector<string> groupusers;
                    for (auto &gruser : it->getUsers()) {
                        json jsuser;
                        jsuser["id"]=gruser.getId();
                        jsuser["name"]=gruser.getName();
                        jsuser["state"]=gruser.getState();
                        jsuser["role"]=gruser.getRole();
                        groupusers.push_back(jsuser.dump());
                    }
                    grp["users"]=groupusers;
                    vec3.push_back(grp.dump());
                }
                response["groups"]=vec3;
            }
            conn->send(response.dump());
        }
    }else {
        json response;
        response["errno"]=1;
        response["msgid"]=LOGIN_MSG_ACK;
        response["errmsg"]="id or password is invalid!";
        conn->send(response.dump());
    }

}
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    string name = js["name"];
    string password = js["password"];
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state=_usermodel.insertUser(user);
    if (state) {
        json response;
        response["id"]=user.getId();
        response["errno"]=0;
        response["msgid"]=REG_MSG_ACK;
        conn->send(response.dump());
    }else {
        json response;
        response["errno"]=1;
        response["msgid"]=REG_MSG_ACK;
        conn->send(response.dump());
    }
}
MsgHandler ChatService::getmsghandler(int msgId) {
    if (_MsgHandlerMap.find(msgId) != _MsgHandlerMap.end()) {
        return _MsgHandlerMap[msgId];
    }else {
        return [=](const TcpConnectionPtr& conn,json& js,Timestamp time) {
            LOG_ERROR <<"msgId:"<<msgId<<"not found msghandler";
        };
    }
}
void ChatService::logout(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int id=js["id"].get<int>();
    auto it=_userconnmap.find(id);
    {
        lock_guard<mutex> _lock(_mutex);
        if (it!=_userconnmap.end()) {
            _userconnmap.erase(it);
        }
    }
    _redis.unsubscribe(id);
    User user(id,"","","offline");
    _usermodel.updateState(user);
}

void ChatService::ClientCloseException(const TcpConnectionPtr& conn) {
    User user;
    {
        lock_guard<mutex> _lock(_mutex);
        for (auto it=_userconnmap.begin(); it!=_userconnmap.end(); it++) {
            if (it->second==conn) {
                user.setId(it->first);
                _userconnmap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(user.getId());
    if (user.getId()!=-1) {
        user.setState("offline");
        _usermodel.updateState(user);
    }
}
void ChatService::reset() {
    _usermodel.resetState();
}

void ChatService::onechat(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int toid=js["to"].get<int>();
    {
        lock_guard<mutex> _lock(_mutex);
        auto it=_userconnmap.find(toid);
        if (it!=_userconnmap.end()) {
            //对方用户在线
            it->second->send(js.dump());
            return;
        }
    }
    User user=_usermodel.query(toid);
    if (user.getState()=="online") {
        _redis.publish(user.getId(),js.dump());
        return;
    }
    //对方用户不在线
    _offlinemsgmodel.insert(toid,js.dump());
}
void ChatService::addfriend(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int userid=js["userid"].get<int>();
    int friendid=js["friendid"].get<int>();

    _friendmodel.insertfriend(userid,friendid);
}
void ChatService::creategroup(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int userid=js["id"].get<int>();
    string name=js["groupname"].get<std::string>();
    string desc=js["groupdesc"].get<std::string>();
    Group group(userid,name,desc);
    if (_groupmodel.createGroup(group)) {
        _groupmodel.addGroup(userid,group.getId(),"creator");
    }
}
void ChatService::addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();
    _groupmodel.addGroup(userid,groupid,"normal");
}
void ChatService::groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time) {
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();
    vector<int> useridvec=_groupmodel.queryGroupUsers(groupid,userid);
    lock_guard<mutex> _lock(_mutex);
    {
        for (int id:useridvec) {
            auto it=_userconnmap.find(id);
            if (it!=_userconnmap.end()) {
                it->second->send(js.dump());
            }else {
                User user=_usermodel.query(id);
                if (user.getState()=="online") {
                    _redis.publish(id,js.dump());
                }else {
                    _offlinemsgmodel.insert(id,js.dump());
                }
            }
        }
    }
}
void ChatService::handleredissubscribemessage(int channel,string message) {
        lock_guard<mutex> _lock(_mutex);
        auto it=_userconnmap.find(channel);
    if (it!=_userconnmap.end()) {
        it->second->send(message);
        return;
    }
    _offlinemsgmodel.insert(channel,message);
}

