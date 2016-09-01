#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <pthread.h>

#include "sem_op.h"

struct sockaddr_in server, client;

void chat(int fd)
{
	printf("请输入对象number:");
	char number[5];
	char chat_buf[1024];
	bzero(number, 5);
	scanf("%s", number);
	send(fd, number, strlen(number), 0);
	printf("-------\n");
	recv(fd, number, 5, 0);
	if(strcmp(number, "error") == 0)
	{
		return ;
	}
	printf("正在与%s聊天\n", number);
	socklen_t length = sizeof(server);
	socklen_t length1 = sizeof(client);
	while(1)
	{
		ssize_t n;
		bzero(chat_buf, 1024);
		printf("开始聊天:");
		scanf("%s", chat_buf);
		//sendto(fd, chat_buf, 1024, 0, (struct sockaddr *)&server, length);
		send(fd, chat_buf, strlen(chat_buf), 0);
		if(strcmp(chat_buf, "exit") == 0){
			printf("退出聊天中...\n");
			//sleep(2);
			exit(0);
		}
		//n = recvfrom(fd, chat_buf, 1024, 0, (struct sockaddr *)&client, &length1);
		char chat_buf[1024];
		bzero(chat_buf, 1024);
		n = recv((int)fd, chat_buf, 1024, 0);
		if(n == -1)
		{
			perror("recv failed\n");
			break;
		}
	/*	if(strcmp(chat_buf, "exit") == 0){
			//printf("退出聊天中...\n");
			//sleep(2);
			break;
		}*/
		printf("回射测试%s\n", chat_buf);
	}
}

void file(int fd)
{
	printf("开始传文件\n************\n");
	printf("请输入文件路径:");
	char line_buf[1024];
	bzero(line_buf, 1024);
	scanf("%s", line_buf);
	printf("\n");
	printf("请输入文件名:");
	char name_buf[100];
	bzero(name_buf, 100);
	scanf("%s", name_buf);
	
	//char file_name[5];
	//bzero(file_name, 5);`
	printf("send file name\n");
	send(fd, name_buf, strlen(name_buf), 0);
	printf("file name send ok\n");
	//sleep(1);
	
	DIR *dir = opendir(line_buf);
	if(dir == NULL)
	{
		perror("opendir() failed");
		exit(1);
	}
	chdir(line_buf);
	struct dirent *ptr;
	int rfd, n;
	char file_buf[1024];
	bzero(file_buf, 1024);
	while((ptr=readdir(dir)) != NULL) //确认文件夹下是否存在目标文件
    {
		if(strcmp(ptr->d_name, name_buf) == 0)
		{
			if((rfd = open(ptr->d_name, O_RDONLY)) < 0)
			{
				fprintf(stderr,"Fail to open %s : %s\n",ptr->d_name, strerror(errno));
				break;
			}
			//开始传输
			if((n = read(rfd,file_buf,sizeof(file_buf))) < 0)
			{
				perror("Fail to read");
				break;
			}
			if(n == 0)
			{
				printf("Read end of file!\n");
				break;
			}
			send(fd, file_buf, strlen(file_buf), 0);
			//printf("文件内容是:%s\n", file_buf);
			sleep(2);
			printf("SEND OK!\n");
		}
	}
}

void download(int fd)
{
	printf("下载服务器文件中\n");
	printf("请输入所要下载的文件名:");
	char file_name[100];
	bzero(file_name, 100);
	scanf("%s", file_name);
	send(fd, file_name, strlen(file_name), 0);
	printf("文件名上传成功\n");
	
	printf("请输入文件保存路径:");
	char save_line[1024];
	bzero(save_line, 1024);
	scanf("%s", save_line);
	
	printf("下载文件中...\n");
	char file_content[1024];
	bzero(file_content, 1024);
	int n = recv(fd, file_content, 1024, 0);
	if(n == -1)
	{
		perror("recv failed\n");
		return ;
	}
	printf("下载的文件内容是:%s\n", file_content);
	int wfd;
	chdir(save_line);
	if((wfd = open(file_name, O_WRONLY | O_CREAT, 0666)) < 0)
	{
		fprintf(stderr, "Fail to open %s : %s\n", file_name, strerror(errno));
		exit(0);
	}
	printf("文件大小为:%d\n", strlen(file_content));

	write(wfd, file_content, strlen(file_content));
	close(wfd);
	/*while(1)
	{
		if((n = read(rfd,file_content,sizeof(file_content))) < 0)
		{
			perror("Fail to read");
			break;
		}
		if(n == 0)
		{
			printf("Read end of file!\n");
			break;
		}
		write(wfd,file_content,n);
		break;
	}*/
	printf("文件下载成功!\n");
}

