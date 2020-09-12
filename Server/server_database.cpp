#include <iostream>
#include "server.h"

//写数据库
void CServerNet::Write_database(
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
		char clientip[])
{
	MYSQL mysql;

	if (NULL == mysql_init(&mysql))
	{    //分配和初始化MYSQL对象  
		cout <<"mysql_init():"<< mysql_error(&mysql)<< endl;
		return;
	}
	//尝试与运行在主机上的MySQL数据库引擎建立连接  
	if (NULL == mysql_real_connect(&mysql,
		yzmond->svr_ip,
		yzmond->user,
		yzmond->paswd,
		yzmond->name,
		0,
		NULL,
		0))
	{
		cout<<"mysql_real_connect():"<< mysql_error(&mysql)<< endl;
		return;
	}
	
	mysql_set_character_set(&mysql, "gbk");
	
/**************************************************/	
		
	authen->struc_num = ntohl(authen->struc_num);//devid
	int devno = 1;//devno
	
	char systemtime[20];//取系统时间
	Get_time(systemtime);//time

	char sid[20];//sid
	sprintf(sid,"%s%d\0",authen->g_seq,authen->seq);//sid
	
	authen->cpu_mhz = ntohs(authen->cpu_mhz);//cpu
	authen->ram = ntohs(authen->ram);//sdram
	authen->flash = ntohs(authen->flash);//flash
	char usbnum[10];//usbnum
	if(authen->usbnum == 1)
		sprintf(usbnum,"%s\0","存在");
	else if(authen->usbnum == 0)
		sprintf(usbnum,"%s\0","不存在");
	
	char prnnum[10];//prnnum
	if(authen->prnnum == 1)
		sprintf(prnnum,"%s\0","'存在'");
	else if(authen->prnnum == 0)
		sprintf(prnnum,"%s\0","不存在");
	
	//system
	system->user_cpu_time = ntohl(system->user_cpu_time);
	system->nice_cpu_time = ntohl(system->nice_cpu_time);
	system->system_cpu_time = ntohl(system->system_cpu_time);
	system->idle_cpu_time = ntohl(system->idle_cpu_time);
	system->freed_memory = ntohl(system->freed_memory);
	
	double cpu_used = (system->user_cpu_time + system->system_cpu_time) * 1.0 / 
	(system->user_cpu_time + system->nice_cpu_time + system->idle_cpu_time);
	
	double sdram_used = (system->freed_memory) * 1.0 / (authen->ram);
	
	//eth0
	eth0->transbyte = ntohl(eth0->transbyte);
	eth0->transpacket = ntohl(eth0->transpacket);
	eth0->recvbyte = ntohl(eth0->recvbyte);
	eth0->recvpacket = ntohl(eth0->recvpacket);
	char eth0_state[5];
	if(eth0->updown == 0)
		sprintf(eth0_state,"%s\0","Up");
	else if(eth0->updown == 1)
		sprintf(eth0_state,"%s\0","Down");
	
	char eth0_speed[10];
	char eth0_duplex[10];
	char eth0_autonego[10];
//	eth0->options = ntohs(eth0->options);
	if(eth0->options == 0x0000)
	{
		sprintf(eth0_speed,"%s\0","10MB");
		sprintf(eth0_duplex,"%s\0","半双工");
		sprintf(eth0_autonego,"%s\0","否");
	}
	else if(eth0->options == 0x0001)
	{
		sprintf(eth0_speed,"%s\0","100MB");
		sprintf(eth0_duplex,"%s\0","半双工");
		sprintf(eth0_autonego,"%s\0","否");
	}
	else if(eth0->options == 0x0002)
	{
		sprintf(eth0_speed,"%s\0","10MB");
		sprintf(eth0_duplex,"%s\0","全双工");
		sprintf(eth0_autonego,"%s\0","否");
	}
	else if(eth0->options == 0x0003)
	{
		sprintf(eth0_speed,"%s\0","100MB");
		sprintf(eth0_duplex,"%s\0","全双工");
		sprintf(eth0_autonego,"%s\0","否");
	}
	else if(eth0->options == 0x0004)
	{
		sprintf(eth0_speed,"%s\0","10MB");
		sprintf(eth0_duplex,"%s\0","半双工");
		sprintf(eth0_autonego,"%s\0","是");
	}
	else if(eth0->options == 0x0005)
	{
		sprintf(eth0_speed,"%s\0","100MB");
		sprintf(eth0_duplex,"%s\0","半双工");
		sprintf(eth0_autonego,"%s\0","是");
	}
	else if(eth0->options == 0x0006)
	{
		sprintf(eth0_speed,"%s\0","10MB");
		sprintf(eth0_duplex,"%s\0","全双工");
		sprintf(eth0_autonego,"%s\0","是");
	}
	else if(eth0->options == 0x0007)
	{
		sprintf(eth0_speed,"%s\0","100MB");
		sprintf(eth0_duplex,"%s\0","全双工");
		sprintf(eth0_autonego,"%s\0","是");
	}
	
	//eth1
	eth1->transbyte = ntohl(eth1->transbyte);
	eth1->transpacket = ntohl(eth1->transpacket);
	eth1->recvbyte = ntohl(eth1->recvbyte);
	eth1->recvpacket = ntohl(eth1->recvpacket);
	char eth1_state[5];
	if(eth1->updown == 0)
		sprintf(eth1_state,"%s\0","Up");
	else if(eth1->updown == 1)
		sprintf(eth1_state,"%s\0","Down");
	
	char eth1_speed[10];
	char eth1_duplex[10];
	char eth1_autonego[10];
	eth1->options = ntohs(eth1->options);
	if(eth1->options == 0x0000)
	{
		sprintf(eth1_speed,"%s\0","10MB");
		sprintf(eth1_duplex,"%s\0","半双工");
		sprintf(eth1_autonego,"%s\0","否");
	}
	else if(eth1->options == 0x0001)
	{
		sprintf(eth1_speed,"%s\0","100MB");
		sprintf(eth1_duplex,"%s\0","半双工");
		sprintf(eth1_autonego,"%s\0","否");
	}
	else if(eth1->options == 0x0002)
	{
		sprintf(eth1_speed,"%s\0","10MB");
		sprintf(eth1_duplex,"%s\0","全双工");
		sprintf(eth1_autonego,"%s\0","否");
	}
	else if(eth1->options == 0x0003)
	{
		sprintf(eth1_speed,"%s\0","100MB");
		sprintf(eth1_duplex,"%s\0","全双工");
		sprintf(eth1_autonego,"%s\0","否");
	}
	else if(eth1->options == 0x0004)
	{
		sprintf(eth1_speed,"%s\0","10MB");
		sprintf(eth1_duplex,"%s\0","半双工");
		sprintf(eth1_autonego,"%s\0","是");
	}
	else if(eth1->options == 0x0005)
	{
		sprintf(eth1_speed,"%s\0","100MB");
		sprintf(eth1_duplex,"%s\0","半双工");
		sprintf(eth1_autonego,"%s\0","是");
	}
	else if(eth1->options == 0x0006)
	{
		sprintf(eth1_speed,"%s\0","10MB");
		sprintf(eth1_duplex,"%s\0","全双工");
		sprintf(eth1_autonego,"%s\0","是");
	}
	else if(eth1->options == 0x0007)
	{
		sprintf(eth1_speed,"%s\0","100MB");
		sprintf(eth1_duplex,"%s\0","全双工");
		sprintf(eth1_autonego,"%s\0","是");
	}
	
	//usb口和usb文件列表
	char usbstate[10];
	char usbfiles[BUFFER];
	if(authen->usbnum == 0)
	{
		sprintf(usbstate,"%s\0","未插入");
		sprintf(usbfiles,"%s\0","NULL");
	}
	else
	{
		if(usbinfo->use_usb == 0)
		{
			sprintf(usbstate,"%s\0","未插入");
			sprintf(usbfiles,"%s\0","NULL");
		}
		else
		{
			sprintf(usbstate,"%s\0","已插入");
			sprintf(usbfiles,"%s\0",usbfile->usb_file);
		}
	}
	
	//打印口和打印队列
	char prnname[33];
	char prnstate[10];
	char prnfiles[BUFFEN];
	if(authen->prnnum == 0)
	{
		sprintf(prnname,"%s\0","NULL");
		sprintf(prnstate,"%s\0","未启动");
		sprintf(prnfiles,"%s\0","NULL");
	}
	else
	{
		if(print->serv == 0)
		{
			sprintf(prnname,"%s\0",print->name);
			sprintf(prnstate,"%s\0","未启动");
			sprintf(prnfiles,"%s\0","NULL");
		}
		else
		{
			if(print->duty == 0)
			{
				sprintf(prnname,"%s\0",print->name);
				sprintf(prnstate,"%s\0","已启动");
				sprintf(prnfiles,"%s\0","NULL");
			}
			else
			{
				sprintf(prnname,"%s\0",print->name);
				sprintf(prnstate,"%s\0","已启动");
				sprintf(prnfiles,"%s\0",printrow->print_info);
			}
		}
	}
	//终端
	term->term_num = ntohs(term->term_num);
	
	char query_str[BUFFEN];
	sprintf(query_str,
	"replace into devstate_base values('%u','1','%s','%s','%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,'%s','%s',%f,%f,%d,%d,'%d.%d.%d.%d','%d.%d.%d.%d','%02x:%02x:%02x:%02x:%02x:%02x','%s','%s','%s','%s',%d,%d,%d,%d,'%d.%d.%d.%d','%d.%d.%d.%d','%02x:%02x:%02x:%02x:%02x:%02x','%s','%s','%s','%s',%d,%d,%d,%d,'%s','%s','%s','%s','%s','NULL','%s','%s',0)",
	authen->struc_num,
	systemtime,
	clientip,
	sid,
	authen->type,
	authen->version_num,
	authen->cpu_mhz,
	authen->ram,
	authen->flash,
	authen->ethnum,
	authen->syncnum,
	authen->asyncnum,
	authen->switchnum,
	usbnum,
	prnnum,
	cpu_used,
	sdram_used,
	term->term_num,
	total,
	eth0->ip[0][0],eth0->ip[0][1],eth0->ip[0][2],eth0->ip[0][3],
	eth0->mask[0][0],eth0->mask[0][1],eth0->mask[0][2],eth0->mask[0][3],
	eth0->mac[0],eth0->mac[1],eth0->mac[2],eth0->mac[3],eth0->mac[4],eth0->mac[5],
	eth0_state,
	eth0_speed,
	eth0_duplex,
	eth0_autonego,
	eth0->transbyte,
	eth0->transpacket,
	eth0->recvbyte,
	eth0->recvpacket,
	eth1->ip[0][0],eth1->ip[0][1],eth1->ip[0][2],eth1->ip[0][3],
	eth1->mask[0][0],eth1->mask[0][1],eth1->mask[0][2],eth1->mask[0][3],
	eth1->mac[0],eth1->mac[1],eth1->mac[2],eth1->mac[3],eth1->mac[4],eth1->mac[5],
	eth1_state,
	eth1_speed,
	eth1_duplex,
	eth1_autonego,
	eth1->transbyte,
	eth1->transpacket,
	eth1->recvbyte,
	eth1->recvpacket,
	usbstate,
	usbfiles,
	prnname,
	prnstate,
	prnfiles,
	allo->info_allo,
	pid->info_pid
	);
//	cout<<query_str<<endl;
	int rc = mysql_real_query(&mysql, query_str, strlen(query_str));
	if (0 != rc)
	{
		printf("mysql_real_query(): %s\n", mysql_error(&mysql));
		return;
	}
	
	
	struct SCRINFO *p = scrinfo;
	
	int i,j;
	for(i = 0;i < total;i++)
	{
		p[i].second = ntohs(p[i].second);
		if(p[i].head[1] == 0x0a)
			p[i].second = p[i].second + 900;
	}
	for(i = 0;i < total;i++)
	{
		if(p[i].second == 0x0b)
		{
			Get_time(systemtime);//time
			sprintf(query_str,
			"replace into devstate_ttyinfo values('%u','1',%d,'%s',%d,'%s','%s','%d.%d.%d.%d',%d)",
			authen->struc_num,
			p[i].second,
			systemtime,
			p[i].alloport_num,
			p[i].term_type,
			p[i].term_stat,
			p[i].ipterm[0],p[i].ipterm[1],p[i].ipterm[2],p[i].ipterm[3],
			p[i].scrtotal
			);
		}
		else
		{
			Get_time(systemtime);//time
			sprintf(query_str,
			"replace into devstate_ttyinfo values('%u','1',%d,'%s',%d,'%s','%s','0',%d)",
			authen->struc_num,
			p[i].second,
			systemtime,
			p[i].alloport_num,
			p[i].term_type,
			p[i].term_stat,
			p[i].scrtotal
			);
		}
		int rc = mysql_real_query(&mysql, query_str, strlen(query_str));
		if (0 != rc)
		{
			printf("mysql_real_query(): %s\n", mysql_error(&mysql));
			return;
		}
	}
	
	for(i = 0;i < total;i++)
	{
		for(j = 0;j < (int)p[i].scrtotal;j++)
		{
			p[i].everyscr[j].tcp_svrport = ntohs(p[i].everyscr[j].tcp_svrport);
			
			p[i].everyscr[j].svr_transbyte = ntohl(p[i].everyscr[j].svr_transbyte);
			p[i].everyscr[j].svr_recvbyte = ntohl(p[i].everyscr[j].svr_recvbyte);
			p[i].everyscr[j].term_transbyte = ntohl(p[i].everyscr[j].term_transbyte);
			p[i].everyscr[j].term_recvbyte = ntohl(p[i].everyscr[j].term_recvbyte);
			
			p[i].everyscr[j].pingmax = ntohl(p[i].everyscr[j].pingmax);
			p[i].everyscr[j].pingavg = ntohl(p[i].everyscr[j].pingavg);
			p[i].everyscr[j].pingmin = ntohl(p[i].everyscr[j].pingmin);
			if(p[i].everyscr[j].scrcode == p[i].act_scrnum + 1)
			{
				Get_time(systemtime);//time
				sprintf(query_str,
				"replace into devstate_scrinfo values('%u','1',%d,%d,'%s','*','%s','%d.%d.%d.%d',%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d)",
				authen->struc_num,
				p[i].second,
				p[i].everyscr[j].scrcode,
				systemtime,
				p[i].everyscr[j].scr_proto,
				p[i].everyscr[j].svrip[0],p[i].everyscr[j].svrip[1],p[i].everyscr[j].svrip[2],p[i].everyscr[j].svrip[3],
				p[i].everyscr[j].tcp_svrport,
				p[i].everyscr[j].scr_stat,
				p[i].everyscr[j].scrterm_type,
				p[i].everyscr[j].svr_transbyte,
				p[i].everyscr[j].svr_recvbyte,
				p[i].everyscr[j].term_transbyte,
				p[i].everyscr[j].term_recvbyte,
				p[i].everyscr[j].pingmax,
				p[i].everyscr[j].pingavg,
				p[i].everyscr[j].pingmin
				);
			}
			else
			{
				Get_time(systemtime);//time
				sprintf(query_str,
				"replace into devstate_scrinfo values('%u','1',%d,%d,'%s',NULL,'%s','%d.%d.%d.%d',%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d)",
				authen->struc_num,
				p[i].second,
				p[i].everyscr[j].scrcode,
				systemtime,
				p[i].everyscr[j].scr_proto,
				p[i].everyscr[j].svrip[0],p[i].everyscr[j].svrip[1],p[i].everyscr[j].svrip[2],p[i].everyscr[j].svrip[3],
				p[i].everyscr[j].tcp_svrport,
				p[i].everyscr[j].scr_stat,
				p[i].everyscr[j].scrterm_type,
				p[i].everyscr[j].svr_transbyte,
				p[i].everyscr[j].svr_recvbyte,
				p[i].everyscr[j].term_transbyte,
				p[i].everyscr[j].term_recvbyte,
				p[i].everyscr[j].pingmax,
				p[i].everyscr[j].pingavg,
				p[i].everyscr[j].pingmin
				);
			}
			int rc = mysql_real_query(&mysql, query_str, strlen(query_str));
			if (0 != rc)
			{
				printf("mysql_real_query(): %s\n", mysql_error(&mysql));
				return;
			}
		}
	}
}