#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>

#include "common.h"

//csp modify 20130519
#include <err.h>

//#define	 ETH_NAME "eth0"
#define	 ETH_NAME "eth0"

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

u32 GetLocalIp()
{
	int	sock;
	struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return 0;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(sock,SIOCGIFADDR,&ifr)<0)
	{
		perror("GetLocalIp-ioctl");
		close(sock);
		return 0;
	}
    
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	//fprintf(stdout,"%s   ip:   %s\n",ETH_NAME,inet_ntoa(sin.sin_addr));
    
	close(sock);
	
	return sin.sin_addr.s_addr;
}

u32 GetLocalIp2(char * eth_name)
{
	int	sock;
	struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return 0;
	}
	//printf("yg 1 ETH_NAME: %s\n", ETH_NAME);
    	if(eth_name==NULL)
    	{
		strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
		//printf("yg 2 ifr.ifr_name: %s\n", ifr.ifr_name);
    	}
	else
	{
		strncpy(ifr.ifr_name,eth_name,IFNAMSIZ);
		//printf("yg 3 ifr.ifr_name: %s\n", ifr.ifr_name);
	}
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(sock,SIOCGIFADDR,&ifr)<0)
	{
		perror("GetLocalIp2-ioctl");
		close(sock);
		return 0;
	}
    
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	if(eth_name==NULL)
	{
		fprintf(stdout,"%s   ip:   %s###\n",ETH_NAME,inet_ntoa(sin.sin_addr));
	}
	else
	{
		fprintf(stdout,"%s   ip:   %s###\n",eth_name,inet_ntoa(sin.sin_addr));
	}
	
	close(sock);
	
	return sin.sin_addr.s_addr;
}