void *only_430(void *fd)
{
	char *buf;
	key_t key = ftok("/home",4300);
	
	int shm_id = shmget(key, 1000,IPC_CREAT|0664);
	if(shm_id == -1)
	{
		perror("shm get failed\n");
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
	sem_set.val = 1;
	if(semctl(sem_id, 1, SETVAL, sem_set) == -1)
		perror("semctl");
	sem_set.val = 0;
	//设EN_FULL的信号值为0,说明没有一个资源可用
	if(semctl(sem_id, 0, SETVAL, sem_set) == -1)
		perror("semctl");
	sem_p(sem_id, 0);
	//while(1)
	//{
		if((buf = (char *)shmat(shm_id,NULL,0)) == (char *)-1)
		{
			perror("shm attch failed\n");
			return ;
		}
	/*	int n;
		n = recv((int)fd, buf, 1024, 0);
		if(n == -1)
		{
			perror("recv failed\n");
			return ;
		}*/
		printf("%s\n", buf);
	//}
	sem_v(sem_id, 1);
	/*IPC必须显示删除。否则会一直留存在系统中*/
	if(shmctl(shm_id, IPC_RMID, NULL)==-1)
		printf("shmctl delete error\n");
}

int main(int argc, char *argv[])
{
	printf("欢迎使用430聊天软件*_*\n");
	// 判断输入参数的个数
	/*if(argc < 3)
	{
		printf("we need %s [ip]:[port]\n", argv[0]);
		return -1;
	}*/		
	char addr_buf[100];
	char port_buf[100];
	while(1)
	{
		printf("请输入登录地址:");
		bzero(addr_buf, 100);
		scanf("%s", addr_buf);
		printf("请输入登录端口:");
		bzero(port_buf, 100);
		scanf("%s", port_buf);
		if((strcmp(addr_buf, "192.168.1.59") == 0) 
					&& (strcmp(port_buf, "6000")) == 0)
						break;
	}
	// prepare
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1)
	{
		perror("socket failed");
		return -1;
	}	
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(port_buf));//主机码转换成网络字节序，s表示short, l 表示long（4B）
	server.sin_addr.s_addr = inet_addr(addr_buf);  // 表示要监听的ip，0.0.0.0表示任意一个IP
	
	printf("登录中...\n");
	int con = connect(fd, (struct sockaddr *)&server, sizeof(server));
	if(con == -1)
	{
		perror("connect failed");
		return -1;
	}
	
	sleep(2);
	printf("登录成功\n");	
	
	char sbuf[1024];	
	while(1)
	{
		/*************************************
		pthread_t pid;
		int pid_pth = pthread_create(&pid, NULL, only_430, (void *)fd);
		if(pid_pth == -1)
		{
			perror("pthread creat failed\n");
			return ;
		}
		//pthread_join(pid, NULL);
		//pthread_exit(NULL);
		/***********************************/
		ssize_t n;
		bzero(sbuf, 1024);
		printf("请输入发送内容(功能other,chat,file,download,exit直接退出):");
		scanf("%s", sbuf);
		send(fd, sbuf, strlen(sbuf), 0);
		if(strcmp(sbuf, "exit") == 0){
			printf("退出中...\n");
			//sleep(2);
			break;
		}
		n = recv(fd, sbuf, 1024, 0);
		if(n == -1)
		{
			perror("recv failed\n");
			return -1;
		}
		if(strcmp(sbuf, "chat") == 0){
			chat(fd);
		}
		else if(strcmp(sbuf, "file") == 0){
			file(fd);
		}
		else if(strcmp(sbuf, "download") == 0){
			download(fd);
		}
		else{
			printf("端口号=%hu 回射内容:%s\n", ntohs( server.sin_port), sbuf);
		}
	}
	close(fd);
	exit(0);
	return 0;
}
