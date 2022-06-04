//
// Created by yang2001 on 2022/5/21.
//
# include "file_system.h"
# include "file_manager.h"
# include <string.h>
# include <stack>
# include "user.h"

extern FileSystem my_file_system;
extern Directory present_directory;
extern OpenFileTable my_open_file_table;
User present_user;
FileManager my_file_manager;
int root_inode_num;//根目录的Inode号，全局变量标志




User getNewUser(int user_id,UserInfo user_info,GroupInfo group_info){
    User new_usr;
    new_usr.u_id = user_id;
    new_usr.g_id = user_info.u_g_id[user_id];
    strcpy(new_usr.u_name,user_info.u_name[user_id]);
    strcpy(new_usr.g_name,group_info.g_name[new_usr.g_id]);
    return new_usr;
}

int checkUserAuth(Inode inode,string mode) {
    // 检查当前用户对于此inode的读写权限
    if (mode == "read") {
        // 根据用户身份验证读权限
        if(present_user.u_id == inode.i_uid)// 当前用户为文件所有者
            return inode.i_permission & Inode::OWNER_R;
        else if(present_user.g_id == inode.i_gid)// 当前用户为文件同组用户
            return inode.i_permission & Inode::GROUP_R;
        else // 为其他用户
            return inode.i_permission & Inode::ELSE_R;

    }
    else if (mode == "write") {
        // 根据用户身份验证写权限
        if(present_user.u_id == inode.i_uid)// 当前用户为文件所有者
            return inode.i_permission & Inode::OWNER_W;
        else if(present_user.g_id == inode.i_gid)// 当前用户为文件同组用户
            return inode.i_permission & Inode::GROUP_W;
        else // 为其他用户
            return inode.i_permission & Inode::ELSE_W;
    }
    else if(mode == "delete"){
        // 根据用户身份验证删除权限 （读+写+可执行）
        if(present_user.u_id == inode.i_uid)// 当前用户为文件所有者
            return (inode.i_permission&Inode::OWNER_R)&&(inode.i_permission&Inode::OWNER_W)&&(inode.i_permission&Inode::OWNER_E);
        else if(present_user.g_id == inode.i_gid)// 当前用户为文件同组用户
            return (inode.i_permission&Inode::GROUP_R)&&(inode.i_permission&Inode::GROUP_W)&&(inode.i_permission&Inode::GROUP_E);
        else // 为其他用户
            return (inode.i_permission&Inode::ELSE_R)&&(inode.i_permission&Inode::ELSE_W)&&(inode.i_permission&Inode::ELSE_E);
    }
    return -1;

}

int checkSpace(SuperBlock superBlock){
    if(superBlock.s_nfree<=0||superBlock.s_ninode<=0){
        cout<<"系统空间不足"<<endl;
        return false;
    }
    return true;
}
int checkValidName(const char* name,string type){
    if (name == NULL || strlen(name) > DIRECTORY_SIZE) {
        if(type=="file"){
            cout << "文件名不合法" << endl;
        }
        else if(type=="directory"){
            cout << "目录名不合法" << endl;
        }
        return false;
    }
    return true;
}

int checkSameName(const char* name,string type,bool create){

        for(int i=0;i<DIRECTORY_SIZE;i++){
            if(strcmp(name,present_directory.d_file_name[i]) == 0 ){
                if(create){//如果是重名就创建失败了
                    if (type == "directory") cout<<"当前目录已经存在同名目录或文件，创建目录失败"<<endl;
                    if (type == "file") cout<<"当前目录已经存在同名目录或文件，创建文件失败"<<endl;
                    return -1;
                }
                else{ // 如果是删除目录或文件操作，存在相同的才能删除
                    return i;
                }
            }
        }
        if(create){//如果是创建目录或者文件，那么没找到重名才可以创建
            return true;
        }
        else{ // 如果是删除或者其他操作，没有找到相同的名称，操作失败
             if (type == "directory") cout<<"没有找到同名目录"<<endl;
             if (type == "file") cout<<"没有找到同名文件"<<endl;
            return -1;
        }
    return create;
}
int checkDirectoryFree(int &pos){
    int cnt = 0;
    for(int i=0;i<DIRECTORY_SIZE;i++){
        if(strlen(present_directory.d_file_name[i])) cnt++;
    }
    pos = cnt;
    if(cnt ==DIRECTORY_SIZE) cout<<"当前目录下目录和文件数已达上限"<<endl;
    return cnt!=DIRECTORY_SIZE;
}


