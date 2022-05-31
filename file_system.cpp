# include "file_system.h"
//
// Created by yang2001 on 2022/5/18.
//
extern IOManager my_io_manager;//全局变量 用于进行文件的输入和读写;
extern Directory present_directory;
FileSystem my_file_system; // 全局变量 用于进行数据块的申请、释放等操作

// 申请一块空闲块
int FileSystem::AllocBlock() {

    int blk_num;
    // 读出superBlock
    SuperBlock superBlock;
    my_io_manager.readSuperBlock(superBlock);

    superBlock.s_nfree--;
    blk_num = superBlock.s_free[superBlock.s_nfree];

    if(superBlock.s_nfree<=0){//SuperBlock没有多余的空闲区空间了
        blk_num = superBlock.s_free[0];//分配
        int empty_manage_block [BLOCK_SIZE/sizeof(int)];//申请512字节大小的空闲管理块
        my_io_manager.readBlock((char*)&empty_manage_block,superBlock.s_free[0]);
        superBlock.s_nfree = empty_manage_block[0]; // 空闲管理块的内容写入到superBlock上面
        cout<<"new_s_nfree"<<superBlock.s_nfree<<" "<<blk_num<<endl;
        if(superBlock.s_nfree==0){
            cout<<"没空间了😭"<<endl;
            return -1;
        }
        memcpy(superBlock.s_free,&empty_manage_block[1],sizeof(superBlock.s_free));
    }

    //写回SuperBlock
    my_io_manager.writeSuperBlock(superBlock);
    cout<<"第"<<blk_num<<"号数据块被分配！！！"<<endl;
    return blk_num;
}

void FileSystem::FreeBlock(int blk_num) {
    // 读出superBlock
    SuperBlock superBlock;
    my_io_manager.readSuperBlock(superBlock);
    if(superBlock.s_nfree == FREE_BLOCK_NUM){
        //superBlock的s_nfree已满 需要让其指向一个空块
        int empty_manage_block [BLOCK_SIZE/sizeof(int)];//申请512字节大小的空闲管理块
        empty_manage_block[0] = superBlock.s_nfree;
        memcpy(&empty_manage_block[1],superBlock.s_free,sizeof(superBlock.s_free));
        superBlock.s_nfree = 1;
        superBlock.s_free[0] = blk_num;
        // 把superBlock的s_free的内容写到刚释放的空闲块上
        my_io_manager.writeBlock((char*)&empty_manage_block,blk_num);
    }
    else{
        superBlock.s_free[superBlock.s_nfree] = blk_num;
        superBlock.s_nfree++;
    }
    //写回SuperBlock
    my_io_manager.writeSuperBlock(superBlock);
    cout<<"第"<<blk_num<<"号数据块被释放！！！"<<endl;

}

// 申请一块Inode 函数参数为Inode的引用，传入一个空的Inode进去，返回分配好的Inode
int FileSystem::AllocInode(Inode &inode){
    // 读出superBlock
    SuperBlock superBlock;
    int Inode_num;
    my_io_manager.readSuperBlock(superBlock);
    if(superBlock.s_ninode>0){//正常的分配
        superBlock.s_ninode--;
        Inode_num = superBlock.s_inode[superBlock.s_ninode];
        cout<<Inode_num<<"号INODE被分配"<<endl;
    } //ninode为0

    if(superBlock.s_ninode==0){
        //扫描Inode区，搜索100个空闲的DiskInode，添加到superBlock上面
        //...
        cout<<"没INODE了，请联系作者修复一下这个bug";
        return false;
    }
    my_io_manager.writeSuperBlock(superBlock);

    DiskInode diskInode; //新的diskInode
   // my_io_manager.readInode(diskInode,Inode_num);

    // 拷贝所有信息
    inode.copyDiskInode(diskInode);
    inode.i_number = Inode_num;//Inode号对应上
    inode.i_time = time(NULL);

    return true;
}

// 释放编号为inode_num的外存Inode，用于删除文件
void FileSystem::FreeInode(int inode_num) {
    // 读出superBlock
    SuperBlock superBlock;
    my_io_manager.readSuperBlock(superBlock);
    //在磁盘上写入一个空的DISKINODE
    DiskInode freeNode;
    my_io_manager.writeInode(freeNode,inode_num);
    if(superBlock.s_ninode>=MAX_NINODE) {
        return;
    }
    else{ // release!!!
        superBlock.s_inode[superBlock.s_ninode] = inode_num;
        superBlock.s_ninode++;
    }
    //写回SuperBlock
    my_io_manager.writeSuperBlock(superBlock);
    cout<<"释放了"<<inode_num<<"号磁盘Inode"<<endl;
}

