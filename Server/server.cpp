#include <iostream>
#include <fstream>
#include "server.h"
#include "encrpty.h"

using namespace std;

//取系统时间
void CServerNet::Get_time(char systemtime[])
{
	time_t tt = time(NULL);//这句返回的只是一个时间cuo
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

//读取配置文件
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
		
		if (strcmp(key, "监听端口号") == 0)
			p->port = atoi(value);
		else if (strcmp(key, "设备连接间隔") == 0)
			p->connect_interval = atoi(value);
		else if (strcmp(key, "设备采样间隔") == 0)
			p->sample_interval = atoi(value);
		
		else if (strcmp(key, "服务器IP地址") == 0)
			strcpy(p->svr_ip,value);
		else if (strcmp(key, "服务器端口号") == 0)
			p->svr_port = atoi(value);
		else if (strcmp(key, "数据库名") == 0)
			strcpy(p->name,value);
		else if (strcmp(key, "用户名") == 0)
			strcpy(p->user,value);
		else if (strcmp(key, "用户口令") == 0)
			strcpy(p->paswd,value);
		
		else if (strcmp(key, "未应答超时") == 0)
			p->non_resp_overtime = atoi(value);
		else if (strcmp(key, "传输超时") == 0)
			p->trans_overtime = atoi(value);
		else if (strcmp(key, "主日志大小") == 0)
			p->mainlog_size = atoi(value);
		else if (strcmp(key, "分日志大小") == 0)
			p->sublog_size = atoi(value);
		
		else if (strcmp(key, "屏幕显示") == 0)
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

//非阻塞
void CServerNet::NonBlock()
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

//初始化
int CServerNet::Init(int port)
{
	int rlt = 0;
	struct sockaddr_in s_addr;
	
	/*建立socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		rlt = 1;
		return rlt;
	}
	
	/*置端口重用*/
	opt = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt)) < 0)
	{
		cout <<"errno = "<<errno<<"("<<strerror(errno)<<")"<< endl;
		close(sockfd);
		rlt = 2;
		return rlt;
	}
	
	NonBlock();//非阻塞
	
	/*设置服务器ip*/
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/*把地址和端口绑定到套接字上*/
	if ((bind(sockfd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("bind");
		rlt = 3;
		return rlt;
	}
	return rlt;
}

/***********************************************/

//加密认证串
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

//认证请求
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
	
	encryption();//加密认证串
	
	msglen = htons(msglen);
	second = htons(second);
	datalen = htons(datalen);
	version = htons(version);
	connect_interval = htons(connect_interval);
	sample_interval = htons(sample_interval);
	random_num = htonl(random_num);
	svr_time = htonl(svr_time);
}

//认证请求放入sendbuf
void IDENTITY::put_into_sendbuf(char *p,u_char sendbuf[])
{
	int i;
	for(i = 0;i < 60;i++)
		sendbuf[i] = p[i];
}

/**********************************************/

//recvbuf放入least_version包
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

//检查格式
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

//检查版本
bool LEAST_VERSION::check_version()
{
	version = ntohs(version);
	if(version < 2)
		return 0;
	else 
		return 1;
}

/***************************************************/

//认证是否通过
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
		return 1; //通过
	else
		return 0; //未通过
}

//放入authen包
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

//检查认证串格式
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
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout <<	"  设备型号："<<type<<" 序列号："<<g_seq<<" 软件版本："<<version_num<<endl;
		cout<<"  CPU主频："<<ntohs(cpu_mhz)<<" 内存："<<ntohs(ram)<<" FLASH："<<ntohs(flash)<<endl;
		cout<<"  以太口："<<(int)ethnum<<" 同步口："<<(int)syncnum;
		cout<<" 异步口："<<(int)asyncnum<<" 交换机口："<<(int)switchnum;
		cout<<" USB口：";
		if(usbnum == 0)
			cout<<"不存在";
		else
			cout<<"存在";
		cout<<" 打印口：";
		if(prnnum == 0)
			cout<<"不存在";
		else
			cout<<"存在";
		cout<<endl;
		cout<<"  机构号："<<ntohl(struc_num)<<" 机构内序列号："<<(int)stru_seq<<endl;
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
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in <<	"  设备型号："<<type<<" 序列号："<<g_seq<<" 软件版本："<<version_num<<endl;
			in<<"  CPU主频："<<ntohs(cpu_mhz)<<" 内存："<<ntohs(ram)<<" FLASH："<<ntohs(flash)<<endl;
			in<<"  以太口："<<(int)ethnum<<" 同步口："<<(int)syncnum;
			in<<" 异步口："<<(int)asyncnum<<" 交换机口："<<(int)switchnum;
			in<<" USB口：";
			if(usbnum == 0)
				in<<"不存在";
			else
				in<<"存在";
			in<<" 打印口：";
			if(prnnum == 0)
				in<<"不存在";
			else
				in<<"存在";
			in<<endl;
			in<<"  机构号："<<ntohl(struc_num)<<" 机构内序列号："<<(int)stru_seq<<endl;
		}
	}
	in.close();
}

