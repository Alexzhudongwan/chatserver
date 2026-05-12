//
// Created by 31132 on 2026/5/6.
//
#include "groupmodel.h"

#include <db.h>

#include "group.h"
bool GroupModel::createGroup(Group &group) {
    char sql[1024]={0};
    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDescription().c_str());
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getconnection()));
            return true;
        }
    }
    return false;
}
void GroupModel::addGroup(int userid,int groupid,string role) {
    char sql[1024]={0};
    sprintf(sql,"insert into groupuser values('%d','%d','%s')",groupid,userid,role.c_str());
    Mysql mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {

        }
    }
}
vector<Group> GroupModel::queryGroups(int userid) {
    char sql[1024]={0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from allgroup a inner join \
    groupuser b on a.id = b.groupid where b.userid=%d",userid);
    vector<Group> groups;
    Mysql mysql;
    if (mysql.connect()) {
        MYSQL_RES *result = mysql.query(sql);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDescription(row[2]);
                groups.push_back(group);
            }
            mysql_free_result(result);
        }
    }
    char sql2[1024]={0};
    for (Group &group : groups) {
        sprintf(sql2,"select a.id,a.name,a.state,b.grouprole from user a \
    inner join groupuser b on b.userid = a.id where b.groupid=%d",group.getId());
        MYSQL_RES *result=mysql.query(sql2);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(result);
        }
    }
    return groups;
}
vector<int> GroupModel:: queryGroupUsers(int groupid,int userid) {
    char sql[1024]={0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid != %d",groupid,userid);
    vector<int> idvec;
    Mysql mysql;
    if (mysql.connect()) {
        MYSQL_RES *result = mysql.query(sql);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                idvec.push_back(atoi(row[0]));
            }
            mysql_free_result(result);
        }
    }
    return idvec;
}