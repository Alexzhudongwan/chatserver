//
// Created by 31132 on 2026/5/6.
//

#ifndef CHAT_GROUP_H
#define CHAT_GROUP_H
#include <string>
#include <vector>
#include "groupuser.h"
using namespace std;
class Group {
    public:
    Group(int id=-1,string name="",string description=""):id_(id),name_(name),description_(description) { }
    int getId() const {return id_;}
    string getName() const {return name_;}
    string getDescription() const {return description_;}
    vector<GroupUser> &getUsers()  {return users_;}
    void setId(int id) {id_ = id;}
    void setName(string name) {name_ = name;}
    void setDescription(string description) {description_ = description;}
private:
    int id_;
    string name_;
    string description_;
    vector<GroupUser> users_;
};
#endif //CHAT_GROUP_H
