#include <iostream>
#include <fstream>

# include "file_system.h"
# include "file_manager.h"
# include "shell.h"
# include "user.h"
extern Directory present_directory;

using namespace std;

int main()
{

    FileManager fm;
    fm.formatSystem();

    shell my_shell;
    my_shell.loop();

    //IOManager im;
    //im.readFile("../1.txt",1000000);

    Inode inode;

    cout<<inode.i_mode<<endl;

    inode.i_mode |= Inode::ELSE_E;

    cout<<(inode.i_mode&Inode::ELSE_E)<<endl;

    inode.i_mode |= Inode::ELSE_R;

    cout<<(inode.i_mode&Inode::ELSE_R)<<endl;

    inode.i_mode|= Inode::OWNER_R;

    cout<<(inode.i_mode&Inode::OWNER_R)<<endl;

    cout<<inode.i_mode<<endl;

    cout<<(inode.i_permission|(Inode::ELSE_E|Inode::ELSE_R|Inode::OWNER_R))<<endl;

    return 0;
}
