#include <iostream>
#include <fstream>
#include "server.h"
#include "encrpty.h"

using namespace std;

//ȡϵͳʱ��
void CServerNet::Get_time(char systemtime[])
{
	time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ��cuo
	tm* t= localtime(&tt);
	sprintf(systemtime,"%d-%02d-%02d %02d:%02d:%02d\0", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}

void CServerNet::change_time(const time_t input_time,char change[])
{
	struct tm *t;
	t = localtime(&input_time);
	sprintf(change,"%d-%02d-%02d %02d:%02d:%02d\0", 
  t->tm_year + 1900,
  t->tm_mon + 1,
  t->tm_mday,
  t->tm_hour,
  t->tm_min,
  t->tm_sec);
}

//��ȡ�����ļ�
void CServerNet::Read_yzmond(struct YZMOND *p)
{
	char ch[200];
	char key[50],value[50];
	char *token;
	ifstream in;
	in.open("yzmond.conf");
	if (!in.is_open())
	{
		printf("open error!\n");
		return;
	}
	while (in.getline(ch, 100))
	{
		token = strstr(ch, "/");
		if (token)
			*token = '\0';
		token = strstr(ch, "=");
		if (token)
			*token = ' ';
		sscanf(ch, "%s %s", key, value);
		
		if (strcmp(key, "�����˿ں�") == 0)
			p->port = atoi(value);
		else if (strcmp(key, "�豸���Ӽ��") == 0)
			p->connect_interval = atoi(value);
		else if (strcmp(key, "�豸�������") == 0)
			p->sample_interval = atoi(value);
		
		else if (strcmp(key, "������IP��ַ") == 0)
			strcpy(p->svr_ip,value);
		else if (strcmp(key, "�������˿ں�") == 0)
			p->svr_port = atoi(value);
		else if (strcmp(key, "���ݿ���") == 0)
			strcpy(p->name,value);
		else if (strcmp(key, "�û���") == 0)
			strcpy(p->user,value);
		else if (strcmp(key, "�û�����") == 0)
			strcpy(p->paswd,value);
		
		else if (strcmp(key, "δӦ��ʱ") == 0)
			p->non_resp_overtime = atoi(value);
		else if (strcmp(key, "���䳬ʱ") == 0)
			p->trans_overtime = atoi(value);
		else if (strcmp(key, "����־��С") == 0)
			p->mainlog_size = atoi(value);
		else if (strcmp(key, "����־��С") == 0)
			p->sublog_size = atoi(value);
		
		else if (strcmp(key, "��Ļ��ʾ") == 0)
			p->show_screen = atoi(value);
		else if (strcmp(key, "tmp_packet") == 0)
			strcpy(p->tmp_packet,value);
		else if (strcmp(key, "tmp_socket") == 0)
			strcpy(p->tmp_socket,value);
		else if (strcmp(key, "dev_packet") == 0)
			strcpy(p->dev_packet,value);
		else if (strcmp(key, "dev_socket") == 0)
			strcpy(p->dev_socket,value);
	}
	in.close();
}

//������
void CServerNet::NonBlock()
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

//��ʼ��
int CServerNet::Init(int port)
{
	int rlt = 0;
	struct sockaddr_in s_addr;
	
	/*����socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		rlt = 1;
		return rlt;
	}
	
	/*�ö˿�����*/
	opt = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt)) < 0)
	{
		cout <<"errno = "<<errno<<"("<<strerror(errno)<<")"<< endl;
		close(sockfd);
		rlt = 2;
		return rlt;
	}
	
	NonBlock();//������
	
	/*���÷�����ip*/
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/*�ѵ�ַ�Ͷ˿ڰ󶨵��׽�����*/
	if ((bind(sockfd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("bind");
		rlt = 3;
		return rlt;
	}
	return rlt;
}

/***********************************************/

//������֤��
void IDENTITY::encryption()
{
	int pos = random_num % 4093;
	int mark[32];
	for(int i = 0;i < 32;i++)
	{
		mark[i] = i + pos;
		if(mark[i] >= 4096)
			mark[i] = (i + pos) % 4096;
	}
	for(int i = 0;i < 32;i++)
		authstring[i] = authstring[i] ^ secret[mark[i]];
}

//��֤����
void IDENTITY::prepare_identity(struct YZMOND *p)
{
	head[0] = 0x11;
	head[1] = 0x01;
	msglen = sizeof(IDENTITY);
	second = 0x0000;
	datalen = msglen - 8;
	version = 5;
	subversion[0] = 0x00;
	subversion[1] = 0x00;
	connect_interval = p->connect_interval;
	sample_interval = p->sample_interval;
	blank = 0x00;
	strcpy((char*)authstring,"yzmond:id*str&to!tongji@by#Auth^");
	random_num = (u_int)rand();
	svr_time = (u_int)time(0);
	svr_time = svr_time ^ (u_int)0xFFFFFFFF;
	
	encryption();//������֤��
	
	msglen = htons(msglen);
	second = htons(second);
	datalen = htons(datalen);
	version = htons(version);
	connect_interval = htons(connect_interval);
	sample_interval = htons(sample_interval);
	random_num = htonl(random_num);
	svr_time = htonl(svr_time);
}

//��֤�������sendbuf
void IDENTITY::put_into_sendbuf(char *p,u_char sendbuf[])
{
	int i;
	for(i = 0;i < 60;i++)
		sendbuf[i] = p[i];
}

/**********************************************/

//recvbuf����least_version��
bool LEAST_VERSION::put_into_least_version(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//����ʽ
bool LEAST_VERSION::check_msg()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//���汾
bool LEAST_VERSION::check_version()
{
	version = ntohs(version);
	if(version < 2)
		return 0;
	else 
		return 1;
}

/***************************************************/

//��֤�Ƿ�ͨ��
bool AUTHEN::check_authen()
{
	int i,j;
	int k = 0;
	int pos = ntohl(random_num) % 4093;
	int mark[104];
	for(i = 0;i < 104;i++)
	{
		mark[i] = i + pos;
		if(mark[i] >= 4096)
			mark[i] = (i + pos) % 4096;
	}
	char *p = (char*)this;
	for(int i = 8;i < 112;i++)
		p[i] = p[i] ^ secret[mark[i - 8]];
	
	if(strncmp((char*)authstring,"yzmond:id*str&to!tongji@by#Auth^",32) == 0)
		return 1; //ͨ��
	else
		return 0; //δͨ��
}

//����authen��
bool AUTHEN::put_into_authen(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//�����֤����ʽ
bool AUTHEN::check_authen_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//log
void AUTHEN::authen_log(struct YZMOND *p,int type)
{
	
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout <<	"  �豸�ͺţ�"<<type<<" ���кţ�"<<g_seq<<" ����汾��"<<version_num<<endl;
		cout<<"  CPU��Ƶ��"<<ntohs(cpu_mhz)<<" �ڴ棺"<<ntohs(ram)<<" FLASH��"<<ntohs(flash)<<endl;
		cout<<"  ��̫�ڣ�"<<(int)ethnum<<" ͬ���ڣ�"<<(int)syncnum;
		cout<<" �첽�ڣ�"<<(int)asyncnum<<" �������ڣ�"<<(int)switchnum;
		cout<<" USB�ڣ�";
		if(usbnum == 0)
			cout<<"������";
		else
			cout<<"����";
		cout<<" ��ӡ�ڣ�";
		if(prnnum == 0)
			cout<<"������";
		else
			cout<<"����";
		cout<<endl;
		cout<<"  �����ţ�"<<ntohl(struc_num)<<" ���������кţ�"<<(int)stru_seq<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in <<	"  �豸�ͺţ�"<<type<<" ���кţ�"<<g_seq<<" ����汾��"<<version_num<<endl;
			in<<"  CPU��Ƶ��"<<ntohs(cpu_mhz)<<" �ڴ棺"<<ntohs(ram)<<" FLASH��"<<ntohs(flash)<<endl;
			in<<"  ��̫�ڣ�"<<(int)ethnum<<" ͬ���ڣ�"<<(int)syncnum;
			in<<" �첽�ڣ�"<<(int)asyncnum<<" �������ڣ�"<<(int)switchnum;
			in<<" USB�ڣ�";
			if(usbnum == 0)
				in<<"������";
			else
				in<<"����";
			in<<" ��ӡ�ڣ�";
			if(prnnum == 0)
				in<<"������";
			else
				in<<"����";
			in<<endl;
			in<<"  �����ţ�"<<ntohl(struc_num)<<" ���������кţ�"<<(int)stru_seq<<endl;
		}
	}
	in.close();
}

/*********��֤ͨ����*************/

//������Ϣ����sendbuf
void INFO_REQ::put_info_into_sendbuf(char *p,u_char sendbuf[])
{
	int i;
	for(i = 0;i < 8;i++)
		sendbuf[i] = p[i];
}

/************************************************/

//recvbuf����system��
bool SYSTEM::put_into_system(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���system��ʽ
bool SYSTEM::check_system_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//log
void SYSTEM::system_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�ϵͳ��Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout <<	"  cpu_user_time��"<<dec<<ntohl(user_cpu_time)<<" cpu_nice_time��"<<dec<<ntohl(nice_cpu_time);
		cout<<" cpu_sys_time��"<<dec<<ntohl(system_cpu_time)<<endl;
		cout<<"  cpu_idle_time��"<<dec<<ntohl(idle_cpu_time)<<" freed_memory��"<<dec<<ntohl(freed_memory)<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�ϵͳ��Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in <<	"  cpu_user_time��"<<dec<<ntohl(user_cpu_time)<<" cpu_nice_time��"<<dec<<ntohl(nice_cpu_time);
			in<<" cpu_sys_time��"<<dec<<ntohl(system_cpu_time)<<endl;
			in<<"  cpu_idle_time��"<<dec<<ntohl(idle_cpu_time)<<" freed_memory��"<<dec<<ntohl(freed_memory)<<endl;
		}
	}
	in.close();
}

