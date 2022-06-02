//
// Created by yang2001 on 2022/4/30.
// shell类，定义了和用户交互的基本逻辑
#include "shell.h"
#include <vector>
#include <sstream>
#include "file_system.h"
#include "file_manager.h"
#include "user.h"
extern Directory present_directory;
extern FileManager my_file_manager;
extern User present_user;
using namespace std;


// 工具函数，用来分割输入的字符串
vector<string> stringSplit(string str)
{
    string buf;
    stringstream ss(str);
    vector<string> v;
    // 字符流ss
    while (ss >> buf) {
        //转小写
        transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
        v.push_back(buf);
    }
    return v;
}


bool shell::shellReact(string cmd) {
    std::vector<std::string> cmd_vector = stringSplit(cmd);
    if(!cmd.size()) return true;
    string cmd_head = cmd_vector[0];
    if(cmd_head=="fformat"){
        my_file_manager.formatSystem();
        cout<<"fformat react"<<endl;
    }
    else if(cmd_head=="ls"){
        if(cmd_vector.size()==1){
            my_file_manager.ls(false);
            return true;
        }
        string arg1 = cmd_vector[1];
        if(arg1 == "-l")
            my_file_manager.ls(true);
        else
            cout<<"错误的参数"<<endl;
    }
    else if(cmd_head =="pwd"){
        my_file_manager.pwd();
    }
    else if(cmd_head=="cd"){
        if(cmd_vector.size()==1){
            return true;
        }
        string path = cmd_vector[1];
        my_file_manager.openDirectory(path.c_str());
    }
    else if(cmd_head=="rm"){
        if(cmd_vector.size()==1){
            return true;
        }
        cout<<"rm react need to be completed!"<<endl;
    }
    else if(cmd_head=="mkdir"){
        cout<<cmd_vector.size()<<endl;
        if(cmd_vector.size()==1){
            return true;
        }
        string directory_name = cmd_vector[1];
        my_file_manager.createDirectory(directory_name.c_str());
    }
    else if(cmd_head=="fcreat"){
        if(cmd_vector.size()==1){
            return true;
        }
        string file_name = cmd_vector[1];
        my_file_manager.creatFile(file_name.c_str());
    }
    else if(cmd_head=="fopen"){
        if(cmd_vector.size()<=2){
            return true;
        }
        string mode_str = cmd_vector[2];
        if(mode_str!="-r" && mode_str!="-w" && mode_str!="-rw"){
            cout<<"fopen读写参数错误"<<endl;
            return true;
        }
        int mode;
        if(mode_str == "-r")
            mode = File::R_FLAG;
        else if(mode_str == "-w" )
            mode = File::W_FLAG;
        else
            mode = File::RW_FLAG;
        string file_name = cmd_vector[1];
        int fd = my_file_manager.openFile(file_name.c_str(),mode);
        if(fd!=-1) cout<<"文件打开成功，fd = "<<fd<<endl;
    }
    else if(cmd_head=="fclose"){
        if(cmd_vector.size()==1){
            return true;
        }
        int fd = stoi(cmd_vector[1]);
        my_file_manager.closeFile(fd);
        cout<<"fread react"<<endl;
    }
    else if(cmd_head=="fread"){
        if(cmd_vector.size()<=2)
            return true;
        int fd = stoi(cmd_vector[1]);
        string file_name = "";
        int length = stoi(cmd_vector[3]);

        int read_byte_num = my_file_manager.readFile(fd,file_name,length);
        if(read_byte_num == -1)
            return true;
        cout<<"成功读入了"<<read_byte_num<<"个字节"<<endl;
        cout<<"fread react"<<endl;
    }
    else if(cmd_head=="fwrite"){
        if(cmd_vector.size()<=2)
            return true;
        int fd = stoi(cmd_vector[1]);
        string file_name = cmd_vector[2];
        int length = stoi(cmd_vector[3]);
        int write_byte_num = my_file_manager.writeFile(fd,file_name,length);
        if(write_byte_num == -1)
            return true;
        cout<<"成功写入了"<<write_byte_num<<"个字节"<<endl;
        cout<<"fwrite react"<<endl;
    }
    else if(cmd_head=="fseek"){
        if(cmd_vector.size()<=2)
            return true;
        int fd = stoi(cmd_vector[1]);
        int offset = stoi(cmd_vector[2]);
        int ptrname = stoi(cmd_vector[3]);

        if(ptrname!=0 && ptrname!=1 && ptrname!=2){
            cout<<"fseek"<<"参数错误"<<endl;
            return true;
        }
        cout<<"fseek react"<<endl;
    }
    else if(cmd_head=="fdelete"){
        if(cmd_vector.size()==1){
            return true;
        }
        string file_name = cmd_vector[1];
        my_file_manager.deleteFile(file_name.c_str());
    }
    else if(cmd_head == "groupadd"){
        if(cmd_vector.size()==1){
            return true;
        }
        string group_name = cmd_vector[1];
        int group_id = my_file_manager.addGroup(group_name.c_str());
        if(group_id == -1)
            return true;
        cout<<"成功添加了用户组"<<group_name<<",其ID为:"<<group_id<<endl;
    }
    else if(cmd_head == "usermod"){
        if(cmd_vector.size()<=2){
            // 参数错误
            return true;
        }
        string user_name = cmd_vector[1];
        string group_name = cmd_vector[2];
        my_file_manager.changeFileGroup(user_name.c_str(),group_name.c_str());

    }
    else if(cmd_head == "chmod"){
        if(cmd_vector.size()<=4){
            return true;
        }
        string file_name = cmd_vector[1];
        string user_mode = cmd_vector[2];
        string group_mode = cmd_vector[3];
        string else_mode = cmd_vector[4];
    }
    else if(cmd_head == "userdel"){
        if(cmd_vector.size()<=1){
            return true;
        }
        string user_name = cmd_vector[1];
        if(my_file_manager.deleteUser(user_name.c_str())){
            cout<<"成功删除了名为"<<user_name<<"的用户"<<endl;
        }
    }
    else if(cmd_head == "chgrp"){
        if(cmd_vector.size()<=2){
            // 参数错误
            return true;
        }
        string file_name = cmd_vector[1];
        string group_name = cmd_vector[2];
        my_file_manager.changeFileGroup(file_name.c_str(),group_name.c_str());
    }
    else if(cmd_head == "useradd"){
        if(cmd_vector.size()<=2){
            // 参数错误
            return true;
        }
        string user_name = cmd_vector[1];
        string user_pwd = cmd_vector[2];
        int user_id = my_file_manager.addUser(user_name.c_str(),user_pwd.c_str());
        if(user_id == -1)
            return true;
        cout<<"成功添加了用户"<<user_name<<"其用户ID为"<<user_id<<endl;
    }
    else if(cmd_head == "su"){
        if(cmd_vector.size()<=2){
            return true;
        }
        string user_name = cmd_vector[1];
        string pwd = cmd_vector[2];
        my_file_manager.su(user_name.c_str(),pwd.c_str());

    }
    else if(cmd_head == "whoami"){
        my_file_manager.whoAmI();
    }
    else if(cmd_head=="test"){
        my_file_manager.createDirectory("cage");
        my_file_manager.openDirectory("cage");
        my_file_manager.creatFile("rage");
        int fd = my_file_manager.openFile("rage",0);
        int write_byte_num = my_file_manager.writeFile(fd,"../1.txt",1000000);
        cout<<"成功写入了"<<write_byte_num<<"个字节"<<endl;
        my_file_manager.seekFile(fd,100,0);
        int read_byte_num = my_file_manager.readFile(fd,"1",1000000);
        cout<<"成功读入了"<<read_byte_num<<"个字节"<<endl;
    }
    else if(cmd_head == "exit"){
        return false;
    }
    else{
        cout<<"command "<<cmd_head<<" dont exist"<<endl;
    }
    return true;
}

void shell::loop() {
    while (1) {
        string cmd, single_cmd;
        cout <<present_directory.directory_name << " " << present_user.u_name << "$ ";
        getline(cin, cmd);
        if (!shellReact(cmd))
            break;
    }
}