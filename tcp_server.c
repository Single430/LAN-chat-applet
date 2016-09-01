#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/sem.h>

#include "sem_op.h"
#include "./kernel_list.h"
#include "thread_pool.h"

#define   MAXSIZE    1024

struct list_head *pos = NULL,*n = NULL;
struct cs *pnode = NULL;
struct cs *head = NULL;
struct sockaddr_in server,client;

struct file_list{
	char file_num[100];
	char list_file_buf[1024];
	struct list_head list_file;
};
struct file_list *file_head = NULL;
struct list_head *fpos = NULL;


//全局退出信号函数
void func(int sig)
{
	exit(0);
}
//用户链表
void creat(struct cs *head_list)
{
	// creat 
	struct cs *q = head_list;// root
	if (q == NULL) {
		printf("file,%s line,%d:malloc error!\n",__FILE__,__LINE__);
		exit(1);
	}
	INIT_LIST_HEAD(&q->list);
}
//文件链表
void creat_file(struct file_list *head_list)
{
	// creat 
	struct file_list *q = head_list;// root
	if (q == NULL) {
		printf("file,%s line,%d:malloc error!\n",__FILE__,__LINE__);
		exit(1);
	}
	INIT_LIST_HEAD(&q->list_file);
}

void *chat(void *sockfd)
{
	printf("聊天功能开启\n");
	printf("选择对象中...\n");
	char chat_return[MAXSIZE];
	char chat_buf[MAXSIZE];
	struct cs *pnode1 = NULL;
	char number[5];
	bzero(number, 5);
	recv((int)sockfd, number, 5, 0);
	printf("对象是:%s\n", number);
	//遍历
	if (list_empty(&head->list))
	{
		printf("list is empty!\n");
	}
	else 
	{	
		list_for_each(pos,&head->list) {
			pnode1 = list_entry(pos,struct cs,list);
			printf("111111111111\n");
			if((strcmp(number, pnode1->number) == 0) 
							&& (pnode1->numfd != -1))
			{
				printf("**********\n");
				printf("OKKKKKK--numfd:%d number:%s\n", pnode1->numfd, pnode1->number);
				send(pnode1->numfd, number, strlen(number), 0);
				printf("开始聊天\n");
			//	pthread_t tid;
				
				while(1)
				{
					printf("sockfd:%d pnode1->numfd:%d\n", (int)sockfd, pnode1->numfd);
					bzero(chat_buf, MAXSIZE);
					bzero(chat_return, MAXSIZE);
					socklen_t length = sizeof(client);
					socklen_t length1 = sizeof(server);
					int n = recv((int)sockfd, chat_buf, MAXSIZE, 0);//success return byte number
					//recvfrom((int)sockfd, chat_buf, MAXSIZE, 0, (struct sockaddr *)&client, &length);
					if(n == -1){
						perror("recv failed\n");
						break;
					}
					if(strcmp(chat_buf, "exit") == 0){
							printf("用户离开聊天了\n");
							exit(0);	
						}
					printf("收到内容为:%s\n", chat_buf);
					strcpy(chat_return, chat_buf);
					send(pnode1->numfd, chat_return, strlen(chat_return), 0);
					//sendto(pnode1->numfd, chat_buf, MAXSIZE, 0, (struct sockaddr *)&server, length1);
					/*if(strcmp(chat_return, "exit") == 0){
						printf("用户离开聊天了\n");
						break;	
					}*/
				}
			}
		}
		strcpy(number, "error");
		send((int)sockfd, number, strlen(number), 0);
	}
	
}

void *file(void *sockfd)
{
	printf("*******************\n传输文件功能开启\n");
	printf("正在接收文件...\n");
	int n;
	char file_name[100];
	bzero(file_name, 100);
	printf("接收文件名中...\n");
	n = recv((int)sockfd, file_name, 100, 0);
	if(n == -1)
	{
		perror("recv failed\n");
		return ;
	}
	printf("文件名是:%s\n", file_name);
	//sleep(1);
	
	char file_buf[MAXSIZE];
	n = recv((int)sockfd, file_buf, MAXSIZE, 0);
	if(n == -1)
	{
		perror("recv failed\n");
		return ;
	}
	//printf("接收的文件是:%s\n", file_buf);
	
	struct file_list *node = NULL; //node
	// insert
	node = (struct file_list*)malloc(sizeof(struct file_list));
	if (node == NULL) {
		printf("file,%s line,%d:malloc error!\n",__FILE__,__LINE__);
		return ;
	}
	//printf("__________@@___\n");
	strcpy(node->list_file_buf, file_buf);
	strcpy(node->file_num, file_name);
	//头部插入
	//printf("其file_num是文件句柄:%d\n接收到的文件内容是:%s\n", node->file_num, node->list_file_buf);
	list_add(&node->list_file, &file_head->list_file);
	sleep(2);
	printf("****接收成功****\n");
	printf("文件名字是:%s\n接收到的文件内容是:%s\n", node->file_num, node->list_file_buf);
	
	printf("退出文件传输功能\n");
}

