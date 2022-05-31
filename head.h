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
static const string DISK_NAME = "../temp.img";
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

#endif //MYFILESYSTEM_HEAD_H
