/***************************************************************************
 *   Copyright (C) 2013 by Yexr   *
 *   yexr@vveye.com   *
 ***************************************************************************/
#include <stdio.h>
#include "libt2usvr.h"

int main(int argc,char* argv[])
{
	int ret;
	char buff[10240];

	//ret = t2u_svr_init("122.225.102.4",8200,NULL,"TEST-123456",10);
	ret = t2u_svr_init("115.29.194.22",8000,NULL,"TEST-WHRZ-1003",10);
	printf("t2u_svr_init:%d\n",ret);

	while(1)
	{
		sleep(2);
		
		ret = t2u_svr_status();
		printf("t2u_svr_status:%d\n",ret);

		ret = t2u_svr_conn_stats(buff,sizeof(buff));
		if (ret>0)
		{
			printf("%s\n",buff);
		}
	}
}