/* 创建一个文件 */
void FileManager::creatFile(const char* file_name) {
    //检查目录名是否合法
    if(!checkValidName(file_name,"file"))
        return;
    // 读出SuperBlock
    SuperBlock superBlock = my_file_system.loadSuperBlock();
    // 查看是否还有空闲的INODE和BLOCK
    if(!checkSpace(superBlock))
        return;
    // 检查当前目录是否有同名文件
    if(checkSameName(file_name,"file",true)==-1)
        return;
    // 检查当前目录是否全满,同时寻找下一个插入的位置
    int new_directory_pos;
    if(!checkDirectoryFree(new_directory_pos))
        return;
    // 创建一个新的Inode
    Inode new_inode;
    my_file_system.AllocInode(new_inode);
    // 对Inode的数据进行对应的赋值
    new_inode.i_mode = Inode::INODE_FILE;
    // 对用户权限相关进行修改
    new_inode.i_uid = short(present_user.u_id);
    new_inode.i_gid = short(present_user.g_id);

    new_inode.i_permission  = 0;
    // 文件所有者具有读写、可执行权限
    new_inode.i_permission |=(Inode::OWNER_R|Inode::OWNER_W|Inode::OWNER_E);
    // 同组用户具有读写权限
    new_inode.i_permission |=(Inode::GROUP_R|Inode::GROUP_W);
    // 其他用户具有读权限
    new_inode.i_permission |=(Inode::ELSE_R);

    // 当前新的数据写入当前directory,同时更新当前新创建的文件在磁盘中。
    strcpy(present_directory.d_file_name[new_directory_pos],file_name);
    present_directory.d_inode_num[new_directory_pos] = new_inode.i_number;
    my_file_system.updatePresentDirectory();//更新
    // 新创建的Inode需要写回内存
    my_file_system.writeInode(new_inode);
    // 没有涉及对于superBlock的修改，因此不需要写回superBlock
}

/* 删除一个文件 */
void FileManager::deleteFile(const char *file_name) {
    //检查目录名是否合法
    if(!checkValidName(file_name,"file"))
        return;
    // 读出SuperBlock
    SuperBlock superBlock = my_file_system.loadSuperBlock();
    // 找出同名文件的位置
    int file_pos = checkSameName(file_name,"file",false);
    // file_pos == -1,说明不存在同名文件 return
    if(file_pos == -1)
        return;
    // 读出文件的inode
    Inode file_inode;
    my_file_system.readInode(file_inode,present_directory.d_inode_num[file_pos]);
    // 确认是否为文件
    if(file_inode.i_mode!=Inode::INODE_FILE){
        cout<<"当前目录不存在名为"<<file_name<<"的有效文件"<<endl;
        return;
    }
    // 对文件的读写权限进行检查
    // 用户如果要删除文件，那么必须具备对于当前文件的读权限、写权限以及可执行权限
    if(!checkUserAuth(file_inode,"delete")){
        cout<<"当前用户不具备对该文件的删除权限"<<endl;
        return;
    }
    // 释放文件的BLOCK
    file_inode.free();
    // 释放文件的inode
    my_file_system.FreeInode(file_inode.i_number);
    // 更新目录 - 原本的位置删除
    present_directory.d_inode_num[file_pos] = 0;
    strcpy(present_directory.d_file_name[file_pos],"");
    present_directory.d_file_name[file_pos][0] = '\0';
    // 当前目录信息写回磁盘
    my_file_system.updatePresentDirectory();
}


/* 打开一个文件 */
// 返回文件的句柄fd.....
int FileManager::openFile(const char* file_name,int mode){
    // 检查文件名
    if(!checkValidName(file_name,"file"))
        return -1;
    // 检查文件是否存在当前目录中
    int pos = checkSameName(file_name,"file", false);
    if(pos == -1)
        return -1;

    // 查看这个文件在不在内存Inode表里面
    int inode_num = present_directory.d_inode_num[pos];
    // 获取对应文件的Inode
    Inode inode;
    my_file_system.readInode(inode,inode_num);
    // 如果不是文件类型，那么打开也是失败的
    if(inode.i_mode != Inode::INODE_FILE){
        cout<<"当前目录不存在名为"<<file_name<<"的文件"<<endl;
        return -1;
    }

    // 定义新的File结构
    int new_fd = my_open_file_table.allocFile();//申请一个文件结构
    if(new_fd == -1){
        cout<<"系统内打开文件的数量已经达到上限"<<endl;
        return -1;
    }
    File* new_file;
    new_file = &my_open_file_table.o_files[new_fd];// 指向对应的文件结构的位置
    new_file->f_count = 1;
    // 当前用户的uid
    new_file->f_uid = present_user.u_id;
    new_file->f_offset = 0;
    new_file->f_inode_num = inode_num;
    new_file->f_inode = &inode;
    new_file->f_flag = mode;
    new_file->f_name = string(file_name);

    return new_fd;

};

/* 关闭一个文件 */
void FileManager::closeFile(int fd){
    int f_uid = my_open_file_table.o_files[fd].f_uid;
    int closeSuccess = my_open_file_table.freeFile(fd);
    if(!closeSuccess){
        cout<<"文件关闭失败,不存在fd为:"<<fd<<"的文件"<<endl;
        return;
    }
    if(f_uid!=present_user.u_id){
        cout<<"文件关闭失败，不能关闭其他用户打开的文件"<<endl;
        return;
    }
    cout<<"文件关闭成功,成功关闭了fd为:"<<fd<<"的文件"<<endl;
};

