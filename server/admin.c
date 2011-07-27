/**************************************************************
文    件:    admin.c
功    能:    管理员的功能
函数列表:
***************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "server.h"
#include "wrap.h"
#include "debug.h"

/**************************************************************
函    数:    manage_speak
功    能:    管理用户是否能发言
传入参数:    tran_msg:客户端法国来的数据包
             confd   :服务器与客户端通信的端口
传出参数:    无
返    回:    0: 程序正确执行结果
             -1:程序错误执行结果
**************************************************************/
int manage_speak(struct chat *tran_msg, int confd)
{ 
    int n_write;

    struct user_link *temp = NULL;

    /*找到用户并设置禁言*/
    temp = find_online(tran_msg->f_name);
    if (temp == NULL)
    {
        strcpy(tran_msg->m_name, "Server\n");
        strcpy(tran_msg->msg, "The people do not exist!\n");
        
	/*将用户信息写入网络端口*/
        n_write = Write(confd, tran_msg, sizeof(struct chat));
        if (n_write <= 0)
        {   
	    fputs("manage_speak:Write faliure\n", stderr);
	    temp = find_by_confd(confd);
            delete_online(temp);
	    notice(tran_msg->m_name, DOWNLINE);
     	    close(confd);
	    return -1;
        }
 
        return 0;
    }

    debug_msg("admin:f_name %s", temp->user_msg.name);
    temp->speak_flag = 0;
    
    strcpy(tran_msg->m_name, "Server\n");
    strcpy(tran_msg->msg, "You have been forbideed speaking by administrator!\n");
    
    n_write = Write(temp->confd, tran_msg, sizeof(struct chat));
    if (n_write <= 0)
    {
	fputs("manage_speak:Write faliure\n", stderr);
        delete_online(temp);
	close(temp->confd);
	notice(tran_msg->m_name, DOWNLINE);
	return -1;
    }
    
    memset(tran_msg, 0, sizeof(struct chat));
     
    strcpy(tran_msg->m_name, "Server\n");
    strcpy(tran_msg->msg, "Set can't speak successful!\n");

    n_write = Write(confd, tran_msg, sizeof(struct chat));
    if (n_write <= 0)
    {   
        temp = find_by_confd(confd);
        delete_online(temp);
     	close(confd);
	notice(tran_msg->m_name, DOWNLINE);
	return -1;
    }

    return 0;
}

/******************************************************************
函    数:    kick_one
功    能:    踢出某一个用户
传入参数:    tran_msg:客户端发过来的数据
             confd   :服务器和客户端的通信端口 
传出参数:    无
返回值  :    0 :程序正确执行结果
             -1:程序错误执行结果
******************************************************************/
int kick_one(struct chat *tran_msg, int confd)
{
    int n_write;
    
    struct user_link *temp = NULL;
    temp = find_online(tran_msg->f_name);   /*找到用户节点*/
    
    debug_msg("进入kick_one\n");

    /*判断需要操作的用户是否存在*/
    if (temp == NULL)
    {
        strcpy(tran_msg->m_name, "Server\n");
        strcpy(tran_msg->msg, "The people do not exist!\n");
    
        n_write = Write(confd, tran_msg, sizeof(struct chat));
        if (n_write <= 0)
        {   
	    fputs("kick_one:Write faliure\n", stderr);
	    temp = find_by_confd(confd);
            delete_online(temp);
     	    close(confd);
	    notice(tran_msg->m_name, DOWNLINE);
	    return -1;
        }

	return 0;
    }	
    
    if (strcmp(temp->user_msg.name, "admin\n") == 0)
    {
        strcpy(tran_msg->m_name, "Server\n");
        strcpy(tran_msg->msg, "The people you whant kick is administrtor, you can't kick yourself!\n");
    
        n_write = Write(confd, tran_msg, sizeof(struct chat));
        if (n_write <= 0)
        {   
	    fputs("kick_one:Write faliure\n", stderr);
	    temp = find_by_confd(confd);
            delete_online(temp);
     	    close(confd);
	    notice(tran_msg->m_name, DOWNLINE);
	    return -1;
        }
        
	return 0;
    }
    
    strcpy(tran_msg->m_name, "Server\n");
    strcpy(tran_msg->msg, "You have been forbide to downline by administrator!\n");
    
    n_write = Write(temp->confd, tran_msg, sizeof(struct chat));
    if (n_write <= 0)
    {   
	fputs("kick_one:Write faliure\n", stderr);
        temp = find_by_confd(temp->confd);
        delete_online(temp);
	close(temp->confd);
	notice(temp->user_msg.name, DOWNLINE);
	return -1;
    }
    
    close(temp->confd);    /*关闭该用户的通信端口*/
    debug_msg("debug ,admin %d\n", temp->confd);

    memset(tran_msg, 0, sizeof(struct chat));
    
    strcpy(tran_msg->m_name, "Server\n");
    strcpy(tran_msg->msg, "Kick successful!\n");
    
    n_write = Write(confd, tran_msg, sizeof(struct chat));
    if (n_write <= 0)
    {   
        temp = find_by_confd(confd);
        delete_online(temp);
     	close(confd);
	notice(tran_msg->m_name, DOWNLINE);
	return -1;
    }

    return 0;
}

