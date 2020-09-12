#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/wait.h>

#include "encrpty.h"

#define BUFFEN 8192
#define BUFFER 4096

extern unsigned int id;
extern int total_term;//�ն�����
extern int total_screen;//��������

unsigned char asynnum_;//�첽��

struct MSG
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned short s_version;
	unsigned char s_subversion[2];
	unsigned short iffail_time;
	unsigned short ifsuccess_time;
	unsigned char blank;
	unsigned char pad[3];//pad
	unsigned char authstring[32];
	unsigned int random_num;
	unsigned int svr_time;
	
	bool version();//�жϰ汾�Ƿ����Ҫ��
	bool system_time_and_check();//�ж�����֤���Ƿ����
	bool check_authstring();//�ж���֤��
}msg;
	
struct LEAST_VERSION
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned short s_version;
	unsigned char s_subversion[2];
	
	void send_least_version(struct MSG *msg);//׼����Ͱ汾����
}least_version;
	
struct MSG1
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned short cpu_mhz;//cpu��Ƶ��
	unsigned short ram;//
	unsigned short flash;
	unsigned short seq;
	unsigned char g_seq[16];
	unsigned char type[16];
	unsigned char version_num[16];
	unsigned char ethnum;
	unsigned char synnum;
	unsigned char asynnum;
	unsigned char exchangenum;
	unsigned char usbnum;
	unsigned char printnum;
	unsigned char pad1[2];
	unsigned int struc_num;
	unsigned char stru_seq;
	unsigned char pad2[3];
	unsigned char authstring[32];
	unsigned int random_num;
	
	void find_mhz_ram();//ȡcpu��Ƶ��ram
	void encrypt_msg1();//����msg1
	void prepare_msg1(unsigned int id);//׼��msg1����
}msg1;


/****************************��֤ͨ����****************************/


struct INFO_REQ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
}info_req;

struct RESP_SYSTEM//ϵͳ��Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned int user_cpu_time;
	unsigned int nice_cpu_time;
	unsigned int system_cpu_time;
	unsigned int idle_cpu_time;
	unsigned int freed_memory;
	
	void find_cpu_data();//ȡcpu��Ϣ
	void find_freed_memory();//ȡreed_memory
	void prepare_resp_system();//׼��ϵͳ��Ϣ����
}resp_system;

struct RESP_ALLO//������Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned char info_allo[BUFFEN];
	
	void get_allo();//ȡ������Ϣ
	void prepare_resp_allo();//׼��������Ϣ����
}resp_allo;

struct RESP_PID//������Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned char info_pid[BUFFEN];
	
	void get_pid();//ȡ������Ϣ
	void prepare_resp_pid();//׼��������Ϣ����
}resp_pid;

struct RESP_ETH0//��̫��0��Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char exist;
	unsigned char allo;
	unsigned char updown;
	unsigned char pad;
	
	unsigned char mac[6];
	unsigned short options;
	
	unsigned char ip[6][4];
	unsigned char mask[6][4];
	
	unsigned int recvbyte;
	unsigned int recvpacket;
	unsigned int recverr;
	unsigned int recvdrop;
	unsigned int recvfifo;
	unsigned int recvframe;
	unsigned int recvcompress;
	unsigned int recvcast;
	
	unsigned int transbyte;
	unsigned int transpacket;
	unsigned int transerr;
	unsigned int transdrop;
	unsigned int transfifo;
	unsigned int transframe;
	unsigned int transcompress;
	unsigned int transcast;
	
	void get_eth0_info();//ȡ��̫��0��Ϣ
	void prepare_resp_eth0();//׼����̫��0��Ϣ����
	
}resp_eth0;	

