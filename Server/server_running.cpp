#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include "server.h"

#define TOTAL_MSG 65536
#define SEND_MSG 16

using namespace std;

//server�շ�����
void CServerNet::Run(struct YZMOND *p)
{
	int pid_;
	int newSocket;
	/*�������ض˿�*/
	if (listen(sockfd, 500) == -1)
	{
		perror("listen");
		exit(errno);
	}
	sockaddr_in tcpAddr;
	socklen_t len = sizeof(sockaddr);
	
	while(1)
	{
		newSocket = accept(sockfd, (struct sockaddr*)&tcpAddr, &len);//����
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
		
		INFO_REQ info_req;     //��Ϣ����
		
		SYSTEM system;         //ϵͳ��Ϣ
		ALLO allo;             //������Ϣ
		PID pid;               //������Ϣ
		ETH0 eth0;             //eth0��Ϣ
		ETH1 eth1;             //eth1��Ϣ
		USBINFO usbinfo;       //usb����Ϣ
		USBFILE usbfile;       //usb�ļ��б���Ϣ
		PRINT print;           //��ӡ����Ϣ
		PRINTROW printrow;     //��ӡ������Ϣ
		TERM term;             //�ն���Ϣ
		SCRINFO scrinfo[50];   //���ն�/IP�ն���Ϣ
		FINISH finish;         //������Ϣ
		
		LOG log;
		
		fd_set wfd,rfd;
		int len;
		
		int sendtime = 0;
		int sendlength = 0;
		int sendbuf_len = 0;
		
		int total = 0;//���նˡ�IP�ն˵�ʹ������
		
		u_char sendbuf[SEND_MSG];
		u_char recvbuf[TOTAL_MSG];
		u_char use_mark[50] = {0}; //��¼���նˣ�ip�ն˵�λ��
		
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
					case 0://������֤��Ϣ
						sendbuf_len = 60;
						identity.prepare_identity(p);
						identity.put_into_sendbuf((char *)&identity,sendbuf);
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 1://����ϵͳ��Ϣ����
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
							cout<<systemtime<<" [9715]��֤δͨ��,�ر�����."<<endl;
							close(newSocket);
							exit(0);
						}
						break;
						
					case 2://����������Ϣ����
						info_req.allo();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 3://���ͽ�����Ϣ����
						info_req.pid();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 4://����eth0��Ϣ����
						info_req.eth0();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 5://����eth1��Ϣ����
						info_req.eth1();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					case 6://����usb����Ϣ
						if(authen.usbnum != 0)
						{
							info_req.usbinfo();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						break;
						
					case 7://����usb�ļ��б���Ϣ
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
					
					case 8://���ʹ�ӡ����Ϣ
						if(authen.prnnum != 0)
						{
							info_req.print();
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						break;
						
					case 9://���ʹ�ӡ�ڶ�����Ϣ
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
						
					case 10://�����ն���Ϣ
						info_req.term();
						info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
						sendbuf_len = 8;
						len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						break;
						
					default:
						if(sendtime - 10 <= total)//�������նˡ�IP�ն���Ϣ
						{
							info_req.dump_ip(use_mark[sendtime - 1 - 10]);
							info_req.put_info_into_sendbuf((char*)&info_req,sendbuf);
							sendbuf_len = 8;
							len = send(newSocket, &sendbuf[sendlength],sendbuf_len - sendlength,0);
						}
						else//���ͽ�����Ϣ
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
					
					if(recvbuf[0] != 0x91)//��ʽ�Ƿ�
					{
						log.Write_Error(p,1);
						close(newSocket);
						exit(0);
					}
					
					switch(recvbuf[1])//������
					{
					case 0x00://�汾����
						if(least_version.put_into_least_version((char*)&least_version,recvbuf,len) == 0)//���ݳ��ȴ���
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
									cout<<systemtime<<" [9715]�汾����,�ر�����."<<endl;
									close(newSocket);
									exit(0);
								}
							}
						}
						break;
						
					case 0x01://��֤��Ϣ
						if(authen.put_into_authen((char *)&authen,recvbuf,len) == 0)//��ʽ�Ƿ�
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
						
					case 0x02://ϵͳ��Ϣ
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
						
					case 0x03://������Ϣ
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
						
					case 0x04://������Ϣ
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
					
					case 0x05://eth0/eth1��Ϣ
						if(recvbuf[4] == 0 && recvbuf[5] == 0)//eth0��Ϣ
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
						else if(recvbuf[4] == 0 && recvbuf[5] == 1)//eth1��Ϣ
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
						
					case 0x07://usb����Ϣ
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
						if(authen.usbnum == 0)//�������
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
						
					case 0x0c: //usb�ļ��б���Ϣ
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
						if(authen.usbnum == 0)//usb�ڲ����ڴ���
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(usbinfo.use_usb == 0)//u��δ�������
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
						
					case 0x08://��ӡ����Ϣ
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
						if(authen.prnnum == 0)//��ӡ�ڲ����ڴ���
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
					
					case 0x0d: //��ӡ������Ϣ
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
						if(authen.prnnum == 0)//��ӡ�ڲ����ڴ���
						{
							log.Write_Error(p,1);
							close(newSocket);
							exit(0);
						}
						else
						{
							if(print.serv == 0 || (print.serv == 1 && print.duty == 0))//����δ������������Ϊ0
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
						
					case 0x09://�ն���Ϣ
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
						
					case 0x0a: //���ն���Ϣ
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
						
					case 0x0b: //ip�ն���Ϣ
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
						
					case 0xff: //������Ϣ
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
							
							Write_database(//д���ݿ�
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