/******************************************************************
函    数:    notice_online
功    能:    当有用户上线的时候通知各个在线用户有人上线
传入参数:    tran_msg:用来传送信息的结构体
传出参数:    无
返    回:    0:程序正确执行结果
             -1:程序错误执行结果
******************************************************************/
int notice(char *name, int choice)
{   
    struct user_link *temp = NULL;          /*链表节点临时指针*/
    struct user_link *temp_online = NULL;   /*在线链表查找临时指针*/
    
    /*传输消息缓存区*/
    struct chat tran_msg;               
    memset(&tran_msg, 0, sizeof(struct chat));
    
    
    int n_write;
    
    /*用来存放编辑时候发送消息的内容*/
    char temp_msg[256];
    memset(temp_msg, 0, 256);
     
    /*将被操作的用户的名字减1后放入消息缓存区中*/
    strncpy(temp_msg, name, (strlen(name) - 1));

    debug_msg("len_name, %d\n", strlen(name) - 1);
    
    /*上线通知*/
    if (choice == ONLINE)
    {
        strcat(temp_msg, " online!\n");
    }

    /*下线通知*/
    if (choice == DOWNLINE)
    {
        strcat(temp_msg, " has downline!\n");
    }

    debug_msg("debug,notice_up name:%s, len:%d", temp_msg, strlen(temp_msg));

    memset(&tran_msg, 0, sizeof(struct chat));
    
    strcpy(tran_msg.m_name, "Server\n");
    strcpy(tran_msg.msg, temp_msg);
   
    /*循环将消息发送给客户*/
    for (temp = online_head; temp != NULL; temp = temp->next)
    {
        n_write = Write(temp->confd, &tran_msg, sizeof(struct chat));
	if (n_write <= 0)
	{
	    fputs("notice_online: write faliure!", stderr);
            temp_online = find_by_confd(temp->confd);
	    delete_online(temp_online);
	    close(temp_online->confd);
	}
    }
     
    return 0;
}

/********************************************************************
函    数:    notice_close_serv
功    能:    通知各个客户端服务器将要关闭的消息
传入参数:    confd:客户端与服务器通信的端口
传出参数:    无
返    回:    0:程序正确执行结果
             -1:程序错误执行结果
*********************************************************************/
int notice_close_serv(int confd)
{
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));

    int n_write;
    
    struct user_link *temp = NULL;

    strcpy(tran_msg.m_name, "Server\n");
    strcpy(tran_msg.msg, "Server will be close after 10 seconds, Please downline as soon as possible!\n");

    for (temp = online_head; temp != NULL; temp = temp->next)
    {
        n_write = Write(temp->confd, &tran_msg, sizeof(tran_msg));
	if (n_write <= 0)
	{
	    fputs("notice_close_serv:Write faliure!\n", stderr);
	    close(temp->confd);
	}
    }
    
    return 0;
}