/* 从in_file_name文件里面读length个字节 写到fd对应的文件里面 */
int FileManager::writeFile(int fd, string in_file_name, int length) {
    // 检查fd是否合法
    if(!my_open_file_table.o_files[fd].f_count || fd<0 || fd>=OpenFileTable::OPEN_FILE_NUM){
        cout<<"读文件失败,不存在fd为:"<<fd<<"的文件"<<endl;
        return -1;
    }
    // 获取对应的file结构
    File * file = &my_open_file_table.o_files[fd];
    if(file->f_flag!=File::W_FLAG && file->f_flag!=File::RW_FLAG){
        cout<<"文件描述符"<<fd<<"对应的文件打开结构不具有写权限"<<endl;
        return -1;
    }
    // 获取对应的inode
    int inode_num = file->f_inode_num;
    Inode inode;
    my_file_system.readInode(inode,inode_num);

    // 根据inode的检查用户权限
    if(!checkUserAuth(inode,"write")){
        cout<<"当前用户不具有对该文件的写权限"<<endl;
        return -1;
    }

    // 获取in_file_name的文件内容
    const char * content = my_file_system.readFile(in_file_name,length);
    if(content==NULL)
        return -1;
    // 检查写入的内容是否超出了本文件系统的上限
    if(file->f_offset+strlen(content)>=Inode::HUGE_FILE_BLOCK*sizeof(BLOCK_SIZE)){
        cout<<"内容超出系统文件上限"<<endl;
        return -1;
    }
    // 分为Block,从file->f_offset开始写入文件
    int file_offset = file->f_offset;//文件读写指针指向的位置
    int write_count = 0;
    int logical_blk_num;//逻辑块的地址
    int physical_blk_num;//物理块的地址
    int byte_nums;//当次需要写入的块的大小
    int content_len = strlen(content);//写入的数据块的长度
    int blk_offset ;//块内偏移地址
    while(write_count<content_len){
        logical_blk_num = file_offset/BLOCK_SIZE;
        blk_offset = file_offset%BLOCK_SIZE;
        // 每次要写的字节数
        byte_nums = min(BLOCK_SIZE-blk_offset,content_len-write_count);
        physical_blk_num = inode.bmap(logical_blk_num);
        if(physical_blk_num == -1) //为-1表示申请失败了 那么直接返回就好
            return -1;
        // 读写使用的缓冲区
        char buffer[512];
        memset(buffer,0,sizeof(buffer));
        if(byte_nums!=BLOCK_SIZE){
            // 没有写满一个数据块，需要读出数据块，保护未读写的区域
            my_file_system.readBlock(buffer,physical_blk_num);
        }
        /* 每次将content[write_offset] - content[write_offset+byte_nums-1]写入
         * buffer+blk_offset起始 byte_nums 个字节
         * */
        memcpy(buffer+blk_offset,content+write_count,byte_nums);
        //cout<<"physical_blk_num"<<physical_blk_num<<endl;
        my_file_system.writeBlock(buffer,physical_blk_num);
        // 写完了，对对应的变量进行操作
        file_offset += byte_nums; // 文件指针向前移动byte_nums 个字节
        write_count += byte_nums; // 总写入的字节数增加了byte_nums

        if(file_offset > inode.i_size) //更新文件的大小
            inode.i_size = file_offset;
    }
    // 写完了，更新Inode
    inode.i_time = time(NULL);
    my_file_system.writeInode(inode);
    // 更新file结构的指针
    file->f_offset = file_offset;
    // 返回成功写入的字节数
    return write_count;
}

/* 读文件内容到content里面，其中读length个字节 */
int FileManager::readFile(int fd,string out_file_name,int length){

    // 检查fd是否合法
    if(!my_open_file_table.o_files[fd].f_count || fd<0 || fd>=OpenFileTable::OPEN_FILE_NUM){
        cout<<"读文件失败,不存在fd为:"<<fd<<"的文件"<<endl;
        return -1;
    }
    // 获取对应的file结构
    File * file = &my_open_file_table.o_files[fd];
    if(file->f_flag!=File::R_FLAG && file->f_flag!=File::RW_FLAG){
        cout<<"文件描述符"<<fd<<"对应的文件打开结构不具有读权限"<<endl;
        return -1;
    }
    // 获取对应的inode
    Inode inode;
    my_file_system.readInode(inode,file->f_inode_num);
    // 根据inode的检查权限
    // 一个用户需要删除文件 必须具有对这个文件的读写和可执行权限
    if(!checkUserAuth(inode,"read")){
        cout<<"当前用户不具有对该文件的读权限"<<endl;
        return -1;
    }
    // 首先计算长度
    int read_count = 0; // 读累计的字节数
    int file_offset = file->f_offset;//文件读写指针指向的位置
    int logical_blk_num;//逻辑块的地址
    int physical_blk_num;//物理块的地址
    int byte_nums;//当次需要读入的字节数
    int content_len = min(length,inode.i_size-file_offset);//总计需要读入的字节数
    int blk_offset ;//块内偏移地址
    // 写出的位置
    char content[1000004];

    while(read_count<content_len){

        logical_blk_num = file_offset/BLOCK_SIZE;//本次读取的逻辑地址
        blk_offset = file_offset%BLOCK_SIZE;//块内偏移地址
        byte_nums = min(BLOCK_SIZE-blk_offset,content_len-read_count);

        physical_blk_num = inode.bmap(logical_blk_num);
        // 缓冲区
        char buffer[BLOCK_SIZE];
        memset(buffer,0,sizeof(buffer));
        // 读出对应逻辑块号对应的物理块，将其保存到buffer中
        my_file_system.readBlock(buffer,physical_blk_num);
        /* 每次将content[write_offset] - content[write_offset+byte_nums-1]读入
        * buffer+blk_offset起始 byte_nums 个字节
        * */
        memcpy(content+read_count,buffer+blk_offset,byte_nums);

        file_offset += byte_nums; // 文件指针向前移动byte_nums 个字节
        read_count += byte_nums; // 总读入的字节数增加了byte_nums


    }
    // 拷贝的内容最后添加尾零
    content[read_count] = '\0';
    if(out_file_name.length() == 0){
        cout<<content<<endl;
    }
    else{
        my_file_system.writeFile(out_file_name,content);
    }

    return read_count;
};

