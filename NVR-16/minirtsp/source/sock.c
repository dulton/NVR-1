#include "sock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma comment (lib,"WS2_32.lib")  
#endif

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

SOCK_t SOCK_new(int af,int type,int protocal)
{
	SOCK_t sock;
#if defined(_WIN32) || defined(_WIN64)
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(1,1),&wsaData);
	if(ret != 0)
	{
		printf("WSAStartup failed!\n");
		exit(1);
	}
	sock = socket(af,type,protocal);
	if(sock == INVALID_SOCKET)
	{
		printf("create socket failed!\n");
		WSACleanup();
		exit(1);
	}
#else
	sock = socket(af,type,protocal);
	if(sock < 0)
	{
		printf("create socket failed!\n");
		exit(1);
	}
#endif
	return sock;
}

#if 0
SOCK_t SOCK_tcp_listen(int listen_port) 
{
    int ret;
	int on = 1;
	SOCKADDR_IN_t addr;
	
    SOCK_t sock = SOCK_new(AF_INET, SOCK_STREAM, 0);
	
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0)
	{
        printf("SOCK-ERROR: set port reuse failed\n");
		exit(1);
	}
	
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sock,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_t));
    if(ret < 0)
	{
        printf("SOCK-ERROR: bind failed @ SOCK_ERR=%d\n",SOCK_ERR);
        exit(1);
    }
	else
	{
        printf("SOCK-INFO: bind ok");
    }
    ret = listen(sock,32);
    if(ret < 0)
	{
        printf("SOCK-ERROR: listen failed @ SOCK_ERR=%d\n",SOCK_ERR);
        exit(1);
    }
	else
	{
        printf("SOCK-INFO: listen start success @%d\n",listen_port);
    }
	
    return sock;
}
#endif

int SOCK_tcp_init(SOCK_t sock,int rwtimeout/* unit: millisecond */) 
{
    int ret;
	int on = 1;
	
#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	printf("sock:%d\n",sock);
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(SOCKOPTARG_t *) &timeo, sizeof(timeo));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set send timeout failed,sock:%d",sock);
		return -1;
	}
    //set receive timeout
    ret = setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set receive timeout failed.");
		return -1;
	}
    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (SOCKOPTARG_t *) &on, sizeof(on));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set nodelay failed.");
		return -1;
	}
    return ret;
}

SOCK_t SOCK_tcp_connect(char *ip,int port,int rwtimeout) 
{
    int ret;
	int on = 1;
	SOCK_t sock;
	SOCKADDR_IN_t addr;
	
#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	
    sock = SOCK_new(AF_INET, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	ret = connect(sock,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_IN_t));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: connect to %s:%d failed.",ip,port);
		SOCK_close(sock);
		return -1;
	}
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (SOCKOPTARG_t *) &timeo, sizeof(timeo));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set send timeout failed.");
		SOCK_close(sock);
		return -1;
	}
    //set receive timeout
    ret = setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set receive timeout failed.");
		SOCK_close(sock);
		return -1;
	}
    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (SOCKOPTARG_t *) &on, sizeof(on));
	if(ret < 0)
	{
    	printf("SOCK-ERROR: set nodelay failed.");
		SOCK_close(sock);
		return -1;
	}
	//printf("SOCK-INFO: connect to %s:%d success, fd=%d",ip,port,sock);
    return sock;
}

