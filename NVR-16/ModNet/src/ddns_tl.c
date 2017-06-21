// file description
#include "ddns_tl.h"

#include <unistd.h>

#include <netinet/in.h> 
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>


//** macro

#define PRINT(x, y)	printf("FILE %s LINE %d check %s code %d\n", __FILE__, __LINE__, x, y)

//** local variable
static s32 eDdnsErr = 0;

//** local functions

static int contact_with_ddnsserver(char *domain_name,char *passwd, int cmd_no);
static int contact_with_ddnsserver_konlan(char *domain_name, char *passwd, int cmd_no);
static int contact_with_jmdvrserver(char *domain_name,char *passwd, int cmd_no);
static int contact_with_jsjdvrserver(char *domain_name,char *passwd, int cmd_no);

int check_ip_tl(int* Time_check);

// init
void ddnstl_Init( void* pInitPara )
{
	eDdnsErr = 0;
}

// regist
void ddnstl_Regist( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;

	//printf("ddnstl_Regist\n");
	
	eDdnsErr = contact_with_ddnsserver(pPara->domain_name, pPara->passwd, DDNS_REGIST);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFB;
	}
	usleep(50*1000);
}

void konlan_Regist( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	//printf("konlan_Regist\n");
	
	eDdnsErr = contact_with_ddnsserver_konlan(pPara->usrname, pPara->passwd, DDNS_REGIST);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFB;
	}
	usleep(50*1000);
}

void jmdvr_Regist( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	//printf("jmdvr_Regist\n");
	
	eDdnsErr = contact_with_jmdvrserver(pPara->usrname, pPara->passwd, DDNS_REGIST);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFB;
	}
	usleep(50*1000);
}

void jsjdvr_Regist( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	printf("jsjdvr_Regist\n");
	
	eDdnsErr = contact_with_jsjdvrserver(pPara->usrname, pPara->passwd, DDNS_REGIST);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFB;
	}
	usleep(50*1000);
}

// unreg
void ddnstl_Unreg( void* pUnRegPara ) {}

// refresh
void ddnstl_Refresh( void* pRefreshPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRefreshPara;
	
	eDdnsErr = contact_with_ddnsserver(pPara->domain_name, pPara->passwd, DDNS_LOGIN);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFC;
	}
	usleep(50*1000);
}

// start 
void ddnstl_Start( void* pRegPara )
{
	printf("popdvr update...\n");
	
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	eDdnsErr = contact_with_ddnsserver(pPara->domain_name, pPara->passwd, DDNS_LOGIN);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFD;
	}
	usleep(50*1000);
}

void konlan_Start( void* pStartPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pStartPara;
	
	printf("konlan_Start domain %s passwd %s\n", pPara->domain_name, pPara->passwd);
	
	eDdnsErr = contact_with_ddnsserver_konlan(pPara->usrname, pPara->passwd, DDNS_LOGIN);
}

void jmdvr_Start( void* pStartPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pStartPara;
	
	printf("jmdvr_Start domain %s passwd %s\n", pPara->domain_name, pPara->passwd);
	
	eDdnsErr = contact_with_jmdvrserver(pPara->usrname, pPara->passwd, DDNS_LOGIN);
}

void jsjdvr_Start( void* pStartPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pStartPara;
	
	printf("jsjdvr_Start domain %s passwd %s\n", pPara->domain_name, pPara->passwd);
	
	eDdnsErr = contact_with_jsjdvrserver(pPara->usrname, pPara->passwd, DDNS_LOGIN);
}

// start try
void ddnstl_StartTry( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	eDdnsErr = contact_with_ddnsserver(pPara->domain_name, pPara->passwd, DDNS_LOGIN_TRY);
	
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFF;
	}
	
	usleep(50*1000);
}

// stop
void ddnstl_Stop( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	eDdnsErr = contact_with_ddnsserver(pPara->domain_name, pPara->passwd, DDNS_CANCEL);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFE;
	}
	
	usleep(50*1000);
}

void konlan_Stop( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	eDdnsErr = contact_with_ddnsserver_konlan(pPara->usrname, pPara->passwd, DDNS_CANCEL);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFE;
	}
	
	usleep(50*1000);
}

