#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <mysql.h>

#define BUFFEN 8192
#define BUFFER 4096

using namespace std;

class CServerNet
{
public:
    void Get_time(char systemtime[]);//取系统时间
	void change_time(const time_t input_time,char change[]);
	void Read_yzmond(struct YZMOND *p);//读取配置文件
	void NonBlock();//非阻塞
	int Init(int port);//初始化
	void Run(struct YZMOND *p);
	void Write_database(
		struct YZMOND *yzmond,
		struct AUTHEN *authen,
		struct SYSTEM *system,
		struct ALLO *allo,             
		struct PID *pid,               
		struct ETH0 *eth0,             
		struct ETH1 *eth1,             
		struct USBINFO *usbinfo,      
		struct USBFILE *usbfile,       
		struct PRINT *print,           
		struct PRINTROW *printrow,     
		struct TERM *term,             
		struct SCRINFO scrinfo[],  
		struct FINISH *finish,
		int total,
		char clientip[]);//写数据库
	
private:
	int sockfd;
	int opt;
};

class LOG//写日志
{
private:
	char systemtime[20];

public:
	void Write_MainLog_send(u_char sendbuf,struct YZMOND *p,const char *msg,int len,char type1,char type2,char clientip[],u_char use_mark);
	void Write_MainLog_recv(u_char recvbuf,struct YZMOND *p,const char *msg,int len,char type1,char type2,char clientip[],u_char use_mark);
	void Write_Data(const char *msg, int len,char type,char type_);
	void Write_Error(struct YZMOND *p,char type);

private:
	void Get_systemtime(char systemtime[]);//取系统时间
};

struct YZMOND//yzmond.conf
{
	//connect
	int port;
	int connect_interval;
	int sample_interval;
	
	//database
	char svr_ip[10];
	int svr_port;
	char name[15];
	char user[15];
	char paswd[15];
	
	//system
	int non_resp_overtime;
	int trans_overtime;
	int mainlog_size;
	int sublog_size;
	
	//debug
	int show_screen;
	char tmp_packet[4];
	char tmp_socket[4];
	char dev_packet[4];
	char dev_socket[4];
};

struct IDENTITY //认证请求
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	u_short version;
	u_char subversion[2];
	u_short connect_interval;
	u_short sample_interval;
	u_char blank;
	u_char pad[3];//pad
	u_char authstring[32];
	u_int random_num;
	u_int svr_time;
	
	void prepare_identity(struct YZMOND *p);
	void encryption();
	void put_into_sendbuf(char *p,u_char sendbuf[]);
};

struct LEAST_VERSION
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	u_short version;
	u_char subversion[2];
	
	bool put_into_least_version(char *p,u_char recvbuf[],int len);
	bool check_msg();//检查格式
	bool check_version();//准备最低版本报文
};

struct AUTHEN //认证串
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	u_short cpu_mhz;//cpu主频
	u_short ram;
	u_short flash;
	u_short seq;
	u_char g_seq[16];
	u_char type[16];
	u_char version_num[16];
	u_char ethnum;//以太口
	u_char syncnum;//同步口
	u_char asyncnum;//异步口
	u_char switchnum;//交换口
	u_char usbnum;//usb口
	u_char prnnum;//打印口
	u_char pad1[2];
	u_int struc_num;
	u_char stru_seq;
	u_char pad2[3];
	u_char authstring[32];
	u_int random_num;
	
	bool check_authen();//认证是否通过
	bool put_into_authen(char *p,u_char recvbuf[],int len);
	bool check_authen_head();
	
	void authen_log(struct YZMOND *p,int type);
};

/**************认证通过后****************/

struct INFO_REQ
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	void display()
	{
		head[0] = 0x11;
		msglen = 8;
		second = 0x0000;
		datalen = msglen - 8;
		
		msglen = htons(msglen);
		second = htons(second);
		datalen = htons(datalen);
	}
	
	void system()//系统信息
	{
		display();
		head[1] = 0x02;
	}
	
	void allo()//配置信息
	{
		display();
		head[1] = 0x03;
	}
	
	void pid()//进程信息
	{
		display();
		head[1] = 0x04;
	}
	
	void eth0()//eth0信息
	{
		display();
		head[1] = 0x05;
	}
	
	void eth1()//eth1信息
	{
		display();
		head[1] = 0x05;
		second = ntohs(second);
		second = 0x0001;
		second = htons(second);
	}
	
	void usbinfo()//usb信息
	{
		display();
		head[1] = 0x07;
	}
	
	void usbfile()//usb文件列表信息
	{
		display();
		head[1] = 0x0c;
	}
	
	void print()//打印口信息
	{
		display();
		head[1] = 0x08;
	}
	
	void printrow()//打印口队列信息
	{
		display();
		head[1] = 0x0d;
	}
	
	void term()//终端信息
	{
		display();
		head[1] = 0x09;
	}
	
	void dump_ip(u_char mark)//ip/哑终端
	{
		display();
		second = ntohs(second);
		if(mark >= 0 && mark < 16)
		{
			second = (u_short)(mark + 1);
			head[1] = 0x0a;
		}
		else
		{
			second = (u_short)(mark + 1 - 16);
			head[1] = 0x0b;
		}
		second = htons(second);
	}
	
	void finish()//结束信息
	{
		display();
		head[1] = 0xff;
	}
	
	void put_info_into_sendbuf(char *p,u_char sendbuf[]);
};

