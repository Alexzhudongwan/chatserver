//
// Created by 31132 on 2026/4/26.
#include "server/db/db.h"
#include <string>
#include "muduo/base/Logging.h"
using namespace std;
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string database = "chat";

    Mysql::Mysql() {
        //初始化连接句柄
        _conn=mysql_init(NULL);
    }
    Mysql::~Mysql() {
        //释放资源
        if (_conn) mysql_close(_conn);
    }
    bool Mysql::connect() {
        //真正连接
        MYSQL* sql=mysql_real_connect(_conn,server.c_str(),user.c_str(),password.c_str(),database.c_str(),3306,nullptr,0);
        if (sql==nullptr)
            return false;
        //防止中文乱码
        mysql_query(_conn,"set names gbk");
        return sql;
    }
    bool Mysql::update(string sql) {
        if (mysql_query(_conn,sql.c_str())) {
            LOG_INFO<<__FILE__<<"l:"<<__LINE__<<":"<<sql<<"更新失败";
            return false;
        }
        return true;
    }
    MYSQL_RES* Mysql::query(string sql) {
        if (mysql_query(_conn,sql.c_str())) {
            LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败";
            return nullptr;
        }
        //返回查询到的结果集
        return mysql_use_result(_conn);
    }
MYSQL* Mysql::getconnection(){
        return _conn;
}
