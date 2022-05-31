# MyFileSystem
Rage的文件系统
文件系统的指令清单
### 系统类型

**fformat**

输入：fformat

输出：无

描述：对文件系统的文件卷进行格式化。

### 文件类型

**fopen**

输入：fopen [file_name] [mode]    mode = [-r,-w,-rw]   只读、只写、读写

输出：如果打开成功，输出文件描述符fd。否则输出错误提示信息。

描述：以 mode 的方式打开当前目录下为file_name的文件，用户得到对应的文件描述符fd。

**fread**

输入：fread [fd] [-o] [file_name] [length] 

输出：如果读失败，输出错误信息。读成功的情况下，输出成功读到的字节数。根据用户的选择，输出读取的信息。

描述：通过文件描述符fd，向其指向的打开文件结构读取length个字节。其中如果用户输入为[-o file_name]，那么指定输出到文件名为file_name的文件中。否则通过Cout输出到屏幕上。

**fwrite**

输入：fwrite [fd] [file_name] [length]

输出：输出成功写入的字节数。

描述：根据文件描述符fd，向其指向的打开文件结构写入length个字节，其中这length个字节的内容从文件名为file_name的文件中读取。

**fseek**

输入：fseek  [fd] [offset] [ptrname]  ptrname = 0(文件起始位置) 1(文件指针当前位置) 2(文件结束位置)

输出：fseek操作的结果。

描述：移动文件指针，根据ptrname选择将指针从对应的位置移动offset个字节，其中offset可以为负数。如果ptrname为0，那么从文件起始位置移动offset个字节，如果ptrname为1，那么从文件读写指针指向的当前位置移动offset个字节，如果ptrname为2，从文件结束为止移动offset个字节

**fcreate**

输入：fcreat [file_name]

输出：如果输入存在错误或系统存在错误，会进行错误提示。

描述：在当前目录下以当前用户的方式创建文件名为file_name的文件。

**fdelete**

输入：fdelete [file_name]

输出：如果输入存在错误或系统存在错误，会进行错误提示。

描述：在当前目录下以当前用户的方式删除名为file_name的文件。

### 目录类型

**mkdir**

输入：mkdir [dir_name]

输出：如果输入存在错误或系统存在错误，会进行错误提示。

描述：在当前目录下创建名为dir_name的文件

**cd**

输入：cd [path]

输出：如果输入存在错误或系统存在错误，会进行错误提示。

描述：前往为path的目录。其中如果path为[..]，代表前往父目录，如果path形式为[/a/b]代表绝对路径，如果path形式为[a/b/c]，为相对路径。

**ls**

输入：ls [-l] 

输出：输出当前目录对应的子目录与子文件，根据参数显示详细信息。

描述：显示当前目录的子目录以及文件。输入[-l],显示目录和文件的详细细节。

**pwd**

输入：pwd

输出：输出用户当前所在目录的绝对路径。

描述：获得当前的绝对路径。

### 用户相关

groupadd

useradd

## 
