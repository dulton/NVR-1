
// file description
#include "ddns_other.h"
#include "DDNS_9508.h"
#include "netcommon.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

//** macro

#define PRINT(x, y)	printf("FILE %s LINE %d check %s code %d\n", __FILE__, __LINE__, x, y)

const char SERIALNUMBER[8] 	= {0xCB, 0xE7, 0x1C, 0x39, 0x07, 0x94, 0x5F, 0x60};
const char KEY[8] 			= {0x95, 0xFF, 0x59, 0x83, 0xF6, 0x1E, 0x37, 0xA7};

//** typedef 

//**
static int g_nLastError = 0;

//** function
int base64_encode(const char *str, int length, char *b64store)
{
	//Conversion table
	static char tbl[64] = {
		'A','B','C','D','E','F','G','H',
			'I','J','K','L','M','N','O','P',
			'Q','R','S','T','U','V','W','X',
			'Y','Z','a','b','c','d','e','f',
			'g','h','i','j','k','l','m','n',
			'o','p','q','r','s','t','u','v',
			'w','x','y','z','0','1','2','3',
			'4','5','6','7','8','9','+','/'
	};
	
	int i;
	const unsigned char *s = (const unsigned char *)str;
	char *p = b64store;
	int pw = 0;//pw
	
	//Transform the 3x8 bits to 4x6 bits, as required by base64
	for(i = 0; i < length; i += 3)
    {
		*p++ = tbl[s[0] >> 2];
		*p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
		*p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
		*p++ = tbl[s[2] & 0x3f];

		pw+=4;
		if(0 == pw%76)
		{
			*p++ = '\r';
			*p++ = '\n';
		}
		
		s += 3;
    }
	
	//Pad the result if necessary...
	if(i == length + 1)
		*(p - 1) = '=';
	else if(i == length + 2)
		*(p - 1) = *(p - 2) = '=';
	
	//and zero-terminate it
	*p = '\0';
	
	return p - b64store;
}

#if 1//csp modify 20130523
#define TEXT_CHK3322LINE_NO		12
#define TEXT_CHKDYNLINE_NO		12
#define TEXT_LINE_LEN_MAX 		512
#else
#define TEXT_CHK3322LINE_NO		7
#define TEXT_CHKDYNLINE_NO		12
#define TEXT_LINE_LEN_MAX 		50
#endif

extern char* strcasestr(const char* str, const char* subStr);