/* 移动文件指针pos个字节 */
void FileManager::seekFile(int fd,int offset,int ptrname){
    // 检查fd是否合法
    if(!my_open_file_table.o_files[fd].f_count || fd<0 || fd>=OpenFileTable::OPEN_FILE_NUM){
        cout<<"读文件失败,不存在fd为:"<<fd<<"的文件"<<endl;
    }
    // 获取对应的file结构
    File * file = &my_open_file_table.o_files[fd];
    // 获得inode结构
    Inode inode;
    my_file_system.readInode(inode,file->f_inode_num);
    int new_ptr_pos; // 新的指针指向的位置
    switch (ptrname) {
        // 文件起始位置 + offset
        case 0:
            new_ptr_pos = offset;
            break;
        // 当前指针位置 + offset
        case 1:
            new_ptr_pos = file->f_offset + offset;
            break;
        // 文件终止位置 + offset
        case 2:
            new_ptr_pos = inode.i_size + offset;
            break;
    }
    // 对文件指针合法性做出判断
    if(new_ptr_pos<0){
        cout<<"文件指针不能为负数"<<endl;
        return;
    }
    if(new_ptr_pos>inode.i_size){
        cout<<"输入的文件指针大于文件长度，定位到文件末尾"<<endl;
        new_ptr_pos = inode.i_size;
    }
    // 修改文件指针的位置
    file->f_offset = new_ptr_pos;
};


/* 创建一个目录 */

void FileManager::createDirectory(const char* directory_name){
    //检查目录名是否合法
    if(!checkValidName(directory_name,"directory")) return;
    // 读出SuperBlock
    SuperBlock superBlock = my_file_system.loadSuperBlock();
    // 查看是否还有空闲的INODE和BLOCK
    if(!checkSpace(superBlock)) return;
    // 检查当前目录是否有同名目录
    if(checkSameName(directory_name,"directory",true) == -1) return;
    // 检查当前目录是否全满,同时寻找下一个插入的位置
    int new_directory_pos;
    if(!checkDirectoryFree(new_directory_pos)) return;
    // 获得一个新的Block
    int new_blk_num = my_file_system.AllocBlock();
    // 创建一个新的Inode
    Inode new_inode;
    my_file_system.AllocInode(new_inode);
    // 对Inode的数据进行对应的赋值
    new_inode.i_addr[0] = new_blk_num;
    new_inode.i_mode = Inode::INODE_DIRECTORY;
    new_inode.i_uid = present_user.u_id;
    new_inode.i_gid = present_user.g_id;

    // 所有者具有读写、可执行权限
    new_inode.i_permission |=(Inode::OWNER_R|Inode::OWNER_W|Inode::OWNER_E);
    // 同组用户具有读写权限
    new_inode.i_permission |=(Inode::GROUP_R|Inode::GROUP_W);
    // 其他用户具有读权限
    new_inode.i_permission |=(Inode::ELSE_R);

    // 当前新的数据写入当前directory,同时更新当前directory在磁盘中。
    strcpy(present_directory.d_file_name[new_directory_pos],directory_name);
    present_directory.d_inode_num[new_directory_pos] = new_inode.i_number;
    my_file_system.updatePresentDirectory();//更新
    // 获得一个新的directory 写入当前的数据 以及对应parent目录的数据
    Directory new_directory;
    new_directory.inode_num = new_inode.i_number;
    new_directory.parent_inode_num = present_directory.inode_num;
    strcpy(new_directory.directory_name,directory_name);
    strcpy(new_directory.parent_directory_name,present_directory.directory_name);
    // 拷贝内容后写入目录
    my_file_system.writeBlock((char*)&new_directory,new_blk_num);
    // Inode写回内存
    my_file_system.writeInode(new_inode);
    // 没有涉及对于superBlock的修改，因此不需要写回superBlock
};