int SOCK_tcp_connect2(char *target_ip, int target_port, int rw_timeout)
{
#if defined(_WIN32) || defined(_WIN64)
	return SOCK_tcp_connect(target_ip,target_port,rw_timeout);
#else
	int ret = 0;
	struct sockaddr_in peer_addr;
	unsigned long unblock_flag = 0;
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(target_port);
	peer_addr.sin_addr.s_addr = inet_addr(target_ip);
	bzero(&(peer_addr.sin_zero), 8);
	
	unblock_flag = 1;
	ioctl(sock, FIONBIO, (typeof(unblock_flag)*)(&unblock_flag)); // unblocked connect mode
	
	///////////////////////////////////////////////////////
	ret = connect(sock, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		if(EINPROGRESS != errno)
		{
			printf("SOCK-ERR: Connect to %s:%d error(\"%s\")!\n", target_ip,target_port,strerror(errno));
			close(sock);
			return -1;
		}
		else
		{
			struct timeval poll_timeo;
			fd_set wfd_set;
			FD_ZERO(&wfd_set);
			FD_SET(sock, &wfd_set);
			poll_timeo.tv_sec = 5;//2;//csp modify
			poll_timeo.tv_usec = 0;
			ret = select(sock + 1, NULL, &wfd_set, NULL, &poll_timeo);
			if(ret <= 0)
			{
				close(sock);
				printf("SOCK-ERR: Connect select error(\"%s\")!\n", strerror(errno));
				return -1;
			}
			else
			{
				if(FD_ISSET(sock, &wfd_set))
				{
					int sock_error;
					socklen_t sock_err_len = sizeof(sock_error);
					if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&sock_error, &sock_err_len) < 0)
					{
						printf("SOCK-ERR: getsockopt: SO_ERROR failed(\"%s\")!\n", strerror(errno));
						close(sock);
						return -1;
					}
					if(0 != sock_error)
					{
						printf("SOCK-ERR: getsockopt: SO_ERROR not zero(\"%s\")!\n", strerror(errno));
						close(sock);
						return -1;
					}
				}
			}
		}
	}
	
	unblock_flag = 0;
	ioctl(sock, FIONBIO, (typeof(unblock_flag)*)(&unblock_flag));
	
	do
	{
		//csp modify 20140219
		//struct timeval sock_timeo;
		//sock_timeo.tv_sec = rw_timeout;
		//sock_timeo.tv_usec = 0;
		struct timeval sock_timeo;
		sock_timeo.tv_sec = rw_timeout/1000;
		sock_timeo.tv_usec = (rw_timeout%1000)*1000;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &sock_timeo, sizeof(sock_timeo));
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &sock_timeo, sizeof(sock_timeo));
	}while(0);
	
	return sock;
#endif
}

SOCK_t SOCK_udp_init(int port,int rwtimeout/* unit: millisecond */)
{
	int ret;
	int on = 1;
	int buf_size;
	SOCKLEN_t optlen;
	SOCK_t sock = SOCK_new(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN_t my_addr;
	
#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	
	// set addr reuse
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0)
	{
		printf("SOCK-ERROR: set port reuse failed");
		return -1;
	}
	//set send timeout
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (SOCKOPTARG_t *) &timeo, sizeof(timeo));
	if(ret < 0)
	{
		printf("SOCK-ERROR: set send timeout failed");
		return -1;
	}
	//set receive timeout
	ret = setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0)
	{
        printf("SOCK-ERROR: set recv timeout failed");
		return -1;
	}
	// set buffer size
	// Get buffer size
	optlen = sizeof(buf_size);
	ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buf_size, &optlen);
	if(ret < 0)
	{
		printf("SOCK-ERROR: get buffer size failed");
		return -1;
	}
	//else
	//{
	//	printf("SOCK-DEBUG: send buffer size = %d\n", buf_size);
	//}
	// Set buffer size
	buf_size = 16*1024;
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
	if(ret < 0)
	{
		printf("SOCK-ERROR: set buffer size:%d failed",buf_size);
		return -1;
	}
	optlen = sizeof(buf_size);
	ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf_size, &optlen);
	if(ret < 0)
	{
		printf("SOCK-ERROR: get recv buffer size failed");
		return -1;
	}
	//else
	//{
	//	printf("SOCK-DEBUG: recv buffer size = %d\n", buf_size);
	//}
	// Set buffer size
	buf_size = UDP_SOCK_BUF_SIZE/2;
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
	if(ret < 0)
	{
		printf("SOCK-ERROR: set recv buffer size:%d failed",buf_size);
		return -1;
	}
	//
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//bind
	ret = bind(sock, (SOCKADDR_t*)&my_addr, sizeof(SOCKADDR_t));
	if(ret < 0)
	{
        printf("SOCK-ERROR: udp bind failed");
		return -1;
	}
	//printf("SOCK-DEBUG:create udp port:%d(sock:%d) ok.",port,sock);
	
	return sock;
}