struct RESP_ETH1//��̫��1��Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char exist;
	unsigned char allo;
	unsigned char updown;
	unsigned char pad;
	
	unsigned char mac[6];
	unsigned short options;
	
	unsigned char ip[6][4];
	unsigned char mask[6][4];
	
	unsigned int recvbyte;
	unsigned int recvpacket;
	unsigned int recverr;
	unsigned int recvdrop;
	unsigned int recvfifo;
	unsigned int recvframe;
	unsigned int recvcompress;
	unsigned int recvcast;
	
	unsigned int transbyte;
	unsigned int transpacket;
	unsigned int transerr;
	unsigned int transdrop;
	unsigned int transfifo;
	unsigned int transframe;
	unsigned int transcompress;
	unsigned int transcast;
	
	void get_eth1_info();//ȡ��̫��1��Ϣ
	void prepare_resp_eth1();//׼����̫��1��Ϣ����
}resp_eth1;	

struct RESP_USBINFO//U����Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned char use_usb;
	unsigned char pad[3];
	
	void prepare_resp_usbinfo();//׼��U����Ϣ����
}resp_usbinfo;

struct RESP_USBFILE//U���ļ���Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	unsigned char usbfile[BUFFER];
	
	void get_resp_usbfile();//ȡ������Ϣ
	void prepare_resp_usbfile();//׼��U���ļ���Ϣ����
}resp_usbfile;

struct RESP_PRINT//��ӡ����Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char serv;
	unsigned char pad;
	unsigned short duty;
	unsigned char name[32];
	
	void prepare_resp_print();//׼����ӡ����Ϣ����
}resp_print;	

struct RESP_PRINTROW//��ӡ������Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char print_info[BUFFEN];
	
	void prepare_resp_printrow();//׼����ӡ������Ϣ����
}resp_printrow;
	
struct RESP_TERM//�ն���Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char dumb_term[16];
	unsigned char ip_term[254];
	unsigned short term_num;
	
	void prepare_resp_term(struct TS *ts);//׼���ն���Ϣ����
}resp_term;


struct RESP_EVERYSCR//ÿ������
{
	unsigned char scrcode;
	unsigned char pad;
	unsigned short tcp_svrport;
	unsigned char svrip[4];
	
	unsigned char scr_proto[12];
	unsigned char scr_stat[8];
	unsigned char scr_mention[24];
	unsigned char  scrterm_type[12];
	
	unsigned int local_time;
	unsigned int term_transbyte;
	unsigned int term_recvbyte;
	unsigned int svr_transbyte;
	unsigned int svr_recvbyte;
	
	unsigned int pingmax;
	unsigned int pingavg;
	unsigned int pingmin;
};

struct RESP_SCRINFO//����������Ϣ
{
	unsigned char first[2];
	unsigned short msglength;
	unsigned short second;
	unsigned short datalength;
	
	unsigned char port_num;
	unsigned char alloport_num;
	unsigned char act_scrnum;
	unsigned char scrtotal;
	unsigned char ipterm[4];
	unsigned char term_type[12];
	unsigned char term_stat[8];
	struct RESP_EVERYSCR resp_everyscr[16];
	
	void prepare_resp_scrinfo(struct TS *ts,struct INFO_REQ *info_req);//׼������������Ϣ����
}resp_scrinfo;


/******************************����ʵ�ֲ��֣���֤ǰ��*******************************/


//�жϰ汾�Ƿ����Ҫ��
bool MSG::version()//0������Ͱ汾
{
	int ver = ntohs(s_version);
	if(ver < 2)
		return 0;
	else
		return 1;
}

//�ж�����֤���Ƿ����
bool MSG::system_time_and_check()
{
	const time_t input_time = (ntohl(svr_time)) ^ (u_int)0xFFFFFFFF;
	struct tm *tt;
	tt = localtime(&input_time);
	tt->tm_year = tt->tm_year + 1900;
	tt->tm_mon = tt->tm_mon + 1;
//	cout << tt->tm_year << '-' << tt->tm_mon << '-' << tt->tm_mday << ' ' << tt->tm_hour << ':' << tt->tm_min << ':' << tt->tm_sec << endl;
	if(tt->tm_year < 2017 || (tt->tm_year == 2017 && tt->tm_mon == 1 && tt->tm_mday == 1 && tt->tm_hour < 8))
		return 0;
	else
		return 1;
}

