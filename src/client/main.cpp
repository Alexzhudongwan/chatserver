//
// Created by 31132 on 2026/5/6.
//
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <errno.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <nlohmann/json_fwd.hpp>
#include <thread>
#include <threads.h>
#include <thread_db.h>

#include "nlohmann/json.hpp"
#include "user.h"
#include "group.h"
#include "friendmodel.h"
#include"public.h"
using namespace std;
using json=nlohmann::json;

User g_currentUser;
vector<User> g_currentUserFriends;
vector<Group> g_currentUserGroups;
//展示当前用户的信息
void showCurrentUserData();
//接收线程
void readHandlerTask(int clientFd);
//显示当前时间
string getCurrentTime();
//主聊天页面
void mainMenu(int clientFd);

bool ismainMenuing = false;

//获取当前时间
string getCurrentTime() ;

void help(int clientFd=0,string command="");

void chat(int clientFd,string command);

void addfriend(int clientFd,string command);

void creategroup(int clientFd,string command);

void addgroup(int clientFd,string command);

void groupchat(int clientFd,string command);

void logout(int clientFd,string command);

unordered_map<string,string> commandMap{
    {"help","显示提供的所有命令，格式：help"},
    {"chat","与好友聊天，格式：chat:friendid:message"},
    {"addfriend","添加好友，格式：addfriend:friendid"},
        {"creategroup","创建群组，格式：creategroup:groupname:groupdesc"},
    {"groupchat","群组聊天，格式：groupchat:groupid:message"},
    {"addgroup","加入群组，格式：addgroup:groupid"},
    {"logout","退出登录，格式：logout"}

};
unordered_map<string,function<void(int,string)>> commandHandlerMap{
        {"help",help},
        {"chat",chat},
        {"addfriend",addfriend},
       {"creategroup",creategroup},
        {"groupchat",groupchat},
        {"addgroup",addgroup},
        {"logout",logout}
};

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Please use the following command line" << endl<< "example :" << " ./ChatClient 127.0.0.1 8080 "<< endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientFd=socket(AF_INET,SOCK_STREAM,0);
    if (clientFd<0) {
        cerr << "Error creating socket" << endl;
        exit(-1);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    if (connect(clientFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0) {
        cerr << "Error connecting" << endl;
        close(clientFd);
        exit(-1);
    }
    for (;;) {
        //显示手页面的菜单 登录 注册 退出
        cout << "========================"<< endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "========================"<< endl;
        cout << "choice: ";
        int choice=0;
        cin >> choice;
        cin.get();//读取缓冲区残留的回车
        switch (choice) {
            case 1: {
                int id=0;
                char pwd[50]={0};
                cout << "Enter your userid : ";
                cin >> id;
                cin.get();
                cout << "Enter your password : ";
                cin.getline(pwd,50);
                json js;
                js["msgid"]=1;
                js["id"]=id;
                js["password"]=pwd;
                string request=js.dump().c_str();
                int len=send(clientFd,request.c_str(),strlen(request.c_str())+1,0);
                if (len<0) {
                    cerr << "Error sending login msg " << endl;
                }else {
                    char buffer[1024]={0};
                    len=recv(clientFd,buffer,1024,0);
                    if (len<0) {
                        cerr << "Error receiving login msg " << endl;
                    }else {
                        json response=json::parse(buffer);
                        if (response["errno"]!=0) {
                            cerr << response["errmsg"] << endl;
                        }else {

                            g_currentUser.setId(response["id"].get<int>());
                            g_currentUser.setName(response["name"].get<std::string>());
                            g_currentUser.setState("online");
                            //记录当前好友列表的信息
                            g_currentUserFriends.clear();
                            if (response.contains("friend")) {
                                vector<string> friends = response["friend"].get<vector<string>>();
                                for (string &str : friends) {
                                    json j=json::parse(str);
                                    User user;
                                    user.setId(j["id"].get<int>());
                                    user.setName(j["name"].get<std::string>());
                                    user.setState(j["state"].get<std::string>());
                                    g_currentUserFriends.push_back(user);
                                }
                            }
                            //记录当前群组列表的信息
                            g_currentUserGroups.clear();
                            if (response.contains("groups")) {
                                vector<string> groups = response["groups"].get<vector<string>>();
                                for (string &str : groups) {
                                    json j=json::parse(str);
                                    Group group;
                                    group.setId(j["id"].get<int>());
                                    group.setName(j["groupname"].get<std::string>());
                                    group.setDescription(j["groupdesc"].get<std::string>());
                                    vector<string> vec=j["users"].get<vector<string>>();
                                    for (auto &g : vec) {
                                        GroupUser groupUser;
                                        json j=json::parse(g);
                                        groupUser.setId(j["id"].get<int>());
                                        groupUser.setName(j["name"].get<std::string>());
                                        groupUser.setState(j["state"].get<std::string>());
                                        groupUser.setRole(j["role"].get<std::string>());
                                        group.getUsers().push_back(groupUser);
                                    }
                                    g_currentUserGroups.push_back(group);
                                }
                            }
                            //显示用户信息
                            showCurrentUserData();
                            if (response.contains("offlinemsg")) {
                                vector<string> vec=response["offlinemsg"].get<vector<string>>();
                                for (string &str : vec) {
                                    json j=json::parse(str);
                                    cout << j["time"] << "[" << j["from"] << "]" << j["name"]
                                    << " said : "<<j["msg"] << endl;
                                }
                            }
                            //登录成功，启动接收线程
                            static int threadnumber=0;
                            if (threadnumber==0) {
                                std::thread readTask(readHandlerTask,clientFd);
                                readTask.detach();
                                threadnumber++;
                            }
                            //
                            //进入主界面
                            ismainMenuing = true;
                            mainMenu(clientFd);
                        }
                    }
                }
            }
                break;
            case 2: {
                char name[50];
                char pwd[50];
                cout << "Enter your name: ";
                cin.getline(name,50);
                cout << "Enter your password: ";
                cin.getline(pwd,50);
                json js;
                js["msgid"]=3;
                js["name"]=name;
                js["password"]=pwd;
                string request=js.dump();
                int len=send(clientFd,request.c_str(),request.size()+1,0);
                if(len<0) {
                    cerr << "Error sending reg msg " <<request<< endl;
                }else {
                    char buffer[1024]={0};
                    len=recv(clientFd,buffer,1024,0);
                    if(len<0) {
                        cerr << "Error receiving reg msg" << endl;
                    }else {
                        json reponse=json::parse(buffer);
                        if(reponse["errno"].get<int>()==1) {
                            cerr<<name<<"is already exited , please input another!"<<endl;
                        }else {
                            cout<<name<<"register success , userid is "<<reponse["id"]<<", do not forget it!"<<endl;
                        }
                    }
                }
            }
                break;
            case 3:
                close(clientFd);
                exit(0);
            default:
                cerr << "invalid choice" << endl;
                break;
        }
    }
    return 0;
}
//展示当前用户的信息
void showCurrentUserData() {
    cout << "====================login user====================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() <<" name: "<< g_currentUser.getName() << endl;
    cout << "====================friend list====================" << endl;
    if (!g_currentUserFriends.empty()) {
        for (User &user : g_currentUserFriends ) {
            cout << user.getId() << " " << user.getName() << " " <<user.getState() <<endl;
        }
    }
    cout << "=====================group list====================" << endl;
    if (!g_currentUserGroups.empty()) {
        for (Group &group : g_currentUserGroups ) {
            cout<<group.getId()<<" "<<group.getName()<<" "<<group.getDescription()<<endl;
            for (GroupUser &user : group.getUsers() ) {
                cout << user.getId() << " " << user.getName() << " " <<user.getState() << " " <<user.getRole() <<endl;
            }
        }
    }
    cout << "====================================================" << endl;
}
//接收线程
void readHandlerTask(int clientFd) {
    for (;;) {
        char buffer[1024];
        int len=recv(clientFd,buffer,1024,0);
        if(len<=0) {
            close(clientFd);
            exit(-1);
        }else {
            json j=json::parse(buffer);
            int msgid=j["msgid"].get<int>();
            if (msgid==ONE_CHAT_MSG) {
                cout << j["time"] << "[" << j["from"] << "]" << j["name"]
                                    << " said : "<<j["msg"] << endl;
            }else if (msgid==GROUP_CHAT_MSG) {
                cout <<"群消息："<<"groupid : "<<"["<<j["groupid"]<<"]"<<endl;
                cout <<j["time"] << "[" << j["from"] << "]" << j["name"]
                << " said : "<<j["msg"] << endl;
            }
        }
        continue;
    }
}
//主界面页面
void mainMenu(int clientFd) {
    help();
    char buffer[1024]={0};
    while(ismainMenuing) {
        cin.getline(buffer,1024);
        string commandstr(buffer);
        string command;
        int idx=commandstr.find(":");
        if (idx==-1) {
            command=commandstr;
        }else {
            command=commandstr.substr(0,idx);
        }
        auto it=commandHandlerMap.find(command);
        if (it==commandHandlerMap.end()) {
            cout<<"Command not found"<<endl;
            continue;
        }else {
            it->second(clientFd,commandstr.substr(idx+1,commandstr.size()-idx));
        }
    }
}

void help(int clientFd,string command) {
    cout<<" list all command! "<<endl;
    auto it=commandMap.begin();
    for(it=commandMap.begin();it!=commandMap.end();it++) {
        cout<<it->first<<" : "<<it->second<<endl;
    }
}
void addfriend(int clientFd,string command) {
    int friendid=atoi(command.c_str());
    json js;
    js["msgid"]=ADD_FRIEND_MSG;
    js["userid"]=g_currentUser.getId();
    js["friendid"]=friendid;
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"addfriend send error ! "<<endl;
    }
}
void chat(int clientFd,string command) {
    int idx=command.find(":");
    if (idx==-1) {
        cerr<<"Command not found"<<endl;
        return;
    }
    int to=atoi(command.substr(0,idx).c_str());
    string msg=command.substr(idx+1,command.size()-idx);
    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["from"]=g_currentUser.getId();
    js["name"]=g_currentUser.getName();
    js["to"]=to;
    js["msg"]=msg;
    js["time"]=getCurrentTime();
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"chat send error ! "<<endl;
    }
}
void creategroup(int clientFd,string command) {
    int idx=command.find(":");
    if (idx==-1) {
        cerr<<"Create group error,command not found ! "<<endl;
    }
    string groupname=command.substr(0,idx);
    string groupdesc=command.substr(idx+1,command.size()-idx);
    json js;
    js["msgid"]=CHEATE_GROUP_MSG;
    js["id"]=g_currentUser.getId();
    js["groupname"]=groupname;
    js["groupdesc"]=groupdesc;
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"create group send error ! "<<endl;
    }
}
void addgroup(int clientFd,string command) {
    json js;
    js["msgid"]=ADD_GROUP_MSG;
    js["id"]=g_currentUser.getId();
    js["groupid"]=atoi(command.c_str());
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"addgroup send error ! "<<endl;
    }
}
void groupchat(int clientFd,string command) {
    int idx=command.find(":");
    if (idx==-1) {
        cerr<<"groupchat command not found"<<endl;
    }
    int groupid=atoi(command.substr(0,idx).c_str());
    string message=command.substr(idx+1,command.size()-idx);
    json js;
    js["msgid"]=GROUP_CHAT_MSG;
    js["id"]=g_currentUser.getId();
    js["groupid"]=groupid;
    js["from"]=g_currentUser.getId();
    js["name"]=g_currentUser.getName();
    js["msg"]=message;
    js["time"]=getCurrentTime();
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"groupchat send error ! "<<endl;
    }
}
void logout(int clientFd,string command) {
    json js;
    js["msgid"]=LOGOUT_MSG;
    js["id"]=g_currentUser.getId();
    string request=js.dump();
    int len=send(clientFd,request.c_str(),strlen(request.c_str()),0);
    if(len<0) {
        cerr<<"logout send error ! "<<endl;
    }else {
        ismainMenuing=false;
    }
}

string getCurrentTime() {
    time_t now = time(nullptr);
    tm *t = localtime(&now);

    char buf[64];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);
    return string(buf);
}
