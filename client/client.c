/***********************************************************************
文    件:    client.c
功    能:    聊天室用的客户端
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include "client.h"
#include "wrap.h"
#include "debug.h"
 
int sockfd = 0;         /*用来存放客户端的通信端口号*/
char my_name[20];       /*用来存放我的名字用来在发送的名字上加上发送者姓名*/

pthread_mutex_t  write_lock = PTHREAD_MUTEX_INITIALIZER;    /*写入文件时候的锁*/

/***********************************************************************
函    数:    client
功能描述:    聊天室的客户端
传入参数:    无
传出参数:    无
返    回:
***********************************************************************/
int client()
{
    struct sockaddr_in servaddr;
    pthread_t thread;
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);    /*获得socket描述符*/
    
    bzero(&servaddr, sizeof(servaddr));          /*将地址结构体清零*/
    servaddr.sin_family = AF_INET;               /*初始化网络协议*/
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr); /*初始化IP地址*/
    servaddr.sin_port = htons(SERV_PORT);                /*初始化服务器端口*/
    
    /*连接到服务器端口*/
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    /*调用处理客户端函数*/
    choice();
}

/******************************************************************
函    数:    choice
功    能:    选择进入客户端时候客户的需求
传入参数:    无
传出参数:    无
返    值:    0：程序执行结果
             -1:程序错误执行结果
******************************************************************/
int choice()
{   
    /*定义临时信息缓存区*/
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    
    struct choice tran_msg;                
    memset(&tran_msg, 0, sizeof(tran_msg));
    
    int n_write;
    int n_read;
    
    while (1)
    {    
        /*输出选择格式*/
        print_in();   
        
	fputs("Please give your choice:", stderr);         /*给出用户选择*/
	fgets(tran_msg.mode, sizeof(tran_msg.mode), stdin); /*从标准输入得到用户选择*/

	if (strlen(tran_msg.mode) != 2)
	{
	    fputs("error: Please give your choice again\n", stderr);
	    continue;
	}

        switch(tran_msg.mode[0])
	{
	    case REGISTER:
	    {   
	        init_msg(&tran_msg.user_msg);     /*初始化用户信息*/

	        /*将用户信息发送给服务器*/
	        n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                if (n_write <= 0)
	        {
	            fputs("choice-Write error\n", stderr);
	            close(sockfd);
	            return -1;
	        }
	        
		/*读取服务器反馈信息*/
		n_read = Read(sockfd, buf, MAXSIZE);
		if (n_read <= 0)
		{
		    fputs("choice reg: read error", stderr);
		    close(sockfd);
		    return -1;
		}

		fputs(buf, stderr);
		break;
	    }

	    case LOAD:
	    {   
	        /*得到用户名和密码*/
	        init_msg(&tran_msg.user_msg);
                
		/*将登录成功的名字保存下来*/
		strcpy(my_name, tran_msg.user_msg.name);

	        /*将用户信息发送给服务器*/
	        n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                if (n_write <= 0)
	        {
	            fputs("choice-Write error\n", stderr);
	            close(sockfd);
	            return -1;
	        }
	        
		/*得到服务器反馈信息*/
		n_read = Read(sockfd, buf, MAXSIZE);
		if (n_read <= 0)
		{
		    fputs("choice reg: read error", stderr);
		    close(sockfd);
		    return -1;
		}

		fputs(buf, stderr);
		if (strcmp("The user have loaded, can't reload\n", buf) == 0)
		{
		   debug_msg("reload\n");
		   break;
         	}

                if (strcmp("load faliure: Your password can't match to your name\n", buf) == 0)
		{
		    debug_msg("not match\n");
		    break;
		}
		
		/*调用聊天模式管理函数*/
		manage_chat();
		
		break;
	    }

	    case QUIT:
	    {   
	        /*将选择告诉服务器*/
		n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                if (n_write <= 0)
	        {
	            fputs("choice-Write error\n", stderr);
	            close(sockfd);
	            return -1;
		}

                /*从服务器读取返回结果*/
		n_read = Read(sockfd, buf, MAXSIZE);
		if (n_read <= 0)
		{
		    fputs("choice reg: read error", stderr);
		    close(sockfd);
		    return -1;
		}
	        
		fputs(buf, stderr);
		close(sockfd);
	        return 0;
	    }

	    default:
	    {
	        fputs("Your choice is error, plese give your choice again\n", stderr);
		break;
	    }
	}

        memset(buf, 0, MAXSIZE);
        memset(&tran_msg, 0, sizeof(tran_msg));
    }

}