void jmdvr_Stop( void* pRegPara )
{
	STlDdnsReg* pPara = (STlDdnsReg*)pRegPara;
	
	//csp modify 20130322
	//eDdnsErr = contact_with_jmdvrserver(pPara->domain_name, pPara->passwd, DDNS_CANCEL);
	eDdnsErr = contact_with_jmdvrserver(pPara->usrname, pPara->passwd, DDNS_CANCEL);
	if(eDdnsErr != 0)
	{
		eDdnsErr = 0xFE;
	}
	
	usleep(50*1000);
}

// deinit
void ddnstl_DeInit() {}
void ddnstl_GetErr(s32* pErr) 
{
	if(pErr) *pErr = eDdnsErr;
}

char ip_check[32] = {0};//tongli DDNS

int check_ip_tl(int* Time_check)
{
	char *ip_check_temp;
	struct hostent* pHost = NULL;
	int ret;
	
	#if 0//
	return 0;
	#else
	pHost = gethostbyname("data002.cngame.org");
	if(pHost == NULL)
	{
		printf("check_ip_rz gethostbyname failed\n");
		return -1;
	}
	#endif
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("check_ip_rz socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0], sizeof(addr.sin_addr));
	//printf("check_ip_rz---dns server addr:%s\n",inet_ntoa(addr.sin_addr));
	
	#if 1//csp modify 20130322
	addr.sin_port = htons(28589);
	#else
	addr.sin_port = htons(28584);
	#endif
	addr.sin_family = AF_INET;
	
	char send_buf[50] = {0};
	char recv_buf[50] = {0};
	
	strcpy(send_buf, "test&&&&&");
	
	ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		//printf("sendto %d bytes\n", ret);
		perror("sendto error");
		close(sock);
		return -1;
	}
	
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	
	ret = select(sock+1, &set, NULL, NULL, &timeout);
	if(ret < 0)
	{
		printf("__func__:%s, select failed\n", __FUNCTION__);
		close(sock);
		*Time_check = 0;
		return -1;
	}
	if(ret == 0)
	{
		printf("__func__:%s, select timeout\n", __FUNCTION__);
		close(sock);
		*Time_check = 0;
		return -1;
	}
	
	socklen_t addr_len = 0;
	ret = recvfrom(sock, recv_buf, sizeof(recv_buf) -1, 0, (struct sockaddr *)&addr, &addr_len);
	if(ret < 0)
	{
		printf("__func__:%s, recv error\n", __FUNCTION__);
		perror("recvfrom error");
		close(sock);
		return -1;
	}
	
	//printf("line:%d, recv len:%d, buf:%s\n", __LINE__, ret, recv_buf);
	
	char *end_s = strstr(recv_buf, "&&&&&");
	if(NULL == end_s)
	{
		printf("the result is not rigth!\n");
		close(sock);
		return -1;
	}
	
	char T_p[3] = {0, 0, 0};
	strncpy(T_p,recv_buf,2);
	T_p[2]='\0';
	*Time_check = atoi(T_p);
	//printf("check ip time:%d\n",*Time_check);
	
	char *c_p = recv_buf+2;
	ip_check_temp = strtok(c_p,"&&&&");
	if(ip_check_temp == NULL)
	{
		printf("the result is not rigth!\n");
		close(sock);
		return -1;
	}
	
	//printf("rcv IP_temp=%s\n",ip_check_temp);
	//printf("rcv IP_static=%s\n",ip_check);
	
	if(0 == strncmp(ip_check, ip_check_temp, strlen(ip_check_temp)+1))//-2?
	{
		//printf("the result is the same with current ip:%s, new ip:%s\n",ip_check,ip_check_temp);
		ret = 0;
	}
	else
	{
		printf("the result is different with current ip:%s, new ip:%s\n",ip_check, ip_check_temp);
		strcpy(ip_check,ip_check_temp);
		ret = 1;
	}
	//printf("%s,%d,check_ip_tl ret=%d\n",__func__,__LINE__,ret);
	close(sock);
	return ret;
}

char* strcasestr(const char* str, const char* subStr)
{
	int len = strlen(subStr);
	if(len == 0)
	{
		return NULL;
	}
	
	//csp modify 20130523
	//while(*str)
	while(*str && strlen(str) >= len)
	{
		if(strncasecmp(str, subStr, len) == 0)
		{
			return (char *)str;
		}
		str++;
	}
	
	return NULL;
}

int check_domain(char* domain, char* keystr)
{
	if(domain==NULL || keystr==NULL)
	{
		return -1;
	}
	
	char* pDomain = strcasestr(domain, keystr);
	if(pDomain == NULL)
	{
		printf("check_domain:invalid domain name.\n");
		return -1;
	}
	else
	{
		if(strlen(keystr) != strlen(pDomain))
		{
			return -1;
		}
	}
	
	return 0;
}