// cd
void FileManager::openDirectory(const char* path){
    // 检查路径是否合法
    if(path==NULL){
        cout<<"路径名不合法"<<endl;
    }
    // cd . 停留在当前目录
    if(strcmp(path,".") == 0){
        return;
    }
    // cd .. 返回父目录
    if(strcmp(path,"..") == 0){
        // 如果当前不为根目录
        if(present_directory.parent_inode_num!=-1){
            Inode inode;
            my_file_system.readInode(inode,present_directory.parent_inode_num);
            int blk_num = inode.i_addr[0];
            Directory new_present_directory;
            my_file_system.readBlock((char*)&new_present_directory,blk_num);
            present_directory = new_present_directory;//更换当前目录
        }
        return;
    }
    // cd /a/b/c 绝对路径
    if(path[0]=='/') {
        if(strcmp(path,"/")==0){//回到根目录指令
            Inode inode;
            my_file_system.readInode(inode,root_inode_num);
            my_file_system.readBlock((char*)&present_directory,inode.i_addr[0]);
            return;
        }
        char relative_path[strlen(path)];
        strcpy(relative_path,path);
        char sep[] = "/";
        char *dir_name = NULL;
        dir_name = strtok(relative_path, sep);
        Directory tmp_dir;//从当前目录开始找
        // 找出根目录
        Inode inode;//存在同名目录,读出这个inode
        my_file_system.readInode(inode,root_inode_num);
        // 根目录读入tmp_dir
        my_file_system.readBlock((char*)&tmp_dir,inode.i_addr[0]);

        bool find_it;
        while(dir_name!=NULL){
            find_it = false;
            for(int i=0;i<DIRECTORY_SIZE;i++){
                // 找到同名的文件或者目录
                if(strcmp(dir_name,tmp_dir.d_file_name[i])==0){
                    Inode inode;//存在同名目录,读出这个inode
                    my_file_system.readInode(inode,tmp_dir.d_inode_num[i]);
                    int blk_num = inode.i_addr[0];
                    if(inode.i_mode==Inode::INODE_DIRECTORY){
                        my_file_system.readBlock((char*)&tmp_dir,blk_num);
                        find_it = true;
                    }
                }
            }
            if(!find_it) break;//没找到，不用搜索了
            dir_name = strtok(NULL, sep);
        }

        if(find_it){ // 找到了，当前目录变为找到的目录
            present_directory = tmp_dir;
        }else{
            cout<<"cd:"<<path<<":不存在对应目录";
        }
    }
    else{// cd a/b/c 相对路径
        char relative_path[strlen(path)];
        strcpy(relative_path,path);
        char sep[] = "/";
        char *dir_name = NULL;
        dir_name = strtok(relative_path, sep);
        Directory tmp_dir;//从当前目录开始找
        tmp_dir = present_directory;
        bool find_it;
        while(dir_name!=NULL){
            find_it = false;
            for(int i=0;i<DIRECTORY_SIZE;i++){
                // 找到同名的文件或者目录
                if(strcmp(dir_name,tmp_dir.d_file_name[i])==0){
                    Inode inode;//存在同名目录,读出这个inode
                    my_file_system.readInode(inode,tmp_dir.d_inode_num[i]);
                    int blk_num = inode.i_addr[0];
                    if(inode.i_mode==Inode::INODE_DIRECTORY){
                        my_file_system.readBlock((char*)&tmp_dir,blk_num);
                        find_it = true;
                    }
                }
            }
            if(!find_it) break;//没找到，不用搜索了
            dir_name = strtok(NULL, sep);
        }

        if(find_it){ // 找到了，当前目录变为找到的目录
            present_directory = tmp_dir;
        }else{
            cout<<"cd:"<<path<<":不存在对应目录";
        }
    }

};


// 显示当前用户所有打开的文件名和文件打开标识符
void FileManager::showOpenFileList(){
    for(int i=0;i<OpenFileTable::OPEN_FILE_NUM;i++){
        if(my_open_file_table.o_files[i].f_count){//这里要加入是否是当前用户的判断
            cout<<"file name:"<<my_open_file_table.o_files[i].f_name<<"  ";
            cout<<"fd:"<<i<<"  ";
            cout<<"mode:";
            cout<<endl;
        }
    }
}

void FileManager::pwd() {
    Directory tmp_dir = present_directory;
    stack<string> res;
    res.push(string(tmp_dir.directory_name));
    while (tmp_dir.parent_inode_num!=-1){
        Inode inode;
        my_file_system.readInode(inode,tmp_dir.parent_inode_num);
        my_file_system.readBlock((char*)&tmp_dir,inode.i_addr[0]);
        res.push(string(tmp_dir.directory_name));
    }

    while(!res.empty()){
        string top = res.top();
        cout<<top;
        res.pop();
        if(!res.empty() && top!="/")
            cout<<"/";
    }
    cout<<endl;
}

/* user relevant instruction */

// 向系统中添加一个user
int FileManager::addUser(const char* user_name,const char* pwd){
    UserInfo user_info;
    my_file_system.readBlock((char*)& user_info,USER_INFO_BLK_NUM);
    // 检查user_name 和 pwd的长度
    if(strlen(user_name)>USER_NAME_SIZE || strlen(pwd)> USER_PWD_SIZE){
        cout<<"用户名或密码长度超出限制"<<endl;
        return -1;
    }
    if(user_info.hasUser(user_name)!=-1){
        cout<<"系统已经存在用户名为"<<user_name<<"的用户"<<endl;
        return -1;
    }
    // 添加用户
    int user_id = user_info.addUser(user_name,pwd,DEFAULT_USER_GROUP_ID);
    if(user_id == -1){
        cout<<"系统用户数量超出限制"<<endl;
        return -1;
    }
    my_file_system.writeBlock((char*)&user_info,USER_INFO_BLK_NUM);
    return user_id;
};

