cmake_minimum_required(VERSION 3.22)
project(MyFileSystem)
set(CMAKE_CXX_STANDARD 14)
add_executable(MyFileSystem main.cpp head.h shell.cpp Inode.cpp io_manager.cpp io_manager.h Inode.h shell.h file_system.cpp file_system.h file_manager.h file_manager.cpp user.h user.cpp)