int SOCK_recv(SOCK_t sock,char *buf,int size,int flag) 
{
	int ret = recv(sock,buf,size,flag);
	if(ret < 0)
	{
		printf("SOCK-ERROR: read failed");
		return -1;
	}
	else if(ret == 0)
	{
		printf("SOCK-ERROR: peer is shut down");
		return -1;
	}
	printf("SOCK-DEBUG:tcp recv %d",ret);
	return ret;
}

int SOCK_recv2(SOCK_t fd,char *buf,int size,int flag) 
{
	int ret = 0;
	int received = 0;
	char *pbuf = buf;
	if(size <= 0) return 0;
	while(1)
	{
		ret = recv(fd,pbuf,size-received,flag);
		if(ret == -1)
		{
			if(SOCK_ERR == SOCK_EINTR)
			{
				//printf"SOCK-DEBUG:######## tcp recv error %d ############",SOCK_ERR);
				//csp modify 20140221
				//continue;
				return -1;
			}
			else if(SOCK_ERR == SOCK_EAGAIN)
			{
				//printf"SOCK-DEBUG:######## tcp recv error %d ############",SOCK_ERR);
				//csp modify 20140221
				//continue;
				return -1;
			}
			else if(SOCK_ERR == SOCK_ETIMEOUT)
			{
				printf("SOCK-ERROR: ######## tcp recv time out ##########");
				return -1;
			}
			//printf("SOCK-ERROR: ####### tcp recv error @%d ##############",SOCK_ERR);
			if(SOCK_ERR == 14)
			{
				printf("buf:%p size:%d\n",buf,size);
			}
			return -1;
		} 
		else if(ret == 0)
		{
			//csp modify 20140221
			//break;
			return -1;
		}
		else
		{
			pbuf += ret;
			received += ret;
		}
		if(received == size) break;
    }
    //buf[received] = 0;
    //printf("SOCK-DEBUG: tcp recv %d\n",received);
    return received;
}

extern int loopsend(SOCK_t s, char *buf, unsigned int sndsize);

int SOCK_send(SOCK_t sock,char *buf,int size) 
{
#if 1//csp modify 20140226
	int ret = loopsend(sock,buf,size);
#else
	int ret = send(sock,buf,size,0);
#endif
	if(ret != size)
	{
		printf("SOCK-ERROR: send @sock=%d ret=%d error=%d failed.\n",sock,ret,SOCK_ERR);
		return RET_FAIL;
	}
	else
	{
		//printf("SOCK-DEBUG: send @sock=%d size=%d success.\n",sock,size);
	}
	return RET_OK;
}

int SOCK_recvfrom(SOCK_t sock,char *ip,int *port,char *buf,int size,int flags) 
{
	int ret;
	SOCKADDR_IN_t addr;
	SOCKLEN_t addrlen = sizeof(SOCKADDR_IN_t);
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	/*
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(ip == NULL)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr.s_addr = inet_addr(ip);
	*/
	ret = recvfrom(sock,buf,size,flags,(SOCKADDR_t *)&addr,&addrlen);
	if(ret < 0)
	{
		printf("SOCK-ERROR: udp recvfrom failed,size:%d sock:%d buf:%p SOCK_ERR:%d",
			size,sock,buf,SOCK_ERR);
		return RET_FAIL;
	}
	else if(ret == 0)
	{
		printf("SOCK-ERROR: peer is shut down");
		return RET_FAIL;
	}
	//printf("udp recvfrom(%s:%d) success,size:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret);
	printf("SOCK-DEBUG: udp recvfrom(%s:%d) success,size:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret);
	strcpy(ip,inet_ntoa(addr.sin_addr));
	
	*port = ntohs(addr.sin_port);
	
	return ret;
}

int SOCK_sendto(SOCK_t sock,char *ip,int port,char *buf,int size) 
{
	int ret;
	SOCKADDR_IN_t addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	
	ret = sendto(sock,buf,size,0,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_IN_t));
	if(ret != size)
	{
		printf("SOCK-ERROR: udp send to %s:%d failed,size:%d sock:%d buf:%p SOCK_ERR:%d",
			ip,port,size,sock,buf,SOCK_ERR);
		return RET_FAIL;
	}
	
	//printf("SOCK-DEBUG: udp send to%s:%d uccess,size:%d",ip,port,size);
	
	return RET_OK;
}

