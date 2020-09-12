#include <iostream>
#include <fstream>
#include <iomanip>
#include "server.h"

using namespace std;

//ȡϵͳʱ��
void LOG::Get_systemtime(char systemtime[])
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

//д����
void LOG::Write_Data(const char *msg, int len,char type,char type_)
{
	fstream in;
    in.open("yzmond.log",ios::out|ios::app);
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
				if(type_ == 1)
					cout << endl << "  " << hex << setw(4) << setfill('0') << i << ":  ";
				if(type == 1)
					in << endl << "  " << hex << setw(4) << setfill('0') << i << ":  ";
			}
			else
			{
				if(type_ == 1)
					cout << "  " << hex << setw(4) << setfill('0') << i << ":  ";
				if(type == 1)
					in << "  " << hex << setw(4) << setfill('0') << i << ":  ";
			}
		}
		if ((i + 8) % 16 == 0 )
		{
			if(type_ == 1)
				cout << "- ";
			if(type == 1)
				in << "- ";
		}
		if(type_ == 1)
			cout << hex << setw(2) << setfill('0') << (u_short)(u_char)msg[i] << " ";
		if(type == 1)
		in << hex << setw(2) << setfill('0') << (u_short)(u_char)msg[i] << " ";

		if ((i + 1) % 16 == 0)
		{
			int j;
			if(type_ == 1)
				cout<< " ";
			if(type == 1)
				in << " ";
			for (j = 0 ; j < 16 ; j++)
				if(isprint(msg[pre + j]))
				{
					if(type_ == 1)
						cout << msg[pre + j];
					if(type == 1)
						in << msg[pre + j];
				}
				else
				{
					if(type_ == 1)
						cout << ".";
					if(type == 1)
						in << ".";
				}
			pre = i + 1;
		}
	}	
	int j;
	for (int j = 16 - i % 16 - 1; j >= 0 ; j--)
	{
		if(type_ == 1)
			cout << "   ";
		if(type == 1)
			in << "   ";
	}
	if (16 - i % 16 >= 8) 
	{
		if(type_ == 1)
			cout << "  ";
		if(type == 1)
			in << "  ";
	}
	if(type_ == 1)
		cout << " ";
	if(type == 1)
		in << " ";
	for (j = 0 ; j < i % 16 - 1 ; j++)
		if(isprint(msg[pre + j]))
		{
			if(type_ == 1)
				cout << msg[pre + j];
			if(type == 1)
				in << msg[pre + j];
		}
		else
		{
			if(type_ == 1)
				cout << ".";
			if(type == 1)
				in << ".";
		}
	if(type_ == 1)
		cout << endl;
	if(type == 1)
		in << endl;
	in.close();
}

//����־(send)
void LOG::Write_MainLog_send(u_char sendbuf,struct YZMOND *p,const char *msg,int len,char type1,char type2,char clientip[],u_char use_mark)
{
	Get_systemtime(systemtime);
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
	if(sendbuf == 0x01)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")������֤."<< endl;
			cout << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")����"<<len<<"�ֽ�"<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")������֤."<< endl;
				in << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")����"<<len<<"�ֽ�"<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x02)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����ϵͳ��Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����ϵͳ��Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x03)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����������Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����������Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x04)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ͽ�����Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ͽ�����Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x05)
	{
		if(p->show_screen == 1)
		{
			if(msg[5] == 0x00)
				cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����Ethernet0��Ϣ."<< endl;
			if(msg[5] == 0x01)
				cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����Ethernet1��Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				if(msg[5] == 0x00)
					in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����Ethernet0��Ϣ."<< endl;
				if(msg[5] == 0x01)
					in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����Ethernet1��Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x07)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����U����Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����U����Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0c)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����U���ļ��б���Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����U���ļ��б���Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x08)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ʹ�ӡ����Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ʹ�ӡ����Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0d)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ʹ�ӡ������Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ʹ�ӡ������Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x09)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����TServer��Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����TServer��Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0a)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�������ն˿�"<< dec <<(int)use_mark <<"��Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")�������ն˿�"<< dec <<(int)use_mark <<"��Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0b)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����IP�ն˿�"<< dec <<(int)(use_mark - 15) <<"��Ϣ."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")����IP�ն˿�"<< dec <<(int)(use_mark - 15) <<"��Ϣ."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0xff)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ͱ��ν������."<< endl;
			cout << systemtime <<" [9175](��������Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")���ͱ��ν������."<< endl;
				in << systemtime <<" [9175](��������Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	in.close();
}

//����־(recv)
void LOG::Write_MainLog_recv(u_char recvbuf,struct YZMOND *p,const char *msg,int len,char type1,char type2,char clientip[],u_char use_mark)
{
	Get_systemtime(systemtime);
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
	
	if(recvbuf == 0x01)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")��ȡ"<<dec<<len<<"�ֽ�"<< endl;
			cout << systemtime <<" [9175](��ȡ����Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);
			cout << systemtime <<" [9175]������ܻ�(IP="<< clientip <<")��֤�ɹ�."<< endl;		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��δ��֤����(IP="<< clientip <<")��ȡ"<<dec<<len<<"�ֽ�"<< endl;
				in << systemtime <<" [9175](��ȡ����Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
			if(type1 == '1')
				in << systemtime <<" [9175]������ܻ�(IP="<< clientip <<")��֤�ɹ�."<< endl;
		}
	}
	else
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")��ȡ"<<dec<<len<<"�ֽ�"<< endl;
			cout << systemtime <<" [9175](��ȡ����Ϊ��)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]��������ܻ�(IP="<< clientip <<")��ȡ"<<dec<<len<<"�ֽ�"<< endl;
				in << systemtime <<" [9175](��ȡ����Ϊ��)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	in.close();
}

//������־
void LOG::Write_Error(struct YZMOND *p,char type)
{
	Get_systemtime(systemtime);
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
	if(p->show_screen == 1)
		cout << systemtime <<" [9175]��ȡ�������ݣ����ӹر�."<< endl;
	if(size < p->mainlog_size * 1024)
	{		
		if(type == '1')
			in << systemtime <<" [9175]��ȡ�������ݣ����ӹر�."<< endl;
	}
	in.close();
}