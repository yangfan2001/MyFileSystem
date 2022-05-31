//
// Created by yang2001 on 2022/4/30.
//
# include "file_system.h"
# include "time.h"
extern IOManager my_io_manager;//全局变量 用于进行文件的输入和读写;
extern FileSystem my_file_system;//全局变量 用于数据的读写
Directory present_directory; // 全局变量 用来表示当前的目录
//Inode inode_table[INODE_TABLE_SIZE];//全局变量 用来表示当前的内存Inode表
OpenFileTable my_open_file_table;
// 全局变量，用来表示当前的文件打开表

// 这里参考了univ6++的实现方式
DiskInode::DiskInode() {
    this->d_mode = 0;
    this->d_permission = 0;
    this->d_nlink = 0;
    this->d_uid = -1;
    this->d_gid = -1;
    this->d_size = 0;
    memset(d_addr, 0, sizeof(d_addr));
    this->d_time = time(NULL);
}

DiskInode::~DiskInode() {
}

void DiskInode::copyInode(Inode inode) {
    this->d_size = inode.i_size;
    this->d_gid = inode.i_gid;
    this->d_uid = inode.i_uid;
    this->d_permission = inode.i_permission;
    this->d_mode = inode.i_mode;
    this->d_nlink = inode.i_nlink;
    memcpy(this->d_addr,inode.i_addr,sizeof(this->d_addr));
}

/*declaration of the methods of Inode Class*/

Inode::Inode() {
    this->i_permission = 0;
    this->i_mode = 0;
    this->i_count = 0;
    this->i_nlink = 0;
    this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size = 0;
    this->i_lastr = -1;
    memset(this->i_addr, 0, sizeof(i_addr));
    this->i_time = time(NULL);
}

Inode::~Inode() {
}

// 将磁盘的DiskInode的内容拷贝到Inode上面
void Inode::copyDiskInode(DiskInode diskInode) {
    this->i_permission = diskInode.d_permission;
    this->i_mode = diskInode.d_mode;
    this->i_count = 0;
    this->i_nlink = diskInode.d_nlink;
    this->i_uid = diskInode.d_uid;
    this->i_gid = diskInode.d_gid;
    this->i_size = diskInode.d_size;
    memcpy(this->i_addr,diskInode.d_addr,sizeof(diskInode.d_addr));
}