int SetLocalIp(u32 dwIp)
{
//	return 0;//RECOVER

	int	sock;
	//struct sockaddr sa;
	struct ifreq ifr;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return -1;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(sock,SIOCGIFADDR,&ifr)<0)
	{
		perror("SetLocalIp:ioctl--1");
		close(sock);
		return 0;
	}

	((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr.s_addr = dwIp;
	if(ioctl(sock,SIOCSIFADDR,&ifr)<0)
	{
		perror("SetLocalIp:ioctl--2");
		printf("SetLocalIp:ioctl:error:(%d,%s)\n",errno,strerror(errno));
		close(sock);
		return -1;
	}

	close(sock);

	//struct in_addr sin;
	//char cmd[64];
	//sin.s_addr = dwIp;
	//sprintf(cmd,"hostname %s",inet_ntoa(sin));
	//system(cmd);
	
	return 0;
}

int SetHWAddr(char *pBuf)
{
	int sock = -1;
	//struct sockaddr_in sin;
	struct sockaddr sa;
	struct ifreq ifr;
	u8 mac[6];
	int i;
	char *strptr = NULL;
	char tempstr[4] = {0};
	
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("SetHWAddr:socket");
		return -1;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	
	strptr = pBuf;
	char *endptr = NULL;
	
	memset(mac,0,sizeof(mac));
	for(i=0;i<6;i++)
	{
		memset(tempstr,0,sizeof(tempstr));
		strncpy(tempstr, strptr, 2);
		mac[i] = strtoul(tempstr,&endptr,16);
		strptr += 3;
	}
	
#if 1//csp modify
	if(ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		perror("SetHWAddr:SIOCGIFFLAGS-1");
	}
	
	//system("ifconfig eth0 down");
	
	ifr.ifr_flags &= (~IFF_UP);
	if(ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
	{
		perror("SetHWAddr:SIOCSIFFLAGS-1");
	}
#endif
	
	if(ioctl(sock,SIOCGIFHWADDR,&ifr)<0)
	{
		perror("SetHWAddr:ioctl-1");
		close(sock);
		return 0;
	}
	
	memcpy(&sa,&ifr.ifr_addr,sizeof(sa));
	memcpy(sa.sa_data,mac,sizeof(mac));
	memcpy(&ifr.ifr_addr,&sa,sizeof(sa));
	//printf("sethwaddr : (%02x:%02x:%02x:%02x:%02x:%02x)\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	if(ioctl(sock,SIOCSIFHWADDR,&ifr)<0)
	{
		perror("SetHWAddr:ioctl-2");
		close(sock);
		return -1;
	}
	
#if 1//csp modify
	//system("ifconfig eth0 up");
	
	ifr.ifr_flags |= IFF_UP;
	if(ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
	{
		perror("SetHWAddr:SIOCSIFFLAGS-2");
	}
#endif
	
	close(sock);
	
	return 0;
}

int GetHWAddr(char *pBuf)
{
	int	sock;
	//struct sockaddr_in sin;
	struct sockaddr sa;
	struct ifreq ifr;
	u8 mac[6];
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return -1;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	memset(mac,0,sizeof(mac));
	if(ioctl(sock,SIOCGIFHWADDR,&ifr)<0)
	{
		perror("GetHWAddr-ioctl");
		close(sock);
		return -1;
	}
    
	memcpy(&sa,&ifr.ifr_addr,sizeof(sa));
	memcpy(mac,sa.sa_data,sizeof(mac));
	fprintf(stdout,"%s   mac:  %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",ETH_NAME,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    sprintf(pBuf,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	close(sock);
	
	return 0;
}

u32 GetNetMask()
{
	int	sock;
	struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return 0;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(sock,SIOCGIFNETMASK,&ifr)<0)
	{
		perror("GetNetMask-ioctl");
		close(sock);
		return 0;
	}
    
	memcpy(&sin,&ifr.ifr_netmask,sizeof(sin));
	fprintf(stdout,"%s   mask: %s\n",ETH_NAME,inet_ntoa(sin.sin_addr));
    
	close(sock);
	
	return sin.sin_addr.s_addr;
}
#define FLAGS_GOOD (IFF_UP          | IFF_BROADCAST)
#define FLAGS_MASK (IFF_UP          | IFF_BROADCAST | \
		    IFF_POINTOPOINT | IFF_LOOPBACK  | IFF_NOARP)
#define SIN_ADDR(x)	(((struct sockaddr_in *) (&(x)))->sin_addr.s_addr)

u32 GetNetMask2(char * eth_name)
{
#if 1
	struct sockaddr_in sin;
	s32 mask=0;//s32 mask=0, nmask=0, ina=0;
	struct ifreq *ifr, *ifend, ifreq;
	struct ifconf ifc;
	struct ifreq ifs[4];
	
	int sock_fd = -1;
	sock_fd = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
	if(sock_fd == -1)
	{
		perror("GetNetMask2-socket");
		return 0;
	}
	/*
	* Scan through the system's network interfaces.
	*/
	ifc.ifc_len = sizeof(ifs);
	ifc.ifc_req = ifs;
	if (ioctl(sock_fd, SIOCGIFCONF, &ifc) < 0) {
	    warn("GetNetMask2-ioctl(SIOCGIFCONF): %m (line %d)", __LINE__);
		return 0;
	}
	
	ifend = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);
	for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
		/*
		* Check the interface's internet address.
		*/
		if (ifr->ifr_addr.sa_family != AF_INET)
		    continue;
		
		/*
		* Check that the interface is up, and not point-to-point nor loopback.
		*/
		if(strncmp(ifr->ifr_name,"ppp0",4))
		{
			printf("if name:%s \n",ifr->ifr_name)	;
			continue;
		}
		//printf("OK get mask\n");
		
		strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
		if (ioctl(sock_fd, SIOCGIFFLAGS, &ifreq) < 0)
		{
			printf("GetNetMask2-ioctrl error2 \n");	
		    continue;
		}
/*
		if (((ifreq.ifr_flags ^ FLAGS_GOOD) & FLAGS_MASK) == 0)
		{
		    continue;
		}
*/		
		/*
		* Get its netmask and OR it into our mask.
		*/
		if (ioctl(sock_fd, SIOCGIFNETMASK, &ifreq) < 0)
		{
			printf("ioctrl error \n");
			continue;
		}
		memcpy(&sin,&ifreq.ifr_netmask,sizeof(sin));
		printf("%s   mask: %s\n",ifr->ifr_name,inet_ntoa(sin.sin_addr));
		close(sock_fd);
		return sin.sin_addr.s_addr;
		//mask = SIN_ADDR(ifreq.ifr_netmask);
		break;
	}
	return mask;

#else
	int	sock,len=0;
	struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return 0;
	}
  	if(eth_name==NULL)  
  	{
  		len=IFNAMSIZ;
		strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
  	}
	else
	{
		printf("get mask eth:%s\n",eth_name);
		len=strlen(eth_name);
		strncpy(ifr.ifr_name,eth_name,len);
	}
	
	ifr.ifr_name[len-1] = 0;
    
	if(ioctl(sock,SIOCGIFNETMASK,&ifr)<0)
	{
		perror("GetNetMask2-ioctl");
		close(sock);
		return 0;
	}
    
	memcpy(&sin,&ifr.ifr_netmask,sizeof(sin));
	if(eth_name==NULL)
		fprintf(stdout,"%s   mask: %s\n",ETH_NAME,inet_ntoa(sin.sin_addr));
    	else
		fprintf(stdout,"%s   mask: %s\n",eth_name,inet_ntoa(sin.sin_addr));
	close(sock);
	
	return sin.sin_addr.s_addr;
	#endif	
}