/**********************************************/

//recvbuf����allo��
bool ALLO::put_into_allo(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}
	
//���allo��ʽ
bool ALLO::check_allo_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}	


//log
void ALLO::allo_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�������Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		for(int i = 0;i < strlen((char*)info_allo);i++)
		{
			if(info_allo[i] == '\r')
				cout<<endl;
			cout<<info_allo[i];
		}
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�������Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			for(int i = 0;i < strlen((char*)info_allo);i++)
			{
				if(info_allo[i] == '\r')
					in<<"  ";
				in<<info_allo[i];
			}
		}
	}
	in.close();
}

/************************************************/

//recvbuf����pid��
bool PID::put_into_pid(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���pid��ʽ
bool PID::check_pid_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

void PID::pid_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�������Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<info_pid<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�������Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in <<info_pid<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf����eth0��
bool ETH0::put_into_eth0(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���eth0��ʽ
bool ETH0::check_eth0_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//log
void ETH0::eth0_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�Ethernet0��Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"  EO��"<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
		cout<<"    ";
		for(int i = 0;i < 6;i++)
		{
			if(i == 5)
				cout<<hex<<(int)mac[i];
			else
				cout<<hex<<(int)mac[i]<<":";
		}
		cout<<" ";
		if(updown == 1)
			cout<<"Down    ";
		else
			cout<<"Up    ";
		int a = ntohs(options);
		if(a == 0x0000 || a == 0x0004)
			cout<<"10MB/��˫��";
		else if(a == 0x0002 || a == 0x0006)
			cout<<"10MB/ȫ˫��";
		else if(a == 0x0001 || a == 0x0005)
			cout<<"100MB/��˫��";
		else if(a == 0x0003 || a == 0x0007)
			cout<<"100MB/ȫ˫��";
		cout<<endl;
		cout<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
		cout<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�Ethernet0��Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in<<"  EO��"<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
			in<<"    ";
			for(int i = 0;i < 6;i++)
			{
				if(i == 5)
					in<<hex<<(int)mac[i];
				else
					in<<hex<<(int)mac[i]<<":";
			}
			in<<" ";
			if(updown == 1)
				in<<"Down    ";
			else
				in<<"Up    ";
			int a = ntohs(options);
			if(a == 0x0000 || a == 0x0004)
				in<<"10MB/��˫��";
			else if(a == 0x0002 || a == 0x0006)
				in<<"10MB/ȫ˫��";
			else if(a == 0x0001 || a == 0x0005)
				in<<"100MB/��˫��";
			else if(a == 0x0003 || a == 0x0007)
				in<<"100MB/ȫ˫��";
			in<<endl;
			in<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
			in<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf����eth1��
bool ETH1::put_into_eth1(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���eth1��ʽ
bool ETH1::check_eth1_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0001)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//log
void ETH1::eth1_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�Ethernet1��Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"  EO��"<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
		cout<<"    ";
		for(int i = 0;i < 6;i++)
		{
			if(i == 5)
				cout<<hex<<(int)mac[i];
			else
				cout<<hex<<(int)mac[i]<<":";
		}
		cout<<" ";
		if(updown == 1)
			cout<<"Down    ";
		else
			cout<<"Up    ";
		int a = ntohs(options);
		if(a == 0x0000 || a == 0x0004)
			cout<<"10MB/��˫��";
		else if(a == 0x0002 || a == 0x0006)
			cout<<"10MB/ȫ˫��";
		else if(a == 0x0001 || a == 0x0005)
			cout<<"100MB/��˫��";
		else if(a == 0x0003 || a == 0x0007)
			cout<<"100MB/ȫ˫��";
		cout<<endl;
		cout<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
		cout<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�Ethernet1��Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in<<"  EO��"<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
			in<<"    ";
			for(int i = 0;i < 6;i++)
			{
				if(i == 5)
					in<<hex<<(int)mac[i];
				else
					in<<hex<<(int)mac[i]<<":";
			}
			in<<" ";
			if(updown == 1)
				in<<"Down    ";
			else
				in<<"Up    ";
			int a = ntohs(options);
			if(a == 0x0000 || a == 0x0004)
				in<<"10MB/��˫��";
			else if(a == 0x0002 || a == 0x0006)
				in<<"10MB/ȫ˫��";
			else if(a == 0x0001 || a == 0x0005)
				in<<"100MB/��˫��";
			else if(a == 0x0003 || a == 0x0007)
				in<<"100MB/ȫ˫��";
			in<<endl;
			in<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
			in<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
		}
	}
	in.close();
}

