#include "ctrlprotocol.h"
//#include "common.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#pragma comment( lib, "Ws2_32.lib" )
#else
#include <sys/types.h>
#include <linux/unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <linux/unistd.h>
_syscall0(pid_t, gettid);
#endif

int main()
{
	ifly_monitor_param_t mp;
	//ifly_remote_playback_t rp;
	u32 dwServerIp;
	u16 wServerPort;
	
	char szHostName[64];
	struct hostent* pHostLocal;
	int  count=0;
	
	u16 wRet;
	//DWORD start,end;
	
	int i;
	
	CPLibInit(0);
	
	{
		int s = socket(AF_INET,SOCK_DGRAM,0);
		int nRealBufSize = 0;
		#ifdef WIN32
		int nSize = sizeof(nRealBufSize);
		#else
		socklen_t nSize = sizeof(nRealBufSize);
		#endif
		int nTotalBufSize = 64000;
		
		if( SOCKET_ERROR == getsockopt( s, SOL_SOCKET, SO_SNDBUF,
			(char*)&nRealBufSize, &nSize) )
		{
			printf("Socket getsockopt SO_SNDBUF Error\n");
		}
		else
		{
			printf("Socket getsockopt SO_SNDBUF success,nRealBufSize=%d,nSize=%d\n",nRealBufSize,nSize);
		}
		
		if( SOCKET_ERROR == setsockopt( s, SOL_SOCKET, SO_SNDBUF,
			(char *)&nTotalBufSize, sizeof(int)) )
		{
			printf("Socket setsockopt SO_SNDBUF : %d error\n",nTotalBufSize);
		}
		else
		{
			printf("Socket setsockopt SO_SNDBUF : %d OK\n",nTotalBufSize);
		}
		
		nRealBufSize = 0;
		if( SOCKET_ERROR == getsockopt( s, SOL_SOCKET, SO_SNDBUF,
			(char*)&nRealBufSize, &nSize) )
		{
			printf("Socket getsockopt SO_SNDBUF Error\n");
		}
		else
		{
			printf("Socket getsockopt SO_SNDBUF success,nRealBufSize=%d,nSize=%d,errno=%d,errstr=%s\n",nRealBufSize,nSize,errno,strerror(errno));
		}
		
		#ifdef WIN32
		closesocket(s);
		#else
		close(s);
		#endif
		
		//return 0;
	}
	
	gethostname(szHostName, sizeof(szHostName));
	pHostLocal = gethostbyname(szHostName);
	
	for(i=0;;i++)
	{
		count++;
		if(pHostLocal->h_addr_list[i] + pHostLocal->h_length >= pHostLocal->h_name)
		{
			break;
		}	
	}
	
	mp.dwIp = inet_addr(inet_ntoa(*(struct in_addr *)pHostLocal->h_addr_list[0]));//inet_addr("224.1.2.3");
	printf("szHostName=%s,mp.dwIp=0x%08x,ip count=%d,all_addr len=%d,in_addr len=%d\n",szHostName,mp.dwIp,count,pHostLocal->h_length,sizeof(struct in_addr));
	
	dwServerIp = inet_addr("192.168.1.112");
	//wServerPort = CTRL_PROTOCOL_SERVERPORT;
	wServerPort = server_port;
	
	{
		ifly_cp_header_t cphead;
		CPHandle cph = CPConnect(dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK,NULL);
		if(cph != NULL)
		{
			mp.byChn = 0;
			mp.dwIp = inet_addr("192.168.1.110");
			mp.wPort = htons(64000);
			wRet = CPSend(cph,CTRL_CMD_STARTVIDEOMONITOR,&mp,sizeof(mp),&cphead,sizeof(cphead),NULL,CTRL_PROTOCOL_CONNECT_DEFAULT);
			printf("result:%d,ack:%d\n",wRet,cphead.event);
		}
		while(1)
		{
			#ifdef WIN32
			Sleep(1000);
			#else
			sleep(1);
			#endif
		}
	}
	
	CPLibCleanup();
	
	return 0;
}
