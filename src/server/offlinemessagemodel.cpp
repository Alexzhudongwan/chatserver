//
// Created by 31132 on 2026/4/29.
//
#include "db/db.h"
#include "offlinemessagemodel.h"
bool OfflineMsgModel::insert(int id,string offlinemsg) {
    // 定义安全的 SQL 缓冲区
    char sql[1024] = {0};

    // 安全拼接
    snprintf(sql, sizeof(sql),
             "insert into offlinemessage values('%d','%s')",id,offlinemsg.c_str());
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool OfflineMsgModel::remove(int id) {
    char sql[1024] = {0};

    // 安全拼接
    snprintf(sql, sizeof(sql),
             "delete from offlinemessage where userid = %d",id);
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}
vector<string> OfflineMsgModel::query(int id) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),"select message from offlinemessage where userid = %d",id);
    Mysql mysql;
    vector<string> msg;
    if (mysql.connect()) {
        MYSQL_RES *result = mysql.query(sql);
        if (result) {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(result)) {
                msg.push_back(row[0]);
            }
        }
        mysql_free_result(result);
    }
    return msg;
}