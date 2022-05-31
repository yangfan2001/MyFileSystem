#include <iostream>
#include <fstream>

# include "file_system.h"
# include "file_manager.h"
# include "shell.h"
extern Directory present_directory;

using namespace std;

int main()
{

    FileManager fm;
    fm.formatSystem();

    shell my_shell;
    my_shell.loop();

    IOManager im;
    im.readFile("../1.txt",1000000);
    cout<<min(1,2)<<endl;
    return 0;
}