void *download(void *sockfd)
{
	int n;
	char file_name[100];
	bzero(file_name, 100);
	printf("接收文件名中...\n");
	n = recv((int)sockfd, file_name, 100, 0);
	if(n == -1)
	{
		perror("recv failed\n");
		return ;
	}
	printf("文件名接收成功\n");
	
	struct file_list *fnode = NULL;
	//遍历
	int i = 0;
	char file_content[MAXSIZE];
	bzero(file_content, MAXSIZE);//content是内容的意思吧，heihei
	if (list_empty(&file_head->list_file))
	{
		printf("list is empty!\n");
	}
	else 
	{	
		list_for_each(fpos,&file_head->list_file) {
			fnode = list_entry(fpos, struct file_list, list_file);
			if(strcmp(fnode->file_num, file_name) == 0)
			{
				printf("文件内容是:%s\n", fnode->list_file_buf);
				strcpy(file_content, fnode->list_file_buf);
				printf("将文件传给用户中...\n");
				send((int)sockfd, file_content, strlen(file_content), 0);
				printf("文件传输OK\n");
			}
		}
		//strcpy(file_content, "file error");
		perror("error");
		send((int)sockfd, file_content, strlen(file_content), 0);
	}
}

void only_430(char *addr, unsigned short port)
{
	/*************信号量锁PV***************/
	char *buf;
	key_t key = ftok("/home", 4300);
	int shm_id = shmget(key, 1000, IPC_CREAT|0664);
	if(shm_id == -1)
	{
		perror("shm get failed\n");
		return ;
	}
	if((buf = (char *)shmat(shm_id, NULL, 0)) == (char *)-1)
	{
		perror("shm attch failed\n");
		return ;
	}
	
	int sem_id = semget(key, 2, IPC_CREAT|0666);//创建两把锁
	if(sem_id == -1)
	{
		perror("sem creat failed\n");
		return ;
	}
	union semun sem_set;
	//设EN_EMPYT的信号值为1
	sem_set.val = 1; //钥匙的数量
	if(semctl(sem_id, 1, SETVAL, sem_set) == -1) //这里的0 或1 代表第几个锁
		perror("semctl");
	sem_set.val = 0;
	//设EN_FULL的信号值为0,说明没有一个资源可用
	if(semctl(sem_id, 0, SETVAL, sem_set) == -1)
		perror("semctl");
	
	/**************************************/
	printf("用户上线通知函数\n");
	char only_4301[MAXSIZE];
	bzero(only_4301, MAXSIZE);
	strcpy(only_4301, buf);
	sprintf(only_4301, "IP is %s PORT is %hu 上线了", addr, port);
	printf("%s\n", only_4301);
	//遍历
	sem_p(sem_id, 1);
	int i = 1;
	if (list_empty(&head->list))
	{
		printf("list is empty!\n");
	}
	else 
	{	
		list_for_each(pos, &head->list) {
			pnode = list_entry(pos, struct cs, list);
			if(pnode->numfd != -1){
				printf("广播文件句柄是:%d\n", pnode->numfd);
				send(pnode->numfd, only_4301, strlen(only_4301), 0);
				//strcpy(buf, pnode->);
				printf("广播成功!\n");
			}
			else if(MAXSIZE == (i++))
				break;
		}
	}
	sem_v(sem_id, 0);
	//printf("广播成功!\n");
}

