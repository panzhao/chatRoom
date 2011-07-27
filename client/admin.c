/**********************************************************
文    件:    admin.c
功    能:    超级用户模块
函数列表:
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "client.h"
#include "wrap.h"
#include "debug.h"

extern int sockfd;
extern char my_name[20];

/**********************************************************
函    数:    manage_speak
功    能:    管理客户是否能够发言的函数
传入参数:    tran_msg:用户的信息包
传出参数:    无
返    回:    0:程序正确执行结果
             -1：程序错误执行结果
**********************************************************/
int manage_speak(struct chat *tran_msg)
{   
   int n_write;
 
   debug_msg("man_sp: %s", my_name);
   
   fputs("Please give the client that you want to manage: ", stderr);
   fgets(tran_msg->f_name, sizeof(tran_msg->f_name), stdin);

   n_write = Write(sockfd, tran_msg, sizeof(struct chat));
   if (n_write <= 0)
   {
       fputs("Write failure!\n", stderr);
       close(sockfd);
       return -1;
   }
    
   return 0;
}

/***************************************************************
函    数:    kick_one
功    能:    踢出某一个用户
传入参数:    tran_msg:需要处理的数据包
传出参数:    无
返    回:    0:程序正确执行结果
             -1:程序错误执行结果
****************************************************************/
int kick_one(struct chat *tran_msg)
{    
     int n_write;
     
     /*加上发送者名字*/
     strcpy(tran_msg->m_name, my_name);
     
     if (strcmp(my_name, "admin\n") != 0)
     {
         fputs("You are not administrator, can't user this function!\n", stderr);
	 return 0;
     }
     
     /*得到需要踢出的用户名字*/
     fputs("Please input the user's name:", stderr);
     fgets(tran_msg->f_name, sizeof(tran_msg->f_name), stdin);

     n_write = Write(sockfd, tran_msg, sizeof(struct chat));
     if (n_write <= 0)
     {
         fputs("kick_one:Write faliure!\n", stderr);
	 close(sockfd);
	 return -1;
     }

     return 0;
}