int parse_result(int len, char* buffer, int chkline)
{
#if 1//csp modify 20130523
	char *linebuf = buffer;
	
	char *p1 = buffer;
	char *p2 = NULL;
	
	if(len >= 1)
	{
		if(buffer[len-1] == '\n')
		{
			buffer[len-1] = '\0';
		}
		
		if(len >= 2)
		{
			if(buffer[len-2] == '\r')
			{
				buffer[len-2] = '\0';
			}
		}
	}
	
	while((p2 = strstr(p1, "\r\n")) != NULL)
	{
		//p1 += 2;
		p1 = p2 + 2;
		
		if(p1 < buffer + len)
		{
			linebuf = p1;
		}
	}
	
	printf("parse_result:line=%s\n", linebuf);
#else
	int  i = 0, j = 0;
	char linebuf[TEXT_LINE_LEN_MAX] = {0};
	
	int linecnr = 0;
	
	if(!buffer) return -1;
	
	while(i < len-1)
	{
		if(buffer[i]==0xd && buffer[i+1]==0xa)
		{
			linebuf[j] = '\0';
			//printf("linebuf %s\n", linebuf);
			
			if(++linecnr>=chkline) break;
			
			i += 2;
			j = 0;
		}
		else
		{
			linebuf[j++] = buffer[i++];
		}
	}
	
	linebuf[j] = '\0';
	
	printf("line %s\n", linebuf);
#endif
	
	if(strcasestr(linebuf, "good") || strcasestr(linebuf, "nochg"))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

extern int check_domain(char* domain, char* keystr);

int rz_ddns_update(char *user_name,char *user_passwd,char *hostname,int typeflag)
{
	int err = 0;
	
	if(user_name == NULL || user_passwd == NULL || hostname == NULL)
	{
		printf("rz_ddns_update param error\n");
		return -1;
	}
	
	if(typeflag==EM_NET_DDNS_3322)
	{
		//csp modify 20130523
		//longsesdi.f3322.org
		//err = check_domain(hostname, ".3322.org");
		err = check_domain(hostname, "3322.org");
	}
	else if(typeflag==EM_NET_DDNS_DYDNS)
	{
		err = check_domain(hostname, ".dyndns.org");
	}
	else if(typeflag==EM_NET_DDNS_NOIP)//csp modify 20130523
	{
		err = 0;
	}
	else if(typeflag==EM_NET_DDNS_CHANGEIP)//csp modify 20130523
	{
		err = 0;
	}
	else
	{
		printf("rz_ddns_update ddns type flag:%d\n", typeflag);
		return -1;
	}
	
	if(err)
	{
		printf("rz_ddns_update:invalid domain name.\n");
		return -1;
	}
	
	struct hostent* pHost = NULL;
	
#if 0//
	return 0;
#else
	if(typeflag==EM_NET_DDNS_3322)
	{
		pHost = gethostbyname("members.3322.org");
	}
	else if(typeflag==EM_NET_DDNS_DYDNS)
	{
		pHost = gethostbyname("members.dyndns.org");
	}
	else if(typeflag==EM_NET_DDNS_NOIP)
	{
		pHost = gethostbyname("dynupdate.no-ip.com");
	}
	else if(typeflag==EM_NET_DDNS_CHANGEIP)
	{
		pHost = gethostbyname("nic.changeip.com");
	}
	
	if(pHost == NULL)
	{
		printf("rz_ddns_update gethostbyname failed\n");
		return -1;
	}
#endif
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		printf("rz_ddns_update socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	
	//addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]));
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0],sizeof(addr.sin_addr));
	
	//printf("rz_ddns_update---dns server addr:%s,count=%d\n",inet_ntoa(addr.sin_addr),count);
	
	addr.sin_port = htons(80);
	addr.sin_family = AF_INET;
	if(-1 == connect(sock,(struct sockaddr *)&addr,sizeof(addr)))
	{
		printf("rz_ddns_update connect failed\n");
		close(sock);
		return -1;
	}
	
	char auth[256];
	char key[512];
	int len1 = strlen(user_name) + 1 + strlen(user_passwd);
	//printf("rz_ddns_update:hostname %s,user_name %s,user_passwd:%s\n",hostname,user_name, user_passwd);
	sprintf(auth, "%s:%s", user_name, user_passwd);
	base64_encode(auth, len1, key);
	
	char buffer[1024] = {0};
	
	if(typeflag==EM_NET_DDNS_3322)
	{
		sprintf(buffer,"GET /dyndns/update?system=dyndns&hostname=%s HTTP/1.0\r\nHost: members.3322.org\r\nAccept: */*\r\nAuthorization: Basic %s\r\n\r\n",hostname,key);
 	}
	else if(typeflag==EM_NET_DDNS_DYDNS)
	{
 		sprintf(buffer,"\r\nGET /nic/update?hostname=%s HTTP/1.1\r\nHost: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: TongLi Inc. - Router 25001 - 1.1\r\n\r\n", hostname, key);
	}
	else if(typeflag==EM_NET_DDNS_NOIP)
	{
 		sprintf(buffer,"GET /nic/update?hostname=%s HTTP/1.0\r\nHost: dynupdate.no-ip.com\r\nAuthorization: Basic %s\r\nUser-Agent: TongLi Inc. - Router 25001 - 1.1\r\n\r\n", hostname,key);
	}
	else if(typeflag==EM_NET_DDNS_CHANGEIP)
	{
		sprintf(buffer,"GET /nic/update?hostname=%s HTTP/1.1\r\nHost: nic.changeip.com\r\nAuthorization: Basic %s\r\nUser-Agent: TongLi Inc. - Router 25001 - 1.1\r\n\r\n", hostname,key);
	}
	
	buffer[1023] = '\0';
	
	if(send(sock,buffer,strlen(buffer),0) == -1)
	{
		printf("rz_ddns_update send failed\n");
		close(sock);
		return -1;
	}
	
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock,&set);
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	int ret = select(sock+1,&set,NULL,NULL,&timeout);
	if(ret < 0)
	{
		printf("rz_ddns_update select failed\n");
		close(sock);
		return -1;
	}
	if(ret == 0)
	{
		printf("rz_ddns_update select timeout\n");
		close(sock);
		return -1;
	}
	
	memset(buffer,0,sizeof(buffer));//csp modify 20130523
	
	len1 = recv(sock,buffer,sizeof(buffer),0);
	if(len1 <= 0)
	{
		//csp modify 20130523
		printf("rz_ddns_update recv error\n");
		close(sock);
		return -1;
	}
	
	#if 0//csp modify 20130523
	FILE *fp = fopen("test.htm","w");
	if(fp != NULL)
	{
		//printf("%d\n", len1);
		if(len1 >= 0)
		{
			fwrite(buffer,1,len1,fp);
		}
		fclose(fp);
	}
	#endif
	
	//printf("rz_ddns_update result:%s\n",buffer);
	//fflush(stdout);
	
	//csp modify 20130523
	//int chkline = (typeflag==1?TEXT_CHK3322LINE_NO:TEXT_CHKDYNLINE_NO);
	int chkline = ((typeflag == EM_NET_DDNS_3322) ? TEXT_CHK3322LINE_NO : TEXT_CHKDYNLINE_NO);
	err = parse_result(len1, buffer, chkline);
	
	close(sock);
	
	printf("rz_ddns_update result:%d\n",err);
	
	return err;
}