// 添加一个Group
int FileManager::addGroup(const char* group_name){
    // 先检查当前用户是否为root用户
    if(strcmp(present_user.u_name,"root")!=0){
        cout<<"Permission denied,只有root用户才能添加用户组"<<endl;
        return -1;
    }
    // 检查group_name是否合法
    if(strlen(group_name)>GROUP_NAME_SIZE){
        cout<<"用户组名称超出限制"<<endl;
        return -1;
    }
    // 添加用户组
    GroupInfo user_group;
    my_file_system.readBlock((char*)&user_group,GROUP_INFO_BLK_NUM);
    // 检查是否存在名为group_name的用户组
    if(user_group.hasGroup(group_name)!=-1){
        cout<<"已经存在名为"<<group_name<<"的用户组"<<endl;
        return -1;
    }
    // 添加group_name
    int group_id = user_group.addGroup(group_name);
    if(group_id == -1){
        cout<<"添加失败,用户组数量超出系统上线"<<endl;
    }
    my_file_system.writeBlock((char*)&user_group,GROUP_INFO_BLK_NUM);
    return group_id;
}

void FileManager::modifyUserGroup(const char *user_name, const char *group_name) {
    // 先检查当前用户是否为root用户
    if(strcmp(present_user.u_name,"root")!=0){
        cout<<"Permission denied,只有root用户才能更改用户的用户组"<<endl;
        return;
    }
    // 检查输入合法性
    if(strlen(group_name)>GROUP_NAME_SIZE|| strlen(user_name)>USER_NAME_SIZE){
        cout<<"用户名或用户组名称长度超出限制"<<endl;
        return;
    }
    // 不能修改root的用户组
    if(strcmp(user_name,"root")==0){
        cout<<"不能修改root用户的用户组"<<endl;
        return;
    }
    // 读出group
    UserInfo user_info;
    GroupInfo user_group;
    my_file_system.readBlock((char*)&user_group,GROUP_INFO_BLK_NUM);
    my_file_system.readBlock((char*)&user_info,USER_INFO_BLK_NUM);

    // 获取group_name对应的ID
    int group_id = user_group.getGroupId(group_name);
    if(group_id == -1){
        cout<<"不存在名称为"<<group_name<<"的用户组"<<endl;
        return;
    }
    // 获取user_name对应的ID
    int user_id  = user_info.hasUser(user_name);
    if(user_id == -1){
        cout<<"不存在用户名为"<<user_name<<"的用户"<<endl;
        return;
    }
    // 修改对应的user磁盘结构对应的group_id
    user_info.u_g_id[user_id] = group_id;
    my_file_system.writeBlock((char*)&user_info,USER_INFO_BLK_NUM);
    my_file_system.writeBlock((char*)&user_group,GROUP_INFO_BLK_NUM);

    cout<<"修改了"<<user_name<<"的用户组为"<<group_name<<endl;

    return;
}
void FileManager::changeFileMode(const char* file_name,int user_mode,int group_mode,int else_mode){
    // 检查输入合法性
    if(strlen(file_name)>FILE_NAME_SIZE){
        cout<<"用户名或文件名称长度超出限制"<<endl;
        return;
    }
    // 检查当前目录下是否存在这个文件
    int f_pos = checkSameName(file_name,"file", false);
    if(f_pos == -1){
        cout<<"当前目录不存在名为file_name的文件"<<endl;
        return;
    }
    // 获得当前文件的Inode
    Inode inode;
    my_file_system.readInode(inode,present_directory.d_inode_num[f_pos]);
    inode.i_permission = 0;
    switch (user_mode) {
        case 0:
            break;
        case 1: // 只读
            inode.i_permission |= Inode::OWNER_R;
            break;
        case 2: // 读写
            inode.i_permission |= (Inode::OWNER_R|Inode::OWNER_W);
            break;
        case 3: // 读写+可执行
            inode.i_permission |= (Inode::OWNER_R|Inode::OWNER_W|Inode::OWNER_E);
            break;
    }
    switch (group_mode) {
        case 0:
            break;
        case 1: // 只读
            inode.i_permission |= Inode::GROUP_R;
            break;
        case 2: // 读写
            inode.i_permission |= (Inode::GROUP_R|Inode::GROUP_W);
            break;
        case 3: // 读写+可执行
            inode.i_permission |= (Inode::GROUP_R|Inode::GROUP_W|Inode::GROUP_E);
            break;
    }
    switch (else_mode) {
        case 0:
            break;
        case 1: // 只读
            inode.i_permission |= Inode::ELSE_R;
            break;
        case 2: // 读写
            inode.i_permission |= (Inode::ELSE_R|Inode::ELSE_W);
            break;
        case 3: // 读写+可执行
            inode.i_permission |= (Inode::ELSE_R|Inode::ELSE_W|Inode::ELSE_E);
            break;
    }
    my_file_system.writeInode(inode);

};

