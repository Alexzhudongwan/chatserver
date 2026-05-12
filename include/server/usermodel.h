//
// Created by 31132 on 2026/4/26.
//

#ifndef CHAT_USERMODEL_H
#define CHAT_USERMODEL_H
#include <iostream>
#include <string>
#include "user.h"
using namespace std;
class UserModel {
    public:
    bool insertUser(User &user) ;
    User query(int id);
    bool updateState(User &user);
    void resetState();
private:
};
#endif //CHAT_USERMODEL_H