// format fileSystem and write it into disk
void FileSystem::FormatDisk() {
    SuperBlock superBlock;
    // 初始化空的superBlock 并写入磁盘
    my_io_manager.writeSuperBlock(superBlock);
    // 分配空闲的inode
    DiskInode tmpNode;
    for(int i=0;i<INODE_NUM;i++){
        if(superBlock.s_ninode<100){//直接管理前100个
            superBlock.s_inode[superBlock.s_ninode++] = i;//初始化直接管理的空闲外存Inode索引表
        }
        my_io_manager.writeInode(tmpNode,i);//写入Inode
    }
    //创建Root根目录


    // 空的数据块，并置零
    char empty_block [BLOCK_SIZE];
    memset(empty_block,0,BLOCK_SIZE);
    // 空闲区管理块,SUPER BLOCK先管理 1-100 块BLOCK
    for(int j=0;j<100;j++) superBlock.s_free[j] = j+1;
    superBlock.s_nfree = FREE_BLOCK_NUM;


    // 空闲区管理块
    int empty_manage_block [BLOCK_SIZE/sizeof(int)];//申请512字节大小的空闲管理块
    memset(empty_manage_block,0,FREE_BLOCK_NUM);

    // 写入空白BLOCK 空闲区BLOCK 块数一共-> BLOCK_NUM
    for(int i=1;i<=BLOCK_NUM;i++){
        //一共写入BLOCK_NUM个BLOCK
        if((i-1)%FREE_BLOCK_NUM==0){//为空闲管理块，进行成组链接
            if(i-1+2*FREE_BLOCK_NUM<BLOCK_NUM){
                empty_manage_block[0] = FREE_BLOCK_NUM;//都为100个空闲块  i[0] -> i+0+100
                for(int j=0;j<100;j++) empty_manage_block[j+1] = i+100+j;
                // 序号为i的空闲管理块 管理 i+100 - i+199的空闲块
                my_io_manager.writeBlock((char*)&empty_manage_block,i);
            }
            else{//如果为最后一块（管理不超过100个） 或者最后一组（这一组后面不再有空闲区，因此不再起到管理作用）
                memset(empty_manage_block,0,FREE_BLOCK_NUM);//初始化
                int nfree = BLOCK_NUM-i-FREE_BLOCK_NUM>0?BLOCK_NUM-i-FREE_BLOCK_NUM:0;
                empty_manage_block[0] = nfree;//都为100个空闲块
                for(int j=0;j<nfree;j++) empty_manage_block[j+1] = i+100+j;
                if(nfree==0) empty_manage_block[1] = 0;//作为标志位，说明后续不再有空闲块
                my_io_manager.writeBlock((char*)&empty_manage_block,i);
            }
            cout<<i<<"号块被作为了空闲管理块"<<endl;
        }
        else{//为空闲块，那么写入一个空块
            my_io_manager.writeBlock(empty_block,i);
        }
    }
    // 最终磁盘大小应该为 BLOCK_SIZE * BLOCK_MAX_NUM
    // ...
    // 最后写入修改后的SuperBlock
    my_io_manager.writeSuperBlock(superBlock);
}

SuperBlock FileSystem::loadSuperBlock() {
    //从磁盘中读取SuperBlock
    SuperBlock superBlock;
    my_io_manager.readSuperBlock(superBlock);
    return superBlock;
}

void FileSystem::updateSuperBlock(SuperBlock& superBlock) {
    // 更新SuperBlock到磁盘中
    my_io_manager.writeSuperBlock(superBlock);
}
// 更新Inode到磁盘中
void FileSystem::writeInode(Inode inode){

    DiskInode diskInode;
    diskInode.copyInode(inode);
    my_io_manager.writeInode(diskInode,inode.i_number);
};
// 从磁盘读一个Inode
void FileSystem::readInode(Inode &inode,int Inode_num){
    DiskInode diskInode;
    my_io_manager.readInode(diskInode,Inode_num);
    inode.copyDiskInode(diskInode);
    inode.i_number = Inode_num;
};
// 写一个Block到磁盘中
void FileSystem::writeBlock(char* content,int blk_num) {
    my_io_manager.writeBlock(content,blk_num);
}
// 从磁盘中读一个Block
void FileSystem::readBlock(char* content,int blk_num) {
    my_io_manager.readBlock(content,blk_num);
}

// 将全局变量当前目录写入磁盘 实现更新
void FileSystem::updatePresentDirectory() {
    Inode present_directory_inode;
    readInode(present_directory_inode,present_directory.inode_num);
    int blk_num = present_directory_inode.i_addr[0];
    writeBlock((char*)&present_directory,blk_num);
}


const char * FileSystem::readFile(string file_name, int length) {
    return my_io_manager.readFile(file_name,length);
}