//
// Created by 31132 on 2026/4/29.
//
#include "friendmodel.h"
#include "db/db.h"
void FriendModel::insertfriend(int userid,int friendid) {
    // 定义安全的 SQL 缓冲区
    char sql[1024] = {0};

    // 安全拼接
    snprintf(sql, sizeof(sql),
             "insert into friend values('%d','%d')",userid,friendid);
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return;
        }
    }
    return;
}
vector<User> FriendModel::queryfriend(int userid) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),"select a.id,a.name,a.state from user a inner join friend b on a.id=b.friendid where b.userid=%d",userid);
    Mysql mysql;
    vector<User> msg;
    if (mysql.connect()) {
        MYSQL_RES *result = mysql.query(sql);
        if (result) {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(result)) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                msg.push_back(user);
            }
        }
        mysql_free_result(result);
    }
    return msg;
}