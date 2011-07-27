#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "debug.h"

/****************************************************************************
函    数:    init_msg
功    能:    对学生信息进行初始化
传入参数:    无
传出参数:    stu: 经初始化后的结构体
返    回:    stu: 经过初始化的结构体地址
****************************************************************************/
int init_msg(struct user *temp)
{   
    /*对学生信息的初始化*/
    fprintf(stdout, "Please input your name:");
    fgets(temp->name, sizeof(temp->name), stdin);
    fprintf(stdout, "Plese input your password:");
    fgets(temp->password, sizeof(temp->password), stdin);

}


/*******************************************************************************
函    数:    write_in
功    能:    将一个结构体中的信息写入文件
传入参数:    stu:需要写入文件的学生结构地址
传出参数:    无
返    回:    0:检测程序执行结果
*******************************************************************************/
int write_in(struct chat *stu)
{   
    int fd_open = open("chat_msg.txt", O_RDWR | O_APPEND);      /*打开用户信息文件*/
    
    if (fd_open == -1)
    {
        perror("write_in.open");
	return;
    }

    int fd_write = write(fd_open, stu, sizeof(struct chat));     /*将传入的结构提写入到文件中去*/
    
    if (fd_write == -1)
    {
        perror("write_in.write");
	return;
    }
    
    close(fd_open);
    
    return 0;
}