u32 GetRemoteIP()
{
	struct sockaddr_in sin;
	s32 mask=0;//s32 mask=0, nmask=0, ina=0;
	struct ifreq *ifr, *ifend, ifreq;
	struct ifconf ifc;
	struct ifreq ifs[4];
	int sock_fd=0;
	
	sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd == -1)
	{
		perror("socket");
		return 0;
	}
	/*
	* Scan through the system's network interfaces.
	*/
	ifc.ifc_len = sizeof(ifs);
	ifc.ifc_req = ifs;
	if (ioctl(sock_fd, SIOCGIFCONF, &ifc) < 0) {
	
	    warn("ioctl(SIOCGIFCONF): %m (line %d)", __LINE__);
		return 0;
	}
	
	ifend = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);
	for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
		/*
		* Check the interface's internet address.
		*/
		if (ifr->ifr_addr.sa_family != AF_INET)
		    continue;
		
		/*
		* Check that the interface is up, and not point-to-point nor loopback.
		*/
		if(strncmp(ifr->ifr_name,"ppp0",4))
		{
			printf("if name:%s \n",ifr->ifr_name)	;
			continue;
		}
		printf("ok get remote ip\n");
		strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
		
		
		/*
		* Get its netmask and OR it into our mask.
		*/
		if (ioctl(sock_fd, SIOCGIFDSTADDR, &ifreq) < 0)
		    continue;
		memcpy(&sin,&ifreq.ifr_dstaddr,sizeof(sin));
		printf("ppp0  remoteip: %s\n",inet_ntoa(sin.sin_addr));
		close(sock_fd);
		mask |= SIN_ADDR(ifreq.ifr_dstaddr);
		break;
	}
	return mask;
}


int SetNetMask(u32 dwIp)
{
	int	sock;
	//struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return -1;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	
	if(ioctl(sock,SIOCGIFNETMASK,&ifr)<0)
	{
		perror("SetNetMask:ioctl--1");
		close(sock);
		return -1;
	}
    
	((struct sockaddr_in*)(&ifr.ifr_netmask))->sin_addr.s_addr = dwIp;
	if(ioctl(sock,SIOCSIFNETMASK,&ifr)<0)
	{
		perror("SetNetMask:ioctl--2");
		close(sock);
		return -1;
	}
	
	close(sock);
	
	return 0;
}

