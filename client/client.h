/****************************************************************
文    件:    client.h
功    能:    对客户端模块函数的声明和对结构体的定义
函数列表:
****************************************************************/
#ifndef _CLIENT_H
#define _CLIENT_H

#define REGISTER '0'        /*注册用宏定义1 */
#define LOAD     '1'        /*登录用宏定义为2*/
#define QUIT     '2'        /*退出用宏定义0 */
#define SERV_PORT 8000      /*服务器端口*/     
#define MAXSIZE   1024      /*缓存最大字节数*/
#define PRIVATE   '3'       /*私聊*/
#define PUBLIC    '4'       /*群聊*/
#define SCANF_HISTORY '5'   /*查看聊天记录*/
#define SCANF_ONLINE  '6'   /*查看在线用户*/
#define QUIT_CHAT     '7'   /*退出聊天模式*/
#define KICK_ALL      '8'   /*超级用户功能*/
#define MANAGE_SPEAK  '9'   /*禁言功能*/
#define MAX_MSG       256   /*定义用户聊天时的临时信息存储区*/
#define KICK_ONE      'k'   /*踢出一个人*/ 

struct  user                /*定义用来存放用户信息的结构体*/
{   
    char name[20];
    char password[20];
};

struct choice
{
    char   mode[10];
    struct user user_msg;
};

/*用来存放聊天记录的结构体*/
struct chat
{   
    char mode[10];
    char f_name[20];
    char m_name[20];
    char msg[256];
};

/*定义用来存放聊天记录的结构体*/
struct store_msg
{
    struct chat history;
    struct store_msg *next;
};


extern struct store_msg *history_head;

int init_msg(struct user *);    /*对用户信息的初始化*/
int write_in(struct chat *);    /*将用户信息写入到文件中去*/ 
int print_in();                 /*一个输出选项格式*/
int choice();                   /*用户进入聊天室的选择*/
int manage_chat();              /*处理用户登录成功后的聊天需求*/
void *read_serv(void *);        /*监听服务器返回信息的线程函数*/
int my_private(struct chat *);  /*私聊处理函数*/
int my_public(struct chat *);   /*群聊处理函数*/
int scan_history();             /*查看历史记录的函数*/
int get_msg();
int traverse(struct store_msg *); 
int insert_link(struct store_msg *);   /*插入链表*/
int free_all();                        /*释放节点*/
int manage_speak(struct chat *);       /*管理禁言的函数*/

#endif