/*********认证通过后*************/

//请求信息放入sendbuf
void INFO_REQ::put_info_into_sendbuf(char *p,u_char sendbuf[])
{
	int i;
	for(i = 0;i < 8;i++)
		sendbuf[i] = p[i];
}

/************************************************/

//recvbuf放入system包
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

//检查system格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到系统信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout <<	"  cpu_user_time："<<dec<<ntohl(user_cpu_time)<<" cpu_nice_time："<<dec<<ntohl(nice_cpu_time);
		cout<<" cpu_sys_time："<<dec<<ntohl(system_cpu_time)<<endl;
		cout<<"  cpu_idle_time："<<dec<<ntohl(idle_cpu_time)<<" freed_memory："<<dec<<ntohl(freed_memory)<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到系统信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in <<	"  cpu_user_time："<<dec<<ntohl(user_cpu_time)<<" cpu_nice_time："<<dec<<ntohl(nice_cpu_time);
			in<<" cpu_sys_time："<<dec<<ntohl(system_cpu_time)<<endl;
			in<<"  cpu_idle_time："<<dec<<ntohl(idle_cpu_time)<<" freed_memory："<<dec<<ntohl(freed_memory)<<endl;
		}
	}
	in.close();
}

/**********************************************/

//recvbuf放入allo包
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
	
//检查allo格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到配置信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到配置信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
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

//recvbuf放入pid包
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

//检查pid格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到进程信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到进程信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in <<info_pid<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf放入eth0包
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

//检查eth0格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到Ethernet0信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"  EO："<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
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
			cout<<"10MB/半双工";
		else if(a == 0x0002 || a == 0x0006)
			cout<<"10MB/全双工";
		else if(a == 0x0001 || a == 0x0005)
			cout<<"100MB/半双工";
		else if(a == 0x0003 || a == 0x0007)
			cout<<"100MB/全双工";
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到Ethernet0信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in<<"  EO："<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
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
				in<<"10MB/半双工";
			else if(a == 0x0002 || a == 0x0006)
				in<<"10MB/全双工";
			else if(a == 0x0001 || a == 0x0005)
				in<<"100MB/半双工";
			else if(a == 0x0003 || a == 0x0007)
				in<<"100MB/全双工";
			in<<endl;
			in<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
			in<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf放入eth1包
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

//检查eth1格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到Ethernet1信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"  EO："<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
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
			cout<<"10MB/半双工";
		else if(a == 0x0002 || a == 0x0006)
			cout<<"10MB/全双工";
		else if(a == 0x0001 || a == 0x0005)
			cout<<"100MB/半双工";
		else if(a == 0x0003 || a == 0x0007)
			cout<<"100MB/全双工";
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到Ethernet1信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in<<"  EO："<<(int)ip[0][0]<<"."<<(int)ip[0][1]<<"."<<(int)ip[0][2]<<"."<<(int)ip[0][3];
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
				in<<"10MB/半双工";
			else if(a == 0x0002 || a == 0x0006)
				in<<"10MB/全双工";
			else if(a == 0x0001 || a == 0x0005)
				in<<"100MB/半双工";
			else if(a == 0x0003 || a == 0x0007)
				in<<"100MB/全双工";
			in<<endl;
			in<<"  Tx "<<dec<<ntohl(transbyte)<<" bytes/"<<dec<<ntohl(transpacket)<<" pks"<<endl;
			in<<"  Rx "<<dec<<ntohl(recvbyte)<<" bytes/"<<dec<<ntohl(recvpacket)<<" pks"<<endl;
		}
	}
	in.close();
}

