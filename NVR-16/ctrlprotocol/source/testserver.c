#include <stdio.h>
#include <string.h>

#include "ctrlprotocol.h"

#ifdef WIN32
#pragma comment( lib, "Ws2_32.lib" )
#else
#include <unistd.h>
#define Sleep(x) usleep(1000*(x))

#include <sys/types.h>
#include <linux/unistd.h>
_syscall0(pid_t, gettid);
#endif

u16 DealCommand(CPHandle cph,u16 event,u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext)
{
	ifly_monitor_param_t mp;
	struct in_addr tmpAddr;
	tmpAddr.s_addr = cph->ip;
	printf("hehe,recv event:%d from(%s,%d)\n", event, inet_ntoa(tmpAddr), ntohs(cph->port));
	if(pAckLen) *pAckLen = 0;
	switch(event)
	{
	case CTRL_CMD_STARTVIDEOMONITOR:
		memcpy(&mp,pbyMsgBuf,sizeof(ifly_monitor_param_t));
		tmpAddr.s_addr = mp.dwIp;
		printf("start mp:(%s,%d,%d)\n", inet_ntoa(tmpAddr), ntohs(mp.wPort), mp.byChn);
		return CTRL_FAILED_OUTOFMEMORY;
		break;
	case CTRL_CMD_STOPVIDEOMONITOR:
		memcpy(&mp,pbyMsgBuf,sizeof(ifly_monitor_param_t));
		tmpAddr.s_addr = mp.dwIp;
		printf("stop mp:(%s,%d,%d)\n", inet_ntoa(tmpAddr), ntohs(mp.wPort), mp.byChn);
		return CTRL_FAILED_RESOURCE;
		break;
	}
	
	return CTRL_SUCCESS;
}

int main()
{
	SetMsgCallBack(DealCommand,NULL);
	//CPLibInit(CTRL_PROTOCOL_SERVERPORT);
	CPLibInit(server_port);
	
	while(1)
	{
		Sleep(1000);
	}

	CPLibCleanup(TRUE);

	return 0;
}
