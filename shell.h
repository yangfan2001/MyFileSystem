//
// Created by yang2001 on 2022/5/17.
//

#ifndef MYFILESYSTEM_SHELL_H
#define MYFILESYSTEM_SHELL_H

# include <iostream>

class shell{
public:
    void loop();
    bool shellReact(std::string cmd);
};

#endif //MYFILESYSTEM_SHELL_H