/***************************************************/

//recvbuf����usbinfo��
bool USBINFO::put_into_usbinfo(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���usbinfo��ʽ
bool USBINFO::check_usbinfo_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

void USBINFO::usbinfo_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�U����Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout << "  U��״̬��";
		if(use_usb == 1)
			cout<<"�Ѳ���";
		else
			cout<<"δ����";
		cout<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�U����Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in << "  U��״̬��";
			if(use_usb == 1)
				in<<"�Ѳ���";
			else
				in<<"δ����";
			in<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf����usbfile
bool USBFILE::put_into_usbfile(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���usbfile��ʽ
bool USBFILE::check_usbfile_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

void USBFILE::usbfile_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�U���ļ��б���Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		for(int i = 0;i < strlen((char*)usb_file);i++)
		{
			cout<<usb_file[i];
		}
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�U���ļ��б���Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			for(int i = 0;i < strlen((char*)usb_file);i++)
			{
				in<<usb_file[i];
			}
		}
	}
	in.close();
}

/****************************************************/

//recvbuf����print
bool PRINT::put_into_print(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���print��ʽ
bool PRINT::check_print_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

void PRINT::print_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ���ӡ����Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"  ��ӡ�����ƣ�"<< name<<endl;
		cout<<"  ����״̬��";
		if(serv == 0)
			cout<<"δ����";
		else 
			cout<<"������";
		cout<<"  ";
		cout<<"������������������"<<dec<<ntohs(duty)<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ���ӡ����Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in<<"  ��ӡ�����ƣ�"<< name<<endl;
			in<<"  ����״̬��";
			if(serv == 0)
				in<<"δ����";
			else 
				in<<"������";
			in<<"  ";
			in<<"������������������"<<dec<<ntohs(duty)<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf����printrow
bool PRINTROW::put_into_printrow(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���printrow��ʽ
bool PRINTROW::check_printrow_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

void PRINTROW::printrow_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ���ӡ������Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"  ��ӡ������Ϣ��"<< endl;
		for(int i = 0;i < strlen((char*)print_info);i++)
		{
			cout<<print_info[i];
		}
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ���ӡ������Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			cout<<"  ��ӡ������Ϣ��"<< endl;
			for(int i = 0;i < strlen((char*)print_info);i++)
			{
				in<<print_info[i];
			}
		}
	}
	in.close();
}

