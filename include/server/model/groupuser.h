//
// Created by 31132 on 2026/5/6.
//

#ifndef CHAT_GROUPUSER_H
#define CHAT_GROUPUSER_H
#include <user.h>
class GroupUser:public User {
    public:
    string getRole() { return this->role_; }
    void setRole(string role) {this->role_ = role;}
private:
    string role_;
};
#endif //CHAT_GROUPUSER_H
