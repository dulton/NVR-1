/******************************************************************************

  Copyright (C), 2013-2020, Juan Optical & Electronical Tech Co., Ltd. All Rights Reserved.

 ******************************************************************************
  File Name    : rtsplib.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : rtsp  utils , reference to rfc2326
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "sock.h"
#include "rtsplib.h"
#include "rtspsession.h"
#include "portmanage.h"

//#include <string.h>

//response status code definitions
typedef struct _RStatusCode
{
	int code;
	const char *info;
}RStatusCode_t;

enum
{
	RTSP_RSC_CONTINUE = 0,			//// -0
	RTSP_RSC_OK,						////-1
	RTSP_RSC_CREATED,
	RTSP_RSC_LOW_STORAGE,
	RTSP_RSC_MULTI_CHOICES,
	RTSP_RSC_MOVED_PERMANENTLY,		////-5
	RTSP_RSC_MOVED_TEMPORARILY,
	RTSP_RSC_SEE_OTHER,
	RTSP_RSC_NOT_MODIFIED,
	RTSP_RSC_USE_PROXY,
	RTSP_RSC_BAD_REQUEST,			////-10
	RTSP_RSC_UNAUTHORIZED,
	RTSP_RSC_PAYMENT_REQUIRED,
	RTSP_RSC_FORBIDDEN,
	RTSP_RSC_NOT_FOUND,
	RTSP_RSC_METHOD_NOT_ALLOWED,		////-15
	RTSP_RSC_NOT_ACCEPTABLE,
	RTSP_RSC_PROXY_AUTHEN_REQUIRED,
	RTSP_RSC_REQUEST_TIMEOUT,
	RTSP_RSC_GONE,
	RTSP_RSC_LENGTH_REQUIRED,		////-20
	RTSP_RSC_PRECONDITION_FAILED,
	RTSP_RSC_REQUEST_ENTITY_TOOLARGE,
	RTSP_RSC_REQUEST_URI_TOOLARGE,
	RTSP_RSC_UNSUPPORTED_MEDIA,
	RTSP_RSC_PARAMETER_NOT_UNDERSTOOD,	////-25
	RTSP_RSC_CONFERENCE_NOT_FOUND,
	RTSP_RSC_NOT_ENOUGHT_BANDWIDTH,
	RTSP_RSC_SESSION_NOT_FOUND,
	RTSP_RSC_METHOD_NOT_VAILD,
	RTSP_RSC_HEADER_FIELD_NOT_VAILD,		////-30
	RTSP_RSC_INVALID_RANGE,
	RTSP_RSC_PARAMETER_IS_READONLY,
	RTSP_RSC_AGGREGATE_NOT_ALLOWED,
	RTSP_RSC_ONLY_AGGREGATE_ALLOWED,
	RTSP_RSC_UNSUPPORTED_TRANSPORT,		////-35
	RTSP_RSC_DESITIATION_UNREACHABLE,
	RTSP_RSC_INTERNAL_SERVER_ERROR,
	RTSP_RSC_NOT_IMPLEMENTED,
	RTSP_RSC_GAD_GATEWAY,
	RTSP_RSC_SERVICE_UNAVAILABLE,		////-40
	RTSP_RSC_GATEWAY_TIMEOUT,
	RTSP_RSC_RTSP_VERSION_NOTSUPPORTED,
	RTSP_RSC_OPTION_NOTSUPPORTED,
	RTSP_RSC_END							////-44
};

static RStatusCode_t rtspRStatusCodes[] = 
{
	{100,"Continue"},					////-0
	{200,"OK"},
	{201,"Created"},
	{250,"Low on Storage Space"},
	{300,"Multiple Choices"},
	{301,"Moved Permanently"},
	{302,"Moved Temporarily"},
	{303,"See Other"},
	{304,"Not MOdified"},
	{305,"Use Proxy"},
	{400,"Bad Request"},				////-10
	{401,"Unauthorized"},
	{402,"Payment Required"},
	{403,"Forbidden"},
	{404,"Not Found"},
	{405,"Method Not Allowed"},
	{406,"Not Acceptable"},
	{407,"Proxy Authentication Required"},
	{408,"Request Time-out"},
	{410,"Gone"},
	{411,"Length Required"},			////-20
	{412,"Precondition Failed"},
	{413,"Request Entity Too Large"},
	{414,"Request-URI Too Large"},
	{415,"Unsupported Media Type"},
	{451,"Parameter Not Understood"},
	{452,"Conference Not Found"},
	{453,"Not Enough Bandwidth"},
	{454,"Session Not Found"},
	{455,"Method Not Valid in This State"},
	{456,"Header Field Not Vaild for Resource"},	////-30
	{457,"Invalid Range"},
	{458,"Parameter Is Read-Only"},
	{459,"Aggregate operation not allowed"},
	{460,"Only aggregate operation allowed"},
	{461,"Unsupported transport"},
	{462,"Destination unreachable"},
	{500,"Internal Server Error"},
	{501,"Not Implemented"},
	{502,"Bad Gateway"},
	{503,"Service Unavailable"},					////-40
	{504,"Gateway Time-out"},
	{505,"RTSP Version not supported"},
	{551,"Option not supported"},					////-43
};

#if 0
static const char *rtspMethods[RTSP_METHOD_CNT] = 
{
	"DESCRIBE",
	"ANNOUNCE",
	"GET PARAMETER",
	"OPTIONS",
	"PAUSE",
	"PLAY",
	"RECORD",
	"REDIRECT",
	"SETUP",
	"SET_PARAMETER",
	"TEARDOWN",
	//""
};
#endif

//increase by 4
#define RTSP_MIN_CHN_PORT	1
#define RTSP_MAX_CHN_PORT	200

#if 0
static int g_server_port = RTSP_SERVER_PORT_BEGIN;
static int g_client_port = RTSP_CLIENT_PORT_BEGIN;
static int g_channel = RTSP_CHANNEL_BEGIN;
#endif

static RtspStreamTable_t g_RtspStreamTable={0,};

static int RTSP_request_describe(Rtsp_t *r);

#if defined(_WIN32) || defined(_WIN64)
char *STRCASESTR(char *s1,char *s2)
{
    char *ptr = s1;
	
    if(!s1 || !s2 || !*s2) return s1;
	
    while(*ptr)
	{
		if(toupper(*ptr) == toupper(*s2))
		{
			char * cur1 = ptr + 1;
			char * cur2 = s2 + 1;
			while(*cur1 && *cur2 && toupper(*cur1) == toupper(*cur2))
			{
				cur1++;
				cur2++;
			}
			if(!*cur2)
			{
				return ptr;
			}
		}
		ptr++;
    }
    return NULL;
}
#else
extern char* strcasestr(const char* str, const char* subStr);
#define STRCASESTR strcasestr
#endif

static inline int http_get_number(char *src,char *key,int *ret)
{
	char *tmp = STRCASESTR(src,key);
	if(tmp == NULL)
	{
		*ret = -1;
		return -1;
	}
	else
	{
		tmp += strlen(key);
		if((*tmp) == ' ')
		{
			tmp++;
		}
		
		//csp modify
		if(tmp != NULL)
		{
			//if(strstr(tmp,"\r\n\r\n") == NULL)
			if(strstr(tmp,"\r\n") == NULL)
			{
				*ret = -1;
				return -1;
			}
		}
		
		sscanf(tmp,"%d",ret);
		return 0;
	}
}

static inline int http_get_string(char *src,char *key,char *ret)
{
	char *tmp = STRCASESTR(src,key);
	if(tmp == NULL)
	{
		*ret = 0;
		return -1;
	}
	else
	{
		tmp += strlen(key);
		if(*tmp == ' ')
		{
			tmp++;
		}
		sscanf(tmp,"%[^\r\n]",ret);
		return 0;
	}
}

static inline int http_get_url(char *src,char *ip,int *port,char *stream)
{
	int i_ip[4];
	char tmp[128], *ptr = tmp;
	char *p = NULL;
	if(strncmp(src,"rtsp://",strlen("rtsp://")) != 0)
	{
		if(sscanf(src,"%*s %s %*s",tmp) != 1)
		{
			VLOG(VLOG_ERROR,"request url format wrong");
			return -1;
		}
		if(strncmp(tmp,"rtsp://",strlen("rtsp://")) != 0)
		{
			VLOG(VLOG_ERROR,"request url format wrong");
			return -1;
		}
		ptr = tmp;
	}
	else
	{
		ptr = src;
	}
	if((p = strstr(ptr+strlen("rtsp://"),":")) == NULL)
	{
		*port = RTSP_DEFAULT_PORT;
		sscanf(ptr,"rtsp://%[^/]/%s",ip,stream);
	}
	else
	{
		sscanf(ptr,"rtsp://%[^:]:%d/%s",ip,port,stream);
	}
	if(sscanf(ip,"%d.%d.%d.%d",&i_ip[0],&i_ip[1],&i_ip[2],&i_ip[3]) != 4)
	{
		SOCK_gethostbyname(ip,tmp);
		strcpy(ip,tmp);
	}
	//VLOG(VLOG_DEBUG,"ip:%s stream:%s port:%d",ip,stream,*port);
	//printf("ip:%s stream:%s port:%d\n",ip,stream,*port);
	//fflush(stdout);
	return 0;
}

static uint32_t hash_string(char *str)
{
#define HASHWORDBITS	(32)
	uint32_t hval=0xFFFFFFFF,g;
	char *pstr=str;
	while(*str)
	{
		hval <<= 4;
		hval += (unsigned char)*str++;
		g = hval & ((unsigned int)0xf << (HASHWORDBITS -4));
		if(g != 0)
		{
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	
	VLOG(VLOG_DEBUG,"string:%s hashval:%x",pstr,hval);
	return hval;
}

#if 0
static int rtsp_check_setup_url(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"setup steam:%s",r->stream);
	return RTSP_RET_OK;
}
#endif

static int rtsp_parse_transport(Rtsp_t *r,char *buf)
{
	char *p,*q;
	char transport[128];
	p=transport;
	*buf=0;
	if(http_get_string(r->payload,"Transport:",transport) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	if((q = strstr(transport,"RTP/AVP/TCP")) != NULL)
	{
		r->low_transport = RTP_TRANSPORT_TCP;
		strcat(buf,"RTP/AVP/TCP");
	}
	else
	{
		strcat(buf,"RTP/AVP/UDP");
		r->low_transport = RTP_TRANSPORT_UDP;
	}
	if((q = strstr(p,"multicast")) != NULL)
	{
		r->cast_type = RTP_MULTICAST;
		strcat(buf,";multicast");
	}
	else
	{
		r->cast_type = RTP_UNICAST;
		strcat(buf,";unicast");
	}
	if((q = strstr(p,"interleaved")) != NULL)
	{
		r->b_interleavedMode = TRUE;
		q += strlen("interleaved=");
		sscanf(q,"%d%*s",&r->channel);
		q = buf+strlen(buf);
		sprintf(q,";interleaved=%d-%d",r->channel,r->channel + 1);
		r->client_port = r->channel;
		r->server_port = r->channel;
	}
	else
	{
		unsigned int port_tmp;
		r->b_interleavedMode = FALSE;
#if	RTSP_ENABLE_RTCP == TRUE
		PORT_MANAGE_apply2_port(&port_tmp);
#else
		PORT_MANAGE_apply1_port(&port_tmp);
#endif
		r->server_port = port_tmp;
		if((q = strstr(p,"client_port")) != NULL)
		{
			q += strlen("client_port=");
			printf("q:%s\n",q);
			sscanf(q,"%d%*s",&r->client_port);
		}
		if((q = strstr(p,"server_port")) != NULL)
		{
			q += strlen("server_port=");
			sscanf(q,"%d%*s",&r->server_port);
		}
		q=buf+strlen(buf);
		sprintf(q,";client_port=%d-%d;server_port=%d-%d",
			r->client_port,r->client_port+ 1,r->server_port,r->server_port+1);
	}
	if(r->role == RTSP_SERVER)
	{
		q = buf+strlen(buf);
		sprintf(q,";ssrc=%x",hash_string(r->stream));
	}
	else
	{
		if((q = strstr(p,"ssrc=")) != NULL)
		{
			q += strlen("ssrc=");
			sscanf(q,"%x%*s",&r->ssrc);
		}
	}
	if((q = strstr(p,"mode")) != NULL)
	{
		q += strlen("mode=\"");
		if(strcmp(q,"PLAY") == 0)
		{
			r->work_mode = RTSP_MODE_PLAY;
			strcat(buf,";mode=\"PLAY\"");
		}
		else if(strcmp(q,"RECORD") == 0)
		{
			r->work_mode = RTSP_MODE_RECORD;
			strcat(buf,";mode=\"RECORD\"");
		}
	}
	//ssrc
	// ttl
	//....
	VLOG(VLOG_DEBUG,"parse transport:client_port:%d server_port:%d,%s",
		r->client_port,r->server_port,r->cast_type ? "multicast" : "unicast");
	VLOG(VLOG_DEBUG,"Transport: %s",buf);
	return RTSP_RET_OK;
}

inline int rtsp_setup_transport(Rtsp_t *r,char *buf)
{
	char *ptr=buf;
	if(r->low_transport == RTP_TRANSPORT_TCP)
		sprintf(ptr,"RTP/AVP/TCP");
	else
		sprintf(ptr,"RTP/AVP");
	ptr = buf + strlen(buf);
	if(r->cast_type == RTP_MULTICAST)
		sprintf(ptr,";multicast");
	else
		sprintf(ptr,";unicast");
	ptr = buf + strlen(buf);
	if(r->b_interleavedMode == true)
		sprintf(ptr,";interleaved=%d-%d",r->channel,r->channel+1);
	else
		sprintf(ptr,";client_port=%d-%d",r->client_port,r->client_port+1);

	return RTSP_RET_OK;
}

#if 0
static void rtsp_gmt_time_string(char *ret,int size)
{
	time_t t;
	time(&t);
	strftime(ret, size, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&t));
}
#endif

static int rtsp_send_packet(Rtsp_t *r)
{
	int ret = send(r->sock,r->payload,r->payload_size,0);
	if(ret != r->payload_size)
	{
		VLOG(VLOG_ERROR,"rtsp send packet(size:%d) failed.",r->payload_size);
		return RTSP_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"rtsp send packet(size:%d) ok",r->payload_size);
	return RTSP_RET_OK;
}

static int rtsp_parse_response(Rtsp_t *r,int *code,char *info)
{
	int cseq;
	char tmp[1024], *ptr = NULL;//char tmp[256], *ptr = NULL;//csp modify 20140221
	if(memcmp(r->payload,RTSP_VERSION,strlen(RTSP_VERSION)) == 0)
	{
		sscanf(r->payload,"%*s %d %s",code,info);
		VLOG(VLOG_DEBUG,"response code:%d info:%s",*code,info);
		if((*code) == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code)
		{
			//AUTH_init(&r->auth,HTTP_AUTH_BASIC);
			r->bLogin = false;
		}
		else if((*code) != rtspRStatusCodes[RTSP_RSC_OK].code)
		{
			VLOG(VLOG_ERROR,"response code not ok,%d",*code);
			return RTSP_RET_FAIL;
		}
	}
	else
	{
		VLOG(VLOG_ERROR,"invalid rtsp response");
		return RTSP_RET_FAIL;
	}
	// check cseq
	if(http_get_number(r->payload,"CSeq:",&cseq) == RTSP_RET_FAIL)
	{
		VLOG(VLOG_ERROR,"invaild request format,not found cseq");
		return RTSP_RET_FAIL;
	}
	else
	{
		if(cseq != r->cseq)
		{
			VLOG(VLOG_ERROR,"cseq number is wrong");
			return RTSP_RET_FAIL;
		}
	}
	if(http_get_string(r->payload,"Public:",tmp) == RTSP_RET_OK)
	{
		strcpy(r->allow_method,tmp);
	}
	if(http_get_string(r->payload,"Content-type:",tmp) == RTSP_RET_OK)
	{
		if(strcmp(tmp,SDP_MEDIA_TYPE) == 0)
		{
			// parse sdp here....
			VLOG(VLOG_DEBUG,"Get SDP, Goto parse it...");
			ptr = strstr(r->payload,"\r\n\r\n");
			if(ptr == NULL)
			{
				VLOG(VLOG_ERROR,"decribe response format err,check it");
				return RTSP_RET_FAIL;
			}
			ptr += strlen("\r\n\r\n");
			r->sdp = SDP_decode(ptr);
			if(r->sdp == NULL)
			{
				return RTSP_RET_FAIL;
			}
		}
	}
	if(http_get_string(r->payload,"Session:",tmp) == RTSP_RET_OK)
	{
		if(strstr(tmp,";") != NULL)
		{
			sscanf(tmp,"%[^;];timeout=%d",r->session_id,&r->session_timeout);
		}
		else
		{
			strcpy(r->session_id,tmp);
		}
		VLOG(VLOG_DEBUG,"\tsessin id:%s",r->session_id);
	}
	if(http_get_string(r->payload,"Transport:",tmp) == RTSP_RET_OK)
	{
		rtsp_parse_transport(r,tmp);
	}
	// auth
	if(http_get_string(r->payload,"WWW-Authenticate:",tmp) == RTSP_RET_OK)
	{
		//需要认证，初始化认证
		if(HTTP_AUTH_client_init(&r->auth,tmp) == AUTH_RET_FAIL)
		{
			VLOG(VLOG_ERROR,"HTTP_AUTH_client_init failed");
			return RTSP_RET_FAIL;
		}
		
		//return RTSP_request_describe(r);
	}
	
	VLOG(VLOG_DEBUG,"parse response success");
	return RTSP_RET_OK;
}

#if 0//csp modify 20140221
static int RTSP_request_options(Rtsp_t *r)
{
	int status_code;
	char tmp[256];
	const char format[] = 
		"OPTIONS %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,++r->cseq,RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	// get response
	if(RTSP_read_message(r) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	
	// get valid methods,(Public)
	//if(http_get_string(r->payload,"Public:",r->allow_method)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	//VLOG(VLOG_DEBUG,"Get Allow Method:%s",r->allow_method);
	
	return RTSP_RET_OK;
}
#endif

static int RTSP_request_describe(Rtsp_t *r)
{
	int status_code;
	char tmp[256];
	char url[200];
	char szAuth[512];
	int auth_flag=false;
	const char format1[]=
		"DESCRIBE %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Accept: %s\r\n"\
		"\r\n";
	const char format2[]=
		"DESCRIBE %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Accept: %s\r\n"\
		"Authorization: %s\r\n"\
		"\r\n";
	
	__TRY_AGIN:
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	if(r->bLogin == true)
	{
		//printf("login already\n");
		//fflush(stdout);
		sprintf(r->payload,format1,tmp,RTSP_VERSION,++r->cseq,RTSP_USER_AGENT,SDP_MEDIA_TYPE);
	}
	else
	{
		//printf("login setup\n");
		//fflush(stdout);
		sprintf(url,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
		if(HTTP_AUTH_setup(r->auth,r->user,r->pwd,url,"DESCRIBE",szAuth,sizeof(szAuth)) == AUTH_RET_FAIL)
		{
			return RTSP_RET_FAIL;
		}
		sprintf(r->payload,format2,tmp,RTSP_VERSION,++r->cseq,RTSP_USER_AGENT,SDP_MEDIA_TYPE,szAuth);
	}	
	r->payload_size = strlen(r->payload);
	//VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	//printf("RTSP_request_describe request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	//fflush(stdout);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	// get response
	if(RTSP_read_message(r) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	// parse response
	//printf("RTSP_request_describe response (size:%d):\r\n%s@@@\r\n",r->payload_size,r->payload);
	//fflush(stdout);
	if(rtsp_parse_response(r,&status_code,tmp) == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	if((status_code == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code) && (auth_flag == false))
	{
		//VLOG(VLOG_CRIT,"REQUIRE AUTH!!!!");
		auth_flag = true;
		goto __TRY_AGIN;
	}
	else if(status_code == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code)
	{
		VLOG(VLOG_ERROR,"AUTH FAILED!!!!");
		return RTSP_RET_FAIL;
	}
	else
	{
		//printf("RTSP_request_describe result:%d\n",status_code);
		//fflush(stdout);
	}
	//printf("RTSP_request_describe ok\n");
	//fflush(stdout);
	// parse sdp here...
	//
	return RTSP_RET_OK;
}

#if 0
static int RTSP_request_annouce(Rtsp_t *r)
{
	return RTSP_RET_OK;
}

static int RTSP_request_get_parameter(Rtsp_t *r)
{
	return RTSP_RET_OK;
}

static int RTSP_request_set_parameter(Rtsp_t *r)
{
	return RTSP_RET_OK;
}
#endif

static int RTSP_request_play(Rtsp_t *r)
{
	int status_code;
	char tmp[256];
	const char format[]=
		"PLAY %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"Range: npt=0.000-\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		r->session_id);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	//if(r->b_interleavedMode = true)//csp ???
	if(r->b_interleavedMode == true)//csp modify
	{
		if(RTSP_read_message4(r) == RTSP_RET_FAIL)
			return RTSP_RET_FAIL;
	}
	else
	{
		if(RTSP_read_message(r) == RTSP_RET_FAIL)
			return RTSP_RET_FAIL;
	}
	//printf("%s",r->payload);//lcy modify
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	//
	r->state = RTSP_STATE_PLAYING;
	//
	return RTSP_RET_OK;
}

#if 0
static int RTSP_request_pause(Rtsp_t *r)
{
	return RTSP_RET_OK;
}
static int RTSP_request_record(Rtsp_t *r)
{
	return RTSP_RET_OK;
}
static int RTSP_request_redirect(Rtsp_t *r)
{
	return RTSP_RET_OK;
}
#endif

static int RTSP_request_setup(Rtsp_t *r,char *control,int type)
{
	int ret;
	int status_code;
	int b_video;
	unsigned int chn_port_tmp = 0;
	char sockname[20];
	Rtp_t **rtp = NULL;
	Rtcp_t **rtcp = NULL;
	int rtp_chn_port = 0,rtcp_chn_port = 0;
	int interleaved_tmp = r->b_interleavedMode;
	char tmp[256],tmp2[256];
	int rtp_sock;
	const char format[] = 
		"SETUP %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Transport: %s\r\n"\
		"\r\n";
	
	//printf("RTSP_request_setup-1\n");
	
	if(interleaved_tmp == RTSP_RTP_AUTO)
	{
		r->b_interleavedMode = RTSP_RTP_OVER_UDP;
		r->low_transport = RTP_TRANSPORT_UDP;
	}
	
TRY_INTERLEAVED_MODE:
	if(memcmp(control,"rtsp://",strlen("rtsp://"))==0)
	{
		sprintf(tmp,"%s",control);
	}
	else
	{
		sprintf(tmp,"rtsp://%s:%d/%s/%s",r->ip_me,r->port,r->stream,control);
	}
	if(r->b_interleavedMode == true)
	{
		if(type == SDP_PAYLOAD_TYPE_ALAW || type == SDP_PAYLOAD_TYPE_ULAW)
		{
			r->channel = 2;
		}
		else
		{
			r->channel = 0;
		}
		r->client_port = r->channel;
		r->server_port = r->channel;
	}
	else
	{
		PORT_MANAGE_apply2_port(&chn_port_tmp);
		r->client_port = chn_port_tmp;
	}
	//printf("RTSP_request_setup-2\n");
	rtsp_setup_transport(r,tmp2);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		tmp2);
	r->payload_size = strlen(r->payload);
	//VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	//printf("request (size:%d):\n%s\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
	{
		printf("RTSP_request_setup:rtsp_send_packet failed\n");
		return RTSP_RET_FAIL;
	}
	// get response
	if(RTSP_read_message(r) == RTSP_RET_FAIL)
	{
		printf("RTSP_request_setup:RTSP_read_message failed\n");
		return RTSP_RET_FAIL;
	}
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp) == RTSP_RET_FAIL)
	{
		if((status_code == RTSP_RSC_UNSUPPORTED_TRANSPORT) && 
			(interleaved_tmp == RTSP_RTP_AUTO) &&
			(r->b_interleavedMode == RTSP_RTP_OVER_UDP))
		{
			r->b_interleavedMode = RTSP_RTP_OVER_RTSP;
			r->low_transport = RTP_TRANSPORT_TCP;
			goto TRY_INTERLEAVED_MODE;
		}
		printf("RTSP_request_setup:rtsp_parse_response failed\n");
		return RTSP_RET_FAIL;
	}
	// setup network
	//init rtp
	if(type == SDP_PAYLOAD_TYPE_ALAW || type == SDP_PAYLOAD_TYPE_ULAW)
	{
		rtp = (Rtp_t **)(&r->rtp_audio);
		rtcp = (Rtcp_t **)&r->rtcp_audio;
		b_video = false;
	}
	else if(type == SDP_PAYLOAD_TYPE_DYNAMIC)
	{
		rtp = (Rtp_t **)(&r->rtp_video);
		rtcp = (Rtcp_t **)&r->rtcp_video;
		b_video = true;
	}
	else
	{
		VLOG(VLOG_ERROR,"SETUP: unsupport payload type:%d",type);
		return RTSP_RET_FAIL;
	}
	//VLOG(VLOG_DEBUG,"%s setup: c_port:%d s_port:%d",control,r->client_port,r->server_port);
	//printf("%s setup: client_port:%d server_port:%d",control,r->client_port,r->server_port);
	if(r->b_interleavedMode)
	{
		if((r->channel % 2) == 0)
		{
			rtp_chn_port = r->channel;
			rtcp_chn_port = r->channel + 1;
		}
		else
		{
			rtcp_chn_port = r->channel;
			rtp_chn_port = r->channel + 1;
		}
		rtp_sock = r->sock;
	}
	else
	{
		if((r->server_port % 2) == 0)
		{
			rtp_chn_port = r->server_port;
			rtcp_chn_port = r->server_port + 1;
		}
		else
		{
			rtcp_chn_port = r->server_port;
			rtp_chn_port = r->server_port + 1;
		}
		rtp_sock = SOCK_udp_init(r->client_port,RTSP_SOCK_TIMEOUT);
		if(rtp_sock == -1) return RTSP_RET_FAIL;
	}
	//printf("RTSP_request_setup-3\n");
	*rtp = RTP_client_new(r->low_transport,r->b_interleavedMode,rtp_sock,r->peername,rtp_chn_port,r->buffer_time);
	if(*rtp == NULL) return RTSP_RET_FAIL;
	//RTP_set_type(rtp, type);//csp ???
	RTP_set_type(*rtp, type);//csp modify
	//init rtcp
	ret = RTCP_init(rtcp,r->role,r->ssrc,r->low_transport,r->cast_type,r->b_interleavedMode,r->sock,rtcp_chn_port,r->client_port+1,*rtp);
	if(ret == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	else
	{
		// nat detect
		SOCK_getsockname(r->sock,sockname);
	}
	
	//printf("RTSP_request_setup-4\n");
	
	#if 0
	if(r->b_interleavedMode==false)
	{
		if(SOCK_isreservedip(sockname)==true)
		{
			if(SOCK_isreservedip((*rtp)->peername)!=true)rtsp_request_nat_detect(*rtp);
		}
	}
	#endif
	
	if(r->b_interleavedMode==false && SOCK_isreservedip(sockname)==true && SOCK_isreservedip((*rtp)->peername)!=true)
	{
		rtsp_request_nat_detect(*rtp);
	}
	
	r->state = RTSP_STATE_READY;
	
	return RTSP_RET_OK;
}

int RTSP_request_teardown(Rtsp_t *r)
{
	//int status_code;
	char tmp[256];
	const char format[]=
		"TEARDOWN %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		r->session_id);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	//if(RTSP_read_message(r) == RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	// parse response
	//if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	//
	r->state = RTSP_STATE_INIT;
	//
	return RTSP_RET_OK;
}

/**************************************************************
* nat detect
***************************************************************/
int rtsp_request_nat_detect(Rtp_t *r)
{
	int ret;
	char buf[1024];
	const char *format = "NatDetect * RTSP/1.0\r\n";
	if(r->interleaved == true)
	{
		VLOG(VLOG_DEBUG,"rtp over rtsp,no need to do nat detect");
		return RTSP_RET_OK;
	}
	ret = SOCK_sendto(r->sock,r->peername,r->peer_chn_port,(char *)format,strlen(format));
	if(ret == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	//csp modify
	//ret = SOCK_recvfrom(r->sock,r->peername,r->peer_chn_port,buf,sizeof(buf),0);
	ret = SOCK_recvfrom(r->sock,r->peername,&r->peer_chn_port,buf,sizeof(buf),0);
	if(ret == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	buf[ret] = 0;
	VLOG(VLOG_DEBUG,"nat detect response:%s",buf);
	if(memcmp(buf,format,strlen(format)) == 0)
	{
		VLOG(VLOG_DEBUG,"Nat detect success");
	}
	else
	{
		VLOG(VLOG_ERROR,"Nat detect failed");
		return RTSP_RET_FAIL;
	}
	
	return RTSP_RET_OK;
}

int rtsp_parse_nat_detect(Rtp_t *r)
{
	int ret;
	char buf[1024];
	fd_set read_set;
	struct timeval timeout;
	if(r->interleaved == true)
	{
		VLOG(VLOG_DEBUG,"rtp over rtsp,no need to do nat detect");
		return RTSP_RET_OK;
	}
	timeout.tv_sec = 1;
	timeout.tv_usec = 500*1000;
	FD_ZERO(&read_set);
	FD_SET(r->sock,&read_set);
	ret = select(r->sock+1,&read_set,NULL,NULL,&timeout);
	if(ret <= RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	//csp modify
	//ret = SOCK_recvfrom(r->sock,r->peername,r->peer_chn_port,buf,sizeof(buf),0);
	ret = SOCK_recvfrom(r->sock,r->peername,&r->peer_chn_port,buf,sizeof(buf),0);
	if(ret == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	buf[ret] = 0;
	VLOG(VLOG_DEBUG,"nat detect response:%s",buf);
	// send back
	ret = SOCK_sendto(r->sock,r->peername,r->peer_chn_port,buf,ret);
	if(ret == RTSP_RET_FAIL)
	{
		return RTSP_RET_FAIL;
	}
	
	return RTSP_RET_OK;
}

Rtsp_t* RTSP_SERVER_init(int fd,int bAudio)
{
	Rtsp_t *r = NULL;
	printf("sock:%d\n",fd);
	r = (Rtsp_t *)malloc(sizeof(Rtsp_t));
	if(r == NULL)
	{
		VLOG(VLOG_ERROR,"maloc for rtsp failed");
		return NULL;
	}
	memset(r,0,sizeof(Rtsp_t));
	r->toggle = true;
	r->data_available = 0;
	r->role = RTSP_SERVER;
	r->sock = fd;
	r->state = RTSP_STATE_INIT;
	r->buffer_time = 0;
	r->cseq = 0;
	r->b_interleavedMode = 0;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->low_transport = RTP_TRANSPORT_UDP;
	SOCK_getpeername(fd,r->peername);
	if(bAudio == true)
	{
		r->stream_type = RTSP_STREAM_VIDEO | RTSP_STREAM_AUDIO;
	}
	else
	{
		r->stream_type = RTSP_STREAM_VIDEO;
	}
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;
	sprintf(r->session_id,"%d",rand());
	strcpy(r->allow_method,RTSP_ALLOW_METHODS);
	if(SOCK_tcp_init(fd,RTSP_SOCK_TIMEOUT) == -1)
	{
		free(r);
		return NULL;
	}
#if RTSP_ENABLE_AUTHTICATION
	HTTP_AUTH_server_init(&r->auth,HTTP_AUTH_DEFAULT_TYPE);
	r->bLogin = false;
#else
	r->auth = NULL;
	r->bLogin = true;
#endif
	
	VLOG(VLOG_CRIT,"rtsp server init done.");
	
	return r;
}

Rtsp_t* RTSP_CLIENT_init(const char *url,char *user,char *pwd,int bInterleaved,int iBufferTime)
{
	Rtsp_t *r = (Rtsp_t *)calloc(1, sizeof(Rtsp_t));
	if(r == NULL)
	{
		VLOG(VLOG_ERROR,"maloc for rtsp failed");
		exit(0);//csp ???
	}
	r->toggle = RTSPC_RUNNING;
	r->data_available = 0;
	r->role = RTSP_CLIENT;
	r->sock = -1;
	r->state = RTSP_STATE_INIT;
	r->cseq = 0;
	r->buffer_time = iBufferTime;
	r->b_interleavedMode = bInterleaved;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->blocal = true;// note .....,with rtpbuf
	if(bInterleaved == true)
	{
		r->low_transport = RTP_TRANSPORT_TCP;
	}
	else
	{
		r->low_transport = RTP_TRANSPORT_UDP;
	}
	
	r->stream_type = RTSP_STREAM_VIDEO | RTSP_STREAM_AUDIO;
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;
	
	r->auth = NULL;
	r->bLogin = true;
	if(user)
	{
		strcpy(r->user,user);
	}
	if(pwd)
	{
		strcpy(r->pwd,pwd);
	}
	
	http_get_url((char *)url,r->ip_me,&r->port,r->stream);
	
	return r;
}

static Rtsp_t* RTSP_CLIENT_init2(Rtsp_t *rtsp,int fd,char *user,char *pwd,int bInterleaved,int iBufferTime)
{
	Rtsp_t *r = NULL;
	if(rtsp == NULL)
	{
		r = (Rtsp_t *)malloc(sizeof(Rtsp_t));
		if(r == NULL)
		{
			VLOG(VLOG_ERROR,"malloc for rtsp failed");
			exit(0);
		}
	}
	else
	{
		r = rtsp;
	}
	//memset(r,0,sizeof(Rtsp_t));
	r->toggle = RTSPC_RUNNING;
	r->data_available = 0;
	r->role = RTSP_CLIENT;
	r->sock = fd;
	r->state = RTSP_STATE_INIT;
	r->cseq = 0;
	r->buffer_time = iBufferTime;
	r->b_interleavedMode = bInterleaved;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->blocal = false;
	if(bInterleaved == true)
	{
		r->low_transport = RTP_TRANSPORT_TCP;
	}
	else
	{
		r->low_transport = RTP_TRANSPORT_UDP;
	}
	SOCK_getpeername(fd,r->peername);
	r->stream_type = RTSP_STREAM_VIDEO | RTSP_STREAM_AUDIO;
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;
	
	r->auth = NULL;
	r->bLogin = true;
	if(user)
	{
		strcpy(r->user,user);
	}
	if(pwd)
	{
		strcpy(r->pwd,pwd);
	}
	
	return r;
}

int RTSP_destroy(Rtsp_t *r)
{
	//unsigned short port;
	//printf("rtsp destroy...\n");
	if(r->role == RTSP_CLIENT)
	{ 
		if((r->state == RTSP_STATE_PLAYING) || (r->state == RTSP_STATE_RECORDING))
		{
			RTSP_request_teardown(r);
		}
	}
	else
	{
		//RTSP_STREAM_destroy(&r->s);
		RTSP_session_del(r->peername,r->low_transport,r->stream);
		if(RTSP_session_entries() == 0)
		{
			RTSP_session_destroy();
		}
	}
	r->toggle = false;
	r->data_available = 0;
	r->state = RTSP_STATE_INIT;
	#if 0//csp modify 20140221
	SOCK_close(r->sock);
	r->sock = -1;
	#endif
	r->cseq = 0;
	if(r->sdp)
	{
		SDP_cleanup(r->sdp);
		r->sdp = NULL;
	}
	if(r->rtp_audio)
	{
		RTP_destroy(r->rtp_audio);
		r->rtp_audio = NULL;
	}
	if(r->rtp_video)
	{
		RTP_destroy(r->rtp_video);
		r->rtp_video = NULL;
	}
	if(r->rtcp_audio)
	{
		RTCP_destroy(r->rtcp_audio);
		r->rtcp_audio = NULL;
	}
	if(r->rtcp_video)
	{
		RTCP_destroy(r->rtcp_video);
		r->rtcp_video = NULL;
	}
	if(r->auth)
	{
		HTTP_AUTH_destroy(r->auth);
		r->auth = NULL;
	}
	#if 1//csp modify 20140221
	SOCK_close(r->sock);
	r->sock = -1;
	#endif
	r->bLogin = false;
	free(r);
	//printf("RTSP_destroy done\n");
	return TRUE;
}

int RTSP_cleanup(Rtsp_t *r)
{
	if(r->sock != (-1))
	{
		RTSP_request_teardown(r);
		SOCK_close(r->sock);
	}
	r->sock = -1;
	r->toggle = RTSPC_RUNNING;
	r->data_available = false;
	r->state = RTSP_STATE_INIT;
	r->sock = -1;
	r->cseq = 0;
	if(r->sdp)
	{
		SDP_cleanup(r->sdp);
		r->sdp = NULL;
	}
	if(r->rtp_audio)
	{
		RTP_destroy(r->rtp_audio);
		r->rtp_audio = NULL;
	}
	if(r->rtp_video)
	{
		RTP_destroy(r->rtp_video);
		r->rtp_video = NULL;
	}
	if(r->rtcp_audio)
	{
		RTCP_destroy(r->rtcp_audio);
		r->rtcp_audio = NULL;
	}
	if(r->rtcp_video)
	{
		RTCP_destroy(r->rtcp_video);
		r->rtcp_video = NULL;
	}
	if(r->auth)
	{
		HTTP_AUTH_destroy(r->auth);
		r->auth = NULL;
	}
	r->bLogin = false;
	r->blocal = false;
	return TRUE;
}

static int rtsp_handle_intreleaved(Rtsp_t *r, fRtpParsePacket frtp, fRtcpParsePacket frtcp)
{
	Rtp_t *rtp = NULL;
	Rtcp_t *rtcp = NULL;
	RtspInterHeader_t *interHeader = NULL;
	
	// check and parse interleaved packet
	interHeader = (RtspInterHeader_t *)r->payload;
	if(r->rtp_audio)
	{
		if(interHeader->channel == r->rtp_audio->peer_chn_port)
		{
			rtp = r->rtp_audio;
		}
	}
	if(r->rtp_video)
	{
		if(interHeader->channel == r->rtp_video->peer_chn_port)
		{
			rtp = r->rtp_video;
		}
	}
	if(r->rtcp_audio)
	{
		if(interHeader->channel == r->rtcp_audio->chn_port_c)
		{
			rtcp = r->rtcp_audio;
		}
	}
	if(r->rtcp_video)
	{
		if(interHeader->channel == r->rtcp_video->chn_port_c)
		{
			rtcp = r->rtcp_video;
		}
	}
	if(rtp)
	{
		if(frtp != NULL)
		{
			//printf("received RTP package size : %d\n",ntohs(interHeader->length));
			return frtp(rtp, NULL, ntohs(interHeader->length));
		}
		else
		{
			return RTSP_RET_OK;
		}
	}
	else if(rtcp)
	{
		if(frtcp != NULL)
		{
			//printf("received RTCP package size : %d\n",ntohs(interHeader->length));
			return frtcp(rtcp, NULL, ntohs(interHeader->length));
		}
		else
		{
			return RTSP_RET_OK;
		}
	}
	else
	{
		VLOG(VLOG_ERROR,"unknown packet ,check it.\n");
		//return RTSP_RET_FAIL;//lcy modify
		return RTSP_RET_OK;
	}
}

int RTSP_read_message(Rtsp_t *r)
{
	char *ptr = r->payload, *q = NULL;
	int ret, readed = 0, expected = 1;
	int content_len = 0, packet_size = -1;
	
	do
	{
		ret = recv(r->sock, ptr, expected, 0);
		if(ret < 0)
		{
			if(SOCK_ERR == SOCK_EINTR)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message: tcp recv error1 %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_EAGAIN)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message: tcp recv error2 %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_ETIMEOUT)
			{
				VLOG(VLOG_ERROR, "RTSP_read_message: tcp recv time out");
				return RTSP_RET_FAIL;
			}
			VLOG(VLOG_ERROR, "RTSP_read_message: tcp recv error3 @%d ###", SOCK_ERR);
			return RTSP_RET_FAIL;
		}
		else if(ret == 0)
		{
			VLOG(VLOG_CRIT, "network is shutdown by peer");
			return RTSP_RET_FAIL;
		}
		
		#if 1
		if(readed + ret >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
		ptr[ret] = 0;
		readed += ret;
		
		#if 0
		if(readed >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
		VLOG(VLOG_DEBUG, "ret:%d readed:%d expected:%d packet_size:%d content:%d", ret, readed, expected, packet_size, content_len);
		
		if(readed == packet_size)
		{
			break;
		}
		
		// check packet complete here ....
		if(r->payload[0] == RTSP_INTERLEAVED_MAGIC)
		{
			RtspInterHeader_t *interHeader = NULL;
			if(readed < sizeof(RtspInterHeader_t))
			{
				//not enouth 
				expected = sizeof(RtspInterHeader_t) - readed;
			}
			else
			{
				//char *pp = r->payload;
				//RTSP_LOG_HEX(pp,16);
				interHeader = (RtspInterHeader_t *)r->payload;
				packet_size = ntohs(interHeader->length) + sizeof(RtspInterHeader_t);
				if(readed < packet_size)
				{
					//not enought
					expected = packet_size - readed;
				}
				else if(readed == packet_size)
				{
					break;
				}
				else
				{
					VLOG(VLOG_ERROR,"occur unhandle error,please check it");
					return RTSP_RET_FAIL;
				}
			}
		}
		else
		{
			//rtsp message
			if(http_get_number(r->payload, "Content-length:", &content_len) == RTSP_RET_FAIL)
			{
				//without content
				if(strstr(ptr, "\r\n\r\n") != NULL)
				{
					//end of http response header
					break;
				}
				else
				{
					expected = RTSP_BUF_SIZE - readed;
				}
			}
			else
			{
				if((q = strstr(ptr,"\r\n\r\n")) != NULL)
				{
					//end of http response header
					packet_size = content_len + (q - r->payload) + 4;
					if(readed == packet_size)
					{
						break;
					}
					else if(readed < packet_size)
					{
						expected = packet_size - readed; 
					}
					else//csp modify
					{
						VLOG(VLOG_ERROR,"occur unhandle error,please check it");
						break;
					}
				}
				else
				{
					expected = RTSP_BUF_SIZE - readed; 
				}
			}
		}
		//
		ptr += ret;
	}while(expected > 0);//while(1);
	
	r->payload[readed] = 0;
	r->payload_size = readed;
	VLOG(VLOG_DEBUG,"read rtsp packet done,size:%d",readed);
	if(r->payload[0] != RTSP_INTERLEAVED_MAGIC)
	{
		VLOG(VLOG_DEBUG,"payload :\r\n:%s",r->payload);
	}
	
	return RTSP_RET_OK;
}

int RTSP_read_message4(Rtsp_t *r)
{
	char *ptr=r->payload,*q;
	int ret,readed=0,expected=3;
	int content_len=0,packet_size=-1;
	
	do
	{
		ret = recv(r->sock, ptr, expected, 0);
		if(ret < 0)
		{
			if(SOCK_ERR == SOCK_EINTR)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message4: tcp recv error %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_EAGAIN)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message4: tcp recv error %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_ETIMEOUT)
			{
				VLOG(VLOG_ERROR, "RTSP_read_message4: tcp recv time out");
				return RTSP_RET_FAIL;
			}
			VLOG(VLOG_ERROR, "RTSP_read_message4: tcp recv error @%d ###", SOCK_ERR);
			return RTSP_RET_FAIL;
		}
		else if(ret == 0)
		{
			VLOG(VLOG_CRIT, "network is shutdown by peer");
			return RTSP_RET_FAIL;
		}
		
		#if 1
		if(readed + ret >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
	    ptr[ret] = 0;
		readed += ret;
		
		#if 0
		if(readed >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
		VLOG(VLOG_DEBUG, "ret:%d readed:%d expected:%d packet_size:%d content:%d",ret, readed, expected, packet_size, content_len);
		
		if(r->payload[0] == RTSP_INTERLEAVED_MAGIC)
		{
			//RtspInterHeader_t *interHeader = NULL;
			if(readed < sizeof(RtspInterHeader_t))
			{
				expected = sizeof(RtspInterHeader_t) - readed;
			}
			else if(readed == sizeof(RtspInterHeader_t))
			{
				//printf("RTCP_HANDLE1 error 0\n");//lcy
				rtsp_handle_intreleaved(r, RTP_HANDLE, RTCP_HANDLE2);
				//printf("RTCP_HANDLE1 error 1\n");//lcy
				expected = 1;
				packet_size = -1;
				ptr = r->payload;
				readed = 0;
				continue;
			}	
		}
		else
		{
			if(http_get_number(r->payload, "Content-length:", &content_len) == RTSP_RET_FAIL)
			{//without content
				if(strstr(r->payload, "\r\n\r\n") != NULL)
				{//end of http response header
					printf("%s",ptr);
					break;
				}
				else
				{
					expected = 1;
				}
			}
			else
			{
				if((q = strstr(ptr,"\r\n\r\n")) != NULL)
				{//end of http response header
					packet_size = content_len + (q - r->payload) + 4;
					if(readed == packet_size)
					{
						break;
					}
					else
					{
						expected = packet_size - readed;
					}
				}
				else
				{
					expected = RTSP_BUF_SIZE - readed;
				}
			}
		}
		ptr += ret;//缓存指针后移
		*(ptr+1) = 0;
	}while(1);
	
	r->payload[readed] = 0;
	r->payload_size = readed;
	VLOG(VLOG_DEBUG,"read rtsp packet done,size:%d",readed);
	if(r->payload[0] != RTSP_INTERLEAVED_MAGIC)
	{
		VLOG(VLOG_DEBUG,"payload :\r\n:%s",r->payload);
	}
	
	return RTSP_RET_OK;
}

int RTSP_read_message5(Rtsp_t *r, fRtpParsePacket frtp, fRtcpParsePacket frtcp)
{
	char *ptr=r->payload;//,*q;
	int ret,readed=0,expected=4;
	int content_len=0,packet_size=-1;
	int MAGIC_flag=0;
	
	do
	{
		ret = recv(r->sock, ptr, expected, 0);
		if(ret < 0)
		{
			if(SOCK_ERR == SOCK_EINTR)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message5: tcp recv error %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_EAGAIN)
			{
				VLOG(VLOG_WARNING, "RTSP_read_message5: tcp recv error %d", SOCK_ERR);
				//csp modify 20140221
				//continue;
				return RTSP_RET_FAIL;
			}
			else if(SOCK_ERR == SOCK_ETIMEOUT)
			{
				VLOG(VLOG_ERROR, "RTSP_read_message5: tcp recv time out");
				return RTSP_RET_FAIL;
			}
			VLOG(VLOG_ERROR, "###RTSP_read_message5: tcp recv error @%d ###", SOCK_ERR);
			return RTSP_RET_FAIL;
		}
		else if(ret == 0)
		{
			VLOG(VLOG_CRIT, "network is shutdown by peer");
			return RTSP_RET_FAIL;
		}
		
		#if 1
		if(readed + ret >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
		ptr[ret] = 0;
		ptr += ret;
		readed += ret;
		
		#if 0
		if(readed >= RTSP_BUF_SIZE)
		{
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d", readed);
			return RTSP_RET_FAIL;
		}
		#endif
		
		VLOG(VLOG_DEBUG, "ret:%d readed:%d expected:%d packet_size:%d content:%d", ret, readed, expected, packet_size, content_len);
		
		if(readed == packet_size)
		{
			break;
		}
		
		// check packet complete here ...
		if(r->payload[0] == RTSP_INTERLEAVED_MAGIC)
		{
			if(MAGIC_flag == 0)
			{
				//RtspInterHeader_t *interHeader = NULL;
				if(readed < sizeof(RtspInterHeader_t))
				{//not enouth 
					expected = sizeof(RtspInterHeader_t) - readed;
				}
				else
				{
					//printf("0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",(unsigned char)r->payload[0],(unsigned char)r->payload[1],(unsigned char)r->payload[2],(unsigned char)r->payload[3]);
					return rtsp_handle_intreleaved(r, frtp, frtcp);
				}
			}
			else
			{
				MAGIC_flag++;
				if(MAGIC_flag == 3)
				{
					if((r->payload[1]!=0x00) && (r->payload[1]!=0x01))
					{
						//printf("Saw nonexistent stream channel id: 0x%02x\n",(unsigned char)r->payload[1]);
						MAGIC_flag = 1;
						ptr = r->payload;
						readed = 0;
						expected = 1;
						continue;
					}
					else
					{
						//csp modify 20140221
						//ret = recv(r->sock, ptr, 2, 0);
						ret = SOCK_recv2(r->sock, ptr, 2, 0);
						if(ret != 2)
						{
							VLOG(VLOG_CRIT, "network recv error");
							return RTSP_RET_FAIL;
						}
						//printf("0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",(unsigned char)r->payload[0],(unsigned char)r->payload[1],(unsigned char)r->payload[2],(unsigned char)r->payload[3]);
						return rtsp_handle_intreleaved(r, frtp, frtcp);
					}
				}
			}
		}
		else
		{//rtsp message 
			//printf("no '$' header\n");
			MAGIC_flag = 1;//第一次 进入函数没有找到 $       
			ptr = r->payload;
			readed = 0;
			expected = 1;
			continue;
		}		
	}while(1);
	
	r->payload[readed] = 0;
	r->payload_size = readed;
	VLOG(VLOG_DEBUG,"read rtsp packet done,size:%d",readed);
	if(r->payload[0] != RTSP_INTERLEAVED_MAGIC)
	{
		VLOG(VLOG_DEBUG,"payload :\r\n:%s",r->payload);
	}
	
	return RTSP_RET_OK;
}

#if 0
Rtsp_t* RTSP_connect_server(Rtsp_t *rtsp,char *url,char *user,char *pwd,int bInterleaved,int iBufferTime)
{
	Rtsp_t *r = NULL;
	char sockname[20];
	char ip_dst[20],stream[128];
	int port;
	SOCK_t sock;
	int i;
	Attribute_t attr;
	int iSetupMedia = 0;
	//char rtpmap[32];
	
	if(http_get_url(url,ip_dst,&port,stream)== RTSP_RET_FAIL)
	{
		return NULL;
	}
	
	sock = SOCK_tcp_connect2(ip_dst,port,RTSP_SOCK_TIMEOUT);
	if(sock == -1)
	{
		return NULL;
	}
	
	// judge the socket buffer if necessary
	SOCK_getsockname(sock,sockname);
#ifdef RTSP_ENABLE_LAN_CHECK
	if(SOCK_isreservedip(sockname) != true || SOCK_isreservedip(ip_dst) != true)
	{
		if(iBufferTime < RTSP_PLAYER_BUFFER_TIME)
		{
			iBufferTime = RTSP_PLAYER_BUFFER_TIME;
		}
		
		#ifdef RTSP_FORCE_RTP_OVER_RTSP
		bInterleaved = true;
		#endif
	}
	else
	{
		iBufferTime = 1000;
	}
#endif
	
	//
	r = RTSP_CLIENT_init2(rtsp,sock,user,pwd,bInterleaved,iBufferTime);
	if(r == NULL)
	{
		return NULL;
	}
	r->port = port;
	strcpy(r->ip_me,ip_dst);
	strcpy(r->stream,stream);
	//
	
	if(RTSP_request_options(r) == RTSP_RET_FAIL)
	{
		goto CONNECT_ERR_EXIT;
	}
	if(RTSP_request_describe(r) == RTSP_RET_FAIL)
	{
		goto CONNECT_ERR_EXIT;
	}
	for(i=0;i<r->sdp->media_num;i++)
	{
		if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_FAIL)
		{
			goto CONNECT_ERR_EXIT;
		}
		if((strncmp(attr.rtpmap.codec_type,"H264",strlen("H264")) == 0) ||
			//(strncmp(attr.rtpmap.codec_type,"PCMA",strlen("PCMA")) == 0) ||
			//(strncmp(attr.rtpmap.codec_type,"PCMU",strlen("PCMU")) == 0) ||
			0)
		{
			if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,SDP_ATTR_CONTROL,(void *)&attr) == RTSP_RET_FAIL)
			{
				goto CONNECT_ERR_EXIT;
			}
			if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.format) == RTSP_RET_FAIL)
			{
				goto CONNECT_ERR_EXIT;
			}
			iSetupMedia++;
		}
	}
	if(iSetupMedia == 0)
	{
		MSLEEP(2000); // I CAN'T FIND ANY MEDIA which is usable for me; 
		goto CONNECT_ERR_EXIT;
	}
	if(RTSP_request_play(r) == RTSP_RET_FAIL)
	{
		goto CONNECT_ERR_EXIT;
	}
	return r;