u32 GetBroadcast()
{
	int	sock;
	struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return 0;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(sock,SIOCGIFBRDADDR,&ifr)<0)
	{
		perror("GetBroadcast-ioctl");
		close(sock);
		return 0;
	}
    
	memcpy(&sin,&ifr.ifr_broadaddr,sizeof(sin));
	fprintf(stdout,"%s   broadcast: %s\n",ETH_NAME,inet_ntoa(sin.sin_addr));
    
	close(sock);
	
	return sin.sin_addr.s_addr;
}

int SetBroadcast(u32 dwIp)
{
	int	sock;
	//struct sockaddr_in sin;
	//struct sockaddr sa;
	struct ifreq ifr;
    
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("socket");
		return -1;
	}
    
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	
	if(ioctl(sock,SIOCGIFBRDADDR,&ifr)<0)
	{
		perror("SetBroadcast:ioctl--1");
		close(sock);
		return -1;
	}
    
	((struct sockaddr_in*)(&ifr.ifr_broadaddr))->sin_addr.s_addr = dwIp;
	if(ioctl(sock,SIOCSIFBRDADDR,&ifr)<0)
	{
		perror("SetBroadcast:ioctl--2");
		close(sock);
		return -1;
	}
	
	close(sock);
	
	return 0;
}

u32 GetDefaultGateway()
{
	FILE *fp;
	char devname[64];//flags[16], sdest[16], sgw[16];
	unsigned long int d, g, m;
	int flgs, ref, use, metric, mtu, win, ir;
	struct in_addr sin;
	//struct in_addr mask;
	
	u32 dwGateway = 0;
	
	fp = fopen("/proc/net/route", "r");
	if(fp == NULL)
	{
		return dwGateway;
	}
	
	/* Skip the first line. */
	if(fscanf(fp, "%*[^\n]\n") < 0)
	{
		fclose(fp);
		return dwGateway;		/* Empty or missing line, or read error. */
	}
	while(1)
	{
		int r;
		r = fscanf(fp,"%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
			devname, &d, &g, &flgs, &ref, &use, &metric, &m,
			&mtu, &win, &ir);
		if(r != 11)
		{
			/* EOF with no (nonspace) chars read. */
			if((r < 0) && feof(fp))
			{
				break;
			}
			else
			{
				continue;
			}
		}
		if(strcmp(devname,ETH_NAME) == 0 && d == 0 && (flgs&RTF_GATEWAY))
		{
			dwGateway = g;
			sin.s_addr = dwGateway;
			printf("default gateway:%s\n",inet_ntoa(sin));
			break;
		}
	}
	
	fclose(fp);
	
	printf("default gateway:0x%08x\n",dwGateway);
	
	return dwGateway;
}

/*int SetDefaultGateway(u32 dwIp)
{
	int ret = 0;
	struct in_addr sin;
	char cmd[64];

	if(dwIp == 0) return -1;
	
	sprintf(cmd,"route del -net default");
	//system(cmd);
	printf("%s\n",cmd);

	sin.s_addr = dwIp;
	sprintf(cmd,"route add default gw %s",inet_ntoa(sin));
	//ret = system(cmd);
	printf("%s\n",cmd);
	
	return ret;
}*/

#if 1
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <net/if.h> 
#include <sys/ioctl.h> 
#include <errno.h> 
#include <string.h> 
#include <net/route.h>
#include <unistd.h>
#include <stdio.h> 
#endif