bool FileManager::deleteUser(const char* user_name){
    // 先检查当前用户是否为root用户
    if(strcmp(present_user.u_name,"root")!=0){
        cout<<"Permission denied,只有root用户才能删除用户"<<endl;
        return false;
    }
    if(strcmp(user_name,"root")==0){
        cout<<"不能删除root用户"<<endl;
        return false;
    }
    // 检查user_name是否合法
    if(strlen(user_name)>USER_NAME_SIZE){
        cout<<"用户名称长度超出限制"<<endl;
        return false;
    }
    //
    UserInfo user_info;
    my_file_system.readBlock((char*)& user_info,USER_INFO_BLK_NUM);
    int del_user_id = user_info.delUser(user_name);
    if(del_user_id == -1){
        cout<<"删除失败,系统中不存在名为"<<user_name<<"的用户"<<endl;
    }
    // 写回磁盘
    my_file_system.writeBlock((char*)& user_info,USER_INFO_BLK_NUM);
    // 遍历打开文件表，对所有删除的用户的文件进行释放
    for(int i=0;i<OpenFileTable::OPEN_FILE_NUM;i++){
        // 如果对应的打开文件结构属于被删除的用户，那么对其进行释放
        if(my_open_file_table.o_files[i].f_uid == del_user_id)
            my_open_file_table.freeFile(i);
    }
    return true;
};

// 改变当前目录下名为file_name的文件的用户组为group_name
void FileManager::changeFileGroup(const char* file_name,const char* group_name){
    // 先检查当前用户是否为root用户
    if(strcmp(present_user.u_name,"root")!=0){
        cout<<"Permission denied,只有root用户才能更改文件的用户组"<<endl;
        return;
    }
    // 检查输入合法性
    if(strlen(group_name)>GROUP_NAME_SIZE|| strlen(file_name)>FILE_NAME_SIZE){
        cout<<"用户名或文件名称长度超出限制"<<endl;
        return;
    }
    // 检查当前目录下是否存在这个文件
    int f_pos = checkSameName(file_name,"file", false);
    if(f_pos == -1){
        cout<<"当前目录不存在名为file_name的文件"<<endl;
        return;
    }
    GroupInfo user_group;
    my_file_system.readBlock((char*)&user_group,GROUP_INFO_BLK_NUM);
    // 获取group_name对应的ID
    int group_id = user_group.getGroupId(group_name);
    if(group_id == -1){
        cout<<"不存在名称为"<<group_name<<"的用户组"<<endl;
    }
    // 获得当前文件的Inode
    Inode inode;
    my_file_system.readInode(inode,present_directory.d_inode_num[f_pos]);
    // 修改对应的group_id
    inode.i_gid = group_id;
    // 写回磁盘
    my_file_system.writeInode(inode);
};
void FileManager::su(const char* user_name,const char* pwd){
    // 检查user_name 和 pwd的长度
    if(strlen(user_name)>USER_NAME_SIZE || strlen(pwd)> USER_PWD_SIZE){
        cout<<"用户名或密码长度超出限制"<<endl;
    }
    // 读取数据
    UserInfo user_info;
    my_file_system.readBlock((char*)& user_info,USER_INFO_BLK_NUM);
    GroupInfo user_group;
    my_file_system.readBlock((char*)&user_group,GROUP_INFO_BLK_NUM);

    int user_id = user_info.login(user_name,pwd);
    if(user_id ==-1){
        cout<<"用户名或密码错误"<<endl;
        return;
    }
    // 获取一个新的用户对象,赋值给当前用户对象
    present_user = getNewUser(user_id,user_info, user_group);
};
void FileManager::whoAmI(){
    cout<<"user_name:"<<present_user.u_name<<endl;
    cout<<"group_name:"<<present_user.g_name<<endl;
};


