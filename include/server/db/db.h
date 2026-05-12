//
// Created by 31132 on 2026/4/26.
//

#ifndef CHAT_DB_H
#define CHAT_DB_H
#include <mysql/mysql.h>
#include<string>


class Mysql {
    public:
    Mysql() ;
    ~Mysql() ;
    bool connect() ;
    bool update(std::string sql) ;
    MYSQL_RES* query(std::string sql) ;
    MYSQL* getconnection();
private:
    MYSQL* _conn;
};
#endif //CHAT_DB_H
