//
// Created by yang2001 on 2022/4/30.
//

#ifndef MYFILESYSTEM_HEAD_H
#define MYFILESYSTEM_HEAD_H
#include <iostream>
# include <fstream>
# include <iomanip>

using namespace std;
// define variable
static const string DISK_NAME = "tmp.img";
// define the max of each value
#define BLOCK_SIZE 512 // each block is 512 byte
#define BLOCK_MAX_NUM 10000 // The MAX OF BLOCK NUM
#define MAX_NFREE 100
#define MAX_NINODE 100
#define INODE_NUM 96 // the num of Inode
#define INODE_PER_BLOCK 8 // INODE is 64 byte on disk,so a block contains 8 diskInode
#define FREE_BLOCK_NUM 100
#define DIRECTORY_SIZE 14 // the size of directory
#define FILE_NAME_SIZE 28
#define INODE_TABLE_SIZE 100 //the size of Inode table in disk memory
#define GROUP_NUM 16
#define GROUP_NAME_SIZE 32
#define USER_NUM 8
#define USER_NAME_SIZE 32
#define USER_PWD_SIZE 28
#define ROOT_BLK_NUM 100 // the block num of the root directory
#define ROOT_INODE_NUM 95 // the inode num of the root directory

#define USER_INFO_BLK_NUM 99// the block num of user info
#define GROUP_INFO_BLK_NUM 98 // the block num of group info

#define ROOT_USER_ID 0 // root is the no 0 user of the system
#define ROOT_GROUP_ID 0 // the root group id
#define DEFAULT_USER_GROUP_ID 1 // the id of the default user group

#endif //MYFILESYSTEM_HEAD_H
