#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h> 
#include <string.h>
#include <errno.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <netinet/in.h>  
#include <sys/types.h>  
#include <sys/prctl.h>
#include<sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

#include "client_ReadConf.h"
#include "client_PrepareMsg.h"
#include "client_Log.h"

#define total 10000

char systemtime[20];
char a[1000];

int separate = 0;//分裂进程数
int reclaim  = 0;//收回进程数

unsigned int id;
int total_term   = 0;//终端总量
int total_screen = 0;//虚屏总量

void Count()
{
	time_t tt = time(NULL);
	tm* t= localtime(&tt);
	FILE *fp;
	fp = fopen("ts_count.xls","a");
	fprintf(fp,"%02d:%02d:%02d\t%d\t%d\t%d\t%d\n",t->tm_hour,t->tm_min,t->tm_sec,id,1,total_term,total_screen);
	fclose(fp);
}

void Recycle(int num)
{
	int state;
	pid_t pid;
	while((pid = waitpid(-1,&state,WNOHANG))>0)
	{
		reclaim++;
		printf("已分裂%d个子进程，已回收%d个子进程，PID:%d\n",separate,reclaim,pid);
	}
}

static int daemon()
{
	switch(fork()) 
	{
	case -1: 
		return -1; 
	case 0:	
		break;		
	default: 
		_exit(EXIT_SUCCESS);
	}
	umask(0); 	/* Clear file mask mode */
	return 0;
}

