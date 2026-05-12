//
// Created by 31132 on 2026/4/29.
//

#ifndef CHAT_FRIENDMODEL_H
#define CHAT_FRIENDMODEL_H
#include "user.h"
#include <vector>
using namespace std;
class FriendModel {
    public:
    void insertfriend(int userid,int friendid) ;
    vector<User> queryfriend(int userid);
};

#endif //CHAT_FRIENDMODEL_H
