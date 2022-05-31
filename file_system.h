//
// Created by yang2001 on 2022/5/18.
//

#ifndef MYFILESYSTEM_FILE_SYSTEM_H
#define MYFILESYSTEM_FILE_SYSTEM_H

# include "file_system.h"
# include "io_manager.h"


/*
 * INODE号为 0 - INODE_NUM-1 // INODE块单独编号
 * BLOCK号为1-BLOCK_NUM // 数据块单独编号
 */
class FileSystem{
public:
    void FormatDisk();//格式化整个磁盘

    int AllocInode(Inode &inode);//申请一个INODE

    void FreeInode(int inode_num);//释放一个INODE

    int AllocBlock();//申请一个数据块,返回BLOCK号

    void FreeBlock(int blk_num);//释放一个数据块

    SuperBlock loadSuperBlock();//获取当前的SuperBlock

    void updateSuperBlock(SuperBlock& superBlock);//更新SuperBlock

    void writeInode(Inode inode);

    void readInode(Inode &inode,int Inode_num);

    void writeBlock(char* content,int blk_num);

    void readBlock(char* content,int blk_num);

    void updatePresentDirectory();

    const char* readFile(string file_name,int length);


};
#endif //MYFILESYSTEM_FILE_SYSTEM_H