int SetDefaultGateway(unsigned int dwIp)
{
	int err = 0;
#if 1
	int skfd;
	struct rtentry rt;
	//int err;
	
	//csp modify
	u32 wlan0_ip = GetLocalIp2("wlan0");
	if(wlan0_ip)
	{
		printf("wifi is link,use wifi route\n");
		return -1;
	}
	
	//pw 2010/8/16 from cyl
	u32 pppoe_ip = GetLocalIp2("ppp0");
	if(pppoe_ip)
	{
		printf("pppoe up use ppp route\n");
		return -1;
	}
	
	skfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
		return -1;
	
	/* Delete existing defalt gateway */
	memset(&rt, 0, sizeof(rt));
	
	rt.rt_dst.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;
	
	rt.rt_genmask.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;
	
	rt.rt_flags = RTF_UP;
	
	err = ioctl(skfd, SIOCDELRT, &rt);
	if ((err == 0 || errno == ESRCH) && dwIp) {
		/* Set default gateway */
		memset(&rt, 0, sizeof(rt));
		
		rt.rt_dst.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;
		
		rt.rt_gateway.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = dwIp;
		
		rt.rt_genmask.sa_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;
		
		rt.rt_flags = RTF_UP | RTF_GATEWAY;
		
		err = ioctl(skfd, SIOCADDRT, &rt);
	}
	
	close(skfd);
#endif

	return err;
}

u32 GetDNSServer()
{
	FILE *fp;
	char name[16],dnsip[16];
	u32 dwDNSIp = 0;

	fp = fopen("/etc/resolv.conf","r");
	if(fp == NULL) 
	{
		return dwDNSIp;
		//printf("-----GetDNSServer--error--\n");
	}

	while(1)
	{
		int r;
		r = fscanf(fp,"%16s%16s\n",
			name, dnsip);
		if(r != 2)
		{
			/* EOF with no (nonspace) chars read. */
			if((r < 0) && feof(fp))
			{
				break;
			}
			else
			{
				continue;
			}
		}
		if(strcmp(name,"nameserver") == 0)
		{
			dwDNSIp = inet_addr(dnsip);
			printf("nameserver:%s\n",dnsip);
			break;
		}
	}

	fclose(fp);
	//printf("-----GetDNSServer--OK--\n");
	printf("nameserver:0x%08x\n",dwDNSIp);

	return dwDNSIp;
}

int GetDNSServer2(u32 *pDns, u32 *pDnsAlt )
{
	FILE *fp;
	char name[16],dnsip[16];
	u32 dwDNSIp = 0;
	u8	idx = 0;

	if(!pDns || !pDnsAlt)
	{
		return -1;
	}

	*pDns 		= 0;
	*pDnsAlt 	= 0;
	
	fp = fopen("/etc/resolv.conf","r");
	if(fp == NULL)
	{
		return -1; // no cfg file
	}

	while(1)
	{
		int r;
		r = fscanf(fp,"%16s%16s\n",
			name, dnsip);
		if(r != 2)
		{
			/* EOF with no (nonspace) chars read. */
			if((r < 0) && feof(fp))
			{
				break;
			}
			else
			{
				continue;
			}
		}
		
		if(strcmp(name,"nameserver") == 0)
		{
			if(idx == 0)
				*pDns = inet_addr(dnsip);
			else if(idx == 1)
				*pDnsAlt = inet_addr(dnsip);
			
			printf("nameserver%d:%s\n",idx, dnsip);
		
			if(idx++ >= 1)
				break;
		}
	}

	fclose(fp);

	return dwDNSIp;
}


int	SetDNSServer(u32 dwIp)
{
	FILE *fp = NULL;
	struct in_addr sin;
	char context[64];
	
	//if(dwIp == 0) return 0;
	
	//pw 2010/8/16 from cyl
	u32 pppoe_ip = GetLocalIp2("ppp0");
	if(pppoe_ip)
	{
		printf("pppoe up use ppp resolve.cnf\n");
		return -1;
	}
	
	fp = fopen("/etc/resolv.conf","w");
	if(fp == NULL)
	{
		return -1;
		//printf("-----SetDNSServer--error--\n");
	}
	sin.s_addr = dwIp;
	sprintf(context,"nameserver %s\n",inet_ntoa(sin));
	fwrite(context,strlen(context),1,fp);
	sprintf(context,"search localdomain");
	fwrite(context,strlen(context),1,fp);
	
	fclose(fp);
	//printf("-----SetDNSServer--OK--\n");
	return 0;
}

