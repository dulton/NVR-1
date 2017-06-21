#include "mail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include "netcommon.h"

static int base64_encode(const char *str, int length, char *b64store)
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
	int pw = 0;
	
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


/*int Base64_Code(unsigned char *chsrc, unsigned char *chdes)
{
 char chadd[3];
 unsigned char temp[4],t;
 int pw = 0; //pw 
 	
 int len,i;
 len = strlen((char *)chsrc);

    while(len>=3)
 {
  temp[0] = (*chsrc)>>2;
  t       = (*chsrc & 0x03)<<4;
  temp[1] = (*(chsrc+1)>>4)|t;
  t       = ((*(chsrc+1)<<2) & 0x3f);
  temp[2] = (*(chsrc+2)>>6)|t;
  temp[3] = (*(chsrc+2) & 0x3f);

  for(i=0;i<4;i++)
  {
   if(temp[i]>=0 && temp[i]<=25) 
    *(chdes+i) = temp[i]+65;
   if(temp[i]>=26 && temp[i]<=51) 
    *(chdes+i) = temp[i]+71;
   if(temp[i]>=52 && temp[i]<=61) 
    *(chdes+i) = temp[i]-4;
   if(temp[i]==62) 
    *(chdes+i) = 43;
   if(temp[i]==63) 
    *(chdes+i) = 47;
  }
        
  len -= 3;
  chsrc += 3;
  chdes += 4;
  pw +=4;
 }
    if(len!=0)
 {
  for(i=0;i<3;i++)
   chadd[i] = 0;
        memcpy(chadd,chsrc,len);

  temp[0] = chadd[0]>>2;
  t       = (chadd[0] & 0x03)<<4;
  temp[1] = (chadd[1]>>4)|t;
  t       = ((chadd[1]<<2) & 0x3f);
  temp[2] = (chadd[2]>>6)|t;
  temp[3] = chadd[2] & 0x3f;

  for(i=0;i<4;i++)
  {
   if(temp[i]>=0 && temp[i]<=25 && (i==0 || i==1)) *(chdes+i) = temp[i]+65;
   else  *(chdes+i) = 61;
   if(temp[i]>=26 && temp[i]<=51) *(chdes+i) = temp[i]+71;
   else if(temp[i]>=52 && temp[i]<=61) *(chdes+i) = temp[i]-4;
   else if(temp[i]==62) *(chdes+i) = 43;
   else if(temp[i]==63) *(chdes+i) = 47;
  }
  chdes += 4;
  pw +=4;
  *chdes = '\0';
  pw+=1;
  //return;
  return pw;
 }
 *chdes = '\0';
 printf("~~~~~~~~~~~~~~~pw=%d\n", pw);  
  return pw;
}*/

static int read_attached_file(char *filepath, char *p_dst, char *encode_type)
{
	if (p_dst && filepath)
	{
		//encode_type
		FILE *p_attached = NULL;
		int i_read = 0, total_size = 0;
		char result[MAX_ATTACHED_FILE_LEN];

		printf("~~~~~~~~~~~~~~~~filepath:%s\n", filepath);
		p_attached = fopen(filepath, "rb");
		if (NULL == p_attached)
		{
			printf("********************open error\n");
			return 0;
		}

		
		while (!feof(p_attached))
		{
			i_read = fread(result + total_size, 1, MAX_ATTACHED_FILE_LEN - total_size, p_attached);
			if (i_read <= 0)
			{
				printf("********************read error\n");
				break;
			}
			total_size += i_read;
		}
		
		fclose(p_attached);

// 		printf("********************read size = %d\n", total_size);

		if (encode_type)
		{
			if (0 == strcmp(encode_type, "base64"))
			{
				total_size = base64_encode(result, total_size, p_dst);
				//int fd = open("base64", O_CREAT |O_RDWR);
				//printf("write bytes:%d\n", write(fd, p_dst, total_size));
				//close(fd);
				return total_size;
			}
		}

		memcpy(p_dst, result, total_size);
		
		return total_size;
	}
	else
	{
		printf("********************parameter error\n");
		return 0;
	}
}

#if 0
//pw 2010/9/15
static int read_attached_buff(char *buff, int buff_len, char *p_dst, char *encode_type)
{
	int total_size = -1;
	
	if (p_dst && buff)
	{
		//encode_type
		//FILE *p_attached = NULL;

		if (encode_type)
		{
			if (0 == strcmp(encode_type, "base64"))
			{
				total_size = base64_encode(buff, buff_len, p_dst);
				return total_size;
			}
		}

		memcpy(p_dst, buff, buff_len);
		return buff_len;
	}
	else
	{
		printf("********************parameter error\n");
		return 0;
	}
}

