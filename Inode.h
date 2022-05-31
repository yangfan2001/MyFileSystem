//
// Created by yang2001 on 2022/5/16.
//

#ifndef MYFILESYSTEM_INODE_H
#define MYFILESYSTEM_INODE_H
class Inode;
class DiskInode;
// The declaration of SuperBlock Inode DiskInodee
// define SuperBlock 磁盘索引节点（这里参考了UNIX V6++的实现）
class SuperBlock{
public:
    int s_isize;       /* 外存Inode区占用的盘块数 */
    int s_fsize;       /* 盘块总数 */
    int s_nfree;       /* 直接管理的空闲盘块数量 */
    int s_free[100];   /* 直接管理的空闲盘块索引表 */

    int s_ninode;      /* 直接管理的空闲外存Inode数量 */
    int s_inode[100];  /* 直接管理的空闲外存Inode索引表 */

    int s_flock;       /* 封锁空闲盘块索引表标志 */
    int s_ilock;       /* 封锁空闲Inode表标志 */

    int s_fmod;        /* 内存中super block副本被修改标志 */
    int s_ronly;       /* 本文件系统只能读出 */
    time_t s_time;        /* 最近一次更新时间 */
    int padding[46];   /* 填充使得SuperBlock块大小为1024字节 */
public:
    SuperBlock();
    ~SuperBlock();
};
// define DiskInode 磁盘索引节点（这里参考了UNIX V6++的实现）

// sizeof DiskInode = 64
class DiskInode{
public:
    DiskInode();//构造函数
    ~DiskInode();//析构函数

    unsigned short d_permission;	// 状态的标志位，定义见enum INodeFlag
    unsigned short d_mode;	// 文件工作方式信息
    int		d_nlink;		/* 文件联结计数，即该文件在目录树中不同路径名的数量 */
    short	d_uid;			/* 文件所有者的用户标识数 */
    short	d_gid;			/* 文件所有者的组标识数 */
    int		d_size;			/* 文件大小，字节为单位 */
    int		d_addr[10];		/* 用于文件逻辑块好和物理块好转换的基本索引表 */
    time_t		d_time;		/* 最后修改时间 */

public:
    void copyInode(Inode inode);
};


// define Inode 内存索引节点（这里参考了UNIX V6++的实现）
class Inode{
public:
    enum InodeMode {
        INODE_FILE = 0x1,//是文件
        INODE_DIRECTORY = 0x2//是目录
    };

    enum InodePermission {//分为文件主、文件主同组和其他用户
        OWNER_R = 0400,
        OWNER_W = 0200,
        OWNER_E = 0100,
        GROUP_R = 040,
        GROUP_W = 020,
        GROUP_E = 010,
        ELSE_R = 04,
        ELSE_W = 02,
        ELSE_E = 01,
    };

    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);	/* 每个间接索引表(或索引块)包含的物理盘块号 */

    static const int SMALL_FILE_BLOCK = 6;	/* 小型文件：直接索引表最多可寻址的逻辑块号 */
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	/* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	/* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */

public:
    unsigned short i_permission;	// 状态的标志位，定义见enum INodeFlag
    unsigned short i_mode;	// 文件工作方式信息

    int		i_count;		// 引用计数
    int		i_nlink;		// 文件联结计数，即该文件在目录树中不同路径名的数量

    short	i_dev;			// 外存INode所在存储设备的设备号
    int		i_number;		// 外存INode区中的编号

    short	i_uid;			// 文件所有者的用户标识数
    short	i_gid;			// 文件所有者的组标识数

    int		i_size;			// 文件大小，字节为单位
    int		i_addr[10];		// 用于文件逻辑块好和物理块好转换的基本索引表
    time_t     i_time;         // 最后一次修改文件的时间
    int		i_lastr;		// 存放最近一次读取文件的逻辑块号，用于判断是否需要预读
public:
    Inode();//构造函数
    ~Inode();//析构函数
    void copyDiskInode(DiskInode diskInode);
    int bmap(int logical_blk_num); // 将文件逻辑块号转化为在磁盘上的物理逻辑块号
    void free();

};

// 计算位置的常量
static const int SUPER_BLOCK_POS = 0; // SuperBLock物理位置（PER BLOCK）
static const int INODE_POS = SUPER_BLOCK_POS+sizeof(SuperBlock)/BLOCK_SIZE;// INODE物理位置的起始区 (DATA PER BLOCK)
static const int BLOCK_POS = INODE_POS+(sizeof(DiskInode)*INODE_NUM/BLOCK_SIZE); // 数据块BLOCK物理位置的起始区  (DATA PER BLOCK)

static const int BLOCK_NUM = BLOCK_MAX_NUM - INODE_NUM*sizeof(DiskInode)/BLOCK_SIZE  -
                             sizeof(SuperBlock)/BLOCK_SIZE;// 实际的逻辑块数 = 盘块逻辑块数 - Inode占据的逻辑块数 - SuperBlock占据的逻辑块数

class Directory{
public:
    int inode_num; // 自己的Inode号
    char directory_name[FILE_NAME_SIZE];
    int parent_inode_num;// 父目录 Inode号
    char parent_directory_name[FILE_NAME_SIZE];
    int d_inode_num[DIRECTORY_SIZE];//INODE
    char d_file_name[DIRECTORY_SIZE][FILE_NAME_SIZE];//the fileName
public:
    /* Constructors */
    Directory();
    /* Destructors */
    ~Directory();
    // 拷贝dir到自身
    void copyDirectory(Directory dir);
};

class File{
public:
    /* Enumerate */
    enum FileFlags
    {
        R_FLAG = 0x1,			/* 读请求类型 */
        W_FLAG = 0x2,			/* 写请求类型 */
        RW_FLAG = 0x3 /* 读写请求类型 */
    };

    /* Functions */
public:
    /* Constructors */
    File();
    /* Destructors */
    ~File();


    /* Member */
    // unsigned int f_flag;		/* 对打开文件的读、写操作要求 */

    int f_uid; // 打开文件user_id
    Inode*	f_inode;			/* 指向打开文件的内存Inode指针 */
    int f_inode_num; // 打开文件的磁盘inode号
    int		f_offset;			/* 文件读写位置指针 */
    int f_flag;//文件的标志
    int f_count;//进程引用文件计数
    string f_name;

};

class OpenFileTable{
public:
    OpenFileTable();
    ~OpenFileTable();
public:
    static const int OPEN_FILE_NUM = 100;
    File o_files[OPEN_FILE_NUM];

    int allocFile();
    int freeFile(int fd);
};


#endif //MYFILESYSTEM_INODE_H