int contact_with_jsjdvrserver(char *domain_name,char *passwd, int cmd_no)
{
	if(domain_name == NULL || passwd == NULL || cmd_no < 0)
	{
		printf("contact_with_jsjdvrserver param error\n");
		return -1;
	}
	
	char send_buf[50] = {0};
	char recv_buf[50] = {0};
	
	printf("~~~~~~~~~~name:%s,passwd:%s,cmd_no:%d\n", domain_name,passwd,cmd_no);
	switch(cmd_no)
	{
		case DDNS_LOGIN:
			printf("%s now is login\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ddnsjsjdv%s&&&%s&&&80008&&&", domain_name, passwd);
			break;
		case DDNS_REGIST:
			printf("%s now is regist\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "jsjdvzhuce%s&&&%s&&&80008&&&", domain_name, passwd);	
			break;
		default:
			printf("The cmd_no is error");
			return -1;
	}
	
	struct hostent* pHost = NULL;
	
	char dns_server[50] = {0};
	snprintf(dns_server, sizeof(dns_server) -1, "%cmain.jsjdvr.com", domain_name[0]);
	printf("~~~~~~~~~~~~~~~~~dns_server:%s\n", dns_server);
	
	#if 0//
	return 0;
	#else
	pHost = gethostbyname(dns_server);
	if(pHost == NULL)
	{
		printf("contact_with_jsjdvrserver gethostbyname failed\n");
		return -1;
	}
	#endif
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("contact_with_jsjdvrserver socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0], sizeof(addr.sin_addr));
	
	printf("contact_with_jsjdvrserver---dns server addr:%s\n",inet_ntoa(addr.sin_addr));
	
	addr.sin_port = htons(21001);
	addr.sin_family = AF_INET;

	printf("~~~~~~~~~~~~send buf:%s\n", send_buf);
	int ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("sendto %d bytes\n", ret);
		perror("sendto error");
		close(sock);
		return -1;
	}	
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	ret = select(sock+1,&set,NULL,NULL,&timeout);
	if(ret < 0)
	{
		printf("__func__:%s, select failed\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	if(ret == 0)
	{
		printf("__func__:%s, select timeout\n", __FUNCTION__);
		close(sock);
		return -2;
	}

	socklen_t addr_len = 0;
	ret = recvfrom(sock, recv_buf, sizeof(recv_buf) -1, 0, (struct sockaddr *)&addr, &addr_len);
	if(ret < 0)
	{
		printf("__func__:%s, recv error\n", __FUNCTION__);
		perror("recvfrom error");
		close(sock);
		return -1;
	}

	printf("line:%d, recv len:%d, buf:%s\n", __LINE__, ret, recv_buf);

	if(0 == strncasecmp(recv_buf, "Successful", strlen("Successful")))
	{
		printf("%s,return = 0\n",__func__);
		ret = 0;
	}
	else
	{
		printf("%s,return = -1\n",__func__);
		ret = -1;
	}
	
	close(sock);
	return ret;
}

int contact_with_ddnsserver_konlan(char *domain_name, char *passwd, int cmd_no)
{
	if(domain_name == NULL || passwd == NULL || cmd_no < 0)
	{
		printf("wget_user param error\n");
		return -1;
	}
	
	char send_buf[256];
	char recv_buf[256];
	char dns_server[128];
	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	memset(dns_server, 0, sizeof(dns_server));
	
	printf("~~~~~~~~~~~~~~~~~cmd_no:%d\n", cmd_no);
	
	switch(cmd_no)
	{
		case DDNS_LOGIN:
			printf("%s now is login\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ddnkonlan%s&&&%s&&&39001&&&", domain_name, passwd);
			break;
		case DDNS_REGIST:
			printf("%s now is regist\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "konlazhuce%s&&&%s&&&39012&&&", domain_name, passwd);	
			break;
		default:
			printf("The cmd_no is error\n");
			return -1;
	}
	
	struct hostent* pHost = NULL;
	
	snprintf(dns_server, sizeof(dns_server) - 1, "%cmain.konlan.net", domain_name[0]);
	printf("~~~~~~~~~~~~~~~~~dns_server:%s,send_buf:%s\n", dns_server,send_buf);
	pHost = gethostbyname(dns_server);
	if(pHost == NULL)
	{
		printf("wget_user gethostbyname failed\n");
		return -1;
	}
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("wget_user socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0], sizeof(addr.sin_addr));
	
	printf("wget_user---dns server addr:%s\n",inet_ntoa(addr.sin_addr));
	
	addr.sin_port = htons(39015);
	addr.sin_family = AF_INET;
	
	printf("~~~~~~~~~~~~send buf:%s\n", send_buf);
	int ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("sendto %d bytes\n", ret);
		perror("sendto error");
		close(sock);
		return -1;
	}
	
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	ret = select(sock+1,&set,NULL,NULL,&timeout);
	if(ret < 0)
	{
		printf("__func__:%s, select failed\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	if(ret == 0)
	{
		printf("__func__:%s, select timeout\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	
	int addr_len = 0;
	ret = recvfrom(sock, recv_buf, sizeof(recv_buf) - 1, 0, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
	if(ret < 0)
	{
		printf("__func__:%s, recv error\n", __FUNCTION__);
		perror("recvfrom error");
		close(sock);
		return -1;
	}
	
	printf("line:%d, recv len:%d, buf:%s\n", __LINE__, ret, recv_buf);
	
	if(0 == strncasecmp(recv_buf, "Successful", strlen("Successful")))
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	
	close(sock);
	return ret;
}

int contact_with_jmdvrserver(char *domain_name, char *passwd, int cmd_no)
{
	if(domain_name == NULL || passwd == NULL || cmd_no < 0)
	{
		printf("contact_with_jmdvrserver param error\n");
		return -1;
	}
	
	char send_buf[256];
	char recv_buf[256];
	char dns_server[128];
	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	memset(dns_server, 0, sizeof(dns_server));
	
	//printf("~~~~~~~~~~~~~~~~~cmd_no:%d\n", cmd_no);
	switch(cmd_no)
	{
		case DDNS_LOGIN:
			//printf("%s now is login\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ddnsipcam%s&&&%s&&&80008&&&", domain_name, passwd);
			break;
		case DDNS_REGIST:
			//printf("%s now is regist\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ipcamzhuce%s&&&%s&&&80008&&&", domain_name, passwd);	
			break;
		case DDNS_CANCEL:
			//printf("%s now is cancel\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ipcamzhuxiao%s&&&%s&&&80008&&&", domain_name, passwd);	
			break;
		default:
			printf("The cmd_no is error\n");
			return -1;
	}
	
	snprintf(dns_server, sizeof(dns_server) - 1, "%cmain.jmdvr.net", domain_name[0]);
	printf("~~~~~~~~~~~~~~~~~dns_server:%s,domain_name:%s\n", dns_server, domain_name);
	
	struct hostent* pHost = NULL;
	
	#if 0//
	return 0;
	#else
	pHost = gethostbyname(dns_server);
	if(pHost == NULL)
	{
		printf("contact_with_jmdvrserver gethostbyname failed\n");
		return -1;
	}
	#endif
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("contact_with_jmdvrserver socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0], sizeof(addr.sin_addr));
	printf("contact_with_jmdvrserver---dns server addr:%s\n",inet_ntoa(addr.sin_addr));
	
	//csp modify 20130322
	//addr.sin_port = htons(8586);
	addr.sin_port = htons(8588);
	addr.sin_family = AF_INET;
	
	printf("~~~~~~~~~~~~send buf:%s\n", send_buf);
	int ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("sendto %d bytes\n", ret);
		perror("sendto error");
		close(sock);
		return -1;
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	ret = select(sock+1,&set,NULL,NULL,&timeout);
	if(ret < 0)
	{
		printf("__func__:%s, select failed\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	if(ret == 0)
	{
		printf("__func__:%s, select timeout\n", __FUNCTION__);
		close(sock);
		return -1;
	}

	socklen_t addr_len = 0;
	ret = recvfrom(sock, recv_buf, sizeof(recv_buf) - 1, 0, (struct sockaddr *)&addr, &addr_len);
	if(ret < 0)
	{
		printf("__func__:%s, recv error\n", __FUNCTION__);
		perror("recvfrom error");
		close(sock);
		return -1;
	}
	
	printf("line:%d, recv len:%d, buf:%s\n", __LINE__, ret, recv_buf);
	
	if(0 == strncasecmp(recv_buf, "Successful", strlen("Successful")))
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	
	close(sock);
	return ret;
}

int contact_with_ddnsserver(char *domain,char *passwd, int cmd_no)
{
	char domain_name[128] = {0};
	
	if(domain_name == NULL || passwd == NULL || cmd_no < 0)
	{
		printf("contact_with_ddnsserver param error\n");
		return -1;
	}
	
	strcpy(domain_name, domain);
	
	if(check_domain(domain_name, ".popdvr.com"))
	{
		return -1;
	}
	
	char* pDomain = (char*)strcasestr(domain_name, ".popdvr.com");
	if( pDomain == NULL )
	{
		printf("contact_with_ddnsserver:invalid domain name.\n");
		return -1;
	}
	else
	{
		for(; *pDomain!='\0'; pDomain++)
		{
			*pDomain='\0';
		}
	}
	
	char send_buf[256];
	char recv_buf[256];
	char dns_server[128];
	memset(send_buf, 0, sizeof(send_buf));
	memset(recv_buf, 0, sizeof(recv_buf));
	memset(dns_server, 0, sizeof(dns_server));
	
	//printf("~~~~~~~~~~~~~~~~~cmd_no:%d\n", cmd_no);
	switch(cmd_no)
	{
		case DDNS_LOGIN_TRY:	
		case DDNS_LOGIN:
			printf("%s now is login\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "ddnsopdvr%s&&&%s&&&iap192.168.1.1&&&83639&&&", domain_name, passwd);
			break;
		case DDNS_REGIST:
			printf("%s now is regist\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "opdvrzhuce%s&&&%s&&&iap192.168.1.1&&&83639&&&", domain_name, passwd);	
			break;
		case DDNS_CANCEL:
			printf("%s now is cancel\n",__FUNCTION__);
			snprintf(send_buf, sizeof(send_buf) - 1, "opdvrzhuxiao%s&&&%s&&&iap192.168.1.1&&&83639&&&", domain_name, passwd);	
			break;
		default:
			printf("The cmd_no is error\n");
			
			return -1;
	}
	
	struct hostent* pHost = NULL;
	
	snprintf(dns_server, sizeof(dns_server) - 1, "%cmain.popdvr.com", domain_name[0]);
	//printf("~~~~~~~~~~~~~~~~~dns_server:%s\n", dns_server);
	
	#if 0//
	return 0;
	#else
	pHost = gethostbyname(dns_server);
	if(pHost == NULL)
	{
		printf("contact_with_ddnsserver gethostbyname failed\n");
		return -1;
	}
	#endif
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("contact_with_ddnsserver socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0], sizeof(addr.sin_addr));
	
	//printf("contact_with_ddnsserver---dns server addr:%s\n",inet_ntoa(addr.sin_addr));
	
	addr.sin_port = htons(8572);
	addr.sin_family = AF_INET;
	
	//printf("~~~~~~~~~~~~send buf:%s\n", send_buf);
	
	int ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("sendto %d bytes\n", ret);
		perror("sendto error");
		close(sock);
		return -1;
	}
	
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	
	struct timeval timeout;
	
	int wait_sec = 0;
	int try_cnr = 0;
	if(DDNS_REGIST == cmd_no)
	{
		wait_sec = 60;
	}
	else
	{
		wait_sec = (cmd_no==DDNS_LOGIN_TRY?5:20);
	}
	
TRY:
	timeout.tv_sec = wait_sec/(1+try_cnr);
	timeout.tv_usec = 0;
	ret = select(sock+1,&set,NULL,NULL,&timeout);
	if(ret < 0)
	{
		printf("__func__:%s, select failed\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	
	if(ret == 0)
	{
		if(try_cnr++<2)
		{
			goto TRY;
		}
		
		printf("__func__:%s, select timeout\n", __FUNCTION__);
		close(sock);
		return -1;
	}
	
	socklen_t addr_len = 0;
	ret = recvfrom(sock, recv_buf, sizeof(recv_buf) - 1, 0, (struct sockaddr *)&addr, &addr_len);
	if(ret < 0)
	{
		printf("__func__:%s, recv error\n", __FUNCTION__);
		perror("recvfrom error");
		close(sock);
		return -1;
	}
	
	printf("line:%d, recv len:%d, buf:%s\n", __LINE__, ret, recv_buf);
	
	if(0 == strncasecmp(recv_buf, "Successful", strlen("Successful")))
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	
	close(sock);
	return ret;
}