#endif

static BOOL recv_string_on_socket(int mail_socket)
{
	int retConnect = 0;
	char Buffer[1024] = {0};
	
    retConnect = recv(mail_socket, Buffer, sizeof(Buffer), 0); 
    if (SOCKET_ERROR == retConnect)
	{ 
		printf("Failed to connect.\n");     
        return FALSE;     
    }
   else
   { 
	printf("%s\n", Buffer);
	if(Buffer[0] == '5')
	{
		printf("the order is not support smtp host\n ");
		return FALSE;
	}
	
	return TRUE;
    }
}


static BOOL send_string_on_socket(int mail_socket, const char *p_str)
{
	int retConnect = 0;

	retConnect = send(mail_socket, p_str, strlen(p_str), 0); 
    if (SOCKET_ERROR == retConnect)
	{ 
		printf("Send string to the Mail Failure: %s\n", p_str); 
        return FALSE;
    }
	else
	{ 
// 		printf("Send string successfully:%s\n", p_str); 
		return TRUE;
    }
}

#define PRINT(s) printf("line:%d, s:%s\n", __LINE__, s);

extern BOOL send_gmail_ssl(char *s_name_from, char *s_mail_from, char *s_name_to, char *s_mail_to, char *s_user, char *s_password, char *s_mailserver, char *s_subject, char *s_body, char *s_attached, char *s_encode_type);

//pw 2010/9/15
BOOL sendmail(char *s_name_from, char *s_mail_from, char *s_name_to, char *s_mail_to, char *s_user, char *s_password, char *s_mailserver, char *s_subject, char *s_body, char *s_attached, char *s_encode_type)
{
#if defined(CHIP_HISI351X_2X) && defined(longse)//csp modify 20130528
	//csp modify 20130519
	//if(strstr(s_mailserver,"smtp.gmail.com"))
	if(strcmp(s_mailserver,"smtp.gmail.com") == 0)
	{
		printf("send gmail......smtp.gmail.com......\n");
		//fflush(stdout);
		return send_gmail_ssl(s_name_from, s_mail_from, s_name_to, s_mail_to, s_user, s_password, s_mailserver, s_subject, s_body, s_attached, s_encode_type);
	}
#else
	printf("Not support gmail");
#endif
	
	printf("send mail to:%s\n",s_mail_to);
	
	struct hostent *pHostent = NULL; 
	int mail_socket = INVALID_SOCKET;
	struct sockaddr_in service;
	int retConnect = 0;
	int data_len;
	char temp_str[100] = {0};
	char send_str[MAX_MAIL_BODY_LEN + MAX_ATTACHED_FILE_LEN];
	
	if(NULL == s_password || NULL == s_user || NULL == s_mail_from || NULL == s_mail_to || NULL == s_mailserver)
	{
		return FALSE;
	}

#ifdef WIN32
    WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	
	if(WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		printf("Error at WSAStartup()\n");
		goto WSACleanup;
    }
#endif

	//mail_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//Create a Socket
	mail_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mail_socket)
	{
		printf("Error at socket():\n");
		//printf("Error at socket(): %ld\n", WSAGetLastError());
		goto WSACleanup;
    }	
	
	printf("~~~~~~~~~~~~~s_mailserver:%s\n", s_mailserver);
	
	#if 0//
	return FALSE;
	#else
	pHostent = gethostbyname(s_mailserver);//Get the Mail Server Name 
	#endif
	if (NULL == pHostent)
	{
		printf("The Host Name is Invalid... h_errno %d\n", h_errno);
		goto WSACleanup;
    }
	
    service.sin_family = AF_INET; 
    memcpy(&service.sin_addr.s_addr, pHostent->h_addr_list[0], pHostent->h_length);
	
	char str[20];
	printf("~~~~~~~h_addr_list[0]:%s, len:%d\n", inet_ntop(pHostent->h_addrtype,pHostent->h_addr_list[0], str, sizeof(str)), pHostent->h_length);
	
#ifdef SET_SMTP_SERVER
	extern u16 smtp_port;
	service.sin_port = htons(smtp_port);
#else
	service.sin_port = htons(25);
