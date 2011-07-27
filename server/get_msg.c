/********************************************************************
文    件:    get_msg.c
功    能:    得到用户文件中的数据并插入到链表中
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "debug.h"

struct user_link *user_head = NULL;

/*********************************************************************
函    数:    insert_link
功    能:    将用户信息做成一个链表
传入参数:    temp:需要插入到链表中的节点
传出参数：   无
返    回:    0:程序正常执行的结果
*********************************************************************/
int insert_link(struct user_link *temp)
{
    if (user_head == NULL)    /*如果链表为空直接放入链表*/
    {
        user_head = temp;
	return;
    }

    temp->next = user_head;   /*头插法建立链表*/
    user_head = temp;

    return 0;
}

/*********************************************************************
函    数:    insert_online
功能描述:    将登录的用户插入到用户链表中
传入参数:    load_user:要插入到用户链表中的用户
传出参数:    无
返    回:    0:程序执行结果
*********************************************************************/
int insert_online(struct user_link *load_user)
{
     if (online_head == NULL)
     {
         online_head = load_user;
	 return;
     }

     load_user->next = online_head;
     online_head = load_user;

     return 0;
}

/********************************************************************
函    数:    traverse
功    能:    对用户链表遍历
传入参数:    temp:用来存放链表头节点
传出参数：   无
返    回:    无
********************************************************************/
int traverse(struct user_link *head)
{
    struct user_link *stu = NULL;
    fputs("用户信息如下\n", stderr);

    for (stu = head; stu != NULL; stu = stu->next)
    {
        fprintf(stderr, "name: %spassword: %s",stu->user_msg.name, stu->user_msg.password);
    }
    
    fputs("查看用户信息完毕\n", stderr);
    return 0;
}


/********************************************************************
函    数:    get_msg
功    能:    得到用户文件里的用户信息并做成链表
传入参数：   无
传出参数：   无
返    回:    0: 程序执行完毕结果
********************************************************************/
int get_msg()
{   
    struct user_link *temp = (struct user_link *)malloc(sizeof(struct user_link));

    int fd_open = open("user_msg.txt", O_RDONLY);   /*打开用户信息文件*/
    
    if (fd_open == -1)
    {
        perror("open");
	return;
    }
    
    int fd_read = 0;

    /*循环将文件中的信息读出来并存放到用户信息链表中*/
    while ((fd_read = read(fd_open, &temp->user_msg, sizeof(struct user))) > 0)
    {   
	temp->next = NULL;
	temp->confd = -1;
        insert_link(temp);
        temp = (struct user_link *)malloc(sizeof(struct user_link));
    }

    if (fd_read == -1)
    {
        perror("read");
	return;
    }
 
    close(fd_open);
    return 0;
}


/***************************************************************************
函    数:    check_user
功    能:    查找用户
传入参数:    name: 需要查找的用户的名字
传出参数:    无
返    回:    1:在链表中找到用户
             0:在链表中没有找到用户
****************************************************************************/
int check_user(char *name)
{   
    struct user_link *temp = user_head;

    if (user_head == NULL)     /*如果链表为空则返回没用用户*/
    {
        return 0;
    }

    for (temp = user_head; temp != NULL; temp = temp->next)   /*循环检测*/
    {
        if (strcmp(name,temp->user_msg.name) == 0)
	{
	    return 1;
	}
    }

    return 0;
}

/***************************************************************************
函    数:    check_user
功    能:    查找用户
传入参数:    name: 需要查找的用户的名字
传出参数:    无
返    回:    1:在链表中找到用户
             0:在链表中没有找到用户
****************************************************************************/
int check_online(char *name)
{   
    struct user_link *temp = online_head;

    if (online_head == NULL)     /*如果链表为空则返回没用用户*/
    {
        return 0;
    }

    for (temp = online_head; temp != NULL; temp = temp->next)   /*循环检测*/
    {
        if (strcmp(name,temp->user_msg.name) == 0)
	{
	    return 1;
	}
    }

    return 0;
}

/*************************************************************************
函    数:    free_all
功    能:    释放所有节点
传入参数:    head:需要释放的链表的头节点
传出参数:    无
返    回:    0：程序执行结果
************************************************************************/
int free_all()
{
    struct user_link *temp = NULL;
     
    while (user_head != NULL)
    {
        temp = user_head;
	user_head = user_head->next;
	free(temp);
    }

    return 0;
}