int Run(struct TS *ts)
{
	int sockfd;
	/*建立socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(errno);
	}
	
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	
	struct sockaddr_in s_addr;
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(ts->svr_port);
	if (inet_aton(ts->svr_ip, (struct in_addr *)&s_addr.sin_addr.s_addr) == 0)
	{
		perror(ts->svr_ip);
		exit(errno);
	}
	int i = 0;
	if (connect(sockfd, (struct sockaddr*)&s_addr, sizeof(struct sockaddr)) == -1)
	{
		struct timeval tm;
		fd_set	confd;
		tm.tv_sec = 10;
		tm.tv_usec = 0;
		FD_ZERO(&confd);
		FD_SET(sockfd, &confd);
		int sel = select(sockfd + 1, NULL, &confd, NULL, &tm);
		if (sel <= 0)
		{
			if(ts->ERR == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] 连接服务端失败",systemtime,id);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			}
			return -1;
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] Connected OK.\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
	}
	
	int t = 0;
	int len;//message长度
	
	char sendbuf[total];
	char recvbuf[total];
//	memset(sendbuf,0,sizeof(sendbuf));
//	memset(recvbuf,0,sizeof(recvbuf));
	
	int sendstr = 0;
	int sendlen = 0;
	
	int recvstr = 0;
	int recvlen = 0;
	
	bool open_rfd = true;
	bool open_wfd = false;
	
	bool recvstatus = true;
	bool sendstatus = true;
	
	fd_set wfd,rfd;
	while(1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		
		if(open_rfd == true)//recv
			FD_SET(sockfd, &rfd);
		if(open_wfd == true)//send
			FD_SET(sockfd, &wfd);
		int sel = select(sockfd + 1, &rfd, &wfd, NULL, NULL);
		if (sel < 0)
		{
			printf("select error!\n");
			return -2;
		}
		else if (sel == 0)
			continue;
		else
		{
			if (FD_ISSET(sockfd, &rfd))//recv
			{
				if(recvstatus == true)
				{
					recvstr = sizeof(msg);
					while(recvlen < recvstr)
					{
						len = recv(sockfd,&recvbuf[recvlen],recvstr - recvlen,0);
						
						if (len <= 0)
						{
							cerr << "C - Failed to recv data " << endl;
							close(sockfd);
						//	exit(EXIT_FAILURE);
							return -3;
						}
						
						recvlen = recvlen + len;
						if(recvlen == recvstr)
						{
							memcpy(&msg,recvbuf,recvlen);
							WriteFirst((const char*)&msg,recvlen,ts,ts->RDATA,&msg);
							recvstr = 0;
							recvlen = 0;
							recvstatus = false;
							open_rfd = false;
							open_wfd = true;
							break;
						}
					}
					
					if(msg.version() == 0)//判断版本是否符合要求
					{
						cout <<"version < 2.0.0"<< endl;
						least_version.send_least_version(&msg);
					}
					if(msg.system_time_and_check() == 0)//判断数字证书是否过期
					{
						cout <<"digital certification expired!"<< endl;
						close(sockfd);
					}
					if(msg.check_authstring() == 0)//check认证串
					{
						cout<<"authenticate illegal!"<< endl;
						close(sockfd);
					}
				}
				else
				{
					recvstr = sizeof(info_req);
					while(recvlen < recvstr)
					{
						len = recv(sockfd,&recvbuf[recvlen],recvstr - recvlen,0);
						
						if(len == 0)
						{
							Count();
							close(sockfd);
							return 0;
						}
					
						if (len < 0)
						{
							cerr << "C - Failed to recv data " << endl;
							close(sockfd);
						//	exit(EXIT_FAILURE);
							return -3;
						}
						
						recvlen = recvlen + len;
						if(recvlen == recvstr)
						{
							memcpy(&info_req,recvbuf,recvlen);
							WriteRecvMsg((const char*)&info_req,recvlen,ts,&info_req,ts->RDATA);
							recvstr = 0;
							recvlen = 0;
							open_rfd = false;
							open_wfd = true;
							break;
						}
					}
				}
			}
			
			if (FD_ISSET(sockfd, &wfd))//send
			{
				if(sendstatus == true)
				{
					if(msg.version() == 0)//判断版本是否符合要求
					{
						sendstr = sizeof(least_version);
						memcpy(sendbuf,(void*)&least_version,sendstr);
					}
					else
					{
						msg1.prepare_msg1(id);
						sendstr = sizeof(msg1);
						memcpy(sendbuf,(void*)&msg1,sendstr);
					}
					while(sendlen < sendstr)
					{
						len = send(sockfd,&sendbuf[sendlen],sendstr - sendlen,0);
						
						if (len <= 0)
						{	
							cerr << " C - Failed to send data" << endl;
							close(sockfd);
						//	exit(EXIT_FAILURE);
							return -3;
						}
						
						sendlen = sendlen + len;
						if(sendlen == sendstr)
						{
							WriteSecond((const char*)sendbuf,sendlen,ts,ts->SDATA);
							sendstr = 0;
							sendlen = 0;
							sendstatus = false;
							open_rfd = true;
							open_wfd = false;
							break;
						}
					}
				}
				else
				{
					switch(info_req.first[1])
					{
					case 0x02://系统信息
						resp_system.prepare_resp_system();
						sendstr = sizeof(resp_system);
						memcpy(sendbuf,(void*)&resp_system,sendstr);
						break;
						
					case 0x03: //配置信息
						resp_allo.prepare_resp_allo();
						sendstr = ntohs(resp_allo.msglength);
						memcpy(sendbuf,(void*)&resp_allo,sendstr);
						break;
						
					case 0x04: //进程信息
						resp_pid.prepare_resp_pid();
						sendstr = ntohs(resp_pid.msglength);
						memcpy(sendbuf,(void*)&resp_pid,sendstr);
						break;
						
					case 0x05: //eth0/eth1信息
						if(ntohs(info_req.second) == 0x0000)
						{
							resp_eth0.prepare_resp_eth0();
							sendstr = sizeof(resp_eth0);
							memcpy(sendbuf,(void*)&resp_eth0,sendstr);
						}
						else if(ntohs(info_req.second) == 0x0001)
						{
							resp_eth1.prepare_resp_eth1();
							sendstr = sizeof(resp_eth1);
							memcpy(sendbuf,(void*)&resp_eth1,sendstr);
						}
						break;
						
					case 0x07: //u盘信息
						resp_usbinfo.prepare_resp_usbinfo();
						sendstr = sizeof(resp_usbinfo);
						memcpy(sendbuf,(void*)&resp_usbinfo,sendstr);
						break;
						
					case 0x0c: //u盘文件信息
						resp_usbfile.prepare_resp_usbfile();
						sendstr = ntohs(resp_usbfile.msglength);
						memcpy(sendbuf,(void*)&resp_usbfile,sendstr);
						break;
						
					case 0x08: //打印口信息
						resp_print.prepare_resp_print();
						sendstr = sizeof(resp_print);
						memcpy(sendbuf,(void*)&resp_print,sendstr);
						break;
						
					case 0x0d: //打印队列信息
						resp_printrow.prepare_resp_printrow();
						sendstr = ntohs(resp_printrow.msglength);
						memcpy(sendbuf,(void*)&resp_printrow,sendstr);
						break;
						
					case 0x09: //终端信息
						resp_term.prepare_resp_term(ts);
						sendstr = sizeof(resp_term);
						memcpy(sendbuf,(void*)&resp_term,sendstr);
						break;
					
					case 0x0a: //哑终端信息
						resp_scrinfo.prepare_resp_scrinfo(ts,&info_req);
						sendstr = ntohs(resp_scrinfo.msglength);
						memcpy(sendbuf,(void*)&resp_scrinfo,sendstr);
						break;
						
					case 0x0b: //IP终端信息
						resp_scrinfo.prepare_resp_scrinfo(ts,&info_req);
						sendstr = ntohs(resp_scrinfo.msglength);
						memcpy(sendbuf,(void*)&resp_scrinfo,sendstr);
						break;
						
					case 0xff: //所有包均收到
						info_req.first[0] = 0x91;
						sendstr = sizeof(info_req);
						memcpy(sendbuf,(void*)&info_req,sendstr);
						break;
						
					default:
						break;
					}
					
					while(sendlen < sendstr)
					{
						len = send(sockfd,&sendbuf[sendlen],sendstr - sendlen,0);
						
						if (len <= 0)
						{	
							cerr << " C - Failed to send data" << endl;
							close(sockfd);
						//	exit(EXIT_FAILURE);
							return -3;
						}
						
						sendlen = sendlen + len;
						if(sendlen == sendstr)
						{
							WriteSendMsg((const char*)sendbuf,sendlen,ts,&info_req,ts->SDATA);
							sendstr = 0;
							sendlen = 0;
							open_rfd = true;
							open_wfd = false;
							break;
						}
					}
				}
			}
		}
	}
	close(sockfd);
	return 0;
}

int main(int argc, char **argv)
{
	if(argc == 3)
	{
		clock_t start,end;
		signal(SIGCHLD,Recycle);
		id = atoi(argv[1]);
		int num = atoi(argv[2]);
		int i;
		int pid;
		daemon();
		ts.ReadConf();
		FILE *fp;
		fp = fopen("ts_count.xls","w");
		fclose(fp);
		if(ts.delfile == 1)
		{
			FILE *fp;
			fp = fopen("ts.log","w");
			fclose(fp);
		}
		for(i = 1;i <= num;)
		{
			if(separate - reclaim < 200)
			{
				pid = fork();
				if(pid == 0)
				{
					prctl( PR_SET_PDEATHSIG, SIGKILL);
					bool flag = true;
					int time = 0;
					int t;
					while(flag)
					{
						t = Run(&ts);
						if(t == -1)
						{
							time++;
							sleep(1);
							if(time >= 20)
							{
								cerr << "出错，退出！"<<endl;
								exit(0);
							}
						}
						else if(t == 0)
						{
							if(ts.markexit == 1)
								flag = false;
							else
							{
								time = 0;
								sleep(ntohs(msg.ifsuccess_time));
							}
						}
						else if(t == -3)
							continue;
					}
					exit(0);
				}
				else if(pid > 0)
				{
					separate++;
					i++;
					id++;
				}
			}
		}
		while(reclaim < num)
			sleep(1);
		end = clock();
		printf("\n成功分裂出%d个子进程，成功回收%d个子进程，用时%fs\n",separate,reclaim,(double)(end - start) / CLOCKS_PER_SEC);
	}
	else
		printf("parameter error!\n");
	return 0;
}