/***************************************************/

//recvbuf����term
bool TERM::put_into_term(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���term��ʽ
bool TERM::check_term_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}

//ͳ��ip�����ն�ʹ�ø���
int TERM::ipuse_num(u_char use_mark[])
{
	int t = 0;
	int i;
	for(i = 0;i < 16;i++)
	{
		if(dumb_term[i] == 1)
		{
			use_mark[t] = i;
			t++;
		}
	}
	for(i = 0;i < 254;i++)
	{
		if(ip_term[i] == 1)
		{
			use_mark[t] = i + 16;
			t++;
		}
	}
	return t;
}

void TERM::term_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�TServer��Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"�����ն�������"<<ntohs(term_num)<<endl;
		for(int i = 0;i < 16;i++)
		{
			if(dumb_term[i] == 1)
				cout<<"  ���ն�"<<dec<<(i + 1)<<endl;
		}
		for(int i = 0;i < 254;i++)
		{
			if(ip_term[i] == 1)
				cout<<"  IP�ն�"<<dec<<(i + 1)<<endl;
		}
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�TServer��Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in <<"�����ն�������"<<ntohs(term_num)<<endl;
			for(int i = 0;i < 16;i++)
			{
				if(dumb_term[i] == 1)
					in <<"  ���ն�"<<dec<<(i + 1)<<endl;
			}
			for(int i = 0;i < 254;i++)
			{
				if(ip_term[i] == 1)
					in <<"  IP�ն�"<<dec<<(i + 1)<<endl;
			}
		}
	}
	in.close();
}