#endif	
	
    //Connect to the remote Mail Server 
    retConnect = connect(mail_socket, (struct sockaddr*)&service, sizeof(service)); 
    if (SOCKET_ERROR == retConnect)
	{
		printf("Failed to connect.\n");
		goto WSACleanup;
    } 
	
	printf("Connect to %s....\n", inet_ntoa(service.sin_addr));
	
    //Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	//send Data to the Mail Server
	PRINT("~~~~~ehlo family\n");
	if (FALSE == send_string_on_socket(mail_socket, "EHLO family-5ae6e0ca\r\n"))
	{
		goto WSACleanup;
	}
	
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	PRINT("~~~~~~AUTH");
	//send Data to the Mail Server
	if (FALSE == send_string_on_socket(mail_socket, "AUTH LOGIN\r\n"))
	{
		goto WSACleanup;
	}
	
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
    //Send UserName to the Mail Server. The UserName is Encoded by Base64. 
	//printf("send username to mail server.\n");
	printf("~~~~~~~~~user:%s\n", s_user);
	base64_encode(s_user, strlen(s_user), send_str);
	strcat(send_str, "\r\n");
	PRINT("~~~~~~~~user");
	if (FALSE == send_string_on_socket(mail_socket, send_str))
	{
		goto WSACleanup;
	}
	
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
    //Send Password to the Mail Server The Password is Encoded by Base64. 
	//printf("send password to mail server.\n");
	printf("~~~~~~~~~~~~~s_password:%s\n", s_password);
	base64_encode(s_password, strlen(s_password), send_str);
	strcat(send_str, "\r\n");
	PRINT("~~~~~~~~passwd");
	if (FALSE == send_string_on_socket(mail_socket, send_str))
	{
		goto WSACleanup;
	}
	
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	//send Data to the Mail Server
	sprintf(send_str, "MAIL FROM: <%s>\r\n", s_mail_from);
	if (FALSE == send_string_on_socket(mail_socket, send_str))
	{
		goto WSACleanup;
	}
	printf("line:%d, send str:%s\n", __LINE__, send_str);
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	//send Data to the Mail Server
	sprintf(send_str,"RCPT TO: <%s>\r\n", s_mail_to);
	if (FALSE == send_string_on_socket(mail_socket, send_str))
	{
		goto WSACleanup;
	}
	
	printf("line:%d, send str:%s\n", __LINE__, s_mail_to);
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	//send commnd Data to the Mail Server
	if (FALSE == send_string_on_socket(mail_socket, "Data\r\n"))
	{
		goto WSACleanup;
	}
	
	printf("line:%d\n", __LINE__);
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	//send mail to the Mail Server
	sprintf(send_str, "From: \"%s\" <%s>\r\nTo: \"%s\" <%s>\r\nSubject: %s\r\n"
		, s_name_from, s_mail_from, s_name_to, s_mail_to, s_subject);
	if(s_attached)
	{
		printf("邮件有附件\n");
		
		strcat(send_str, "Content-Type: multipart/mixed;\r\n");
		sprintf(send_str + strlen(send_str), "%cboundary=\"%s\"\r\n", 9, MAIL_BOUNDARY1);
		strcat(send_str, "Content-Transfer-Encoding: 8Bit\r\n");
		strcat(send_str, "\r\n");
		strcat(send_str, "This is a multi-part message in MIME format.\r\n");
		strcat(send_str, "\r\n");
		
		sprintf(send_str + strlen(send_str), "--%s\r\n", MAIL_BOUNDARY1);
		strcat(send_str, "Content-Type: multipart/alternative;\r\n");
		sprintf(send_str + strlen(send_str), "%cboundary=\"%s\"\r\n\r\n", 9, MAIL_BOUNDARY2);
		sprintf(send_str + strlen(send_str), "--%s\r\n", MAIL_BOUNDARY2);
		
		strcat(send_str, "Content-Type: text/plain;\r\n");
		strcat(send_str, "Content-Transfer-Encoding: base64\r\n\r\n");
		base64_encode(s_body, strlen(s_body), send_str + strlen(send_str));
		sprintf(send_str + strlen(send_str), "==\r\n\r\n--%s--\r\n\r\n", MAIL_BOUNDARY2);
		
		sprintf(send_str + strlen(send_str), "--%s\r\n", MAIL_BOUNDARY1);
		strcat(send_str, "Content-Type: image/jpeg;\r\n");
		
		//sprintf(send_str + strlen(send_str), " name=\"%s\"\r\n", s_attached);
		sprintf(send_str + strlen(send_str), "%cname=\"%s\"\r\n", 9, s_attached);
		//strcat(send_str, "Content-Transfer-Encoding: base64\r\n");
		if (s_encode_type)
		{
			sprintf(send_str + strlen(send_str), "Content-Transfer-Encoding: %s\r\n", s_encode_type);
		}
		strcat(send_str, "Content-Disposition: attachment;\r\n");
		//sprintf(send_str + strlen(send_str), " filename=\"%s\"\r\n", s_attached);
		sprintf(send_str + strlen(send_str), "%cfilename=\"%s\"\r\n", 9, s_attached);
		strcat(send_str, "\r\n");
		data_len = strlen(send_str);
		printf("datalen = %d\n", data_len);
		
		//pw 2010/9/15
		data_len += read_attached_file(s_attached, send_str + data_len, s_encode_type);
		
		//sprintf(temp_str, "==\r\n\r\n--%s--\r\n", MAIL_BOUNDARY1);
		sprintf(temp_str, "\r\n\r\n--%s--\r\n\r\n", MAIL_BOUNDARY1);
		strcpy(send_str + data_len, temp_str);
		data_len += strlen(temp_str);
	}
	else
	{
		strcat(send_str, "Content-Type: multipart/alternative;\r\n");
		sprintf(send_str + strlen(send_str), "%cboundary=\"%s\"\r\n", 9, MAIL_BOUNDARY1);
		strcat(send_str, "Content-Transfer-Encoding: 8Bit\r\n");
		strcat(send_str, "This is a multi-part message in MIME format.\r\n\r\n");
		
		sprintf(send_str + strlen(send_str), "--%s\r\n", MAIL_BOUNDARY1);
		strcat(send_str, "Content-Type: text/plain;\r\n");
		
		//csp modify
		//sprintf(send_str + strlen(send_str), "%ccharset=\"gb2312\"\r\n", 9);
		//strcat(send_str, "	charset=\"utf-8\"\r\n");//csp modify
		
		printf("邮件无附件\n");
		
		strcat(send_str, "Content-Transfer-Encoding: base64\r\n\r\n");
		base64_encode(s_body, strlen(s_body), send_str + strlen(send_str));
		sprintf(send_str + strlen(send_str), "==\r\n\r\n--%s--\r\n", MAIL_BOUNDARY1);
		
		data_len = strlen(send_str);
	}
	strcpy(temp_str, ".\r\n");
	memcpy(send_str + data_len, temp_str, strlen(temp_str));
	data_len += strlen(temp_str);	
	//retConnect = send(mail_socket, send_str, data_len, 0); 
	retConnect = loopsend(mail_socket, send_str, data_len);
	if (SOCKET_ERROR == retConnect)
	{
		printf("Send Context Of Mail to Mail Failure...\n");
		goto WSACleanup;
    }
	printf("~~~~~line:%d~~~~~~~~retconnect:%d\n", __LINE__, retConnect);