//�ж���֤��
bool MSG::check_authstring()
{
	int i,j;
	int num = ntohl(random_num);
	int pos = num % 4093;
	char str[33];
	for(i = pos,j = 0;j < 32;i++,j++)
	{
		if(i == 4096)
			i = 0;
		authstring[j] = (int)authstring[j] ^ (int)secret[i];
	}
	for(i = 0;i < 32;i++)
		str[i]=authstring[i];
	str[i] = 0;
	if(strcmp(str,"yzmond:id*str&to!tongji@by#Auth^") == 0)
		return 1;
	else
		return 0;
}

//׼����Ͱ汾����
void LEAST_VERSION::send_least_version(struct MSG *msg)
{
	first[0] = 0x91;
	first[1] = 0x00;
	msglength = sizeof(LEAST_VERSION);
	second = 0x0000;
	datalength = 4;
	s_version = ntohs(msg->s_version);
	s_subversion[0] = msg->s_subversion[0];
	s_subversion[1] = msg->s_subversion[1];
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	s_version = htons(msg->s_version);
}

//ȡcpu��Ƶ��ram
void MSG1::find_mhz_ram()
{
	char ch[50];
	int i;
	FILE *fp;
	fp = fopen("/proc/cpuinfo","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	while(!feof(fp))
	{
		fgets(ch,sizeof(ch),fp);
		if(strstr(ch,"cpu MHz"))
			break;
	}
	for(i = 0;i < 50;i++)
	{
		if(ch[i] == ':')
			break;
	}
	strcpy(ch,&ch[++i]);
	cpu_mhz = (unsigned short)(atoi(ch));
	fclose(fp);
	fp = fopen("/proc/meminfo","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	while(!feof(fp))
	{
		fgets(ch,sizeof(ch),fp);
		if(strstr(ch,"MemTotal"))
			break;
	}
	for(i = 0;i < 50;i++)
	{
		if(ch[i] == ':')
			break;
	}
	strcpy(ch,&ch[++i]);
	ram = (unsigned short)((atoi(ch))/1024);
	fclose(fp);
}

//����msg1
void MSG1::encrypt_msg1()
{
	cpu_mhz = htons(cpu_mhz);
	ram = htons(ram);
	flash = htons(flash);
	seq = htons(seq);
	struc_num = htonl(struc_num);
	
	srand((unsigned int)(time(0)));
	int i,j;
	u_int random_no;
	int pos;
	int mark[104],k = 0;
	char *p;
	random_no = (u_int)rand();
	pos = random_no % 4093;
	for(i = 0;i < 104;i++)
	{
		mark[i] = i + pos;
		if(mark[i] < 4096)
			mark[i] = i + pos;
		else
			mark[i] = (i + pos) % 4096;
	}
	p = (char*)this;
	p = p + 8;
	for(i = 0;i < 104;i++)
		p[i] = p[i] ^ secret[mark[i]];
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	random_num = htonl(random_no);
}

//׼��msg1����
void MSG1::prepare_msg1(unsigned int id)
{
	int i,b;
	first[0] = 0x91;
	first[1] = 0x01;
	msglength = sizeof(MSG1);

	second = 0x0000;
	datalength = 108;

	find_mhz_ram();
	
	flash = 1024;
	seq = 88;
	
	srand((unsigned int)(time(0)));
	for(i = 0;i < 16;i++)
	{
		b = rand() % 3 + 1;
		if (b == 1)
			g_seq[i] = rand() % 26 + 65;
		else if(b == 2)
			g_seq[i] = rand() % 26 + 97;
		else
			g_seq[i] = rand() % 10 + 48;
	}
	g_seq[15] = 0;
	for(i = 0;i < 16;i++)
	{
		b = rand() % 3 + 1;
		if (b == 1)
			type[i] = rand() % 26 + 65;
		else if(b == 2)
			type[i] = rand() % 26 + 97;
		else
			type[i] = rand() % 10 + 48;
	}
	type[15] = 0;
	for(i = 0;i < 16;i++)
	{
		b = rand() % 3 + 1;
		if (b == 1)
			version_num[i] = rand() % 26 + 65;
		else if(b == 2)
			version_num[i] = rand() % 26 + 97;
		else
			version_num[i] = rand() % 10 + 48;
	}
	version_num[15] = 0;
	int a = (u_int)rand();
	ethnum = a % 3;
	synnum = a % 3;
	asynnum = (a % 3) * 8;
	asynnum_ = asynnum;
	exchangenum = (a % 4) * 8;
	usbnum = a % 2;
	printnum = a % 2;
	
	struc_num = id;
	stru_seq = 1;
	strcpy((char *)authstring,"yzmond:id*str&to!tongji@by#Auth^");
	
	encrypt_msg1();
}


/******************************����ʵ�ֲ��֣���֤��*******************************/


//ȡcpu��Ϣ
void RESP_SYSTEM::find_cpu_data()
{
	char ch[50];
	int i,j;
	char data[4][15];
	FILE *fp;
	fp = fopen("/proc/stat","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fgets(ch,sizeof(ch),fp);
	//user
	for(i = 2;;i++)
	{
		if(ch[i] == ' ' && ch[i + 1] !=' ')
			break;
	}
	i++;
	for(j = 0;;j++,i++)
	{
		data[0][j] = ch[i];
		if(ch[i] != ' ' && ch[i + 1] == ' ')
		{
			data[0][j + 1] = 0;
			break;
		}
	}
	i++;
	//nice
	for(;;i++)
	{
		if(ch[i] == ' ' && ch[i + 1] !=' ')
			break;
	}
	i++;
	for(j = 0;;j++,i++)
	{
		data[1][j] = ch[i];
		if(ch[i] != ' ' && ch[i + 1] == ' ')
		{
			data[1][j + 1] = 0;
			break;
		}
	}
	i++;
	//system
	for(;;i++)
	{
		if(ch[i] == ' ' && ch[i + 1] !=' ')
			break;
	}
	i++;
	for(j = 0;;j++,i++)
	{
		data[2][j] = ch[i];
		if(ch[i] != ' ' && ch[i + 1] == ' ')
		{
			data[2][j + 1] = 0;
			break;
		}
	}
	i++;
	//idle
	for(;;i++)
	{
		if(ch[i] == ' ' && ch[i + 1] !=' ')
			break;
	}
	i++;
	for(j = 0;;j++,i++)
	{
		data[3][j] = ch[i];
		if(ch[i] != ' ' && ch[i + 1] == ' ')
		{
			data[3][j + 1] = 0;
			break;
		}
	}
	fclose(fp);
	
	user_cpu_time = atoi(data[0]);
	nice_cpu_time = atoi(data[1]);
	system_cpu_time = atoi(data[2]);
	idle_cpu_time = atoi(data[3]);
}

//ȡreed_memory
void RESP_SYSTEM::find_freed_memory()
{
	char ch[50];
	int i,j;
	char data[3][50];
	unsigned int num[3];
	FILE *fp;
	fp = fopen("/proc/meminfo","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	while(!feof(fp))
	{
		fgets(ch,sizeof(ch),fp);
		if(strstr(ch,"MemFree"))
			strcpy(data[0],ch);
		if(strstr(ch,"Buffers"))
			strcpy(data[1],ch);
		if(strstr(ch,"Cached"))
		{
			strcpy(data[2],ch);
			break;
		}
	}
	for(i = 0;i < 3;i++)
	{
		for(j = 0;;j++)
		{
			if(data[i][j] == ':')
				break;
		}
		strcpy(data[i],&data[i][++j]);
		num[i] = atoi(data[i]);
	}
	freed_memory = 0;
	for(i = 0;i < 3;i++)
		freed_memory = freed_memory + num[i];
	fclose(fp);
}

//׼��ϵͳ��Ϣ����
void RESP_SYSTEM::prepare_resp_system()
{
	first[0] = 0x91;
	first[1] = 0x02;
	msglength = sizeof(RESP_SYSTEM);
	second = 0x0000;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	find_cpu_data();
	find_freed_memory();
	
	user_cpu_time = htonl(user_cpu_time);
	nice_cpu_time = htonl(nice_cpu_time);
	system_cpu_time = htonl(system_cpu_time);
	idle_cpu_time = htonl(idle_cpu_time);
	freed_memory = htonl(freed_memory);
}

//ȡ������Ϣ
void RESP_ALLO::get_allo()
{
	FILE *fp;
	fp = fopen("config.dat","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fread(info_allo,BUFFEN - 1,1,fp);
	fclose(fp);
}

//׼��������Ϣ����
void RESP_ALLO::prepare_resp_allo()
{
	first[0] = 0x91;
	first[1] = 0x03;
	second = 0x0000;
	
	get_allo();

	msglength = 8 + strlen((char*)info_allo);
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
}

//ȡ������Ϣ
void RESP_PID::get_pid()
{
	FILE *fp;
	fp = fopen("process.dat","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fread(info_pid,BUFFEN - 1,1,fp);
	fclose(fp);
}

//׼��������Ϣ����
void RESP_PID::prepare_resp_pid()
{
	first[0] = 0x91;
	first[1] = 0x04;
	second = 0x0000;
	
	get_pid();
	
	msglength = 8 + strlen((char*)info_pid);
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
}

//ȡ��̫��0��Ϣ
void RESP_ETH0::get_eth0_info()
{
	char ch[300];
	char data[16][15];
	int i,j;
	FILE *fp;
	fp = fopen("/proc/net/dev","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fgets(ch,sizeof(ch),fp);
	fgets(ch,sizeof(ch),fp);
	fgets(ch,sizeof(ch),fp);
	for(i = 0;;i++)
	{
		if(ch[i] == ':')
			break;
	}
	strcpy(ch,&ch[++i]);
	for(i = 0;;i++)
	{
		if(ch[i] >= '0' && ch[i] <= '9')
			break;
	}
	strcpy(ch,&ch[i]);
	sscanf(ch,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
	
	recvbyte = atoi(data[0]);
	recvpacket = atoi(data[1]);
	recverr = atoi(data[2]);
	recvdrop = atoi(data[3]);
	recvfifo = atoi(data[4]);
	recvframe = atoi(data[5]);
	recvcompress = atoi(data[6]);
	recvcast = atoi(data[7]);
	
	transbyte = atoi(data[8]);
	transpacket = atoi(data[9]);
	transerr = atoi(data[10]);
	transdrop = atoi(data[11]);
	transfifo = atoi(data[12]);
	transframe = atoi(data[13]);
	transcompress = atoi(data[14]);
	transcast = atoi(data[15]);
	fclose(fp);
}

//׼����̫��0��Ϣ����
void RESP_ETH0::prepare_resp_eth0()
{
	int i,j;
	first[0] = 0x91;
	first[1] = 0x05;
	msglength = sizeof(RESP_ETH0);  
	second = 0x0000;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	srand((unsigned int)(time(0)));
	int a = (u_int)rand();
	exist = a % 2;
	allo = a % 2;
	updown = a % 2;
	
	mac[0] = 0x00;
	mac[1] = 0x50;
	mac[2] = 0x56;
	mac[3] = 0xC0;
	mac[4] = 0x00;
	mac[5] = 0x08;
	options = 0x0000;
	options = htons(options);

	for(i = 0;i < 6;i++)
	{
		if(i == 0)
			ip[i][3] = 230;
		else
			ip[i][3] = ip[i - 1][3] + 1;
		ip[i][0] = 192;
		ip[i][1] = 168;
		ip[i][2] = 1;
	}
	for(i = 0;i < 6;i++)
	{
		for(j = 0;j < 4;j++)
			mask[i][j] = 255;
	}
	get_eth0_info();
	
	recvbyte = htonl(recvbyte);
	recvpacket = htonl(recvpacket);
	recverr = htonl(recverr);
	recvdrop = htonl(recvdrop);
	recvfifo = htonl(recvfifo);
	recvframe = htonl(recvframe);
	recvcompress = htonl(recvcompress);
	recvcast = htonl(recvcast);
	
	transbyte = htonl(transbyte);
	transpacket = htonl(transpacket);
	transerr = htonl(transerr);
	transdrop = htonl(transdrop);
	transfifo = htonl(transfifo);
	transframe = htonl(transframe);
	transcompress = htonl(transcompress);
	transcast = htonl(transcast);
}

//ȡ��̫��1��Ϣ
void RESP_ETH1::get_eth1_info()
{
	char ch[300];
	char data[16][15];
	int i,j;
	FILE *fp;
	fp = fopen("/proc/net/dev","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fgets(ch,sizeof(ch),fp);
	fgets(ch,sizeof(ch),fp);
	fgets(ch,sizeof(ch),fp);
	fgets(ch,sizeof(ch),fp);
	for(i = 0;;i++)
	{
		if(ch[i] == ':')
			break;
	}
	strcpy(ch,&ch[++i]);
	for(i = 0;;i++)
	{
		if(ch[i] >= '0' && ch[i] <= '9')
			break;
	}
	strcpy(ch,&ch[i]);
	sscanf(ch,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
	
	recvbyte = atoi(data[0]);
	recvpacket = atoi(data[1]);
	recverr = atoi(data[2]);
	recvdrop = atoi(data[3]);
	recvfifo = atoi(data[4]);
	recvframe = atoi(data[5]);
	recvcompress = atoi(data[6]);
	recvcast = atoi(data[7]);
	
	transbyte = atoi(data[8]);
	transpacket = atoi(data[9]);
	transerr = atoi(data[10]);
	transdrop = atoi(data[11]);
	transfifo = atoi(data[12]);
	transframe = atoi(data[13]);
	transcompress = atoi(data[14]);
	transcast = atoi(data[15]);
	fclose(fp);
}

//׼����̫��1��Ϣ����
void RESP_ETH1::prepare_resp_eth1()
{
	int i,j;
	first[0] = 0x91;
	first[1] = 0x05;
	msglength = sizeof(RESP_ETH1);  
	second = 0x0001;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	srand((unsigned int)(time(0)));
	int a = (u_int)rand();
	exist = a % 2;
	allo = a % 2;
	updown = a % 2;
	
	mac[0] = 0x00;
	mac[1] = 0x50;
	mac[2] = 0x56;
	mac[3] = 0xC0;
	mac[4] = 0x01;
	mac[5] = 0x08;
	options = 0x0007;
	options = htons(options);
	
	for(i = 0;i < 6;i++)
	{
		if(i == 0)
			ip[i][3] = 203;
		else
			ip[i][3] = ip[i - 1][3] + 1;
		ip[i][0] = 192;
		ip[i][1] = 168;
		ip[i][2] = 1;
	}
	for(i = 0;i < 6;i++)
	{
		for(j = 0;j < 4;j++)
			mask[i][j] = 255;
	}
	get_eth1_info();
	
	recvbyte = htonl(recvbyte);
	recvpacket = htonl(recvpacket);
	recverr = htonl(recverr);
	recvdrop = htonl(recvdrop);
	recvfifo = htonl(recvfifo);
	recvframe = htonl(recvframe);
	recvcompress = htonl(recvcompress);
	recvcast = htonl(recvcast);
	
	transbyte = htonl(transbyte);
	transpacket = htonl(transpacket);
	transerr = htonl(transerr);
	transdrop = htonl(transdrop);
	transfifo = htonl(transfifo);
	transframe = htonl(transframe);
	transcompress = htonl(transcompress);
	transcast = htonl(transcast);
}

//׼��U����Ϣ����
void RESP_USBINFO::prepare_resp_usbinfo()
{
	first[0] = 0x91;
	first[1] = 0x07;
	msglength = sizeof(RESP_USBINFO);  
	second = 0x0000;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	use_usb = rand() % 2;
}

//ȡu���ļ���Ϣ
void RESP_USBFILE::get_resp_usbfile()
{
	FILE *fp;
	fp = fopen("usbfiles.dat","r");
	if(fp == NULL)
	{
		printf("open error!\n");
		return;
	}
	fread(usbfile,BUFFER - 1,1,fp);
	fclose(fp);
}

//׼��U���ļ���Ϣ����
void RESP_USBFILE::prepare_resp_usbfile()
{
	first[0] = 0x91;
	first[1] = 0x0c;
	second = 0x0000;
	
	get_resp_usbfile();
	
	msglength = 8 + strlen((char*)usbfile);
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
}	

//׼����ӡ����Ϣ����
void RESP_PRINT::prepare_resp_print()
{
	int i;
	first[0] = 0x91;
	first[1] = 0x08;
	msglength = sizeof(RESP_PRINT);  
	second = 0x0000;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	srand((unsigned int)(time(0)));
	serv = rand() % 2;
	if(serv == 1)
		duty = rand() % 26 + 0;
	else
		duty = 0;
	for(i = 0;i < 32;i++)
	{
		int b = rand() % 3 + 1;
		if (b == 1)
			name[i] = rand() % 26 + 65;
		else if(b == 2)
			name[i] = rand() % 26 + 97;
		else
			name[i] = rand() % 10 + 48;
	}
	name[31] = '\0';
	duty = htons(duty);
}	
	
//׼����ӡ������Ϣ����
void RESP_PRINTROW::prepare_resp_printrow()
{
	first[0] = 0x91;
	first[1] = 0x0d;
	second = 0x0000;
	
	print_info[0] = '\0';
	
	msglength = 8 + strlen((char*)print_info) + 1;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
}

//׼���ն���Ϣ����
void RESP_TERM::prepare_resp_term(struct TS *ts)
{
	srand((unsigned int)(time(0)));
	int i,j;
	
	int total;
	int async_term_num;
	int ipterm_num;
	int *s = NULL,*p = NULL;
	
	first[0] = 0x91;
	first[1] = 0x09;
	msglength = sizeof(RESP_TERM);
	second = 0x0000;
	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
	
	total = rand()%(ts->max_term - ts->min_term + 1) + ts->min_term;
	
	if(asynnum_ == 8 || asynnum_ == 16)
		async_term_num = rand() % ((int)asynnum_) + 1;
	else
		async_term_num = 0;
	
	if(async_term_num > total)
		total = async_term_num;
	
	if(async_term_num > 0)
		s = new int[async_term_num];
	
	for(i = 0;i < async_term_num;)
	{
		s[i] = rand() % ((int)asynnum_) + 0;
		for(j = 0;j < i;j++)
		{
			if(s[j] == s[i])
				break;
		}
		if(j == i)
			i++;
	}
	
	for(i = 0;i < 16;i++)
		dumb_term[i] = 0;
	
	for(i = 0;i < async_term_num;i++)
		dumb_term[s[i]] = 1;
	
	delete []s;
	
	ipterm_num = total - async_term_num;
	
	if(ipterm_num > 0)
		p = new int[ipterm_num];
	
	for(i = 0;i < ipterm_num;)
	{
		p[i] = rand() % 254 + 0;
		for(j = 0;j < i;j++)
		{
			if(p[j] == p[i])
				break;
		}
		if(j == i)
			i++;
	}
	for(i = 0;i < 254;i++)
		ip_term[i] = 0;
	
	for(i = 0;i < ipterm_num;i++)
		ip_term[p[i]] = 1;
	
	delete []p;
	
	total_term = total;
	term_num = rand() % (270 - total + 1) + total;
	term_num = htons(term_num);
}

//׼������������Ϣ����
void RESP_SCRINFO::prepare_resp_scrinfo(struct TS *ts,struct INFO_REQ *info_req)
{
	int i;
	int a;
	first[0] = 0x91;
	first[1] = info_req->first[1];

	second = ntohs(info_req->second);
	
	port_num = (unsigned char)second;
	
	scrtotal = rand() % (ts->max_screen - ts->min_screen + 1) + ts->min_screen;
	act_scrnum = rand() % scrtotal;
	
	total_screen = total_screen + (int)scrtotal;
	
	if(first[1] == 0x0a)
	{
		alloport_num = rand() % 15 + 1;
		for(i = 0;i < 4;i++)
			ipterm[i] = 0;
		strcpy((char*)term_type,"�����ն�");
	}
	if(first[1] == 0x0b)
	{
		alloport_num = rand() % 254 + 1;
		for(i = 0;i < 4;i++)
			ipterm[i] = rand() % 254 + 1;
		strcpy((char*)term_type,"IP�ն�");
	}
	
	a = (u_int)rand();
	if(a % 2 == 0)
		strcpy((char*)term_stat,"����");
	else
		strcpy((char*)term_stat,"�˵�");
	
	/****ÿ������****/
	srand((unsigned int)(time(0)));
	for(i = 0;i < (int)scrtotal;i++)
		resp_everyscr[i].scrcode = i + 1;
	for(i = 0;i < (int)scrtotal;i++)
	{
		resp_everyscr[i].tcp_svrport = 53601;
		resp_everyscr[i].tcp_svrport = htons(resp_everyscr[i].tcp_svrport);
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		resp_everyscr[i].svrip[0] = 192;
		resp_everyscr[i].svrip[1] = 168;
		resp_everyscr[i].svrip[2] = 2;
		resp_everyscr[i].svrip[3] = i + 101;
	}
	for(i = 0;i < (int)scrtotal;i++)
		stpcpy((char*)resp_everyscr[i].scr_proto,"ר��SSH");
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		a = (u_int)rand();
		if(a % 3 == 0)
			strcpy((char*)resp_everyscr[i].scr_stat,"����");
		else if(a % 3 == 1)
			strcpy((char*)resp_everyscr[i].scr_stat,"�ػ�");
		else
			strcpy((char*)resp_everyscr[i].scr_stat,"�ѵ�¼");
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		a = (u_int)rand();
		if(a % 2 == 0)
			strcpy((char*)resp_everyscr[i].scr_mention,"����ϵͳ");
		else
			strcpy((char*)resp_everyscr[i].scr_mention,"���𿪻�");
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		a = (u_int)rand();
		if(a % 2 == 0)
			strcpy((char*)resp_everyscr[i].scrterm_type,"vt100");
		else
			strcpy((char*)resp_everyscr[i].scrterm_type,"vt220");
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		time_t tt = time(0);
		resp_everyscr[i].local_time = (unsigned int)tt;
		resp_everyscr[i].local_time = htonl(resp_everyscr[i].local_time);
	}

	for(i = 0;i < (int)scrtotal;i++)
	{
		resp_everyscr[i].term_transbyte = rand();
		resp_everyscr[i].term_recvbyte = rand();
		resp_everyscr[i].svr_transbyte = rand();
		resp_everyscr[i].svr_recvbyte = rand();
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		resp_everyscr[i].pingmax = rand() % 123457;
		resp_everyscr[i].pingavg = rand() % 123457;
		resp_everyscr[i].pingmin = rand() % 123457;
	}
	
	for(i = 0;i < (int)scrtotal;i++)
	{
		resp_everyscr[i].term_transbyte = htonl(resp_everyscr[i].term_transbyte);
		resp_everyscr[i].term_recvbyte = htonl(resp_everyscr[i].term_recvbyte);
		resp_everyscr[i].svr_transbyte = htonl(resp_everyscr[i].svr_transbyte);
		resp_everyscr[i].svr_recvbyte = htonl(resp_everyscr[i].svr_recvbyte);
	
		resp_everyscr[i].pingmax = htonl(resp_everyscr[i].pingmax);
		resp_everyscr[i].pingavg = htonl(resp_everyscr[i].pingavg);
		resp_everyscr[i].pingmin = htonl(resp_everyscr[i].pingmin);
	}
	
	msglength = 36 + sizeof(resp_everyscr[0]) * ((int)scrtotal);

	datalength = msglength - 8;
	
	msglength = htons(msglength);
	second = htons(second);
	datalength = htons(datalength);
//	cout<<"��������: "<<(int)scrtotal<<endl;
}