// bmap函数 实现文件的逻辑块号到物理块号的转化
int Inode::bmap(int logical_blk_num) {
    if(logical_blk_num<SMALL_FILE_BLOCK){//小型文件
        // 如果对应的物理盘块
        // 对应的位置还没有申请数据块
        if(i_addr[logical_blk_num] == 0)
        {
            int new_blk_num = my_file_system.AllocBlock();//申请一块新的数据块
            if(new_blk_num ==-1)
                return -1;
            i_addr[logical_blk_num] = new_blk_num;
            return new_blk_num;
        }else{
            return i_addr[logical_blk_num];
        }
    }
    else{//巨型文件或者大型文件
        int index;
        if(logical_blk_num<LARGE_FILE_BLOCK){//大型文件
            index = (logical_blk_num- SMALL_FILE_BLOCK) / ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else{//巨型文件
            index = (logical_blk_num - LARGE_FILE_BLOCK) / (ADDRESS_PER_INDEX_BLOCK * ADDRESS_PER_INDEX_BLOCK) + 8;
        }
        int map_physical_blk_num1 = i_addr[index];//一次索引表的物理块号
        int map_physical_blk_num2;//二次索引表的物理块号
        int first_blk_map[BLOCK_SIZE/sizeof(int)];
        int second_blk_map[BLOCK_SIZE/sizeof(int)];//二次索引表
        memset(first_blk_map,0,sizeof(first_blk_map));
        memset(second_blk_map,0,sizeof(second_blk_map));
        if(map_physical_blk_num1== 0){//如果该数据块还没有被分配的话
            //分配一个空闲盘块存放索引表
            map_physical_blk_num1 = my_file_system.AllocBlock();
            if(map_physical_blk_num1 == -1)
                return  -1;
            i_addr[index] =  map_physical_blk_num1;
        }
        else{
            //读出索引表
            my_io_manager.readBlock((char*)&first_blk_map,map_physical_blk_num1);
        }

        if(index>=8){//巨型文件的情况

            index = ((logical_blk_num - LARGE_FILE_BLOCK) / ADDRESS_PER_INDEX_BLOCK ) % ADDRESS_PER_INDEX_BLOCK;
            map_physical_blk_num2 = first_blk_map[index];//得到第二个索引表的地址
            if(map_physical_blk_num2 == 0){//还未分配二次索引表
                map_physical_blk_num2 = my_file_system.AllocBlock();
                if(map_physical_blk_num2 == -1)
                    return -1;
                first_blk_map[index] = map_physical_blk_num2;
                //修改了，写回磁盘
                my_io_manager.writeBlock((char*)&first_blk_map,map_physical_blk_num1);
            }
            else{
                //读出二次索引表
                my_io_manager.readBlock((char*)&second_blk_map,map_physical_blk_num2);
            }
        }

        int physical_blk_num;
        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
        if( logical_blk_num < LARGE_FILE_BLOCK )
        {   // 大型文件
            index = (logical_blk_num - SMALL_FILE_BLOCK) % ADDRESS_PER_INDEX_BLOCK;
            if(!first_blk_map[index]){//还没有分配对应的数据块
                first_blk_map[index] = my_file_system.AllocBlock();
                // 修改了，写回磁盘
                my_io_manager.writeBlock((char*)&first_blk_map,map_physical_blk_num1);
            }
            physical_blk_num = first_blk_map[index];
        }
        else
        {   // 巨型文件
            index = (logical_blk_num - LARGE_FILE_BLOCK) % ADDRESS_PER_INDEX_BLOCK;
            if(!second_blk_map[index]){//还没有分配数据
                second_blk_map[index] = my_file_system.AllocBlock();
                // 修改了，写回磁盘
                my_io_manager.writeBlock((char*)&second_blk_map,map_physical_blk_num2);
            }
            physical_blk_num = second_blk_map[index];
        }
        return physical_blk_num;
    }
    return -1;
}

// 释放Inode自身在磁盘上面的空间
void Inode::free() {
    for(int i=9;i>=0;i--){
        if(i<=5){
            // 直接索引存在，释放直接索引块
            if(this->i_addr[i])
                my_file_system.FreeBlock(this->i_addr[i]);
        }
        else if(i<=7){
            // 一次间接索引存在，读取一次索引块
            if(this->i_addr[i]){
                int first_blk_map[BLOCK_SIZE/sizeof(int)];
                my_io_manager.readBlock((char *)&first_blk_map,this->i_addr[i]);
                for(int j=0;j<BLOCK_SIZE/sizeof(int);j++){
                    if(first_blk_map[j]) //释放一次索引表的每一项
                        my_file_system.FreeBlock(first_blk_map[j]);
                }
                // 释放一次索引表
                my_file_system.FreeBlock(this->i_addr[i]);
            }
        }
        else if(i<=9){
            // 一次间接索引存在，读取一次索引块
            if(this->i_addr[i]){
                int first_blk_map[BLOCK_SIZE/sizeof(int)];
                my_io_manager.readBlock((char *)&first_blk_map,this->i_addr[i]);
                for(int j=0;j<BLOCK_SIZE/sizeof(int);j++){
                    if(first_blk_map[j]){
                        // 二次索引块存在，读取二次索引块
                        int second_blk_map[BLOCK_SIZE/sizeof(int)];
                        my_io_manager.readBlock((char *)&second_blk_map,first_blk_map[j]);
                        for(int k=0;k<BLOCK_SIZE/sizeof(int);k++){
                            if(second_blk_map[k]) //释放一次索引表的每一项
                                my_file_system.FreeBlock(second_blk_map[j]);
                        }
                        // 释放二次索引块
                        my_file_system.FreeBlock(first_blk_map[j]);
                    }
                }
                // 释放一次索引块
                my_file_system.FreeBlock(this->i_addr[i]);
            }
        }
    }
}
/*---------------------------------------------------------------------------------------------------------*/
/*declaration of the methods of SuperBlock Class*/
SuperBlock::SuperBlock() {
    this->s_isize = INODE_NUM;       /* 外存Inode区占用的盘块数 */
    this->s_fsize = BLOCK_MAX_NUM;       /* 盘块总数 */
    this->s_nfree = 0;        /* 直接管理的空闲盘块数量 */
    this->s_free[0] = -1;     /* 直接管理的空闲盘块索引表 */
    this->s_ninode = 0;       /* 直接管理的空闲外存Inode数量 */
    this->s_flock = 0;        /* 封锁空闲盘块索引表标志 */
    this->s_ilock = 0;        /* 封锁空闲Inode表标志 */
    this->s_fmod = 0;         /* 内存中super block副本被修改标志 */
    this->s_ronly = 0; /* 本文件系统只能读出 */
    this->s_time = time(NULL);
     /* 最近一次更新时间 */
}

SuperBlock::~SuperBlock() {

}

/*---------------------------------------------------------------------------------------------------------*/
/* declaration of the methods of Directory Class */
Directory::Directory() {
    memset(this->d_inode_num,0,sizeof(this->d_inode_num));
    for(int i=0;i<DIRECTORY_SIZE;i++){
        this->d_file_name[i][0] = '\0';
    }
}
Directory::~Directory(){

}

// 拷贝dir到自身
void Directory::copyDirectory(Directory dir){
    this->inode_num = dir.inode_num;
    this->parent_inode_num = dir.parent_inode_num;
    strcpy(this->directory_name,dir.directory_name);
    strcpy(this->parent_directory_name,dir.parent_directory_name);
    memcpy(this->d_inode_num,dir.d_inode_num,sizeof(this->d_inode_num));
    memcpy(this->d_file_name,dir.d_file_name,sizeof(this->d_file_name));
}

/* declaration of the methods of the file class*/
File::File() {
    f_count = 0;
    f_flag = 0;
    f_inode = NULL;
    f_inode_num = -1;
    f_offset = -1;
    f_uid = -1;
    f_name = "";
}
File::~File() {

}
/* declaration of the methods of  OpenFileTable Class */

OpenFileTable::OpenFileTable() {
    for(int i=0;i<OPEN_FILE_NUM;i++){
        File empty_file;
        o_files[i] = empty_file;
    }
}
OpenFileTable::~OpenFileTable() {

}


// 分配一个File结构

int OpenFileTable::allocFile() {
    int fd = -1;
    for(int i=0;i<OPEN_FILE_NUM;i++){
        if(o_files[i].f_count == 0){
            fd = i;
            break;
        }
    }
    return fd;
}

int OpenFileTable::freeFile(int fd) {
    // 如果对应的fd不存在，那么释放这个file class就不需要执行
    if(!o_files[fd].f_count || fd<0||fd>=OPEN_FILE_NUM) return false;

    // 只需要将其全部滞空
    o_files[fd].f_count = 0;
    o_files[fd].f_name = "";
    o_files[fd].f_flag = -1;
    // 释放Inode...
    o_files[fd].f_inode = NULL;

    o_files[fd].f_offset = -1;
    o_files[fd].f_uid = -1;
    o_files[fd].f_inode_num = -1;
    return true;
}