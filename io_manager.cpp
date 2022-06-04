//
// Created by yang2001 on 2022/5/14.
//

# include "io_manager.h"

IOManager my_io_manager;//全局变量 用于进行文件的输入和读写

IOManager::IOManager() {

}
IOManager::~IOManager() {

}
void IOManager::openFile() {
    // 打开文件，对异常进行了错误处理
    fd.open(DISK_NAME,ios::in|ios::out|ios::binary);// 打开文件
    if(!fd.is_open())
    {
        cout<<"无法打开磁盘"<<DISK_NAME<<endl;
    }
}
void IOManager::readSuperBlock(SuperBlock &superBlock) {
    openFile();
    fd.seekg(SUPER_BLOCK_POS,ios::cur); // 找到写入指针的正确位置
    fd.read((char*)&(superBlock),sizeof(superBlock)); // 读出SuperBlock
    fd.close();// 关闭文件
}

void IOManager::writeSuperBlock(SuperBlock &superBlock) {
    openFile();
    fd.seekg(SUPER_BLOCK_POS,ios::cur); // 找到写入指针的正确位置
    fd.write((char*)&(superBlock),sizeof(superBlock)); // 写入SuperBlock
    fd.close();// 关闭文件
}

void IOManager::readInode(DiskInode &diskInode, int num) {
    if(num>=INODE_NUM)
        return;
    openFile();// open files
    fd.seekg(INODE_POS*BLOCK_SIZE+num*sizeof(diskInode),ios::cur); // 找到写入指针的正确位置
    fd.read((char*)&(diskInode),sizeof(diskInode)); // 读出Inode
    fd.close();// 关闭文件
}

void IOManager::writeInode(DiskInode &diskInode, int num) {
    if(num>=INODE_NUM)
        return;
    openFile();// open files
    fd.seekg(INODE_POS*BLOCK_SIZE+num*sizeof(diskInode),ios::cur); // 找到写入指针的正确位置
    fd.write((char*)&(diskInode),sizeof(diskInode)); // 写入SuperBlock
    fd.close();// 关闭文件
}
// 写入一个Block，一次写入512 byte
void IOManager::writeBlock(char *content, int blknum) {
    if(blknum>=BLOCK_NUM-1)
        return;
    openFile();// open files
    fd.seekg(BLOCK_POS*BLOCK_SIZE+(blknum-1)*BLOCK_SIZE,ios::cur); // 找到写入指针的正确位置
    fd.write(content,BLOCK_SIZE); // 写入SuperBlock
    fd.close();// 关闭文件
}

// 读一个Block，一次读512 byte
void IOManager::readBlock(char *content, int blknum) {
    if(blknum>=BLOCK_NUM-1)
        return;
    openFile();// open files
    fd.seekg(BLOCK_POS*BLOCK_SIZE+(blknum-1)*BLOCK_SIZE,ios::cur); // 找到写入指针的正确位置
    fd.read(content,BLOCK_SIZE); // 写入SuperBlock
    fd.close();// 关闭文件
}

void IOManager::printMyDisk(){

}
bool IOManager::updateFreeInode() {
    DiskInode tmpNode;
    int cnt=0;
    for(int i=0;i<INODE_NUM;i++){
        readInode(tmpNode,i);//read an Inode
        if(tmpNode.d_nlink==0){
            cnt++;
        }
        // allocate enough Inode
        if(cnt==MAX_NINODE){
            break;
        }
    }
    if(cnt!=0){
        // update the SuperBlock
        return true;
    }
    else  // if there is no Inode available return
        return false;
}

bool IOManager::updateFreeBlock() {
    return true;
}

const char* IOManager::readFile(string file_name,int length){
    std::ifstream file(file_name);                            // 打开文件
    if(!file.is_open()){
        cout<<"打开文件"<<file_name<<"失败"<<endl;
        return NULL;
    }
    char* content;
    content = (char*)malloc(sizeof(char) * 409600);// 假设文件最大为400K，可自定义
    int count = 0;
    if (content)
    {
        char buff[1024];                                     // 1Kb的缓冲区
        int pt = 0;
        while (file.getline(buff, 1024))                     // 按行读取文件到缓冲区
        {
            if(count==length) break;
            for (int i = 0; i < 1024; i++) {
                if(count==length) break;
                count++;
                char tmp = buff[i];
                if (tmp == '\0') {                           // 遇到\0时换行，继续读取下一行
                    content[pt] = '\n';
                    pt++;
                    break;
                }
                content[pt] = tmp;
                pt++;
            }
        }
        content[pt] = '\0';                                  // 读取结束，构建字符串尾
        char* result = (char*)malloc(sizeof(char) * (++pt)); // 申请pt+1个字节的内存空间
        if (!result)
            return NULL;
        for (int i = 0; i < pt; i++) {
            result[i] = content[i];                          // 字符串复制
        }
        return result;
    }
    return NULL;
};
void IOManager::writeFile(string file_name,const char* buffer){
    ofstream file(file_name,ios::out|ios::binary);
    file<<buffer;
    file.close();
};