int AddDNSServer(u32 dwIp)
{
	FILE *fp = NULL;
	struct in_addr sin;
	char context[64];
	
	//if(dwIp == 0) return 0;
	
	//pw 2010/8/16 from cyl
	u32 pppoe_ip = GetLocalIp2("ppp0");
	if(pppoe_ip)
	{
		printf("pppoe up use ppp resolve.cnf\n");
		return -1;
	}
	
	fp = fopen("/etc/resolv.conf","a");
	if(fp == NULL)
	{
		return -1;
	}
	
	sin.s_addr = dwIp;
	sprintf(context,"\nnameserver %s",inet_ntoa(sin));
	fwrite(context,strlen(context),1,fp);
	
	fclose(fp);
	
	return 0;
}

BOOL CheckNetLink2(char* pIFName)
{
	BOOL rtn = FALSE;
	
	int s = -1;
	
	if(!pIFName)
	{
		goto END;
	}
	
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        perror("CheckNetLink2-socket");
		goto END;
    }
	
	struct ifreq ifr;
    strncpy(ifr.ifr_name, pIFName, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;
    
	if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
	{
		perror("CheckNetLink2-SIOCGIFFLAGS");
		goto END;
	}
	else
	{
		/*printf("flags=0x%x,IFF_UP=0x%x,IFF_RUNNING=0x%x,flags&IFF_UP=0x%x,flags&IFF_RUNNING=0x%x\n",
			ifr.ifr_flags,IFF_UP,IFF_RUNNING,ifr.ifr_flags&IFF_UP,ifr.ifr_flags&IFF_RUNNING);*/
		rtn = (ifr.ifr_flags&IFF_UP) && (ifr.ifr_flags&IFF_RUNNING);
	}
	
END:
	close(s);
	
	printf("check interface %s, connection status %d\n", pIFName, rtn);
	
	return rtn;	
}

BOOL CheckNetLink()
{
    int s = -1;
	
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        perror("CheckNetLink-socket");
        return FALSE;
    }
	
	struct ifreq ifr;
    strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;
	
	//printf("CheckNetLink interface:%s,IFNAMSIZ:%d\n",ifr.ifr_name,IFNAMSIZ);
	
	BOOL bRet = FALSE;
	
	if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
	{
		//system("ifconfig");
		perror("CheckNetLink-SIOCGIFFLAGS");
	}
	else
	{
		//system("ifconfig");
		//printf("flags=0x%x,IFF_UP=0x%x,IFF_RUNNING=0x%x,flags&IFF_UP=0x%x,flags&IFF_RUNNING=0x%x\n",
		//	ifr.ifr_flags,IFF_UP,IFF_RUNNING,ifr.ifr_flags&IFF_UP,ifr.ifr_flags&IFF_RUNNING);
		
		bRet = (ifr.ifr_flags&IFF_UP) && (ifr.ifr_flags&IFF_RUNNING);
	}
	
	close(s);
	
    return bRet;
}

#include <linux/if_packet.h>//struct sockaddr_ll
#include <linux/if_ether.h>//PF_PACKET ETH_P_ALL ETH_P_ARP

