#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <alloca.h>
#include <strings.h>
#include <common.h>
/*void *debugging_malloc(size_t size, const char *source_file, int source_line)
{
	void *ptr = malloc(size);
	if(ptr == NULL)
	{
		printf("malloc failed at:(%s,%d)\n",source_file,source_line);
	}
	return ptr;
}

void debugging_free(void *ptr, const char *source_file, int source_line)
{
	if(ptr == NULL)
    {
		fprintf(stderr, "xfree(NULL) at %s:%d\n", source_file, source_line);
		return;
    }
	free(ptr);
}

void *debugging_realloc(void *ptr, size_t newsize, const char *source_file, int source_line)
{
	void *newptr;
	if(ptr)
	{
		newptr = realloc(ptr, newsize);
	}
	else
	{
		newptr = malloc(newsize);
	}
	if(newptr == NULL)
	{
		printf("realloc failed at:(%s,%d)\n",source_file,source_line);
	}
	return newptr;
}

#define VA_START(args, arg1) va_start(args, arg1)

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define xmalloc(s) debugging_malloc(s, __FILE__, __LINE__)
#define xfree(p) debugging_free(p, __FILE__, __LINE__)
#define xrealloc(p, s) debugging_realloc(p, s, __FILE__, __LINE__)

#define BASE64_LENGTH(len) (4 * (((len) + 2) / 3))*/

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