/*******************************************************************
函    数:    manage_chat
功    能:    处理和服务器的数据链接
传入参数:    无
传出参数:    无
返    回:    0:程序正常退出
             1:程序错误退出
********************************************************************/
int manage_chat()
{   
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    
    int n_read;
    int n_write;
   
    pthread_t thread;     /*用来存放用户线程号*/
    
    /*开启接收服务器信息线程*/
    int res_thread =  pthread_create(&thread, 0, read_serv, (void *) 0);
    if(res_thread != 0)
    {
       fputs("creat thread faliure:\n", stderr);
       return -1;
    }

    while (1)
    {    
         print_chat();
	 
         fgets(tran_msg.mode, sizeof(tran_msg.mode), stdin);   /*得到用户聊天模式请求*/
         
	 if (strlen(tran_msg.mode) != 2)
	 {
	     fputs("Your choice does not conform to rules\n", stderr);
	     continue;
	 }

	 switch(tran_msg.mode[0])
	 {
	     case PRIVATE:  /*私聊*/
	     {   
	         my_private(&tran_msg);
	         break;
	     }

	     case PUBLIC:   /*群聊*/
	     {   
	         my_public(&tran_msg);
	         break;  
	     }
             
	     case SCANF_HISTORY:  /*查看聊天记录*/
	     {   
	         scan_history();         
	         break;
	     }
	     
	     case SCANF_ONLINE:  /*查看在线用户*/
	     {   
	         n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                 if (n_write <= 0)
                 {
                     fputs("manage_chat.quit_chat: write failure\n", stderr);
	             close(sockfd);
	             return -1;
                 }
                 
		 debug_msg("scanf_online, tran_msg->mode %s", tran_msg.mode);
	         break;
	     }
	     
	     case QUIT_CHAT:   /*退出*/
	     {    
	         strcpy(tran_msg.m_name, my_name);
	         n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                 if (n_write <= 0)
                 {
                     fputs("manage_chat.quit_chat: write failure\n", stderr);
	             close(sockfd);
	             return -1;
                 }
		 
		 pthread_join(thread, NULL);
                 return 0;
	     }

	     case KICK_ALL:          /*超级用户功能*/
	     {    
	          if (strcmp("admin\n", my_name) != 0)
		  {
		       fputs("You are not administrator! Can't use this function!\n", stderr);
		       break;
		  }

	          strcpy(tran_msg.m_name, my_name);
	          
		  n_write = Write(sockfd, &tran_msg, sizeof(tran_msg));
                  if (n_write <= 0)
                  {
                      fputs("manage_chat.quit_chat: write failure\n", stderr);
	              close(sockfd);
	              return -1;
                  }

		  break;
	     }

	     case MANAGE_SPEAK:        /*管理是否禁言*/
	     {   
		 strcpy(tran_msg.m_name, my_name);
	         if (strcmp("admin\n", my_name) != 0)
		 {
		     fputs("You are not administrator! Can't use this function!\n", stderr);
		     break;
		 }
                 
		 debug_msg("d:spe- %s", tran_msg.mode);
		 manage_speak(&tran_msg);
		 
		 break;
	     }

	     case KICK_ONE:           /*踢出某个确定的用户*/
	     {
	         kick_one(&tran_msg);
		 break;
	     }

	     default:     /*输出错误情况*/
	     {
	         fputs("default:输入有误, 请重新选择\n", stderr);
		 break;
	     }
	 }
	 
         memset(&tran_msg, 0, sizeof(tran_msg));
    }
}

