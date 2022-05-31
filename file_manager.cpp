//
// Created by yang2001 on 2022/5/21.
//
# include "file_system.h"
# include "file_manager.h"
# include <string.h>
# include <stack>

extern FileSystem my_file_system;
extern Directory present_directory;
extern OpenFileTable my_open_file_table;
FileManager my_file_manager;
int root_inode_num;//根目录的Inode号，全局变量标志

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
    // .... need to be completed

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
    // 在则从表里获取，不在那么就读取

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
    // 定义用户，现在还没想好
    // new_file->f_uid =
    new_file->f_offset = 0;
    new_file->f_inode_num = inode_num;
    new_file->f_inode = &inode;
    new_file->f_flag = mode;
    new_file->f_name = string(file_name);

    return new_fd;

};

/* 关闭一个文件 */
void FileManager::closeFile(int fd){
    int closeSuccess = my_open_file_table.freeFile(fd);
    if(!closeSuccess){
        cout<<"文件关闭失败,不存在fd为:"<<fd<<"的文件"<<endl;
    }
    else{
        cout<<"文件关闭成功,成功关闭了fd为:"<<fd<<"的文件"<<endl;
    }
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
        cout<<123<<endl;
        cout<<content<<endl;
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

// 删了
void FileManager::deleteDirectory(const char* directory_name){

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

void FileManager::getCurrentDirectory(){

};


/* 对系统进行格式化的操作 */
void FileManager::formatSystem() {
    my_file_system.FormatDisk();//格式化磁盘
    // 申请一个盘块
    int new_blk_num = my_file_system.AllocBlock();
    // 申请一个Inode
    Inode root_inode;
    my_file_system.AllocInode(root_inode);
    // Inode 初始化操作
    root_inode.i_mode = Inode::INODE_DIRECTORY;
    root_inode.i_addr[0] = new_blk_num;//新申请的目录盘块号
    // Inode 权限和用户部分
    // ......还没有实现哈

    //创建根目录
    Directory root;
    root.inode_num = root_inode.i_number;
    root_inode_num = root_inode.i_number;//全局变量记录
    root.parent_inode_num = -1;// -1代表不存在父目录
    strcpy(root.directory_name,"/");
    // 当前目录为root
    present_directory = root;
    // root 写入磁盘块
    my_file_system.writeBlock((char*)&present_directory,new_blk_num);
    // Inode 写入磁盘更新
    my_file_system.writeInode(root_inode);
}

/* 展示当前目录INODE节点的文件项 */
void FileManager::ls(bool detail) {
    if(detail){
        for (int i = 0; i <DIRECTORY_SIZE; ++i) {
            cout<<"Name:"<<"Mode"<<"Size"<<"Alter_time";
            if(strlen(present_directory.d_file_name[i])){
                Inode inode;
                my_file_system.readInode(inode,present_directory.d_inode_num[i]);
                // 输出文件细节
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