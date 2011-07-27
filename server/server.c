/***********************************************************************
文    件:    server.c
文件描述:    提供聊天服务功能
***********************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "server.h"
#include "wrap.h"
#include "debug.h"

/*在线用户链表头指针*/
struct user_link *online_head = NULL;

pthread_mutex_t reg_lock  = PTHREAD_MUTEX_INITIALIZER;     /*定义线程注册互斥锁*/
pthread_mutex_t load_lock = PTHREAD_MUTEX_INITIALIZER;     /*定义线程登录互斥锁*/

/***********************************************************
函    数:    server
功    能:    提供聊天服务
传入参数:    无
传出参数:    无
返    回:    无
***********************************************************/
int server()
{
    struct sockaddr_in servaddr;     /*服务器通信地址*/
    struct sockaddr_in cliaddr;      /*与客户端建立连接后的通信地址*/
    
    socklen_t cliaddr_len;           /*存放客户端的通信地址的长度*/
    
    int sockfd;                      /*服务起端口*/
    int confd;                       /*服务器与客户端链接时候的通信端口*/
   
    pthread_t thread;
    get_msg();                       /*加载用户配置文件*/

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    /*给服务器结构体地址初始化*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    /*绑定服务器在上面的服务器地址中*/
    Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 
    
    int opt = 1;
    setsockopt(sockfd, IPPROTO_TCP, SO_REUSEADDR, &opt, sizeof(opt));

    /*设置服务器为监听状态*/
    Listen(sockfd, 20);         

    while(1)
    {
        fputs("准备接受链接...\n", stderr);
        cliaddr_len = sizeof(cliaddr);
        
	/*接受客户端连接请求*/
	confd = Accept(sockfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
	
	/*创建客户管理线程*/
	pthread_create(&thread, 0, manage_client, (void *)confd);
    }
}

/************************************************************************
函    数:    manage_client
功    能:    处理客户端的线程函数
传入参数:    服务器与客户端通信的端口
传出参数:    无
返    回:    0:程序执行结果
***********************************************************************/
void *manage_client(void *arg)
{   
    /*将通信端口强制转换成整形*/
    int confd = (int )arg;
    
    /*调用解析用户要求函数*/
    get_choice(confd);

    pthread_exit((void *)0);
}

/************************************************************************
函    数:    get_choice
功    能:    得到用户请求
传入参数:    confd: 用来与客户端通信的端口 
传出参数:    无
返    回:    0: 程序正确执行
             -1:程序错误执行
************************************************************************/
int get_choice(int confd)
{    
     /*临时信息缓冲区*/
     char buf[MAXSIZE];
     memset(buf, 0, MAXSIZE);
     
     /*用户与服务器通信缓存区*/
     struct choice tran_msg;                
     memset(&tran_msg, 0, sizeof(tran_msg));
     
     int n_write;
     int n_read;

     while(1)
     {   
         n_read = Read(confd, &tran_msg, sizeof(tran_msg));    /*读取客户请求*/
	 if (n_read <= 0)
	 {
	     fputs("get_choice :进入时服务器读取错误\n", stderr);
	     close(confd);
	     return -1; 
	 }

         switch(tran_msg.mode[0])
	 {    
	      case REGISTER:   /*用户注册请求*/
	      {
	          my_register(confd, &tran_msg.user_msg);
		  break;
	      } 

	      case LOAD:       /*用户登录请求*/
	      {    
		  load(confd, &tran_msg.user_msg);
	          break;
	      }

	      case QUIT:       /*用户推出请求*/
	      {
                  strcpy(buf, "You have exited successfully!\n");
   	          
		  n_write = Write(confd, buf, strlen(buf));
	          if (n_write <= 0)
	          {
                      fputs("Write error in my_register\n", stderr);
	              close(confd);
                      return -1;
	          }  
	          
		  close(confd);
		  return 0;  
	      }
         }
         
	 memset(&tran_msg, 0, sizeof(tran_msg));
         memset(buf, 0, MAXSIZE);
    }
}

/******************************************************************************
函    数:    my_register
功    能:    完成用户的注册工作
传入参数:    confd : 服务器和客户端的通信接口
传出参数:    无
返    回:    -1:程序错误返回
              0:程序正确执行
******************************************************************************/
int my_register(int confd, struct user *temp_user)
{
    debug_msg("d: my_reg- %s", temp_user->name);
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    int n_write;

    /*判断用户名是否已经存在*/
    if (check_user(temp_user->name) == 1) /*如果用户存在*/
    {   
        strcpy(buf, "sorry! Register faliure, this user have exist\n");
   	n_write = Write(confd, buf, strlen(buf));
	if (n_write <= 0)
	{
           fputs("Write error in my_register\n", stderr);
	   close(confd);
           return -1;
	}  
    }	      
    else                                   /*用户不存在*/
    {   
        pthread_mutex_lock(&reg_lock);       /*链表和文件操作地方上锁*/

        write_in(temp_user);
	struct user_link *temp = user_node(temp_user, -1);
 	insert_link(temp);
        
	pthread_mutex_unlock(&reg_lock);     /*解锁让其他用户可以注册*/
     
        strcpy(buf, "Register success\n");
        n_write = Write(confd, buf, strlen(buf));
	if (n_write <= 0)
	{
           fputs("Write error in my_register\n", stderr);
	   close(confd);
           return -1;
	}  
    }

    return 0;
}


/************************************************************************
函    数:    load
功    能:    处理客户登录的函数
传入参数:    confd:服务器与客户端通信的端口
传出参数:    无
返    回:    0:程序正确执行
             -1：程序错误执行
************************************************************************/
int load(int confd, struct user *temp_user)
{   
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    int n_write;
    
    debug_msg("查看注册用户\n");
    traverse(user_head);
    debug_msg("查看注册用户完毕\n");

    /*查看是否该用户已经登录*/
    if (check_online(temp_user->name) == 1)
    {
        strcpy(buf, "The user have loaded, can't reload\n");
	
	n_write = Write(confd, buf, strlen(buf));
	if (n_write <= 0)
	{   
	    fputs("load: write error\n", stderr);
	    close(confd);
	    return -1;
        }

	return 0;
    }
    
    /*查看用户名和密码是否匹配*/
    if (check_password(temp_user) == 1)        
    {   
	strcpy(buf, "load success: Welcome to chatroom\n");
        
	n_write = Write(confd, buf, strlen(buf)); /*将服务器处理信息发给客户端*/
	if (n_write <= 0)
	{
	    fputs("load: write faliure\n", stderr);
	    close(confd);
	    return -1;
	}

	pthread_mutex_lock(&load_lock);     /*涉及到用户在线链表的操作的地方上锁*/
        
        struct user_link *temp = user_node(temp_user, confd); /*将登录用户信息做成节点*/
	
        notice(temp_user->name, ONLINE);
	
        insert_online(temp);
	
        debug_msg("登录处\n");
	traverse(online_head);
	debug_msg("插入在线列表成功\n");
	
        pthread_mutex_unlock(&load_lock);   /*解锁让其他用户可以登录*/
    }
    else
    {
        strcpy(buf, "load faliure: Your password can't match to your name\n");
	
	n_write = Write(confd, buf, strlen(buf));
	if (n_write <= 0)
	{
	    fputs("load: Write failure\n", stderr);
	    close(confd);
	    return -1;
	}

	return 0;
    }
    
    memset(buf, 0, MAXSIZE);
    manage_chat(confd);            /*调用聊天处理函数*/
    return 0;
}

/*******************************************************************
函    数:    manage_chat
功    能:    管理聊天模式
传入参数:    confd:聊天用的端口号
传出参数:    无
返    回:    1:程序错误返回
*******************************************************************/
int manage_chat(int confd)
{
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    
    int n_read;
    int n_write;
    
    struct user_link *temp_online = NULL;   /*存放在在线链表中找到的该成员*/

    while (1)
    {
        n_read = Read(confd, &tran_msg, sizeof(tran_msg));    /*读取客户端要求*/
	if (n_read <= 0)
	{
	    fputs("manage_chat: Read faliure\n", stderr);
	    temp_online = find_by_confd(confd);
	    debug_msg("manage_chat,测m_name %s", temp_online->user_msg.name);
	    delete_online(temp_online);
            notice(temp_online->user_msg.name, DOWNLINE);
	    close(confd);
	    return -1;
	}
       
        /*检测读取数字*/
	debug_msg("manage_chat:n_read = %d  buf[0] = %c\n", n_read, tran_msg.mode[0]);

	switch (tran_msg.mode[0])
	{
	    case PRIVATE:       /*客户要求私聊*/
	    {   
                my_private(confd, &tran_msg);
	        break;
	    }

	    case PUBLIC:        /*客户要求群聊*/
	    {   
		my_public(confd, &tran_msg);
		break;
	    }

            case QUIT_CHAT:          /*客户要求退出登录*/
	    {   
		struct user_link *temp_link = find_online(tran_msg.m_name);
		debug_msg("要删除的名字:%s", temp_link->user_msg.name);
		debug_msg("要删除的通信端口号:%d\n", temp_link->confd);
		
		pthread_mutex_lock(&load_lock);    /*涉及在线链表操作地方上锁*/
		delete_online(temp_link);
		notice(temp_link->user_msg.name, DOWNLINE);
		pthread_mutex_unlock(&load_lock);
		
		debug_msg("查看删除后的在线用户链表\n");
                traverse(online_head);
		debug_msg("查看删除后的在线用户链表完毕\n:");

	        strcpy(tran_msg.m_name, "Server\n");
		strcpy(tran_msg.msg, "您已经成功退出聊天模式\n");
		n_write = Write(confd, &tran_msg, sizeof(tran_msg));   
                if (n_write <= 0)
		{
                    temp_online = find_by_confd(confd);
                    delete_online(temp_online);
                    close(confd);
                    notice(temp_online->user_msg.name, DOWNLINE);
		    fputs("服务器读取错误\n", stderr);
		    return -1;
		}

	        return 0;
	    }

	    case SCANF_ONLINE: /*客户要求查看在线用户*/
	    {
	        scan_online(confd);
		debug_msg("scanf_online: %s", tran_msg.mode);
	        break;
	    }
            
	    case KICK_ALL:     /*超级用户功能*/
	    {
	        struct user_link *temp = NULL;
		notice_close_serv(confd);
		    
		sleep(10);

		strcpy(tran_msg.m_name, "Server\n");
		strcpy(tran_msg.msg, "Server have been closed, You are forced refferals!\n");

		for (temp = online_head; temp != NULL; temp = temp->next)
		{
		    n_write = Write(temp->confd, &tran_msg, sizeof(tran_msg));
	            if (n_write <= 0)
	            {
			fputs("The user is not online!\n", stderr);
			temp_online = find_by_confd(temp->confd);
			delete_online(temp_online);
                        notice(temp_online->user_msg.name, DOWNLINE);
			close(temp->confd);
			return -1;
	             }
		}

		exit(0);
	    }

	    case MANAGE_SPEAK:       /*设置是否允许用户发言功能*/
	    {   
	         debug_msg("Man-sp:进入服务\n");
		 debug_msg("Man-sp:f_name :%s confd:%d\n", tran_msg.f_name, confd);
	         manage_speak(&tran_msg, confd);
		 break;
	    }

	    case KICK_ONE:          /*踢出一个人*/
	    {   
	        debug_msg("kick_one\n");
	        kick_one(&tran_msg, confd);
		break;
	    }

	    default:           /*其他情况*/
	    {  
	        strcpy(tran_msg.msg, "您输入的选项有误\n");
		
		n_write = Write(confd, &tran_msg, sizeof(tran_msg));   /*写回给客户端*/
                if (n_write <= 0)
		{
		    temp_online = find_by_confd(confd);
		    delete_online(temp_online);
                    notice(temp_online->user_msg.name, DOWNLINE);
		    close(confd);
		    fputs("服务器读取错误\n", stderr);
		    return -1;
		}

	        break;
	    }
	}
        
        memset(&tran_msg, 0, sizeof(tran_msg));
    }	

}

/************************************************************************
函    数:    my_private
功    能:    处理用户私聊的函数
传入参数:    confd:用来和客户通信的网络端口
传出参数:    无
返    回:    
************************************************************************/
int my_private(int confd, struct chat *temp)
{   
    debug_msg("private: confd = %d\n", confd);
    struct user_link *temp_user = NULL;
    
    int n_read;
    int n_write;
    
    int temp_confd;    /*存放临时通信节点*/

    struct user_link *temp_online = NULL;
    
    debug_msg("private: NAME = %s", temp->f_name);
    debug_msg("private: NAME = %s", temp->m_name);
    
    temp_user = find_online(temp->m_name);     /*得到客户的节点*/
    if (temp_user->speak_flag == 0)
    {
        strcpy(temp->m_name, "Server\n");
	strcpy(temp->msg, "Sorry ,you can't speak! Plead conect to administrator!");
	
	n_write = Write(confd, temp, sizeof(struct chat));  /*发送到客户端告诉他已经被禁言*/
	if (n_write <= 0)
	{   
	    fputs("private : 读取错误\n", stderr);
            temp_online = find_by_confd(confd);
	    delete_online(temp_online);
            notice(temp_online->user_msg.name, DOWNLINE);
	    close(confd);
	    return -1;
	}

	return 0;
    }
    
    temp_user = find_online(temp->f_name);     /*得到需要聊天的对象的节点*/
    
    if (temp_user == NULL)
    {   
        strcpy(temp->m_name, "Serverd\n");
        strcpy(temp->msg, "The people you want to connect are not online\n");
	
	n_write = Write(confd, temp, sizeof(struct chat));  /*反馈信息非客户端*/
	if (n_write <= 0)
	{   
	    fputs("private : 读取错误\n", stderr);
            temp_online = find_by_confd(confd);
	    delete_online(temp_online);
            notice(temp_online->user_msg.name, DOWNLINE);
	    close(confd);
	    return -1;
	}

        return 0;
    }

    debug_msg("private: name = %s", temp->f_name);
    temp_confd = temp_user->confd;
    debug_msg("private: confd = %d\n", temp_confd);
    
    debug_msg("开始通信\n");
    
    n_write = Write(temp_confd, temp, sizeof(struct chat));  /*发送到指定的聊天对象*/
    if (n_write <= 0)
    {
        fputs("private : Write faliure\n", stderr);
        temp_online = find_by_confd(temp_confd);
	delete_online(temp_online);
	notice(temp_online->user_msg.name, DOWNLINE);
        close(temp_confd);
	return -1;
    }

    debug_msg("结束通信\n");
    return 0;
}

/***************************************************************************
函    数:    my_public
功    能:    处理客户端的群聊模式
传入参数:    confd
传出参数:    无
返    回:    无
***************************************************************************/
int my_public(int confd, struct chat *tran_msg)
{   
    int n_read;
    int n_write;
    
    struct user_link *temp = NULL;
    struct user_link *temp_online = NULL;

    temp = find_online(tran_msg->m_name);     /*得到客户的节点*/
    
    /*判断是否被禁言*/
    if (temp->speak_flag == 0)
    {
        strcpy(tran_msg->m_name, "Server\n");
	strcpy(tran_msg->msg, "Sorry ,you can't speak! Plead conect to administrator!");
	
	n_write = Write(confd, tran_msg, sizeof(struct chat));  /*发送到指定的聊天对象*/
	if (n_write <= 0)
	{   
	    fputs("private : 读取错误\n", stderr);
            temp_online = find_by_confd(confd);
	    delete_online(temp_online);
            notice(temp_online->user_msg.name, DOWNLINE);
	    close(confd);
	    return -1;
	}

	return 0;
    }	
    
    debug_msg("my_public:tran_msg->msg: %s\n", tran_msg->msg);
    
    /*循环将信息发送给在线的各个用户*/
    for (temp = online_head; temp != NULL; temp = temp->next)
    {
	n_write = Write(temp->confd, tran_msg, sizeof(struct chat));
	if (n_write <= 0)
        {
	    fputs("public：写入错误\n", stderr);
            temp_online = find_by_confd(temp->confd);
	    delete_online(temp_online);
            notice(temp->user_msg.name, DOWNLINE);
            close(temp->confd);
	}

	debug_msg("my_public:temp->confd %d\n", temp->confd);
    }
    
    return 0;
}

/************************************************************************
函    数:    scan_online
功    能:    查看在线用户
传出参数:    服务器与客户端通信的端口
传出参数:    无
返    回:    0: 程序成功执行结果
*************************************************************************/
int scan_online(int confd)
{   
    struct user_link *temp_online = NULL;
    struct user_link *temp = NULL;
    
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    
    int n_write;
    int debug_flag = 0;
    
    strcpy(tran_msg.m_name, "Server\n");/*将在线用户的名字存进需要传送的缓存区*/
    strcpy(tran_msg.msg, "Begin to browser the online user\n");
	
    /*将用户姓名写入到通信端口*/
    n_write = Write(confd, &tran_msg, sizeof(tran_msg));  
    if (n_write <= 0) 
    {
        temp_online = find_by_confd(confd);
	delete_online(temp_online);
        fputs("scan_online：读取错误\n", stderr); 
	notice(temp_online->user_msg.name, DOWNLINE);
        close(confd);
        return -1;
    }

    memset(&tran_msg, 0, sizeof(tran_msg));
    
    /*循环将在线用户发送给需要查看在线用户的客户*/
    for (temp = online_head; temp != NULL; temp = temp->next)
    {
        /*将在线用户的名字存进需要传送的缓存区*/
        strcpy(tran_msg.m_name, "Server\n");
	
	strcpy(tran_msg.msg, temp->user_msg.name);
	
	/*将用户姓名写入到通信端口*/
	n_write = Write(confd, &tran_msg, sizeof(tran_msg));  
        if (n_write <= 0) 
	{
            temp_online = find_by_confd(confd);
	    delete_online(temp_online);
	    notice(temp_online->user_msg.name, DOWNLINE);
            fputs("scan_online：读取错误\n", stderr); 
            close(confd);
	    return -1;
	}
        
	debug_msg("debug_flag = %d\n", debug_flag++);
        memset(&tran_msg, 0, sizeof(tran_msg));
    }
    
    strcpy(tran_msg.m_name, "Server\n");     
    strcpy(tran_msg.msg, "Browser the online user over\n");
	
    /*将用户姓名写入到通信端口*/
    n_write = Write(confd, &tran_msg, sizeof(tran_msg));  
    if (n_write <= 0) 
    {   
        close(confd);
        temp_online = find_by_confd(confd);
	delete_online(temp_online);
	notice(temp_online->user_msg.name, DOWNLINE);
        fputs("scan_online：读取错误\n", stderr); 
        return -1;
    }

    return 0;
}