/******************************************************************************
函    数:    read_serv
功    能:    监听服务器反馈的信息
传入参数:    无
传出参数:    无
返    回:    
******************************************************************************/
void *read_serv(void *arg)
{   
    /*存放用户临时聊天信息的缓存区*/
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));

    int fd_open;  /*存放文件描述符*/
    int fd_write; /*写如文件时的返回值*/

    while (1)
    { 
         /*读取服务起返回的信息*/
         int n_read = Read(sockfd, &tran_msg, sizeof(tran_msg));
         if (n_read <= 0)
         { 
	     fputs("read_serv:read faliure\n", stderr);
	     close(sockfd);
	     pthread_exit ((void *)-1);
         }
         
	 fputs("Recieve msg from:", stderr);
	 fputs(tran_msg.m_name, stderr);
	 fputs("Msg:", stderr);
	 fputs(tran_msg.msg, stderr);

	 fd_open = open("chat_msg.txt", O_RDWR | O_APPEND);
	 if (fd_open < 0)
	 {
	     perror("open chat_msg");
	     return ((void *)-1);
	 }
         
	 pthread_mutex_lock(&write_lock);    /*写数据时候上锁*/

	 fd_write = write(fd_open, &tran_msg, sizeof(tran_msg));
	 if (fd_write <= 0)
	 {
	     perror("read_serv: write faliure");
             return ((void *)-1);
	 }
         
	 close(fd_open);
	 
	 pthread_mutex_unlock(&write_lock);    /*解锁*/
	 
	 /*如果用户要退出登录模式关闭监听服务器线程*/
	 if (strcmp("您已经成功退出聊天模式\n", tran_msg.msg) == 0)
	 {
	     pthread_exit((void *) 0);
	 }
	 
	 if (strcmp("You have been forbide to downline by administrator!\n", tran_msg.msg) == 0)
	 {
	     exit(0);
	 }
	 
	 if (strcmp("Server have been closed, You are forced refferals!\n", tran_msg.msg) == 0)
	 {
	     exit(0);
	 }
         
         memset(&tran_msg, 0, sizeof(tran_msg));
     }
}

/**********************************************************************
函    数:    private
功    能:    客户端私聊处理函数 
传入参数:    无
传出参数:    无
返    回:    0：程序正确执行结果
***********************************************************************/
int my_private(struct chat *temp)
{
     int n_write; 

     strcpy(temp->m_name, my_name);                            /*加上发送者名字*/
     
     fputs("Friend name:", stderr);
     fgets(temp->f_name, sizeof(temp->f_name), stdin);         /*得到想联系的用户名字*/
     
     fputs("I said:", stderr);
     fgets(temp->msg, sizeof(temp->msg), stdin);               /*从标准输入得到聊天信息*/
     
     /*将聊天记录写入到文件中*/
     //pthread_mutex_lock(&write_lock);
     
     /*将接收到的信息写入文件*/
     //write_in(temp);

     //pthread_mutex_unlock(&write_lock);

     n_write = Write(sockfd, temp, sizeof(struct chat));     /*将用户的名字写入到网络通信端口*/
     if (n_write <= 0)
     {
         fputs("private：write faliure\n", stderr);
	 close(sockfd);
         return -1;
     }

     return 0;
}

/*****************************************************************************
函    数:    my_public
功    能:    处理用户的群聊要求
传出参数:    无
传出参数:    无
返    回:    0：程序正确执行结果
******************************************************************************/
int my_public(struct chat *temp)
{    
     int n_write;
     
     strcpy(temp->m_name, my_name);                  /*加入发送者姓名*/     
     
     fputs("I said:", stderr);
     fgets(temp->msg, sizeof(temp->msg), stdin);      /*从标准输入得到聊天内容*/
      
     debug_msg("d: my_pub temp->m_name:%s\n", temp->m_name);
     debug_msg("d: my_pub temp->mode:%s", temp->mode);
     
     /*将聊天记录写入到文件中*/
     //pthread_mutex_lock(&write_lock);

     //write_in(temp);
 
     //pthread_mutex_unlock(&write_lock);
     
     /*将得到的聊天内容写入到通信端口*/
     n_write = Write(sockfd, temp, sizeof(struct chat));
     if (n_write <= 0)
     {
	 fputs("private: Write faliure\n", stderr);
	 close(sockfd);
	 return -1;
     }
     
     return 0;
}

/**************************************************************************
函    数:    scan_history
功    能:    查看用户聊天记录
传入参数:    无
传出参数:    无
返    回:   
**************************************************************************/
int scan_history()
{
     get_msg();                           /*将聊天记录做成链表*/

     struct store_msg *temp = NULL;
     
     fputs("Begin to view chat log!\n", stderr);

     for (temp = history_head; temp != NULL; temp = temp->next)
     {
         if (strcmp(temp->history.m_name, my_name) == 0)
	 {
	     fputs("I said\n", stderr);
	     fputs(temp->history.msg, stderr);
	     debug_msg("f聊天记录\n");
	 }

	 if (strcmp(temp->history.f_name, my_name) == 0)
	 {
	     debug_msg("查看聊天记录");
	     fputs(temp->history.m_name, stderr);
	     fputs(temp->history.msg, stderr);
	 }
     }
     
     free_all();  /*释放所有的节点*/
     fputs("View chat log over!\n", stderr);
     return 0;
}