/*char *concat_strings(const char *str0, ...)
{
	va_list args;
	int saved_lengths[5];
	char *ret, *p;
	
	const char *next_str;
	int total_length = 0;
	int argcount;
	
	argcount = 0;
	VA_START(args, str0);
	for (next_str = str0; next_str != NULL; next_str = va_arg(args, char *))
    {
		int len = strlen(next_str);
		if(argcount < countof(saved_lengths))
			saved_lengths[argcount++] = len;
		total_length += len;
    }
	va_end(args);
	p = ret = xmalloc(total_length + 1);
	
	argcount = 0;
	VA_START(args, str0);
	for(next_str = str0; next_str != NULL; next_str = va_arg(args, char *))
    {
		int len;
		if(argcount < countof(saved_lengths))
			len = saved_lengths[argcount++];
		else
			len = strlen(next_str);
		memcpy(p, next_str, len);
		p += len;
    }
	va_end(args);
	*p = '\0';
	
	return ret;
}

static char *basic_authentication_encode(const char *user, const char *passwd)
{
	char *t1, *t2;
	int len1 = strlen(user) + 1 + strlen(passwd);
	
	t1 = (char *)alloca(len1 + 1);
	sprintf (t1, "%s:%s", user, passwd);
	
	t2 = (char *)alloca(BASE64_LENGTH(len1) + 1);
	base64_encode(t1, len1, t2);
	
	return concat_strings("Basic ", t2, (char *)0);
}

enum rp
{
	rel_none,
	rel_name,
	rel_value,
	rel_both
};

struct request_header
{
	char *name;
	char *value;
	enum rp release_policy;
};

struct request
{
	const char *method;
	char *arg;
	
	struct request_header *headers;
	
	int hcount;
	int hcapacity;
};

static void release_header(struct request_header *hdr)
{
	switch (hdr->release_policy)
    {
    case rel_none:
		break;
    case rel_name:
		xfree(hdr->name);
		break;
    case rel_value:
		xfree(hdr->value);
		break;
    case rel_both:
		xfree(hdr->name);
		xfree(hdr->value);
		break;
    }
}

static void request_set_header(struct request *req, char *name, char *value, enum rp release_policy)
{
	struct request_header *hdr;
	int i;
	
	if(!value)
    {
		if(release_policy == rel_name || release_policy == rel_both)
		{
			xfree(name);
		}
		return;
    }
	
	for(i = 0; i < req->hcount; i++)
    {
		hdr = &req->headers[i];
		if(0 == strcasecmp(name, hdr->name))
		{
			release_header(hdr);
			hdr->name = name;
			hdr->value = value;
			hdr->release_policy = release_policy;
			return;
		}
    }
	
	if (req->hcount >= req->hcapacity)
    {
		req->hcapacity <<= 1;
		req->headers = xrealloc(req->headers, req->hcapacity * sizeof (*hdr));
    }
	hdr = &req->headers[req->hcount++];
	hdr->name = name;
	hdr->value = value;
	hdr->release_policy = release_policy;
}*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

int wget_user(char *user_name,char *user_passwd,char *hostname,int typeflag)
{
	if(user_name == NULL || user_passwd == NULL || hostname == NULL)
	{
		printf("wget_user param error\n");
		return -1;
	}
	
	struct hostent* pHost = NULL;
	
	//printf("type type=%d\n",typeflag);
	if(typeflag==1)
	{
		//printf("type 3322\n");
		pHost = gethostbyname("members.3322.org");
	}
	else if(typeflag==2)
	{
		//printf("type dyndns\n");
		pHost = gethostbyname("members.dyndns.org");
	}
	else//csp modify 20130523
	{
		return -1;
	}
	
	if(pHost == NULL)
	{
		printf("wget_user gethostbyname failed\n");
		return -1;
	}
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		printf("wget_user socket failed\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	
	/*
	int i = 0;
	int count = 0;
	for(i=0;;i++)
	{
		count++;
		if(pHost->h_addr_list[i] + pHost->h_length >= pHost->h_name)
		{
			break;
		}	
	}
	*/
	
	//addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]));
	memcpy(&addr.sin_addr,(struct in_addr *)pHost->h_addr_list[0],sizeof(addr.sin_addr));
	
	//printf("wget_user---dns server addr:%s,count=%d\n",inet_ntoa(addr.sin_addr),count);
	
	addr.sin_port = htons(80);
	addr.sin_family = AF_INET;
	if(-1 == connect(sock,(struct sockaddr *)&addr,sizeof(addr)))
	{
		printf("wget_user connect failed\n");
		close(sock);
		return -1;
	}
	
	char auth[256];
	char key[512];
	int len1 = strlen(user_name) + 1 + strlen(user_passwd);
	//printf("hostname %s,user_name %s,user_passwd:%s\n",hostname,user_name, user_passwd);
	sprintf(auth, "%s:%s", user_name, user_passwd);
	base64_encode(auth, len1, key);
	
	char buffer[1024] = {0};
	
	if(typeflag==1)
	{
		sprintf(buffer,"GET /dyndns/update?system=dyndns&hostname=%s HTTP/1.0\r\nHost: members.3322.org\r\nAccept: */*\r\nAuthorization: Basic %s\r\n\r\n",hostname,key);
 	}
	else if(typeflag==2)
	{
 		sprintf(buffer,"\r\nGET /nic/update?hostname=%s HTTP/1.1\r\nHost: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: TongLi Inc. - Router 25001 - 1.1\r\n\r\n", hostname, key);
	}
	
	buffer[1023] = '\0';
	
	if(send(sock,buffer,strlen(buffer),0) == -1)
	{
		printf("wget_user send failed\n");
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
		printf("wget_user select failed\n");
		close(sock);
		return -1;
	}
	if(ret == 0)
	{
		printf("wget_user select timeout\n");
		close(sock);
		return -1;
	}
	
	memset(buffer,0,sizeof(buffer));//csp modify 20130523
	
	len1 = recv(sock,buffer,sizeof(buffer),0);
	
	if(len1 <= 0)
	{
		//csp modify 20130523
		printf("wget_user recv error\n");
		close(sock);
		return -1;
	}
	
	#if 0//csp modify 20130523
	FILE *fp = fopen("test.htm","w");
	if(fp != NULL)
	{
		if(len1 >= 0)
		{
			fwrite(buffer,1,len1,fp);
		}
		fclose(fp);
	}
	#endif
	
	close(sock);
	
	return 0;
}

#ifdef TONGLI_DDNS
int contact_with_ddnsserver(char *domain_name,char *passwd, int cmd_no)
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
	printf("~~~~~~~~~~~~~~~~~dns_server:%s\n", dns_server);
	
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
	
	addr.sin_port = htons(8572);
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
	if(DDNS_REGIST == cmd_no)
	{
		timeout.tv_sec = 60;
	}
	else
	{
		timeout.tv_sec = 20;
	}
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
#endif

int contact_with_ddnsserver_konlan(char *domain_name,char *passwd, int cmd_no)
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
	printf("~~~~~~~~~~~~~~~~~dns_server:%s\n", dns_server);
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