struct SYSTEM//系统信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_int user_cpu_time;
	u_int nice_cpu_time;
	u_int system_cpu_time;
	u_int idle_cpu_time;
	u_int freed_memory;
	
	bool put_into_system(char *p,u_char recvbuf[],int len);
	bool check_system_head();
	
	void system_log(struct YZMOND *p,char clientip[],int type);
};

struct ALLO//配置信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char info_allo[BUFFEN];
	
	bool put_into_allo(char *p,u_char recvbuf[],int len);
	bool check_allo_head();
	
	void allo_log(struct YZMOND *p,char clientip[],int type);
};

struct PID//进程信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char info_pid[BUFFEN];
	
	bool put_into_pid(char *p,u_char recvbuf[],int len);
	bool check_pid_head();
	
	void pid_log(struct YZMOND *p,char clientip[],int type);
};

struct ETH0//以太口0信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char exist;
	u_char allo;
	u_char updown;
	u_char pad;
	
	u_char mac[6];
	u_short options;
	
	u_char ip[6][4];
	u_char mask[6][4];
	
	u_int recvbyte;
	u_int recvpacket;
	u_int recverr;
	u_int recvdrop;
	u_int recvfifo;
	u_int recvframe;
	u_int recvcompress;
	u_int recvcast;
	
	u_int transbyte;
	u_int transpacket;
	u_int transerr;
	u_int transdrop;
	u_int transfifo;
	u_int transframe;
	u_int transcompress;
	u_int transcast;
	
	bool put_into_eth0(char *p,u_char recvbuf[],int len);
	bool check_eth0_head();
	
	void eth0_log(struct YZMOND *p,char clientip[],int type);
};	

struct ETH1//以太口1信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char exist;
	u_char allo;
	u_char updown;
	u_char pad;
	
	u_char mac[6];
	u_short options;
	
	u_char ip[6][4];
	u_char mask[6][4];
	
	u_int recvbyte;
	u_int recvpacket;
	u_int recverr;
	u_int recvdrop;
	u_int recvfifo;
	u_int recvframe;
	u_int recvcompress;
	u_int recvcast;
	
	u_int transbyte;
	u_int transpacket;
	u_int transerr;
	u_int transdrop;
	u_int transfifo;
	u_int transframe;
	u_int transcompress;
	u_int transcast;
	
	bool put_into_eth1(char *p,u_char recvbuf[],int len);
	bool check_eth1_head();
	
	void eth1_log(struct YZMOND *p,char clientip[],int type);
};

struct USBINFO//U盘信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char use_usb;
	u_char pad[3];
	
	bool put_into_usbinfo(char *p,u_char recvbuf[],int len);
	bool check_usbinfo_head();
	
	void usbinfo_log(struct YZMOND *p,char clientip[],int type);
};

struct USBFILE//U盘文件信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char usb_file[BUFFER];
	
	bool put_into_usbfile(char *p,u_char recvbuf[],int len);
	bool check_usbfile_head();
	
	void usbfile_log(struct YZMOND *p,char clientip[],int type);
};

struct PRINT//打印口信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char serv;
	u_char pad;
	u_short duty;
	u_char name[32];
	
	bool put_into_print(char *p,u_char recvbuf[],int len);
	bool check_print_head();
	
	void print_log(struct YZMOND *p,char clientip[],int type);
};	

struct PRINTROW//打印队列信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char print_info[BUFFEN];
	
	bool put_into_printrow(char *p,u_char recvbuf[],int len);
	bool check_printrow_head();
	
	void printrow_log(struct YZMOND *p,char clientip[],int type);
};
	
struct TERM//终端信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char dumb_term[16];
	u_char ip_term[254];
	u_short term_num;
	
	bool put_into_term(char *p,u_char recvbuf[],int len);
	bool check_term_head();
	int ipuse_num(u_char use_mark[]);
	
	void term_log(struct YZMOND *p,char clientip[],int type);
};

struct EVERYSCR//每个虚屏
{
	u_char scrcode;
	u_char pad;
	u_short tcp_svrport;
	u_char svrip[4];
	
	u_char scr_proto[12];
	u_char scr_stat[8];
	u_char scr_mention[24];
	u_char  scrterm_type[12];
	
	u_int local_time;
	u_int term_transbyte;
	u_int term_recvbyte;
	u_int svr_transbyte;
	u_int svr_recvbyte;
	
	u_int pingmax;
	u_int pingavg;
	u_int pingmin;
};

struct SCRINFO//虚屏配置信息
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	u_char port_num;
	u_char alloport_num;
	u_char act_scrnum;
	u_char scrtotal;
	u_char ipterm[4];
	u_char term_type[12];
	u_char term_stat[8];
	struct EVERYSCR everyscr[16];
	
	bool put_into_scrinfo(char *p,u_char recvbuf[],int len);
	bool check_scrinfo_head(u_char mark);
	
	void scrinfo_log(struct YZMOND *p,char clientip[],int type);
};

struct FINISH//结束
{
	u_char head[2];
	u_short msglen;
	u_short second;
	u_short datalen;
	
	bool put_into_finish(char *p,u_char recvbuf[],int len);
	bool check_finish_head();
	
	void finish_log(struct YZMOND *p,char clientip[],int type,u_int struc_num);
};

























	