int main(int argc, char *argv[])
{
	printf("--- 430聊天服务器*_* ---\n");
	printf("--- 正在开启中...\n");
	sleep(2);
	//头链表
	head = (struct cs*)malloc(sizeof(struct cs));
	//文件链表头链表
	file_head = (struct file_list*)malloc(sizeof(struct file_list));
	//struct cs *head = (struct cs*)malloc(sizeof(struct cs));
	creat(head);//创建链表
	creat_file(file_head);
	// prepare
	char buf[MAXSIZE];
	int nready, clienta[FD_SETSIZE];
	fd_set rset, allset;
	int i, ret, maxi, maxfd, sockfd;
	int listfd;
	ssize_t n;
	int newfd;
	listfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listfd == -1)
	{
		perror("socket failed");
		return -1;
	}	
	bzero(&server, sizeof(server));
	//bzero(&client, sizeof(client));	
	server.sin_family = AF_INET;
	server.sin_port = htons(6000);//主机码转换成网络字节序，s表示short, l 表示long（4B）
	server.sin_addr.s_addr = htonl(INADDR_ANY);  // 表示要监听的ip，0.0.0.0表示任意一个IP

	ret = bind(listfd, (struct sockaddr *)&server, sizeof(server)); //和本服务器信息绑在一起
	if(ret == -1 )
	{
		perror("bind error");
		return -1;
	}	
	listen (listfd, 5);	
	maxfd = listfd;
	maxi = -1;
	
	for(i = 0; i<MAXSIZE; i++){
		struct cs *node = (struct cs *)malloc(sizeof(struct cs));
		if (node == NULL) {
			printf("file,%s line,%d:malloc error!\n",__FILE__,__LINE__);
			exit(1);
		}
		node->numfd = -1;
		sprintf(node->number, "%d", i);//
		//printf("用户%s 文件句柄是:%d\n", node->number, node->numfd);
		list_add_tail(&node->list, &head->list);
	}
	FD_ZERO(&allset);
	FD_SET(listfd, &allset);
	thread_pool *pool = malloc(sizeof(thread_pool)); //申请线程池结构体空间
	init_pool(pool, 5);	//初始化线程池
	while(1)
	{
		printf("开启成功，请等待客户登录....\n ");
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listfd, &rset)){
			socklen_t length = sizeof(client);
			newfd = accept(listfd, (struct sockaddr *)&client, (socklen_t *)&length);	
			printf("文件句柄%d\n", newfd);
			if(newfd == -1)
			{
				perror("accept failed\n");
				return -1;
			}
			printf("来自IP为: %s 端口%hu的用户上线了\n", inet_ntoa(client.sin_addr), ntohs( client.sin_port));
			//遍历
			if (list_empty(&head->list))
			{
				printf("list is empty!\n");
			}
			else 
			{	
				list_for_each(pos,&head->list) {
					pnode = list_entry(pos,struct cs,list); // struct student * pnode= （位置 ， 大结构体类型 ， struct list_head list）				
					if(pnode->numfd < 0){
						pnode->numfd = newfd;
						printf("用户%s 文件句柄是:%d\n", pnode->number, pnode->numfd);
						break;
					}
				}
			}
			//only_430(inet_ntoa(client.sin_addr), ntohs( client.sin_port));//上线通知
			if((i -= 1) == FD_SETSIZE)
				perror("too many client\n");
			FD_SET(newfd, &allset);
			if(newfd > maxfd)
				maxfd = newfd;
			if(i > maxi)
				maxi = i;
			if(--nready <= 0)
				continue;
		}
		if (list_empty(&head->list))
		{
			printf("list is empty!\n");
		}
		else 
		{	
			list_for_each(pos,&head->list) {
				pnode = list_entry(pos,struct cs,list); // struct student * pnode= （位置 ， 大结构体类型 ， struct list_head list）
				if((sockfd = pnode->numfd) < 0)
					continue;
				if(FD_ISSET(sockfd, &rset))
				{							
					printf("用户%s 文件句柄是:%d\n", pnode->number, pnode->numfd);
					bzero(buf, MAXSIZE);
					n = recv(sockfd, buf, MAXSIZE, 0);//success return byte number
					if(n == -1){
						perror("recv failed\n");
						return -1;
					}
					printf("用户%s IP为 %s发送了:%s\n",pnode->number, inet_ntoa(client.sin_addr), buf);
			/*********************************
				功能选择区
					*******************************/
					if(strcmp(buf, "chat") == 0){
							add_task(pool, chat, (void *)sockfd); 
							send(sockfd, buf, strlen(buf), 0);
							//chat(head, sockfd);
					}
					else if(strcmp(buf, "file") == 0){
						add_task(pool, file, (void *)sockfd); 
						send(sockfd, buf, strlen(buf), 0);
					}
					else if(strcmp(buf, "download") == 0){
						add_task(pool, download, (void *)sockfd); 
						send(sockfd, buf, strlen(buf), 0);
					}
					else
						send(sockfd, buf, strlen(buf), 0);
					if(strcmp(buf, "exit") == 0){
						printf("用户%s下线了\n", pnode->number);
						//list_del(pos); //删除链表用户
						close(sockfd);
						FD_CLR(sockfd, &rset);
						pnode->numfd = -1;
						break;			
					}
					if(--nready <= 0)
						break;
				}
			}
		}
		if(fork() == 0)
		{
			printf("######################\n");
			printf("全局退出功能实时提示中\n");
			printf("输入quit,将关闭服务器\n");
			printf("!!否则请忽略此信息!!\n");
			char quit_buf[5];
			scanf("%s", quit_buf);
			if(strcmp(quit_buf, "quit") == 0){
				printf("即将关闭服务器\n");
				printf("I also don't want to die...\n");
				sleep(1);
				printf("!@!#@#*#@$*@$(@(#@*#&@&&#@\n");
				//先注册，在发信号
				signal(SIGUSR1, func);
				signal(SIGUSR2, func);
				kill(getppid(), SIGUSR1);
				kill(getpid(), SIGUSR2);
				//exit(0);这样是无法退出的
			}
		}
	}
	destroy_pool(pool);	//取消线程池

	close(listfd);
	close(newfd);
	
	exit(0);
	return 0;
}