CONNECT_ERR_EXIT:
	RTSP_destroy(r);
	return NULL;
}
#endif

int RTSP_connect_server2(Rtsp_t *rtsp,char *url,char *user,char *pwd,int bInterleaved,int iBufferTime)
{
	Rtsp_t *r = NULL;
	char sockname[20];
	char ip_dst[20],stream[128];
	int port;
	SOCK_t sock;
	int i;
	Attribute_t attr;
	int iSetupMedia = 0;
	//char rtpmap[32];
	
	//printf("RTSP_connect_server2: http_get_url...\n");
	//fflush(stdout);
	if(http_get_url(url,ip_dst,&port,stream) == RTSP_RET_FAIL)
	{
		printf("http_get_url ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	//printf("RTSP_connect_server2: http_get_url success\n");
	//fflush(stdout);
	
	sock = SOCK_tcp_connect2(ip_dst,port,RTSP_SOCK_TIMEOUT);
	if(sock == -1)
	{
		printf("SOCK_tcp_connect2 ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	
	// judge the socket buffer if necessary
	SOCK_getsockname(sock,sockname);
#ifdef RTSP_ENABLE_LAN_CHECK
	if(SOCK_isreservedip(sockname) != true || SOCK_isreservedip(ip_dst) != true)
	{
		if(iBufferTime < RTSP_PLAYER_BUFFER_TIME)
		{
			iBufferTime = RTSP_PLAYER_BUFFER_TIME;
		}
		
		#ifdef RTSP_FORCE_RTP_OVER_RTSP
		bInterleaved = true;
		#endif
	}
	else
	{
		iBufferTime = 1000;
	}
#endif
	
	//
	//printf("RTSP_connect_server2: RTSP_CLIENT_init2 ip_dst = %s\n",ip_dst);
	//fflush(stdout);
	r = RTSP_CLIENT_init2(rtsp,sock,user,pwd,bInterleaved,iBufferTime);
	r->port = port;
	strcpy(r->ip_me,ip_dst);
	strcpy(r->stream,stream);
	//
	
	#if 0//csp modify 20140221
	//printf("RTSP_connect_server2: RTSP_request_options...\n");
	//fflush(stdout);
	if(RTSP_request_options(r) == RTSP_RET_FAIL)
	{
		printf("RTSP_request_options ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	#endif
	
	//printf("RTSP_connect_server2: RTSP_request_describe...\n");
	//fflush(stdout);
	if(RTSP_request_describe(r) == RTSP_RET_FAIL)
	{
		printf("RTSP_request_describe ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	
	//printf("RTSP_connect_server2: RTSP_request_describe success###\n");
	//fflush(stdout);
	
	for(i=0;i<r->sdp->media_num;i++)
	{
		//printf("RTSP_connect_server2: SDP_get_media_attr-1...\n");
		//fflush(stdout);
		if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_FAIL)
		{
			printf("SDP_get_media_attr [SDP_ATTR_RTP_MAP] ERROR\n");
			goto CONNECT_ERR_EXIT;
		}
		//printf("RTSP_connect_server2: SDP_get_media_attr-2...\n");
		//fflush(stdout);
		if((strncmp(attr.rtpmap.codec_type,"H264",strlen("H264")) == 0) ||
			//(strncmp(attr.rtpmap.codec_type,"PCMA",strlen("PCMA")) == 0) ||
			//(strncmp(attr.rtpmap.codec_type,"PCMU",strlen("PCMU")) == 0) ||
			0)
		{
			//printf("RTSP_connect_server2: SDP_get_media_attr-3...\n");
			//fflush(stdout);
			if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,SDP_ATTR_CONTROL,(void *)&attr) == RTSP_RET_FAIL)
			{
				printf("SDP_get_media_attr [SDP_ATTR_CONTROL] ERROR\n");
				goto CONNECT_ERR_EXIT;
			}
			//printf("RTSP_connect_server2: SDP_get_media_attr-4...\n");
			//fflush(stdout);
			if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.format) == RTSP_RET_FAIL)
			{
				//printf("RTSP_request_setup ERROR\n");
				goto CONNECT_ERR_EXIT;
			}
			iSetupMedia++;
			//printf("RTSP_connect_server2: RTSP_request_setup success\n");
			//fflush(stdout);
		}
	}
	
	//printf("RTSP_connect_server2: loop finish\n");
	//fflush(stdout);
	
	if(iSetupMedia == 0)
	{
		printf("iSetupMedia ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	
	//printf("RTSP_connect_server2: RTSP_request_play...\n");
	//fflush(stdout);
	if(RTSP_request_play(r) == RTSP_RET_FAIL)
	{
		printf("RTSP_request_play ERROR\n");
		goto CONNECT_ERR_EXIT;
	}
	
	//printf("RTSP_connect_server2 success\n");
	//fflush(stdout);
	
	return RTSP_RET_OK;
	
CONNECT_ERR_EXIT:
	MSLEEP(2000);
	return RTSP_RET_FAIL;
}

/**********************************************************************
* interface for rtsp stream table *
***********************************************************************/
int RTSP_find_stream(const char *stream_name,char *media_name)
{
	int i,flag=false;
	for(i=0;i<g_RtspStreamTable.entries;i++)
	{
		if(strcmp(g_RtspStreamTable.stream[i],stream_name) == 0)
		{
			flag=true;
			break;
		}
	}
	if(flag == true)
	{
		if(media_name)
		{
			strcpy(media_name,g_RtspStreamTable.media[i]);
		}
		return RTSP_RET_OK;
	}
	VLOG(VLOG_ERROR,"invalid stream: %s",stream_name);
	return RTSP_RET_FAIL;
}

int RTSP_add_stream(const char *stream_name,const char *media_name)
{
	if((strlen(stream_name) > RTSP_MAX_STREAM_LEN) || (strlen(media_name) > RTSP_MAX_STREAM_LEN))
	{
		VLOG(VLOG_ERROR,"strlen(stream-name) exceed the buffer size");
		return RTSP_RET_FAIL;
	}
	if(g_RtspStreamTable.entries >=RTSP_MAX_STREAM)
	{
		VLOG(VLOG_ERROR,"the entries of streams exceed the buffer size");
		return RTSP_RET_FAIL;
	}
	strcpy(g_RtspStreamTable.stream[g_RtspStreamTable.entries],stream_name);
	strcpy(g_RtspStreamTable.media[g_RtspStreamTable.entries],media_name);
	g_RtspStreamTable.entries++;
	return RTSP_RET_OK;
}

int RTSP_remove_stream(const char *stream_name)
{
	int i,flag=false;
	for(i=0;i<g_RtspStreamTable.entries;i++)
	{
		if(strcmp(g_RtspStreamTable.stream[i],stream_name) == 0)
		{
			flag=true;
		}
	}
	if(flag == true)
	{
		for(;i<g_RtspStreamTable.entries;i++)
		{
			strcpy(g_RtspStreamTable.stream[i],g_RtspStreamTable.stream[i+1]);
			strcpy(g_RtspStreamTable.media[i],g_RtspStreamTable.media[i+1]);
		}
		g_RtspStreamTable.entries--;
	}
	return RTSP_RET_OK;
}

