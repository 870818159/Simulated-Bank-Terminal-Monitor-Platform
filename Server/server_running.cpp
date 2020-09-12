#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include "server.h"

#define TOTAL_MSG 65536
#define SEND_MSG 16

using namespace std;

//server收发数据
void CServerNet::Run(struct YZMOND *p)
{
	int pid_;
	int newSocket;
	/*侦听本地端口*/
	if (listen(sockfd, 500) == -1)
	{
		perror("listen");
		exit(errno);
	}
	sockaddr_in tcpAddr;
	socklen_t len = sizeof(sockaddr);
	
	while(1)
	{
		newSocket = accept(sockfd, (struct sockaddr*)&tcpAddr, &len);//接收
		if (newSocket != -1)
		{
			pid_ = fork();
			if (pid_ > 0)
			{
				close(newSocket);
			}
			if(pid_ == 0)
				break;
		}
	}
	if(pid_ == 0)
	{
		close(sockfd);
		char clientip[20];
		sprintf(clientip,"%s\0",inet_ntoa(tcpAddr.sin_addr));
		IDENTITY identity;
		LEAST_VERSION least_version;
		AUTHEN authen;
		
		INFO_REQ info_req;     //信息请求
		
		SYSTEM system;         //系统信息
		ALLO allo;             //配置信息
		PID pid;               //进程信息
		ETH0 eth0;             //eth0信息
		ETH1 eth1;             //eth1信息
		USBINFO usbinfo;       //usb口信息
		USBFILE usbfile;       //usb文件列表信息
		PRINT print;           //打印口信息
		PRINTROW printrow;     //打印队列信息
		TERM term;             //终端信息
		SCRINFO scrinfo[50];   //哑终端/IP终端信息
		FINISH finish;         //结束信息
		
		LOG log;
		
		fd_set wfd,rfd;
		int len;
		
		int sendtime = 0;
		int sendlength = 0;
		int sendbuf_len = 0;
		
		int total = 0;//哑终端、IP终端的使用数量
		
		u_char sendbuf[SEND_MSG];
		u_char recvbuf[TOTAL_MSG];
		u_char use_mark[50] = {0}; //记录哑终端，ip终端的位置
		
		struct timeval timeout;
		timeout.tv_sec = p->non_resp_overtime;
		timeout.tv_usec = 0;
		
		bool flag = true;
		while(1)
		{
			FD_ZERO(&wfd);
			FD_ZERO(&rfd);
			if (flag == true)
				FD_SET(newSocket, &wfd);
			FD_SET(newSocket, &rfd);
			
			int sel = select(newSocket + 1, &rfd, &wfd, NULL, &timeout);
			
			if(sel < 0)
			{
				cout<<"select error!"<<endl;
				return;
			}
			else if(sel == 0)
				continue;
			else
			{
				if (FD_ISSET(newSocket, &wfd))//send
				{
					switch(sendtime)
					{
					case 0://发送认证信息
						sendbuf_len = 60;
						identity.prepare_identity(p);
						identity.put_into_sendbuf((char *)&identity,sendbuf);
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 1://发送系统信息请求
						if(authen.check_authen() == 1)
						{
							authen.authen_log(p,p->tmp_packet[3]);
							info_req.system();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						else
						{
							char systemtime[20];
							Get_time(systemtime);
							cout<<systemtime<<" [9715]认证未通过,关闭连接."<<endl;
							close(newSocket);
							exit(0);
						}
						break;
						
					case 2://发送配置信息请求
						info_req.allo();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 3://发送进程信息请求
						info_req.pid();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 4://发送eth0信息请求
						info_req.eth0();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 5://发送eth1信息请求
						info_req.eth1();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 6://发送usb口信息
						if(authen.usbnum != 0)
						{
							info_req.usbinfo();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						break;
						
					case 7://发送usb文件列表信息
						if(authen.usbnum != 0)
						{
							if(usbinfo.use_usb == 1)
							{
								info_req.usbfile();
								info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
								sendbuf_len = 8;
								len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
							}
						}
						break;
					
					case 8://发送打印口信息
						if(authen.prnnum != 0)
						{
							info_req.print();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						break;
						
					case 9://发送打印口队列信息
						if(authen.prnnum != 0)
						{
							if(print.serv == 1 && print.duty != 0)
							{
								info_req.printrow();
								info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
								sendbuf_len = 8;
								len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
							}
						}
						break;
						
					case 10://发送终端信息
						info_req.term();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					default:
						if(sendtime - 10 <= total)//发送哑终端、IP终端信息
						{
							info_req.dump_ip(use_mark[sendtime - 1 - 10]);
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						else//发送结束信息
						{
							info_req.finish();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						break;
					}
					
					if (len <= 0)
					{
						cerr << " S - Failed to send data" << endl;
						close(newSocket);
						break;
					}
					sendlength += len;
					if (sendlength >= sendbuf_len)
					{
						flag = false;
						
						if(sendtime == 6 && authen.usbnum == 0)
							flag = true;
						if(sendtime == 7 && authen.usbnum == 0)
							flag = true;
						if(sendtime == 7 && authen.usbnum != 0 && usbinfo.use_usb == 0)
							flag = true;
						
						if(sendtime == 8 && authen.prnnum == 0)
							flag = true;
						if(sendtime == 9 && authen.prnnum == 0)
							flag = true;
						if(sendtime == 9 && authen.prnnum != 0 && print.serv == 0)
							flag = true;
						if(sendtime == 9 && authen.prnnum != 0 && print.serv == 1 && print.duty == 0)
							flag = true;
						
						if(sendbuf[1] == 0x01)
							log.Write_MainLog_send(sendbuf[1],p,(const char*)(&identity),len, p->tmp_socket[0],p->tmp_socket[2],clientip,use_mark[sendtime - 1 - 10]);
						else
							log.Write_MainLog_send(sendbuf[1],p,(const char*)(&info_req),len, p->dev_socket[0],p->dev_socket[2],clientip,use_mark[sendtime - 1 - 10]);
						
						sendlength = 0;
						sendbuf_len = 0;
						sendtime++;
						memset(sendbuf,0,sizeof(sendbuf));
					}
				}
				
				if(FD_ISSET(newSocket, &rfd))//recv
				{
					len = recv(newSocket, &recvbuf, TOTAL_MSG, 0);
					
					if (len <= 0)
					{
						cerr << " S - Failed to recv data" << endl;
						close(newSocket);
						break;
					}
					
					if(recvbuf[0] != 0x91)//格式非法
					{
						log.Write_Error(p,1);
						close(newSocket);
						exit(0);
					}
					
					switch(recvbuf[1])//处理报文
					{
					case 0x00://版本过低
						if(least_version.put_into_least_version((char*)&least_version,recvbuf,len) == 0)//数据长度错误
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(least_version.check_msg() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							else
							{
								if(least_version.check_version() == 0)
								{
									log.Write_Error(p,1);
									close(newSocket);
									exit(0);
								}
								else
								{
									char systemtime[20];
									Get_time(systemtime);
									cout<<systemtime<<" [9715]版本过低,关闭连接."<<endl;
									close(newSocket);
									exit(0);
								}
							}
						}
						break;
						
					case 0x01://认证信息
						if(authen.put_into_authen((char *)&authen,recvbuf,len) == 0)//格式非法
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(authen.check_authen_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&identity),len, p->tmp_socket[1],p->tmp_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							flag = true;						
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
						
					case 0x02://系统信息
						if(system.put_into_system((char*)&system,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(system.check_system_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&system),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							
							system.system_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
						
					case 0x03://配置信息
						if(allo.put_into_allo((char*)&allo,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(allo.check_allo_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&allo),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							allo.allo_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
						
					case 0x04://进程信息
						if(pid.put_into_pid((char*)&pid,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(pid.check_pid_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&pid),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							pid.pid_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
					
					case 0x05://eth0/eth1信息
						if(recvbuf[4] == 0 && recvbuf[5] == 0)//eth0信息
						{
							if(eth0.put_into_eth0((char*)&eth0,recvbuf,len) == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							else
							{
								if(eth0.check_eth0_head() == 0)
								{
									log.Write_Error(p,1);
									close(newSocket);
									exit(0);
								}
								log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&eth0),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
								eth0.eth0_log(p,clientip,p->dev_packet[3]);
								flag = true;
								memset(recvbuf,0,sizeof(recvbuf));
							}
						}
						else if(recvbuf[4] == 0 && recvbuf[5] == 1)//eth1信息
						{
							if(eth1.put_into_eth1((char*)&eth1,recvbuf,len) == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							else
							{
								if(eth1.check_eth1_head() == 0)
								{
									log.Write_Error(p,1);
									close(newSocket);
									exit(0);
								}
								log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&eth1),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
								eth1.eth1_log(p,clientip,p->dev_packet[3]);
								flag = true;
								memset(recvbuf,0,sizeof(recvbuf));
							}
						}
						else
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						break;
						
					case 0x07://usb口信息
						if(usbinfo.put_into_usbinfo((char*)&usbinfo,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(usbinfo.check_usbinfo_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
						}
						if(authen.usbnum == 0)//处理错误
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&usbinfo),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
						usbinfo.usbinfo_log(p,clientip,p->dev_packet[3]);
						flag = true;
						memset(recvbuf,0,sizeof(recvbuf));
						break;
						
					case 0x0c: //usb文件列表信息
						if(usbfile.put_into_usbfile((char*)&usbfile,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(usbfile.check_usbfile_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}	
						}
						if(authen.usbnum == 0)//usb口不存在错误
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(usbinfo.use_usb == 0)//u盘未插入错误
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
						}
						log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&usbfile),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
						usbfile.usbfile_log(p,clientip,p->dev_packet[3]);
						flag = true;
						memset(recvbuf,0,sizeof(recvbuf));
						break;
						
					case 0x08://打印口信息
						if(print.put_into_print((char*)&print,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(print.check_print_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
						}
						if(authen.prnnum == 0)//打印口不存在错误
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&print),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
						print.print_log(p,clientip,p->dev_packet[3]);
						flag = true;
						memset(recvbuf,0,sizeof(recvbuf));
						break;
					
					case 0x0d: //打印队列信息
						if(printrow.put_into_printrow((char*)&printrow,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(printrow.check_printrow_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
						}
						if(authen.prnnum == 0)//打印口不存在错误
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(print.serv == 0 || (print.serv == 1 && print.duty == 0))//服务未启动或任务数为0
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
						}
						log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&printrow),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
						printrow.printrow_log(p,clientip,p->dev_packet[3]);
						flag = true;
						memset(recvbuf,0,sizeof(recvbuf));
						break;
						
					case 0x09://终端信息
						if(term.put_into_term((char*)&term,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(term.check_term_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&term),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							term.term_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						total = term.ipuse_num(use_mark);
						break;
						
					case 0x0a: //哑终端信息
						if(scrinfo[sendtime - 12].put_into_scrinfo((char*)&scrinfo[sendtime - 12],recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(scrinfo[sendtime - 12].check_scrinfo_head(use_mark[sendtime - 12]) == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&scrinfo[sendtime - 2 - 10]),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							scrinfo[sendtime - 2 - 10].scrinfo_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
						
					case 0x0b: //ip终端信息
						if(scrinfo[sendtime - 12].put_into_scrinfo((char*)&scrinfo[sendtime - 12],recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(scrinfo[sendtime - 12].check_scrinfo_head(use_mark[sendtime - 12]) == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&scrinfo[sendtime - 2 - 10]),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							scrinfo[sendtime - 2 - 10].scrinfo_log(p,clientip,p->dev_packet[3]);
							flag = true;
							memset(recvbuf,0,sizeof(recvbuf));
						}
						break;
						
					case 0xff: //结束信息
						if(finish.put_into_finish((char*)&finish,recvbuf,len) == 0)
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(finish.check_finish_head() == 0)
							{
								log.Write_Error(p,1);
								close(newSocket);
								exit(0);
							}
							log.Write_MainLog_recv(recvbuf[1],p,(const char*)(&finish),len, p->dev_socket[1],p->dev_socket[3],clientip,use_mark[sendtime - 2 - 10]);
							memset(recvbuf,0,sizeof(recvbuf));
							
							Write_database(//写数据库
							p,
							&authen,
							&system,
							&allo,             
							&pid,               
							&eth0,             
							&eth1,             
							&usbinfo,      
							&usbfile,       
							&print,           
							&printrow,     
							&term,             
							scrinfo,  
							&finish,
							total,
							clientip);
							
							finish.finish_log(p,clientip,p->dev_packet[3],authen.struc_num);
							
							close(newSocket);
							exit(0);
						}
						break;
					
					default:
						log.Write_Error(p,1);
						close(newSocket);
						exit(0);
					}
				}
			}
		}
	}
}
