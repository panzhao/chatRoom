/********************************************************************
文    件:    get_msg.c
功    能:    得到用户文件中的数据并插入到链表中
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "debug.h"

struct store_msg *history_head = NULL;

/*********************************************************************
函    数:    insert_link
功    能:    将用户信息做成一个链表
传入参数:    temp:需要插入到链表中的节点
传出参数：   无
返    回:    0:程序正常执行的结果
*********************************************************************/
int insert_link(struct store_msg *temp)
{
    if (history_head == NULL)    /*如果链表为空直接放入链表*/
    {
        history_head = temp;
	return;
    }

    temp->next = history_head;   /*头插法建立链表*/
    history_head = temp;

    return 0;
}


/********************************************************************
函    数:    traverse
功    能:    对用户链表遍历
传入参数:    temp:用来存放链表头节点
传出参数：   无
返    回:    无
********************************************************************/
int traverse(struct store_msg *head)
{
    struct store_msg *stu = NULL;
    fputs("用户信息如下\n", stderr);

    for (stu = head; stu != NULL; stu = stu->next)
    {
	fprintf(stderr, "name: %s Msg: %s",stu->history.m_name, stu->history.msg);
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
    struct store_msg *temp = (struct store_msg *)malloc(sizeof(struct store_msg));

    int fd_open = open("chat_msg.txt", O_RDONLY);   /*打开用户信息文件*/
    
    if (fd_open == -1)
    {
        perror("open");
	return;
    }
    
    int fd_read = 0;

    /*循环将文件中的信息读出来并存放到用户信息链表中*/
    while ((fd_read = read(fd_open, &temp->history, sizeof(struct chat))) > 0)
    {   
	temp->next = NULL;
        insert_link(temp);
	debug_msg("debug, get_msg m_name%s", temp->history.m_name);
	debug_msg("debug, get_msg f_name%s", temp->history.f_name);
	debug_msg("debug, get_msg msg%s", temp->history.msg);
        temp = (struct store_msg *)malloc(sizeof(struct store_msg));
    }

    if (fd_read == -1)
    {
        perror("read");
	return;
    }

    return 0;
}

/*****************************************************************
函    数:    free_all
功    能:    得到用户的聊天记录
传入参数:    user_name:用户姓名
传出参数:    无
返    回:    0
*****************************************************************/
int free_all()
{
    struct store_msg *temp = history_head;
    
    while (temp != NULL)
    {
        history_head = history_head->next;
	free(temp);
	temp = history_head;
    }

    return 0;
}
