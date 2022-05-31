//
// Created by yang2001 on 2022/5/14.
//

#ifndef MYFILESYSTEM_IO_MANAGER_H
#define MYFILESYSTEM_IO_MANAGER_H

# include "head.h"
# include "Inode.h"

class IOManager{
private:
    fstream fd;
public:
    IOManager();
    ~IOManager();
    void openFile();
    void writeSuperBlock(SuperBlock &superBlock);//write a SuperBlock
    void readSuperBlock(SuperBlock &superBlock); // read SuperBlock
    void writeInode(DiskInode &diskInode,int num); // write the num th inode
    void readInode(DiskInode &diskInode,int num); // read the num th inode
    void writeBlock(char* content,int blknum); // write a blk
    void readBlock(char* content,int blknum); // read a blk
    void printMyDisk(); // print a disk
    bool updateFreeInode(); // update the free Inode array of superBlock
    bool updateFreeBlock(); // update the free BLock array of superBLock

    const char* readFile(string file_name,int length);
    void writeFile(string file_name,const char* buffer);
    void writeFileByBlock(string file_name,const char* buffer,int offset);
};
#endif //MYFILESYSTEM_IO_MANAGER_H
