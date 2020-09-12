#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <string.h>

using namespace std;

struct TS//�����ļ�
{
	char   svr_ip[20];
	int    svr_port;
	int    netpercent;//����ٷֱ�
	int    markexit;//���̽��ܳɹ����˳�
	int    min_term;
	int    max_term;
	int    min_screen;
	int    max_screen;
	int    delfile;//ɾ����־�ļ�
	char   ENV;
	char   ERR;
	char   SPACK;
	char   RPACK;
	char   SDATA;
	char   RDATA;
	int    debugscr;//debug��Ļ��ʾ
	
	void ReadConf();
}ts;

//��ȡts.conf
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
		if (strcmp(key, "������IP��ַ") == 0)	
			strcpy(svr_ip, value);
		else if (strcmp(key, "�˿ں�") == 0)
			svr_port = atoi(value);
		else if (strcmp(key, "���̽��ճɹ����˳�") == 0)	
			markexit = atoi(value);
		else if (strcmp(key, "��С�����ն�����") == 0)	
			min_term = atoi(value);
		else if (strcmp(key, "��������ն�����") == 0)
			max_term = atoi(value);
		else if (strcmp(key, "ÿ���ն���С��������") == 0)	
			min_screen = atoi(value);
		else if (strcmp(key, "ÿ���ն������������") == 0)
			max_screen = atoi(value);
		else if (strcmp(key, "����ٷֱ�") == 0)	
			netpercent = atoi(value);
		else if (strcmp(key, "ɾ����־�ļ�") == 0)	
			delfile = atoi(value);
		else if (strcmp(key, "DEBUG����") == 0)	
		{
			ENV = value[0];
			ERR = value[1];
			SPACK = value[2];
			RPACK = value[3];
			SDATA = value[4];
			RDATA = value[5];
		}
		else if (strcmp(key, "DEBUG��Ļ��ʾ") == 0)	
			debugscr = atoi(value);
	}
	in.close();
}