/**************************************************************************
函    数:    find_user
功    能:    在用户链表中找到用户
传入参数:    user_name:需要查找的用户的头指针
传出参数:    无
返    回:    temp_user:找到用户
             0:在链表中没有找到用户
**************************************************************************/
struct user_link *find_online(char *user_name)
{
    struct user_link *temp_user = NULL;
    
    /*循环检测链表中是否有该用户如果有的话返回该用户的节点地址*/
    for (temp_user = online_head; temp_user != NULL; temp_user = temp_user->next)
    {
        if (strcmp(temp_user->user_msg.name, user_name) == 0)
	{   
	    debug_msg("find_online:测试中找到用户\n");
	    return temp_user;
	}
    }

    return NULL;
}

/**************************************************************************
函    数:    check_password
功    能:    检测用户和密码是否匹配
传入参数:    temp_user:需要被检测的用户信息
传出参数:    无
返    回:    1: 用户名和密码匹配
             0: 用户名和密码不匹配
***************************************************************************/
int check_password(struct user *temp_user)
{   
    struct user_link *temp = user_head;

    for (temp; temp != NULL; temp = temp->next)
    {
        /*判断密码和账户是否匹配*/
        if ((strncmp(temp->user_msg.name, temp_user->name, strlen(temp->user_msg.name)) 
	   || strncmp(temp->user_msg.password, temp_user->password, strlen(temp->user_msg.password))) == 0)
	{
	    return 1;    /*匹配返回1*/
	}
    }

    return 0;
    
}

/*************************************************************************
函    数:    user_node
功    能:    做一个用户节点
传入参数:    usr_msg:需要做成节点的用户信息
             confd  :用户与服务器通信的端口号
传出参数:    无
返    回:    temp_user做好的节点首地址
*************************************************************************/
struct user_link *user_node(struct user *user_msg, int confd)
{
    struct user_link *temp = (struct user_link *)malloc(sizeof(struct user_link));

    strcpy(temp->user_msg.name, user_msg->name);
    strcpy(temp->user_msg.password, user_msg->password);
    temp->confd = confd;
    temp->speak_flag = 1;       
    temp->next = NULL;

    return temp;
}

/***************************************************************************
函    数:    delete_online
功    能:    当用户下线的时候删除在线节点
传入参数:    user_node:需要被删除的节点
传出参数:    无
返    回:    0:程序成功执行结果
***************************************************************************/
int delete_online(struct user_link *user_node)
{
    struct user_link *temp = NULL;

    if (user_node == NULL)
    {
        return 1;
    }

    /*如果删除的节点是第一个节点*/
    if (user_node == online_head)      /*此处因名字写错调一个上午*/
    {   
        debug_msg("测试中删除第一个节点\n");
        online_head = online_head->next;
	return 1;
    }
    
    /*循环查找用户传入的用户节点*/
    for (temp = online_head; temp != NULL; temp = temp->next)
    {
        if (temp->next == user_node)
	{
	    debug_msg("delete_online temp:name:%s", temp->next->user_msg.name);
	    debug_msg("delete_online name:%s", user_node->user_msg.name);
	    temp->next = user_node->next;
            debug_msg("测试中删除节点\n");
	    return 0;
	}
    }

    return 1;
}

/***********************************************************
函    数:    find_by_confd
功    能:    通过通信端口找到用户
传入参数:    confd: 通信端口
传出参数:    无
返    回:    
***********************************************************/
struct user_link *find_by_confd(int confd)
{
    struct user_link *temp_user = NULL;
    
    /*循环检测链表中是否有该用户如果有的话返回该用户的节点地址*/
    for (temp_user = online_head; temp_user != NULL; temp_user = temp_user->next)
    {
        if (temp_user->confd == confd)
	{   
	    debug_msg("find_by_confd:测试中找到用户\n");
	    debug_msg("find_by_confd:%s%d\n", temp_user->user_msg.name, temp_user->confd);
	    return temp_user;
	}
    }

    return NULL;
}

/***********************************************************************
函    数:    kick_all
功    能:    踢出所有在线用户
传入参数:    
传出参数:
返    回:    0:程序争取执行结果
***********************************************************************/
int kick_all()
{
    struct user_link *temp = NULL;

    for (temp = online_head; temp != NULL; temp = temp->next)
    {
        delete_online(temp);
    }

    return 0;
}