/************************************************/

//recvbuf����scrinfo
bool SCRINFO::put_into_scrinfo(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���scrinfo��ʽ
bool SCRINFO::check_scrinfo_head(u_char mark)
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(b != c - 8)
		return 0;
	else
	{
		if(head[1] == 0x0a)
		{
			if(mark >= 0 && mark < 16)
			{
				if(a == (u_short)(mark + 1))
					return 1;
				else
					return 0;
			}
			else
				return 0;
		}
		else
		{
			if(mark >= 16 && mark < 270)
			{
				if(a == (u_short)(mark + 1 - 16))
					return 1;
				else
					return 0;
			}
			else
				return 0;
		}
	}
}

void SCRINFO::scrinfo_log(struct YZMOND *p,char clientip[],int type)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		if(head[1] == 0x0a)
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ����ն�"<<dec<< ntohs(second) <<"��Ϣ."<< endl;
		else
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�IP�ն�"<<dec<< ntohs(second) <<"��Ϣ."<< endl;
		cout << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
		cout<<"  �ն˺ţ�"<<(int)port_num<<"("<<(int)port_num<<") ";
		cout<<"����������"<<(int)scrtotal<<"  ";
		cout<<"���ͣ�"<<term_type<<" ";
		cout<<"״̬��"<<term_stat<<" ";
		cout<<"IP��"<<(int)ipterm[0]<<"."<<(int)ipterm[1]<<"."<<(int)ipterm[2]<<"."<<(int)ipterm[3]<<endl;
		for(int i = 0;i < (int)scrtotal;i++)
		{
			cout << systemtime <<" [9175] --�����ţ�";
			cout<<(int)everyscr[i].scrcode<<"  ";
			char change[50];
			s.change_time(ntohl(everyscr[i].local_time),change);
			cout<<"����ʱ�䣺"<< change <<"  ";
			cout<<"��ʾ����"<<everyscr[i].scr_mention<<endl;
			
			cout<<"  ������IP��"<<(int)everyscr[i].svrip[0]<<"."<<(int)everyscr[i].svrip[1]<<"."<<(int)everyscr[i].svrip[2]<<"."<<(int)everyscr[i].svrip[3]<<"  ";
			cout<<"�˿ڣ�"<<dec<<ntohs(everyscr[i].tcp_svrport)<<"  ";
			cout<<"Э�飺"<<everyscr[i].scr_proto<<"  ";
			cout<<"״̬��"<<everyscr[i].scr_stat<<"  ";
			cout<<"���ͣ�"<<everyscr[i].scrterm_type<<endl;
			
			cout<<"  ���նˣ�"<< dec << ntohl(everyscr[i].term_transbyte) <<"    ";
			cout<<"���նˣ�"<< dec << ntohl(everyscr[i].term_recvbyte) <<"    ";
			cout<<"����������"<< dec << ntohl(everyscr[i].svr_transbyte) <<"    ";
			cout<<"�շ�������"<< dec << ntohl(everyscr[i].svr_recvbyte) <<endl;
			
			cout<<"  ping��С��"<< dec << ntohl(everyscr[i].pingmin) <<"  ";
			cout<<"pingƽ����"<< dec << ntohl(everyscr[i].pingavg) <<"  ";
			cout<<"ping���"<< dec << ntohl(everyscr[i].pingmax) <<endl;
		}
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			if(head[1] == 0x0a)
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ����ն�"<<dec<< ntohs(second) <<"��Ϣ."<< endl;
			else
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ�IP�ն�"<<dec<< ntohs(second) <<"��Ϣ."<< endl;
			in << systemtime <<" [9175](�յ�����Ϊ��)"<<endl;
			in<<"  �ն˺ţ�"<<(int)port_num<<"("<<(int)port_num<<") ";
			in<<"����������"<<(int)scrtotal<<"  ";
			in<<"���ͣ�"<<term_type<<" ";
			in<<"״̬��"<<term_stat<<" ";
			in<<"IP��"<<(int)ipterm[0]<<"."<<(int)ipterm[1]<<"."<<(int)ipterm[2]<<"."<<(int)ipterm[3]<<endl;
			for(int i = 0;i < (int)scrtotal;i++)
			{
				in << systemtime <<" [9175] --�����ţ�";
				in<<(int)everyscr[i].scrcode<<"  ";
				char change[50];
				s.change_time(ntohl(everyscr[i].local_time),change);
				in <<"����ʱ�䣺"<< change <<"  ";
				in<<"��ʾ����"<<everyscr[i].scr_mention<<endl;
			
				in<<"  ������IP��"<<(int)everyscr[i].svrip[0]<<"."<<(int)everyscr[i].svrip[1]<<"."<<(int)everyscr[i].svrip[2]<<"."<<(int)everyscr[i].svrip[3]<<"  ";
				in<<"�˿ڣ�"<<dec<<ntohs(everyscr[i].tcp_svrport)<<"  ";
				in<<"Э�飺"<<everyscr[i].scr_proto<<"  ";
				in<<"״̬��"<<everyscr[i].scr_stat<<"  ";
				in<<"���ͣ�"<<everyscr[i].scrterm_type<<endl;
			
				in<<"  ���նˣ�"<< dec << ntohl(everyscr[i].term_transbyte) <<"    ";
				in<<"���նˣ�"<< dec << ntohl(everyscr[i].term_recvbyte) <<"    ";
				in<<"����������"<< dec << ntohl(everyscr[i].svr_transbyte) <<"    ";
				in<<"�շ�������"<< dec << ntohl(everyscr[i].svr_recvbyte) <<endl;
			
				in<<"  ping��С��"<< dec << ntohl(everyscr[i].pingmin) <<"  ";
				in<<"pingƽ����"<< dec << ntohl(everyscr[i].pingavg) <<"  ";
				in<<"ping���"<< dec << ntohl(everyscr[i].pingmax) <<endl;
			}
		}
	}
	in.close();
}

