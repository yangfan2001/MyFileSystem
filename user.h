//
// Created by yang2001 on 2022/5/31.
//

#ifndef MYFILESYSTEM_USER_H
# include "head.h"

// 内存中的用户信息
class User{
public:
    int u_id;
    char u_name[USER_NAME_SIZE];
    int g_id;
    char g_name[USER_NAME_SIZE];
};

// 在磁盘上的用户相关信息

class UserInfo{
public:
    int u_g_id[USER_NUM];
    char u_pwd[USER_NUM][USER_PWD_SIZE];
    char u_name[USER_NUM][USER_NAME_SIZE];
    UserInfo();
    int addUser(const char* user_name,const char* pwd,int group_id);
    int delUser(const char* user_name);
    int hasUser(const char* user_name);
    int login(const char* user_name,const char* pwd);
};

class GroupInfo{
public:
    char g_name[GROUP_NUM][GROUP_NAME_SIZE];
public:
    GroupInfo();
    int addGroup(const char* group_name);
    const char* getGroupName(int group_id);
    int getGroupId(const char* group_name);
    int hasGroup(const char* group_name);
};
//
#define MYFILESYSTEM_USER_H

#endif //MYFILESYSTEM_USER_H