/***************************************************/

//recvbuf放入usbinfo包
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

//检查usbinfo格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到U盘信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout << "  U盘状态：";
		if(use_usb == 1)
			cout<<"已插入";
		else
			cout<<"未插入";
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到U盘信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in << "  U盘状态：";
			if(use_usb == 1)
				in<<"已插入";
			else
				in<<"未插入";
			in<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf放入usbfile
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

//检查usbfile格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到U盘文件列表信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到U盘文件列表信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			for(int i = 0;i < strlen((char*)usb_file);i++)
			{
				in<<usb_file[i];
			}
		}
	}
	in.close();
}

/****************************************************/

//recvbuf放入print
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

//检查print格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到打印口信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"  打印机名称："<< name<<endl;
		cout<<"  服务状态：";
		if(serv == 0)
			cout<<"未启动";
		else 
			cout<<"已启动";
		cout<<"  ";
		cout<<"队列中现有任务数："<<dec<<ntohs(duty)<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到打印口信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in<<"  打印机名称："<< name<<endl;
			in<<"  服务状态：";
			if(serv == 0)
				in<<"未启动";
			else 
				in<<"已启动";
			in<<"  ";
			in<<"队列中现有任务数："<<dec<<ntohs(duty)<<endl;
		}
	}
	in.close();
}

/**************************************************/

//recvbuf放入printrow
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

//检查printrow格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到打印队列信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"  打印队列信息："<< endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到打印队列信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			cout<<"  打印队列信息："<< endl;
			for(int i = 0;i < strlen((char*)print_info);i++)
			{
				in<<print_info[i];
			}
		}
	}
	in.close();
}

/***************************************************/

//recvbuf放入term
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

//检查term格式
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

//统计ip，哑终端使用个数
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到TServer信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"配置终端总数："<<ntohs(term_num)<<endl;
		for(int i = 0;i < 16;i++)
		{
			if(dumb_term[i] == 1)
				cout<<"  哑终端"<<dec<<(i + 1)<<endl;
		}
		for(int i = 0;i < 254;i++)
		{
			if(ip_term[i] == 1)
				cout<<"  IP终端"<<dec<<(i + 1)<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到TServer信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in <<"配置终端总数："<<ntohs(term_num)<<endl;
			for(int i = 0;i < 16;i++)
			{
				if(dumb_term[i] == 1)
					in <<"  哑终端"<<dec<<(i + 1)<<endl;
			}
			for(int i = 0;i < 254;i++)
			{
				if(ip_term[i] == 1)
					in <<"  IP终端"<<dec<<(i + 1)<<endl;
			}
		}
	}
	in.close();
}

/************************************************/

//recvbuf放入scrinfo
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

