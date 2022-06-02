//
// Created by yang2001 on 2022/6/1.
//
#include "user.h"

UserInfo::UserInfo() {
    for(int i=0;i<USER_NUM;i++){
        this->u_g_id[i] = -1;
        this->u_name[i][0]='\0';
        this->u_pwd[i][0]='\0';
    }
}

// 添加一个用户，若添加失败return -1
int UserInfo::addUser(const char *user_name, const char *pwd,int group_id) {
    for(int i=0;i<USER_NUM;i++){
        if(strlen(this->u_name[i])==0){
            strcpy(this->u_name[i],user_name);
            strcpy(this->u_pwd[i],pwd);
            this->u_g_id[i] = group_id;
            return i;
        }
    }
    return -1;
}

// 查看系统中是否存在这个用户，存在返回这个用户的ID,不存在返回-1
int UserInfo::hasUser(const char *user_name) {
    for(int i=0;i<USER_NUM;i++){
        if(strcmp(this->u_name[i],user_name)==0)
            return i;
    }
    return -1;
}

// 删除系统中名为user_name的用户，删除失败返回-1，否则返回删除的这个用户的ID
int UserInfo::delUser(const char * user_name){
    int pos = this->hasUser(user_name);
    if(pos == -1)
        return -1;
    else{
        strcpy(this->u_name[pos],"");
        this->u_name[pos][0]='\0';
        this->u_g_id[pos]=-1;

        strcpy(this->u_pwd[pos],"");
        this->u_pwd[pos][0]='\0';
        return pos;
    }
}

// 登录，匹配用户的密码和账号，存在返回ID，不存在返回-1
int UserInfo::login(const char *user_name, const char *pwd) {
    for(int i=0;i<USER_NUM;i++){
        if(strcmp(this->u_name[i],user_name)==0 && strcmp(this->u_pwd[i],pwd)==0){
            return i;
        }
    }
    return -1;
}



GroupInfo::GroupInfo() {
    for(int i=0;i<GROUP_NUM;i++){
        this->g_name[i][0] = '\0';
    }
}




int GroupInfo::addGroup(const char *group_name) {
    for(int i=0;i<GROUP_NUM;i++){
        if(strlen(this->g_name[i])==0){
            strcpy(this->g_name[i],group_name);
            return i;
        }
    }
    return -1;
}


const char* GroupInfo::getGroupName(int group_id){
    if(group_id<0 || group_id>= GROUP_NUM)
        return NULL;
    else if(strlen(this->g_name[group_id]) == 0)
        return NULL;
    else
        return this->g_name[group_id];
}

int GroupInfo::getGroupId(const char* group_name){
    for(int i=0;i<GROUP_NUM;i++){
        if(strcpy(this->g_name[i],group_name)==0)
            return i;
    }
    return -1;
}

int GroupInfo::hasGroup(const char *group_name) {
    for(int i=0;i<GROUP_NUM;i++){
        if(strcmp(this->g_name[i],group_name)==0)
            return i;
    }
    return -1;
}
