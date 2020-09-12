#include <iostream>
#include "server.h"

using namespace std;

int main()
{
	CServerNet servernet;
	YZMOND yzmond;
	
	servernet.Read_yzmond(&yzmond);//读yzmond.conf
	
	int iRlt = servernet.Init(yzmond.port);
	if (iRlt == 0)
	{
		servernet.Run(&yzmond);
	}
	else
		cout <<"serverNet init failed with error:"<< iRlt << endl;
	return 0;
}