//
// Created by 31132 on 2026/5/6.
//

#ifndef CHAT_GROUPMODEL_H
#define CHAT_GROUPMODEL_H
#include <string>
#include <vector>
#include "group.h"
using namespace std;
class GroupModel {
    public:
    bool createGroup(Group &group);
    void addGroup(int userid,int groupid,string role);
    //获取这个群组中除了自己以外的其他人的信息
    vector<int> queryGroupUsers(int groupid,int userid);
    //查询用户所在群组的信息
    vector<Group> queryGroups(int userid);
};
#endif //CHAT_GROUPMODEL_H
