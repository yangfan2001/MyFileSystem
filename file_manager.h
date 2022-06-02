//
// Created by yang2001 on 2022/5/21.
//

#ifndef MYFILESYSTEM_FILE_MANAGER_H
#define MYFILESYSTEM_FILE_MANAGER_H

/*
 * 文件管理类
 * 调用FileSystem类的方法
 * */
class FileManager{
public:

public:
    /**/
    void creatFile(const char* file_name);
    void deleteFile(const char* file_name);
    int openFile(const char* file_name,int mode);
    void closeFile(int fd);
    int readFile(int fd,string out_file_name,int length);
    int writeFile(int fd,string in_file_name,int length);
    void seekFile(int fd,int offset,int ptrname);

    void createDirectory(const char* directory_name);
    void deleteDirectory(const char* directory_name);
    void openDirectory(const char* directory_name);
    void getCurrentDirectory();
    void formatSystem();
    void showOpenFileList();
    void pwd();
    void ls(bool detail);
    void rm(const char* name);

    /*user type*/
    int addUser(const char* user_name,const char* pwd);
    void modifyUserGroup(const char* user_name,const char* group_name);
    int addGroup(const char* group_name);
    void changeFileMode(const char* file_name,int user_mode,int group_mode,int else_mode);
    bool deleteUser(const char* user_name);
    void changeFileGroup(const char* file_name,const char* group_name);
    void su(const char* user_name,const char* pwd);
    void whoAmI();

    // 文件系统启动时的引导程序
    void boost();


};

#endif //MYFILESYSTEM_FILE_MANAGER_H
