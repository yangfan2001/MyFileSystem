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

    cout<<"操作系统课程设计 BY 1952651 杨凡"<<endl;
    cout<<"目前系统默认用户root,密码123456"<<endl;

    shell my_shell;
    my_shell.loop();

    //IOManager im;
    //im.readFile("../1.txt",1000000);

    return 0;
}
