//
// Created by 31132 on 2026/4/26.
//
#include "user.h"
#include "usermodel.h"
#include "db/db.h"
using namespace std;
bool UserModel::insertUser(User &user) {
    // 定义安全的 SQL 缓冲区
    char sql[1024] = {0};

    // 安全拼接
    snprintf(sql, sizeof(sql),
             "insert into user(name,password,state) values('%s','%s','%s')",
             user.getName().c_str(),
             user.getPassword().c_str(),
             user.getState().c_str());
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getconnection()));
            return true;
        }
    }
    return false;
}
User UserModel::query(int id) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),"select * from user where id = %d",id);
    Mysql mysql;
    if (mysql.connect()) {
        MYSQL_RES *result = mysql.query(sql);
        if (result) {
            MYSQL_ROW row=mysql_fetch_row(result);
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPassword(row[2]);
            user.setState(row[3]);
            mysql_free_result(result);
            return user;
        }
    }
    return User();
}
bool UserModel::updateState(User& user) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),"update user set state = '%s' where id = %d",user.getState().c_str(),user.getId());
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}
void UserModel::resetState() {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),"update user set state = 'offline' where state = 'online'");
    Mysql mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
    return ;
}