int iface_get_id(int fd, const char *device)
{
	struct ifreq ifr;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	
	if(ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
	{
		printf("ioctl SIOCGIFINDEX failed,errcode=%d,errstr=%s",errno,strerror(errno));
		return -1;
	}
	
	return ifr.ifr_ifindex;
}

int iface_get_arptype(int fd, const char *device)
{
	struct ifreq ifr;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	
	if(ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
	{
		printf("ioctl SIOCGIFHWADDR failed,errcode=%d,errstr=%s",errno,strerror(errno));
		return -1;
	}
	
	return ifr.ifr_hwaddr.sa_family;
}

int iface_bind(int fd, const char *device)
{
	struct sockaddr_ll	sll;
	int					err;
	socklen_t			errlen = sizeof(err);
	
	memset(&sll, 0, sizeof(sll));
	sll.sll_family		= AF_PACKET;
	sll.sll_ifindex		= iface_get_id(fd, device);
	sll.sll_protocol	= htons(ETH_P_ARP);
	//printf("index=%d\n",sll.sll_ifindex);
	
	if(bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1)
	{
		printf("iface_bind failed,errcode=%d,errstr=%s",errno,strerror(errno));
		return -1;
	}
	/*else
	{
		printf("bind success!!!\n");
	}*/
	
	/* Any pending errors, e.g., network is down? */
	
	if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1)
	{
		printf("getsockopt failed,code:%d,str:%s",errno,strerror(errno));
		return -2;
	}
	/*else
	{
		printf("getsockopt success,err=%d,errlen=%d\n",err,errlen);
	}*/
	
	if(err > 0)
	{
		printf("getsockopt error,code:%d,str:%s",err,strerror(err));
		return -2;
	}
	
	return 0;
}

int open_arp_socket()
{
	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if(fd<0)
	{
		printf("arp socket fd=%d,errcode=%d,errstr=%s\n" ,fd, errno, strerror(errno));
	}
	iface_bind(fd,ETH_NAME);
	return fd;
}

int sendarppacket(int s,unsigned int dstIp,unsigned int srcIp)
{
	struct ifreq ifr;
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	
	struct arp_struct myarp;
	struct in_addr host;
	
	memcpy(myarp.dst_mac, "\xff\xff\xff\xff\xff\xff", 6);
	/*printf("dst mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
		myarp.dst_mac[0],
		myarp.dst_mac[1],
		myarp.dst_mac[2],
		myarp.dst_mac[3],
		myarp.dst_mac[4],
		myarp.dst_mac[5]);*/
	
	if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
	{
		perror("SIOCGIFHWADDR");
		return -1;
	}
	memcpy(myarp.src_mac, ifr.ifr_hwaddr.sa_data, 6);
	/*printf("src mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
		myarp.src_mac[0],
		myarp.src_mac[1],
		myarp.src_mac[2],
		myarp.src_mac[3],
		myarp.src_mac[4],
		myarp.src_mac[5]);*/
	
	myarp.pkt_type = htons(ETH_P_ARP);
	
	//unsigned short hw_type = iface_get_arptype(s, ETH_NAME);
	//printf("hardware type=%d\n",hw_type);
	myarp.hw_type	= htons(0x0001);//ÒÔÌ«ÍøµØÖ·
	myarp.hw_len	= 6;
	
	myarp.pro_type	= htons(0x0800);//IPµØÖ·
	myarp.pro_len	= 4;
	
	myarp.arp_op	= htons(0x0001);//htons(ARPOP_REQUEST);
	
	memcpy(myarp.sender_eth,myarp.src_mac,6);
	
#if 0
	// get local ip address
	if(ioctl(s, SIOCGIFADDR, &ifr) < 0)
	{
		perror("SIOCGIFADDR");
		return -1;
	}
	unsigned int myip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
#else
	unsigned int myip = srcIp;
#endif
	memcpy(myarp.sender_ip, &myip, 4);
	host.s_addr = myip;
	//printf("my ip:%s\n",inet_ntoa(host));
	
	memcpy(myarp.target_eth, "\0\0\0\0\0\0", 6);
	
	memcpy(myarp.target_ip, &dstIp, 4);
	host.s_addr = dstIp;
	//printf("dst ip:%s\n",inet_ntoa(host));
	
	int index = iface_get_id(s,ETH_NAME);
	//printf("index=%d,AF_PACKET=%d,ETH_P_ALL=%d,ETH_P_ARP=%d\n",index,AF_PACKET,ETH_P_ALL,ETH_P_ARP);
	
	struct sockaddr_ll sa;
	memset(&sa, 0, sizeof (sa));
	sa.sll_family    = AF_PACKET;
    sa.sll_ifindex   = index;
    sa.sll_protocol  = htons(ETH_P_ALL);
	if(sendto(s, &myarp, sizeof(struct arp_struct), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		perror("sendto");
		return -1;
	}
	/*else
	{
		printf("sendto success\n");
	}*/
	
	return 0;
}