/* 对系统进行格式化的操作 */
void FileManager::formatSystem() {
    my_file_system.FormatDisk();//格式化文件卷
    // 申请一个盘块
    int new_blk_num = my_file_system.AllocBlock();
    // 申请一个Inode
    Inode root_inode;
    my_file_system.AllocInode(root_inode);
    // Inode 初始化操作
    root_inode.i_mode = Inode::INODE_DIRECTORY;
    root_inode.i_addr[0] = new_blk_num;//新申请的目录盘块号
    if(debug)
        cout<<"root_blk_num"<<new_blk_num<<endl;

    // Inode 权限和用户部分

    //创建根目录
    Directory root;
    root.inode_num = root_inode.i_number;
    root_inode_num = root_inode.i_number;//全局变量记录
    root.parent_inode_num = -1;// -1代表不存在父目录
    strcpy(root.directory_name,"/");
    // 当前目录为root
    present_directory = root;
    my_file_system.writeBlock((char*)&present_directory,new_blk_num);
    // root 写入磁盘块
    my_file_system.writeBlock((char*)&present_directory,new_blk_num);
    // Inode 写入磁盘更新
    my_file_system.writeInode(root_inode);

    // 申请2个块给UserInfo
    int user_info_blk_num = my_file_system.AllocBlock();
    int group_info_blk_num = my_file_system.AllocBlock();
    if(debug)
        cout<<"user_info_blk_num"<<user_info_blk_num<<endl;
    if(debug)
        cout<<"group_info_blk_num"<<group_info_blk_num<<endl;

    UserInfo user_info;
    GroupInfo group_info;
    // 创建两个用户组 root_group 与 user_group
    int root_group_id = group_info.addGroup("root_group");
    int user_group_id = group_info.addGroup("user_group");
    if(debug)
        cout<<root_group_id<<endl;
    if(debug)
        cout<<user_group_id<<endl;
    // 创建了root用户
    int root_user_id = user_info.addUser("root","123456",0);
    User root_user;
    root_user.u_id = root_user_id;
    root_user.g_id = root_group_id;
    strcpy(root_user.u_name,"root");
    strcpy(root_user.g_name,"root_group");
    present_user = root_user;
    // 写回信息
    my_file_system.writeBlock((char*)&user_info,USER_INFO_BLK_NUM);
    my_file_system.writeBlock((char*)&group_info,GROUP_INFO_BLK_NUM);

    // 创建子目录
    this->createDirectory("bin");
    this->createDirectory("home");
    this->createDirectory("etc");
    this->createDirectory("dev");
    // 进入home里面
    this->openDirectory("home");
    this->createDirectory("texts");
    this->createDirectory("reports");
    this->createDirectory("photos");
    // 创建文件texts
    this->openDirectory("/home/texts");
    this->creatFile("text");
    int text_fd = this->openFile("text",File::RW_FLAG);
    this->writeFile(text_fd,"ReadMe.txt",100000);
    this->closeFile(text_fd);
    // 创建文件reports
    this->openDirectory("/home/reports");
    this->creatFile("report");
    int report_fd = this->openFile("report",File::RW_FLAG);
    this->writeFile(report_fd,"report.md",100000);
    this->closeFile(report_fd);
    // 创建文件
    this->openDirectory("/home/photos");
    this->creatFile("photo");
    int photo_fd = this->openFile("photo",File::RW_FLAG);
    this->writeFile(photo_fd,"photo.jpeg",100000);
    this->closeFile(photo_fd);
    // 回到root
    this->openDirectory("/");

}

// 工具函数输出程序的
string outputMode(int permission){
    string res="";
    if(permission & Inode::OWNER_R) res +="r";
    else res+="-";
    if(permission & Inode::OWNER_W) res +="w";
    else res+="-";
    if(permission & Inode::OWNER_E) res +="x";
    else res+="-";

    res+=" ";

    if(permission & Inode::GROUP_R) res +="r";
    else res+="-";
    if(permission & Inode::GROUP_W) res +="w";
    else res+="-";
    if(permission & Inode::GROUP_E) res +="x";
    else res+="-";

    res+=" ";

    if(permission & Inode::ELSE_R) res +="r";
    else res+="-";
    if(permission & Inode::ELSE_W) res +="w";
    else res+="-";
    if(permission & Inode::ELSE_E) res +="x";
    else res+="-";

    return res;
}
/* 展示当前目录INODE节点的文件项 */
void FileManager::ls(bool detail) {
    if(detail){
        cout<<setw(15)<<setiosflags(ios::left)<<"type";
        cout<<setw(10)<<"uid";
        cout<<setw(10)<<"gid";
        cout<<setw(20)<<"Name:";
        cout<<setw(15)<<"Mode";
        cout<<setw(10)<<"Size";
        cout<<setw(20)<<"Altered time";
        cout<<endl;

        for (int i = 0; i <DIRECTORY_SIZE; ++i) {
            if(strlen(present_directory.d_file_name[i])){
                Inode inode;
                my_file_system.readInode(inode,present_directory.d_inode_num[i]);
                // 输出文件细节
                cout<<setw(15)<<((inode.i_mode==Inode::INODE_FILE)?"file":"directory");
                cout<<setw(10)<<inode.i_uid;
                cout<<setw(10)<<inode.i_gid;
                cout<<setw(20)<<present_directory.d_file_name[i];
                cout<<setw(15)<<outputMode(inode.i_permission);
                cout<<setw(10)<<inode.i_size;
                cout<<setw(20)<<put_time(localtime(&inode.i_time),"%Y-%m-%d %H.%M.%S");
                cout<<" "<<endl;

            }
        }
    }
    else{
        for (int i = 0; i <DIRECTORY_SIZE; ++i) {
            if(strlen(present_directory.d_file_name[i])){
                cout<<present_directory.d_file_name[i]<<"  ";
            }
        }
        cout<<endl;
    }
}

void FileManager::boost() {
    root_inode_num = ROOT_INODE_NUM;
    my_file_system.readBlock((char*)&present_directory,ROOT_BLK_NUM);
    present_user.u_id = ROOT_USER_ID;
    present_user.g_id = ROOT_GROUP_ID;
    strcpy(present_user.u_name,"root");
    strcpy(present_user.g_name,"root_group");
    // 其他的全局变量都通过构造函数完成了初始化
}