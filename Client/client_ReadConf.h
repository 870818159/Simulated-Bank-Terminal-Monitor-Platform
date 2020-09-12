#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <string.h>

using namespace std;

struct TS//配置文件
{
	char   svr_ip[20];
	int    svr_port;
	int    netpercent;//网点百分比
	int    markexit;//进程接受成功后退出
	int    min_term;
	int    max_term;
	int    min_screen;
	int    max_screen;
	int    delfile;//删除日志文件
	char   ENV;
	char   ERR;
	char   SPACK;
	char   RPACK;
	char   SDATA;
	char   RDATA;
	int    debugscr;//debug屏幕显示
	
	void ReadConf();
}ts;

//读取ts.conf
void TS::ReadConf()
{
	ifstream in;
    in.open("ts.conf");
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
    char ch[200], key[100], value[100];
    while(in.getline(ch, 100))
	{
		char *token = strstr(ch, "#");
    	if (token)
    		*token = '\0';
		sscanf(ch, "%s %s", key, value);
		if (strcmp(key, "服务器IP地址") == 0)	
			strcpy(svr_ip, value);
		else if (strcmp(key, "端口号") == 0)
			svr_port = atoi(value);
		else if (strcmp(key, "进程接收成功后退出") == 0)	
			markexit = atoi(value);
		else if (strcmp(key, "最小配置终端数量") == 0)	
			min_term = atoi(value);
		else if (strcmp(key, "最大配置终端数量") == 0)
			max_term = atoi(value);
		else if (strcmp(key, "每个终端最小虚屏数量") == 0)	
			min_screen = atoi(value);
		else if (strcmp(key, "每个终端最大虚屏数量") == 0)
			max_screen = atoi(value);
		else if (strcmp(key, "网点百分比") == 0)	
			netpercent = atoi(value);
		else if (strcmp(key, "删除日志文件") == 0)	
			delfile = atoi(value);
		else if (strcmp(key, "DEBUG设置") == 0)	
		{
			ENV = value[0];
			ERR = value[1];
			SPACK = value[2];
			RPACK = value[3];
			SDATA = value[4];
			RDATA = value[5];
		}
		else if (strcmp(key, "DEBUG屏幕显示") == 0)	
			debugscr = atoi(value);
	}
	in.close();
}