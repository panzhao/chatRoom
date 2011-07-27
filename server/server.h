/************************************************************************
文    件:    store_msg.h
功    能:    对函数的声明和结构体的定义
************************************************************************/
#ifndef _INIT_MSG_H_
#define _INIT_MSG_H_

#define SERV_PORT 8000      /*服务器绑定的端口号*/
#define MAXSIZE   1024      /*缓存的最大字节数*/
#define MAX_MSG   256       /*聊天缓存区的最大值*/
#define REGISTER  '0'       /*注册*/
#define LOAD      '1'       /*登录*/
#define QUIT      '2'       /*退出*/
#define PRIVATE   '3'       /*私聊*/
#define PUBLIC    '4'       /*群聊*/
#define SCANF_HISTORY '5'   /*参看记录*/
#define SCANF_ONLINE  '6'   /*产看在线*/
#define QUIT_CHAT     '7'   
#define KICK_ALL      '8'   /*超级用户剔除所有用户*/
#define MANAGE_SPEAK  '9'   /*管理用户是否能够发言*/
#define KICK_ONE      'k'   /*踢出一个人*/
#define ONLINE         1
#define DOWNLINE       0

/*对学生结构体的定义*/
struct user
{
    char name[20];
    char password[20];
};

/*定义用来链入用户链表的结构体*/
struct user_link 
{
    struct user  user_msg;
    int confd;               /*用户通信端口*/
    int speak_flag;          /*是否允许用户通话的标志,1能发言，0为禁言*/     
    struct user_link *next;
};

/*定义用来得到用户进入客户端时候的选择*/
struct choice
{   
    char mode[10];
    struct user user_msg;
};

/*得到用户聊天模式*/
struct chat
{
    char mode[10];
    char f_name[20];
    char m_name[20];
    char msg[256];
};

extern struct user_link *user_head;      /*对连便头指针的声明*/
extern struct user_link *online_head;    /*用户在线链表头指针*/

struct user *init_msg();                 /*用来对用户信息的初始化*/
int write_in(struct user *);             /*用来将用户信息写入到文件*/
int insert_link(struct user_link *);     /*将初始化好的信息插入到用户链表中去*/
int traverse(struct user_link *);        /*对用户链表遍历*/
int server();                            /*聊天室服务器*/
void *manage_client(void *);             /*用来处理与用户联系的线程函数*/
int insert_online(struct user_link *);   /*将登录的用户插入到在线链表中*/
int check_user(char *);                  /*检测注册链表中是用户是否存在*/
int my_register(int , struct user *);   /*处理用户注册的函数*/
int load(int , struct user *);           /*用户登录函数*/
int free_all();                          /*释放链表内存*/
struct user_link *find_online(char *);   /*查找在线用户*/
int check_password(struct user *);       /*检测用户和密码是否匹配*/
struct user_link *user_node(struct user *, int ); /*制作在线链表的结构体的地址*/
int manage_chat(int );                   /*管理用户聊天的函数*/
int delete_online(struct user_link *);   /*删除在线用户节点*/
int my_private(int , struct chat *);     /*管理私聊的情况*/
int my_public(int , struct chat * );     /*处理客户群聊*/
struct user_link *find_by_confd(int);    /*通过通信端口找到用户*/
int kick_all();                          /*踢出所有在线用户*/
int scan_online(int );                   /*查看在线用户*/
int manage_speak(struct chat *, int);    /*管理用户是否能发言的函数*/
int kick_one(struct chat *, int);        /*踢出某个用户*/
int notice(char *, int);
int notice_close_serv(int);              /*通知服务器关闭函数*/

#endif