/************************************************/

//recvbuf����finish
bool FINISH::put_into_finish(char *p,u_char recvbuf[],int len)
{
	p[2] = recvbuf[2];
	p[3] = recvbuf[3];
	if(len == ntohs(msglen))
	{
		int i;
		for(i = 0;i < len;i++)
			p[i] = recvbuf[i];
		return 1;
	}
	else
		return 0;
}

//���finish��ʽ
bool FINISH::check_finish_head()
{
	u_short a = ntohs(second);
	u_short b = ntohs(datalen);
	u_short c = ntohs(msglen);
	if(a != 0x0000)
		return 0;
	else
	{
		if(b != c - 8)
			return 0;
		else
			return 1;
	}
}
	
void FINISH::finish_log(struct YZMOND *p,char clientip[],int type,u_int struc_num)
{
	CServerNet s;
	char systemtime[20];
	s.Get_time(systemtime);
	if(p->show_screen == 1)
	{
		cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ����ν������(deptid="<<dec<< struc_num <<")."<< endl;
		cout << systemtime <<" [9175]�ر�������ܻ�����"<<endl;
	}
	FILE *fp;
	fp = fopen("yzmond.log","a");
	if(fp == NULL)
	{
		cout<<"open error!"<<endl;
		return;
	}
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	
	if(size < p->mainlog_size * 1024)
	{
		if(type == '1')
		{
			in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�յ����ν������(deptid="<<dec<< struc_num <<")."<< endl;
			in << systemtime <<" [9175]�ر�������ܻ�����"<<endl;
		}
	}
	in.close();
}
	
	
	
	
	
	
	
	