// init
void ddnsother_Init( void* pInitPara )
{
	g_nLastError = 0;//SHX_Init();
}

void dvrnet_Init( void* pInitPara )//cw_ddns
{
	g_nLastError = 0;
	SHX_Init();
}

// regist
void ddnsother_Regist( void* pRegPara )
{
	return;
	
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;

	SHX_ReqDomain(
		pPara->domain_name, 
		strlen(pPara->domain_name), 
		pPara->passwd, 
		strlen(pPara->passwd),
		(char*)SERIALNUMBER,
		sizeof(SERIALNUMBER),
		(char*)KEY,
		sizeof(KEY)
	);
}

void dvrnet_Regist( void* pRegPara )  //cw_ddns
{	
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;
	int ret;
	
	ret=SHX_ReqDomain(
		pPara->usrname, 
		strlen(pPara->usrname), 
		pPara->passwd, 
		strlen(pPara->passwd),
		(char*)SERIALNUMBER,
		sizeof(SERIALNUMBER),
		(char*)KEY,
		sizeof(KEY)
	);
	
	SHX_UnInit();
	
	g_nLastError=!ret;
}

// unreg
void ddnsother_Unreg( void* pUnRegPara ) 
{
	
}

// refresh
void dyndns_Refresh( void* pRefreshPara )
{
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRefreshPara;

	printf("ddnsother_Refresh dyndns user %s passwd %s \n", pPara->domain_name, pPara->passwd);

	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_DYDNS);
}

void ddns3322_Refresh( void* pRefreshPara )
{
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRefreshPara;
	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_3322);//csp modify 20130523
}

// start 
void dyndns_Start( void* pRegPara )
{
	printf("dyndns update...\n");
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;
	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_DYDNS);
	if(g_nLastError!=0)
	{
		g_nLastError = 0xFE;
	}
}

void ddns3322_Start( void* pRegPara )
{
	printf("3322 update...\n");
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;
	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_3322);
	//printf("3322 update end.\n");
	if(g_nLastError!=0)
	{
		g_nLastError = 0xFF;
	}
}

void dvrnet_Start(void * pStartPara)//cw_ddns
{
	//printf("dvrnet update...\n");
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pStartPara;
	printf("dvrnet_Start domain %s passwd %s \n", pPara->usrname, pPara->passwd);
		SHX_ReqStartClient(
		pPara->usrname, 
		strlen(pPara->usrname), 
		pPara->passwd, 
		strlen(pPara->passwd)
	);
}

void ddnsnoip_Start( void* pStartPara )
{
	//printf("noip update...\n");
	
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pStartPara;
	
	printf("ddnsnoip_Start domain %s passwd %s\n", pPara->domain_name, pPara->passwd);
	
	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_NOIP);
}

void changeip_Start( void* pStartPara )
{
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pStartPara;
	
	printf("changeip_Start  domain %s passwd %s\n", pPara->domain_name, pPara->passwd);
	
	g_nLastError = rz_ddns_update(pPara->usrname, pPara->passwd, pPara->domain_name, EM_NET_DDNS_CHANGEIP);
}

// stop
void ddnsother_Stop( void* pRegPara )
{
	return;
	
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;
	
	SHX_ReqStopClient(
		pPara->domain_name, 
		strlen(pPara->domain_name), 
		pPara->passwd, 
		strlen(pPara->passwd)
	);
}

void dvrnet_Stop( void* pRegPara )
{	
	SOtherDdnsReg* pPara = (SOtherDdnsReg*)pRegPara;
	printf("dvrnet_Stop  domain %s passwd %s\n", pPara->usrname, pPara->passwd);
	SHX_ReqStopClient(
		pPara->usrname, 
		strlen(pPara->usrname), 
		pPara->passwd, 
		strlen(pPara->passwd)
	);
}

// deinit
void ddnsother_DeInit() 
{
	return;
	
	SHX_UnInit();
}

void dvrnet_DeInit() 
{	
	SHX_UnInit();
}

void ddnsother_GetErr(s32* pErr) 
{
	*pErr = (s32)g_nLastError;
}

