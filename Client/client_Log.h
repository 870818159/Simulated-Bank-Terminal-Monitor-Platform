#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

extern unsigned int id;
extern char systemtime[20];
extern char a[1000];

void GetTime()
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

void WriteFile(char str[])
{
	FILE *fp;
	fp = fopen("ts.log","a");
	fprintf(fp,"%s\n",str);
	fclose(fp);
}

void WriteData(const char *p,int len,struct TS *ts,char datastatus)
{
	fstream in;
    in.open("ts.log",ios::out|ios::app);
    if (!in.is_open())
    {
    	printf("open error!\n");
    	return;
    }
	char str[100];
	char ti[20];

	int line = 0, i, pre = 0;
	for (i = 0 ; i < len ; i++)
	{
		if (i % 16 == 0)
		{
			if(i > 0)
			{
				if(datastatus == '1')
				{
					if(ts->debugscr == 1)
						cout << endl << "  " << hex << setw(4) << setfill('0') << i << ":  ";
					in << endl << "  " << hex << setw(4) << setfill('0') << i << ":  ";
				}
			}
			else
			{
				if(datastatus == '1')
				{
					if(ts->debugscr == 1)
						cout << "  " << hex << setw(4) << setfill('0') << i << ":  ";
					in << "  " << hex << setw(4) << setfill('0') << i << ":  ";
				}
			}
		}
		if ((i + 8) % 16 == 0 )
		{
			if(datastatus == '1')
			{
				if(ts->debugscr == 1)
					cout << "- ";
				in << "- ";
			}
		}
		if(datastatus == '1')
		{
			if(ts->debugscr == 1)
				cout << hex << setw(2) << setfill('0') << (u_short)(u_char)p[i] << " ";
			in << hex << setw(2) << setfill('0') << (u_short)(u_char)p[i] << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			int j;
			if(datastatus == '1')
			{
				if(ts->debugscr == 1)
					cout << " ";
				in << " ";
			}
			for (j = 0 ; j < 16 ; j++)
				if(isprint(p[pre + j]))
				{
					if(datastatus == '1')
					{
						if(ts->debugscr == 1)
							cout << p[pre + j];
						in << p[pre + j];
					}
				}
				else
				{
					if(datastatus == '1')
					{
						if(ts->debugscr == 1)
							cout << ".";
						in << ".";
					}
				}
			pre = i + 1;
		}
	}	
	int j;
	for (int j = 16 - i % 16 - 1; j >= 0 ; j--)
	{
		if(datastatus == '1')
		{
			if(ts->debugscr == 1)
				cout << "   ";
			in << "   ";
		}
	}
	if (16 - i % 16 >= 8) 
	{
		if(datastatus == '1')
		{
			if(ts->debugscr == 1)
				cout << "  ";
			in << "  ";
		}
	}
	if(datastatus == '1')
	{
		if(ts->debugscr == 1)
			cout << " ";
		in << " ";
	}
	for (j = 0 ; j < i % 16 - 1 ; j++)
		if(isprint(p[pre + j]))
		{
			if(datastatus == '1')
			{
				if(ts->debugscr == 1)
					cout << p[pre + j];
				in << p[pre + j];
			}
		}
		else
		{
			if(datastatus == '1')
			{
				if(ts->debugscr == 1)
					cout << ".";
				in << ".";
			}
		}
	if(datastatus == '1')
	{
		if(ts->debugscr == 1)
			cout << endl;
		in << endl;
	}
	in.close();
}

void WriteFirst(const char *p,int len,struct TS *ts,char datastatus,struct MSG *msg)
{
	if(ts->RPACK == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] ��ȡ%d�ֽ�.\0",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	if(ts->ENV == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] (��ȡ����Ϊ:)",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	WriteData(p,len,ts,datastatus);
	if(ts->RDATA == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] �յ���������ʱ��:%s\0",systemtime,id,systemtime);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
		
		GetTime();
		sprintf(a,"%s [%d] �յ����Ӽ��=%d/�������=%d\0",systemtime,id,ntohs(msg->iffail_time),ntohs(msg->ifsuccess_time));
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
		
		GetTime();
		sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=��֤��Ϣ]\0",systemtime,id);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
}

