#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctrlprotocol.h"

#ifdef WIN32
#pragma comment( lib, "Ws2_32.lib" )
#else
#include <sys/types.h>
#include <linux/unistd.h>
//_syscall0(pid_t, gettid);
#endif

//#define __DEBUG__

int main(int argc,char **argv)
{
	ifly_cp_header_t cphead;
	u32 tmplen = 0;
	
#ifdef __DEBUG__
	char *envvalue = NULL;
	FILE *fp = NULL;
#endif
	
	if(argc < 2)
	{
		return CTRL_FAILED_PARAM;
	}
	
#ifdef WIN32
	//³õÊ¼»¯Ì×½á×Ö¿â
	{
		WSADATA wsaData;
		u32 err;
		
		err = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if(err != 0)
		{
			return CTRL_FAILED_PARAM;
		}
	}
#endif
	
#ifdef __DEBUG__
	fp = fopen("mydebug.log","a");
	if(fp == NULL)
	{
		return -1;
	}
	fputs("ppp log\n",fp);
#endif
	
	cphead.length	= sizeof(ifly_cp_header_t);
	cphead.type		= CTRL_COMMAND;
	cphead.number	= GetTransactionNum();
	cphead.version  = CTRL_VERSION;
	
	switch(atoi(argv[1]))
	{
	case CTRL_CMD_PPPOE_PREUP:
		{
			#ifdef __DEBUG__
			envvalue = getenv("IPLOCAL");
			if(envvalue != NULL)
			{
				fputs("IPLOCAL:",fp);
				fputs(envvalue,fp);
				fputs("\n",fp);
			}
			envvalue = getenv("IPREMOTE");
			if(envvalue != NULL)
			{
				fputs("IPREMOTE:",fp);
				fputs(envvalue,fp);
				fputs("\n",fp);
			}
			envvalue = getenv("DNS1");
			if(envvalue != NULL)
			{
				fputs("DNS1:",fp);
				fputs(envvalue,fp);
				fputs("\n",fp);
			}
			envvalue = getenv("DNS2");
			if(envvalue != NULL)
			{
				fputs("DNS2:",fp);
				fputs(envvalue,fp);
				fputs("\n",fp);
			}
			envvalue = getenv("USEPEERDNS");
			if(envvalue != NULL)
			{
				fputs("USEPEERDNS:",fp);
				fputs(envvalue,fp);
				fputs("\n",fp);
			}
			#endif
			cphead.event = CTRL_CMD_PPPOE_PREUP;

			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,&cphead,sizeof(cphead));
		}
		break;
	case CTRL_CMD_PPPOE_UP:
		{
			cphead.event = CTRL_CMD_PPPOE_UP;

			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,&cphead,sizeof(cphead));
		}
		break;
	case CTRL_CMD_PPPOE_DOWN:
		{
			cphead.event = CTRL_CMD_PPPOE_DOWN;

			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,&cphead,sizeof(cphead));
		}
		break;
	case CTRL_CMD_PPPOE_DISCONNECT:
		{
			cphead.event = CTRL_CMD_PPPOE_DISCONNECT;

			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,&cphead,sizeof(cphead));
		}
		break;
	case CTRL_CMD_DHCP_DECONFIG:
		{
			ifly_cp_dhcp_t dhcp;
			u8 buffer[256];
			memset(&dhcp,0,sizeof(dhcp));
			strncpy(dhcp.ifrname,argv[2],sizeof(dhcp.ifrname));
			//strcpy(dhcp.ifrname,argv[2]);
			/*dhcp.ip = atoi(argv[3]);
			dhcp.subnet = atoi(argv[4]);
			dhcp.broadcast = atoi(argv[5]);
			dhcp.gateway = atoi(argv[6]);
			dhcp.dns = atoi(argv[7]);*/
			cphead.event = CTRL_CMD_DHCP_DECONFIG;
			cphead.length = sizeof(cphead)+sizeof(dhcp);

			tmplen	= cphead.length;
			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);

			memcpy(buffer,&cphead,sizeof(cphead));
			memcpy(buffer+sizeof(cphead),&dhcp,sizeof(dhcp));

			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,buffer,tmplen);
		}
		break;
	case CTRL_CMD_DHCP_BOUND:
		{
			ifly_cp_dhcp_t dhcp;
			u8 buffer[256];
			memset(&dhcp,0,sizeof(dhcp));
			strncpy(dhcp.ifrname,argv[2],sizeof(dhcp.ifrname));
			//strcpy(dhcp.ifrname,argv[2]);
			/*dhcp.ip = atoi(argv[3]);
			dhcp.subnet = atoi(argv[4]);
			dhcp.broadcast = atoi(argv[5]);
			dhcp.gateway = atoi(argv[6]);
			dhcp.dns = atoi(argv[7]);*/
			cphead.event = CTRL_CMD_DHCP_BOUND;
			cphead.length = sizeof(cphead)+sizeof(dhcp);
			tmplen	= (cphead.length);
			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);

			memcpy(buffer,&cphead,sizeof(cphead));
			memcpy(buffer+sizeof(cphead),&dhcp,sizeof(dhcp));
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,buffer,tmplen);
		}
		break;
	case CTRL_CMD_DHCP_RENEW:
		{
			ifly_cp_dhcp_t dhcp;
			u8 buffer[256];
			memset(&dhcp,0,sizeof(dhcp));
			strncpy(dhcp.ifrname,argv[2],sizeof(dhcp.ifrname));
			//strcpy(dhcp.ifrname,argv[2]);
			/*dhcp.ip = atoi(argv[3]);
			dhcp.subnet = atoi(argv[4]);
			dhcp.broadcast = atoi(argv[5]);
			dhcp.gateway = atoi(argv[6]);
			dhcp.dns = atoi(argv[7]);*/
			cphead.event = CTRL_CMD_DHCP_RENEW;
			cphead.length = sizeof(cphead)+sizeof(dhcp);
			tmplen	= (cphead.length);
			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			memcpy(buffer,&cphead,sizeof(cphead));
			memcpy(buffer+sizeof(cphead),&dhcp,sizeof(dhcp));

			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,buffer,tmplen);
		}
		break;
	case CTRL_CMD_DHCP_NAK:
		{
			cphead.event = CTRL_CMD_DHCP_NAK;

			cphead.length	= htonl(cphead.length);
			cphead.type		= htons(cphead.type);
			cphead.number	= htons(cphead.number);
			cphead.version  = htons(cphead.version);
			cphead.event = htons(cphead.event);
			SendToHost(inet_addr("127.0.0.1"),CTRL_PROTOCOL_CONNECT_DEFAULT,&cphead,sizeof(cphead));
		}
		break;
	default:
		break;
	}
	
#ifdef __DEBUG__
	fclose(fp);
#endif
	
	return CTRL_SUCCESS;
}
