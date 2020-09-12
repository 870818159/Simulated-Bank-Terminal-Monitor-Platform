#include <iostream>
#include <fstream>
#include <iomanip>
#include "server.h"

using namespace std;

//取系统时间
void LOG::Get_systemtime(char systemtime[])
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

//写数据
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

//主日志(send)
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
			cout << systemtime <<" [9175]向未认证连接(IP="<< clientip <<")发送认证."<< endl;
			cout << systemtime <<" [9175]向未认证连接(IP="<< clientip <<")发送"<<len<<"字节"<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向未认证连接(IP="<< clientip <<")发送认证."<< endl;
				in << systemtime <<" [9175]向未认证连接(IP="<< clientip <<")发送"<<len<<"字节"<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x02)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送系统信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送系统信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x03)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送配置信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送配置信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x04)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送进程信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送进程信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
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
				cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送Ethernet0信息."<< endl;
			if(msg[5] == 0x01)
				cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送Ethernet1信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				if(msg[5] == 0x00)
					in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送Ethernet0信息."<< endl;
				if(msg[5] == 0x01)
					in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送Ethernet1信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x07)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送U盘信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);	
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送U盘信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0c)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送U盘文件列表信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送U盘文件列表信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x08)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送打印口信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送打印口信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0d)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送打印队列信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送打印队列信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x09)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送TServer信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送TServer信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0a)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送哑终端口"<< dec <<(int)use_mark <<"信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送哑终端口"<< dec <<(int)use_mark <<"信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0x0b)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送IP终端口"<< dec <<(int)(use_mark - 15) <<"信息."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送IP终端口"<< dec <<(int)(use_mark - 15) <<"信息."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	else if(sendbuf == 0xff)
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送本次接收完成."<< endl;
			cout << systemtime <<" [9175](发送数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]向网点加密机(IP="<< clientip <<")发送本次接收完成."<< endl;
				in << systemtime <<" [9175](发送数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	in.close();
}

//主日志(recv)
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
			cout << systemtime <<" [9175]从未认证连接(IP="<< clientip <<")读取"<<dec<<len<<"字节"<< endl;
			cout << systemtime <<" [9175](读取数据为：)"<<endl;
			Write_Data(msg,len,0,1);
			cout << systemtime <<" [9175]网点加密机(IP="<< clientip <<")认证成功."<< endl;		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]从未认证连接(IP="<< clientip <<")读取"<<dec<<len<<"字节"<< endl;
				in << systemtime <<" [9175](读取数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
			if(type1 == '1')
				in << systemtime <<" [9175]网点加密机(IP="<< clientip <<")认证成功."<< endl;
		}
	}
	else
	{
		if(p->show_screen == 1)
		{
			cout << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")读取"<<dec<<len<<"字节"<< endl;
			cout << systemtime <<" [9175](读取数据为：)"<<endl;
			Write_Data(msg,len,0,1);		
		}
		if(size < p->mainlog_size * 1024)
		{
			if(type1 == '1')
			{
				in << systemtime <<" [9175]从网点加密机(IP="<< clientip <<")读取"<<dec<<len<<"字节"<< endl;
				in << systemtime <<" [9175](读取数据为：)"<<endl;
			}
			if(type2 == '1')
				Write_Data(msg,len,1,0);
		}
	}
	in.close();
}

//错误日志
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
		cout << systemtime <<" [9175]读取错误数据，连接关闭."<< endl;
	if(size < p->mainlog_size * 1024)
	{		
		if(type == '1')
			in << systemtime <<" [9175]读取错误数据，连接关闭."<< endl;
	}
	in.close();
}