//	else
//	{
//		printf("send mail successfully\n"); 
//	}
	
	//Receive Data From the Mail Server
	if (FALSE == recv_string_on_socket(mail_socket))
	{
		goto WSACleanup;
	}
	
	printf("~~~~~line:%d~~~~~~~~recv_string_on_socket\n", __LINE__);
    //Send "QUIT" Context to the Mail Server
	if (FALSE == send_string_on_socket(mail_socket, "QUIT\r\n"))
	{
		goto WSACleanup;
	}
	
	printf("~~~~~line:%d~~~~~~~~send_string_on_socket\n", __LINE__);
	//printf("Send Mail Successful!\n");
	
	if (INVALID_SOCKET != mail_socket)
	{
#ifdef WIN32
		closesocket(mail_socket);
#else
		close(mail_socket);
#endif
	}
#ifdef WIN32
	WSACleanup();
#endif
	
	return TRUE;
	
WSACleanup:
	{ 
		if (INVALID_SOCKET != mail_socket)
		{
#ifdef WIN32
			closesocket(mail_socket);
#else
			close(mail_socket);
			mail_socket = -1;
#endif	
		}
#ifdef WIN32
        WSACleanup();
#endif
	}
	
#ifdef WIN32
    system("pause");
#endif
	
	return FALSE;
}

// 	sendmail("chinadvr2009", "chinadvr2009@sina.com", "lbzhu", "lbzhu@tl-tek.com"
// 		, "chinadvr2009", "9002dvr?_-", "smtp.sina.com", "this is a mail test", "wonderful mail"
// 		, "1.jpg", "base64");