void WriteSecond(const char *p,int len,struct TS *ts,char datastatus)
{
	if(ts->SPACK == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=��֤��Ϣ len=%d]\0",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);

		GetTime();
		sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	if(ts->ENV == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	WriteData(p,len,ts,datastatus);
}

void WriteRecvMsg(const char *p,int len,struct TS *ts,struct INFO_REQ *info_req,char datastatus)
{
	if(ts->RPACK == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] ��ȡ%d�ֽ�.\0",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	if(ts->ENV == '1')
	{
		GetTime();
		sprintf(a,"%s [%d] (��ȡ����Ϊ:)",systemtime,id,len);
		if(ts->debugscr == 1)
		{
			printf(a);
			printf("\n");
		}
		WriteFile(a);
	}
	
	WriteData(p,len,ts,datastatus);
	
	switch(info_req->first[1])
	{
	case 0x02:
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=ϵͳ��Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
	case 0x03: //������Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=������Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x04: //������Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=������Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x05: //eth0/eth1��Ϣ
		if(ntohs(info_req->second) == 0x0000)
		{
			if(ts->ENV == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=Ethernet0����Ϣ]\0",systemtime,id);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			}
		}
		else if(ntohs(info_req->second) == 0x0001)
		{
			if(ts->ENV == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=Ethernet1����Ϣ]\0",systemtime,id);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			}
		}
		break;
		
	case 0x07: //u����Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=U����Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0c: //u���ļ���Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=U���ļ���Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x08: //��ӡ����Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=��ӡ����Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0d: //��ӡ������Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=��ӡ�ڶ�����Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x09: //�ն���Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=TServer������Ϣ]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0a: //���ն���Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=Async%d ����Ϣ]\0",systemtime,id,ntohs(info_req->second));
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0b: //IP�ն���Ϣ
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=IPTERM%d ����Ϣ]\0",systemtime,id,ntohs(info_req->second));
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0xff: //���а����յ�
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] �յ��ͻ���״̬����[intf=���ν������]\0",systemtime,id);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	default:
		break;
	}
}

void WriteSendMsg(const char *p,int len,struct TS *ts,struct INFO_REQ *info_req,char datastatus)
{
	switch(info_req->first[1])
	{
	case 0x02:
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=ϵͳ��Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);

			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		
		break;
	case 0x03: //������Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=������Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);

			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x04: //������Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=������Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x05: //eth0/eth1��Ϣ
		if(ntohs(info_req->second) == 0x0000)
		{
			if(ts->SPACK == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=Ethernet0����Ϣ len=%d]\0",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			
				GetTime();
				sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}	
				WriteFile(a);	
			}
			if(ts->ENV == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			}
		}
		else if(ntohs(info_req->second) == 0x0001)
		{
			if(ts->SPACK == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=Ethernet1����Ϣ len=%d]\0",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			
				GetTime();
				sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);	
			}
			if(ts->ENV == '1')
			{
				GetTime();
				sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
				if(ts->debugscr == 1)
				{
					printf(a);
					printf("\n");
				}
				WriteFile(a);
			}
		}
		break;
		
	case 0x07: //u����Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=U����Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0c: //u���ļ���Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=U���ļ���Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x08: //��ӡ����Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=��ӡ����Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0d: //��ӡ������Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=��ӡ�ڶ�����Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x09: //�ն���Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬Ӧ��[intf=TServer������Ϣ len=%d]\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0a: //���ն���Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬����[intf=Async%d ����Ϣ len=%d]\0",systemtime,id,ntohs(info_req->second),len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)	
				cout << a << endl;
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0x0b: //IP�ն���Ϣ
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ͷ�״̬����[intf=IPTERM%d ����Ϣ len=%d]\0",systemtime,id,ntohs(info_req->second),len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	case 0xff: //���а����յ�
		if(ts->SPACK == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] ���Ϳͻ���״̬����[intf=���ν������] len=%d\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		
			GetTime();
			sprintf(a,"%s [%d] ����%d�ֽ�.\0",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		if(ts->ENV == '1')
		{
			GetTime();
			sprintf(a,"%s [%d] (��������Ϊ:)",systemtime,id,len);
			if(ts->debugscr == 1)
			{
				printf(a);
				printf("\n");
			}
			WriteFile(a);
		}
		break;
		
	default:
		break;
	}
	
	WriteData(p,len,ts,datastatus);
}