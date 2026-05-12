//
// Created by 31132 on 2026/4/29.
//

#ifndef CHAT_OFFLINEMESSAGEMODEL_H
#define CHAT_OFFLINEMESSAGEMODEL_H
#include <string>
#include <iostream>
#include <vector>
using namespace std;
class OfflineMsgModel {
    public:
    bool insert(int id,string offlinemsg);
    bool remove(int id);
    vector<string> query(int id);
};
#endif //CHAT_OFFLINEMESSAGEMODEL_H