int SOCK_getpeername(SOCK_t sock,char *peer)
{
	SOCKADDR_IN_t addr;//SOCKADDR_t addr;//csp modify
	SOCKADDR_IN_t *addr_in = (SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len = sizeof(addr);//sizeof(SOCKADDR_t);//csp modify
	if(getpeername(sock,(SOCKADDR_t *)&addr,&sock_len) != 0)
	{
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	
	strcpy(peer,inet_ntoa(addr_in->sin_addr));
	//printf("SOCK-DEBUG: peer name:%s",peer);
	
	return RET_OK;
}

int SOCK_getsockname(SOCK_t sock,char *ip)
{
	SOCKADDR_IN_t addr;//SOCKADDR_t addr;//csp modify
	SOCKADDR_IN_t *addr_in = (SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len = sizeof(addr);//sizeof(SOCKADDR_t);//csp modify
	if(getsockname(sock,(SOCKADDR_t *)&addr,&sock_len) != 0)
	{
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	
	strcpy(ip,inet_ntoa(addr_in->sin_addr));
	//printf("SOCK-DEBUG: sock name:%s",ip);
	
	return RET_OK;
}

int SOCK_getsockport(SOCK_t sock,unsigned short * const port)
{
	SOCKADDR_IN_t addr;//SOCKADDR_t addr;//csp modify
	SOCKADDR_IN_t *addr_in = (SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len = sizeof(addr);//sizeof(SOCKADDR_t);//csp modify
	if(getsockname(sock,(SOCKADDR_t *)&addr,&sock_len) != 0)
	{
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	
	*port = ntohs(addr_in->sin_port);
	printf("SOCK-PORT: sock port:%d",*port);
	
	return RET_OK;
}

int SOCK_gethostbyname(char *name,char *ip) 
{
    struct hostent *hent;
	struct in_addr addr;
	int i;
	
    hent = gethostbyname(name);
	if(hent == NULL)
	{
		printf("gethostbyname failed!\n");
		return -1;
	}
	for(i = 0; hent->h_aliases[i];i++)
	{
		printf("aliase%d %s\n",i+1,hent->h_aliases[i]);
	}
    printf("hostname: %s addrtype:%d\naddress list: ", hent->h_name,hent->h_addrtype);
	if(hent->h_addrtype == AF_INET)
	{
		for(i = 0; hent->h_addr_list[i]; i++)
		{
#if defined(_WIN32) || defined(_WIN64)
			addr.s_addr = *(u_long *)hent->h_addr_list[i];
#else
			addr.s_addr = *(unsigned int *)hent->h_addr_list[i];
#endif
			printf("<<%d>> %s\n", i+1,inet_ntoa(addr));
			if(i == 0)
			{
				strcpy(ip,inet_ntoa(addr));
			}
		}
	}
	else
	{
		return -1;
	}
    return 0;
}

int SOCK_gethostname(char *ip) 
{
#if defined(_WIN32) || defined(_WIN64)
	printf("not support!!!!!!!!!!\n");
#else
    struct ifaddrs *ifAddrStruct = NULL, *ifaddr_bak = NULL;
    void *tmpAddrPtr = NULL;
	
    getifaddrs(&ifAddrStruct);
	ifaddr_bak = ifAddrStruct;
	
    while(ifAddrStruct != NULL)
	{
        if(ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			// check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
			if((strcmp(ifAddrStruct->ifa_name,"lo") != 0) && (strchr(ifAddrStruct->ifa_name,':') == NULL))
			{
				strcpy(ip,addressBuffer);
				printf("select ip address:%s\n",ip);
				break;
			}
        }
		else if(ifAddrStruct->ifa_addr->sa_family == AF_INET6)
		{
			// check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }	
	if(ifaddr_bak) freeifaddrs(ifaddr_bak);
#endif
    return 0;
}

int SOCK_getsockinfo(SOCK_t sock,char *ip,char *mask)
{
#if defined(_WIN32) || defined(_WIN64)
	return -1;
#else
	SOCKADDR_IN_t *my_ip;
	SOCKADDR_IN_t *addr;
	SOCKADDR_IN_t myip;	
	struct ifreq ifr;
	
	my_ip = &myip;
	// get local ip of eth0
	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		printf("SOCK-ERROR: get local ip failed");
		return -1;
	}
	my_ip->sin_addr = ((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr;
	strcpy(ip,inet_ntoa(my_ip->sin_addr));
	
	// get local netmask
	if(ioctl(sock, SIOCGIFNETMASK, &ifr) == -1)
	{
		printf("SOCK-ERROR: get netmask failed");
		return -1;
	}
	addr = (struct sockaddr_in *) & (ifr.ifr_addr);
	strcpy(mask,inet_ntoa( addr->sin_addr));
	
	printf("SOCK-INFO: sockinfo: ip:%s mask:%s",ip,mask);
	return 0;
#endif
}

int SOCK_get_blockmode(SOCK_t sock)
{
	int ret = 0;
#if defined(_WIN32) || defined(_WIN64)
	return -1;
#else
	ret= fcntl(sock,F_GETFL,0);
	if(ret < 0)
	{
		printf("SOCK-ERROR: SOCK fcntl failed");
		return RET_FAIL;
	}
	if(ret & O_NONBLOCK)
	{
		printf("SOCK-INFO: SOCK in nonblock mode");
		return FALSE;
	}
	else
	{
		printf("SOCK-INFO: SOCK in block mode");
		return TRUE;
	}
#endif
}

int SOCK_set_blockmode(SOCK_t sock,int enable)
{
	int ret = 0;
#if defined(_WIN32) || defined(_WIN64)
#else
	ret = fcntl(sock,F_GETFL,0);
	if(ret < 0)
	{
		printf("SOCK-ERROR: SOCK fcntl failed");
		return RET_FAIL;
	}	
	if(ret & O_NONBLOCK)
	{
		printf("SOCK-INFO: SOCK in nonblock mode");
	}
	else
	{
		printf("SOCK-INFO: SOCK in block mode");
	}
	if(enable == TRUE)
	{
		ret &= ~O_NONBLOCK;
	}
	else
	{
		ret |= O_NONBLOCK;
	}
	if(fcntl(sock,F_SETFL,ret) < 0)
	{
		printf("SOCK-ERROR: SOCK set fcntl failed");
		return RET_FAIL;
	}
#endif
	return RET_OK;
}

int SOCK_isreservedip(char *szIp)
{
	int ret = 0;
	int flag = FALSE;
	
	//csp modify:avoid warning
	//unsigned char ip[4];
	unsigned int ip[4];
	
	ret = sscanf(szIp,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
	//printf("ret = %d\n",ret);
	if(ret != 4)
	{
		printf("SOCK_isreservedip: ret = %d\n",ret);
		printf("SOCK-ERROR: ipaddr:%s invalid",szIp);
		return -1;
	}
	if(ip[0] == 10) flag = TRUE;
	if((ip[0] == 192) && (ip[1] == 168)) flag = TRUE;
	if((ip[0] == 172) && (ip[1] >= 16) && (ip[1] <= 31)) flag = TRUE;
	
	return flag;
}

// multicast ip address
// use for router and other function:224.0.0.0~224.0.0.255
// reserved: 224.0.1.0~238.255.255.255
// 239.0.0.0~239.255.255.255
int SOCK_add_membership(SOCK_t sock,char *multi_ip)
{
	struct ip_mreq mcaddr;
	memset(&mcaddr,0,sizeof(struct ip_mreq));
	// set src ip
	mcaddr.imr_interface.s_addr = htonl(INADDR_ANY);
	// set multicast address
	mcaddr.imr_multiaddr.s_addr = inet_addr(multi_ip);
	//if(inet_pton(AF_INET,multi_ip,&mcaddr.imr_multiaddr) <= 0)
	//{
	//	printf("wrong multicast ipaddress!\n");
	//	return -1;
	//}
	//add membership
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcaddr,sizeof(struct ip_mreq)) < 0)
	{
		printf("add to membership failed,errno:%d!\n",SOCK_ERR);
		return -1;
	}
	printf("add to membership:%s ok!\n",multi_ip);
	return 0;
}

int SOCK_set_broadcast(int sock)
{
	int so_broadcast = 1;
	int ret = setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&so_broadcast,sizeof(so_broadcast));
	if(ret < 0)
	{
		printf("set broadcast failed1\n");
		return -1;
	}
	return 0;
}