//检查scrinfo格式
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
			cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到哑终端"<<dec<< ntohs(second) <<"信息."<< endl;
		else
			cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到IP终端"<<dec<< ntohs(second) <<"信息."<< endl;
		cout << systemtime <<" [9175](收到数据为：)"<<endl;
		cout<<"  终端号："<<(int)port_num<<"("<<(int)port_num<<") ";
		cout<<"虚屏数量："<<(int)scrtotal<<"  ";
		cout<<"类型："<<term_type<<" ";
		cout<<"状态："<<term_stat<<" ";
		cout<<"IP："<<(int)ipterm[0]<<"."<<(int)ipterm[1]<<"."<<(int)ipterm[2]<<"."<<(int)ipterm[3]<<endl;
		for(int i = 0;i < (int)scrtotal;i++)
		{
			cout << systemtime <<" [9175] --虚屏号：";
			cout<<(int)everyscr[i].scrcode<<"  ";
			char change[50];
			s.change_time(ntohl(everyscr[i].local_time),change);
			cout<<"连接时间："<< change <<"  ";
			cout<<"提示串："<<everyscr[i].scr_mention<<endl;
			
			cout<<"  服务器IP："<<(int)everyscr[i].svrip[0]<<"."<<(int)everyscr[i].svrip[1]<<"."<<(int)everyscr[i].svrip[2]<<"."<<(int)everyscr[i].svrip[3]<<"  ";
			cout<<"端口："<<dec<<ntohs(everyscr[i].tcp_svrport)<<"  ";
			cout<<"协议："<<everyscr[i].scr_proto<<"  ";
			cout<<"状态："<<everyscr[i].scr_stat<<"  ";
			cout<<"类型："<<everyscr[i].scrterm_type<<endl;
			
			cout<<"  发终端："<< dec << ntohl(everyscr[i].term_transbyte) <<"    ";
			cout<<"收终端："<< dec << ntohl(everyscr[i].term_recvbyte) <<"    ";
			cout<<"发服务器："<< dec << ntohl(everyscr[i].svr_transbyte) <<"    ";
			cout<<"收服务器："<< dec << ntohl(everyscr[i].svr_recvbyte) <<endl;
			
			cout<<"  ping最小："<< dec << ntohl(everyscr[i].pingmin) <<"  ";
			cout<<"ping平均："<< dec << ntohl(everyscr[i].pingavg) <<"  ";
			cout<<"ping最大："<< dec << ntohl(everyscr[i].pingmax) <<endl;
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
				in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到哑终端"<<dec<< ntohs(second) <<"信息."<< endl;
			else
				in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到IP终端"<<dec<< ntohs(second) <<"信息."<< endl;
			in << systemtime <<" [9175](收到数据为：)"<<endl;
			in<<"  终端号："<<(int)port_num<<"("<<(int)port_num<<") ";
			in<<"虚屏数量："<<(int)scrtotal<<"  ";
			in<<"类型："<<term_type<<" ";
			in<<"状态："<<term_stat<<" ";
			in<<"IP："<<(int)ipterm[0]<<"."<<(int)ipterm[1]<<"."<<(int)ipterm[2]<<"."<<(int)ipterm[3]<<endl;
			for(int i = 0;i < (int)scrtotal;i++)
			{
				in << systemtime <<" [9175] --虚屏号：";
				in<<(int)everyscr[i].scrcode<<"  ";
				char change[50];
				s.change_time(ntohl(everyscr[i].local_time),change);
				in <<"连接时间："<< change <<"  ";
				in<<"提示串："<<everyscr[i].scr_mention<<endl;
			
				in<<"  服务器IP："<<(int)everyscr[i].svrip[0]<<"."<<(int)everyscr[i].svrip[1]<<"."<<(int)everyscr[i].svrip[2]<<"."<<(int)everyscr[i].svrip[3]<<"  ";
				in<<"端口："<<dec<<ntohs(everyscr[i].tcp_svrport)<<"  ";
				in<<"协议："<<everyscr[i].scr_proto<<"  ";
				in<<"状态："<<everyscr[i].scr_stat<<"  ";
				in<<"类型："<<everyscr[i].scrterm_type<<endl;
			
				in<<"  发终端："<< dec << ntohl(everyscr[i].term_transbyte) <<"    ";
				in<<"收终端："<< dec << ntohl(everyscr[i].term_recvbyte) <<"    ";
				in<<"发服务器："<< dec << ntohl(everyscr[i].svr_transbyte) <<"    ";
				in<<"收服务器："<< dec << ntohl(everyscr[i].svr_recvbyte) <<endl;
			
				in<<"  ping最小："<< dec << ntohl(everyscr[i].pingmin) <<"  ";
				in<<"ping平均："<< dec << ntohl(everyscr[i].pingavg) <<"  ";
				in<<"ping最大："<< dec << ntohl(everyscr[i].pingmax) <<endl;
			}
		}
	}
	in.close();
}

/************************************************/

//recvbuf放入finish
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

//检查finish格式
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
		cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到本次接收完成(deptid="<<dec<< struc_num <<")."<< endl;
		cout << systemtime <<" [9175]关闭网点加密机连接"<<endl;
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
			in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")收到本次接收完成(deptid="<<dec<< struc_num <<")."<< endl;
			in << systemtime <<" [9175]关闭网点加密机连接"<<endl;
		}
	}
	in.close();
}
	
	
	
	
	
	
	
	


