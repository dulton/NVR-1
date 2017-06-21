/******************************************************************************

  Copyright (C), 2013-2020, Juan Optical & Electronical Tech Co., Ltd. All Rights Reserved.

 ******************************************************************************
  File Name    : sdplib.c
  Version       : Initial Draft
  Author        : (kejiazhw@gmail.com)kaga
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : session description protocal  utils , reference to rfc4566(sdp)
  	rfc5391(rtp payload format for g711)
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_base64.h"
#include "sdplib.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static const char *sdpSpecs[SDP_SPEC_CNT]=
{	
	"v=",
	"o=",
	"s=",
	"i=",
	"u=",
	"e=",
	"p=",
	"c=",
	"b=",
	"t=",
	"r=",
	"z=",
	"k=",
	"a=",
	"m=",
	"y=",
};
static const char *const sdpAttrs[]=
{
	"cat",
	"keywds",
	"tool",
	"ptime",
	"maxptime",
	"rtpmap",
	"recvonly",
	"sendrecv",
	"sendonly",
	"inactive",
	"orient",
	"type",
	"charset",
	"sdplang",
	"lang",
	"framerate",
	"quality",
	"fmtp",
	"control",
	"range",
	"etag",
	"framesize", // [playload type] %d-%d
	""	
};

static inline void sdp_add_line(const char *key,char *value,char **dest)
{
	sprintf(*dest,"%s%s\r\n",key,value);
	*dest += strlen(*dest);
}

static inline void sdp_add_attr_line(char *name,char *value,char **dest)
{
	sprintf(*dest,"%s%s:%s\r\n",sdpSpecs[SDP_SPEC_ATTRIBUTE],name,value);
	*dest += strlen(*dest);
}

static inline int sdp_read_line(char *src,char *dst)
{
	int specode = -1;
	switch(*src)
	{
		case 'v':
			specode = SDP_SPEC_VERSION;
			break;
		case 'o':
			specode = SDP_SPEC_ORIGIN;
			break;
		case 's':
			specode = SDP_SPEC_SESSION_NAME;
			break;
		case 'i':
			specode = SDP_SPEC_SESSION_INFO;
			break;
		case 'u':
			specode = SDP_SPEC_URI;
			break;
		case 'e':
			specode = SDP_SPEC_EMAIL;
			break;
		case 'p':
			specode = SDP_SPEC_PHONE;
			break;
		case 'c':
			specode = SDP_SPEC_CONNECTION;
			break;
		case 'b':
			specode = SDP_SPEC_BANDWIDTH;
			break;
		case 't':
			specode = SDP_SPEC_TIMING;
			break;
		case 'r':
			specode = SDP_SPEC_REPEAT_TIME;
			break;
		case 'z':
			specode = SDP_SPEC_TIME_ZONE;
			break;
		case 'k':
			specode = SDP_SPEC_ENCRYPT_KEY;
			break;
		case 'a':
			specode = SDP_SPEC_ATTRIBUTE;
			break;
		case 'm':
			specode = SDP_SPEC_MEDIA_DESC;
			break;
		case 'y':
			specode = SDP_SPEC_SSRC;
			break;
		default:
			if(*src == '\r')
			{
				//printf("SDP-ERROR: unexpect type: %d\n",src[0]);
			}
			else
			{
				printf("SDP-ERROR: unexpect type: %d\n",src[0]);
			}
			return -1;
	}
	sscanf(src+2,"%[^\r\n]",dst);
	return specode;
}

static inline char *sdp_next_line(char *src)
{
	while(*src)
	{
		if(*src == '\n')
			break;
		src++;
	}
	if(*src)
	{
		src++;
		return src; 
	}
	else
	{
		return NULL;
	}
}

static int sdp_add(SessionDesc_t *sdp,int type,char **dest,int ext_arg)
{
	char tmp[512];
	switch(type){
		case SDP_SPEC_VERSION:
			sdp_add_line(sdpSpecs[SDP_SPEC_VERSION],sdp->version,dest);
			break;
		case SDP_SPEC_ORIGIN:
			sprintf(tmp,"%s %d %d %s %s %s",sdp->originer.user_name,sdp->originer.session_id,
				sdp->originer.session_ver,sdp->originer.nettype,sdp->originer.addrtype,
				sdp->originer.addr);
			sdp_add_line(sdpSpecs[SDP_SPEC_ORIGIN],tmp,dest);
			break;
		case SDP_SPEC_SESSION_NAME:
			sdp_add_line(sdpSpecs[SDP_SPEC_SESSION_NAME],sdp->session_name,dest);
			break;
		case SDP_SPEC_SESSION_INFO:
			sdp_add_line(sdpSpecs[SDP_SPEC_SESSION_INFO],sdp->session_info,dest);
			break;
		case SDP_SPEC_URI:
			sdp_add_line(sdpSpecs[SDP_SPEC_URI],sdp->uri,dest);
			break;
		case SDP_SPEC_EMAIL:			
			sdp_add_line(sdpSpecs[SDP_SPEC_EMAIL],sdp->email,dest);
			break;
		case SDP_SPEC_PHONE:
			sdp_add_line(sdpSpecs[SDP_SPEC_PHONE],sdp->phone,dest);
			break;
		case SDP_SPEC_CONNECTION:
			if(ext_arg < 0){
				sprintf(tmp,"%s %s %s",sdp->conn_info.nettype,sdp->conn_info.addrtype,
					sdp->conn_info.addr);
			}else{
				sprintf(tmp,"%s %s %s",sdp->media[ext_arg].conn_info.nettype,
					sdp->media[ext_arg].conn_info.addrtype,
					sdp->media[ext_arg].conn_info.addr);
			}
			sdp_add_line(sdpSpecs[SDP_SPEC_CONNECTION],tmp,dest);
			break;
		case SDP_SPEC_BANDWIDTH:
			sdp_add_line(sdpSpecs[SDP_SPEC_BANDWIDTH],sdp->bandwidth,dest);
			break;
		case SDP_SPEC_TIMING:
			sprintf(tmp,"%d %d",sdp->time_active.start,sdp->time_active.end);
			sdp_add_line(sdpSpecs[SDP_SPEC_TIMING],tmp,dest);
			break;
		case SDP_SPEC_SSRC:
			sprintf(tmp,"%u",sdp->media[ext_arg].ssrc);
			sdp_add_line(sdpSpecs[SDP_SPEC_SSRC],tmp,dest);
			break;
		//case SDP_SPEC_REPEAT_TIME:
		//	break;
		//case SDP_SPEC_TIME_ZONE:break;
		case SDP_SPEC_ENCRYPT_KEY:break;
		//case SDP_SPEC_ATTRIBUTE:break;
		case SDP_SPEC_MEDIA_DESC:
			sprintf(tmp,"%s %d %s %d",sdp->media[ext_arg].media_n.type,
				sdp->media[ext_arg].media_n.port,
				sdp->media[ext_arg].media_n.protocal,
				sdp->media[ext_arg].media_n.format);
			sdp_add_line(sdpSpecs[SDP_SPEC_MEDIA_DESC],tmp,dest);
			break;
		default:
			break;
	}

	return 0;
}

SessionDesc_t *SDP_new_default(char *session_name,char *ip)
{
	SessionDesc_t *sdp=NULL;
	sdp = (SessionDesc_t *)malloc(sizeof(SessionDesc_t));
	memset(sdp,0,sizeof(SessionDesc_t));
	if(sdp == NULL){
		printf("SDP-ERROR: malloc for sdp failed\r\n");
		return NULL;
	}
	sdp->buffer = (char *)calloc(1,SDP_BUFFER_SIZE);
	if(sdp->buffer == NULL){
		printf("SDP-ERROR: malloc for sdp buffer failed\r\n");
		free(sdp);
		return NULL;
	}
	sdp->spec_flag |= (1 << SDP_SPEC_VERSION);
	strcpy(sdp->version,SDP_VERSION);
	sdp->spec_flag |= (1 << SDP_SPEC_ORIGIN);
	strcpy(sdp->originer.user_name,SDP_NOTSUPPORT_USERID);
	sdp->originer.session_id = rand();
	sdp->originer.session_ver = SDP_DEFAULT_SESSION_VER;
	strcpy(sdp->originer.nettype,SDP_NETTYPE_INTERNET);
	strcpy(sdp->originer.addrtype,SDP_ADDR_TYPE_IPV4);
	strcpy(sdp->originer.addr,ip);
	sdp->spec_flag |= (1 << SDP_SPEC_SESSION_NAME);
	strcpy(sdp->session_name,session_name);	
	sdp->spec_flag |= (1 << SDP_SPEC_SESSION_INFO);
	strcpy(sdp->session_info,SDP_DEFAULT_SESSION_INFO);
	sdp->spec_flag |= (1 << SDP_SPEC_CONNECTION);
	strcpy(sdp->conn_info.nettype,SDP_NETTYPE_INTERNET);
	strcpy(sdp->conn_info.addrtype,SDP_ADDR_TYPE_IPV4);
	strcpy(sdp->conn_info.addr,ip);
	sdp->spec_flag |= (1 << SDP_SPEC_TIMING);
	sdp->time_active.start = 0;
	sdp->time_active.end = 0;
	// session attributions
	sdp->attr_num = 0;
	sdp->spec_flag |= (1 << SDP_SPEC_ATTRIBUTE);
	strcpy(sdp->attri[sdp->attr_num].name,sdpAttrs[SDP_ATTR_CONTROL]);
	sprintf(sdp->attri[sdp->attr_num].value,"*");
	sdp->attr_num++;
	
	// media descriptions
	sdp->media_num = 0;

	return sdp;
}

int SDP_add_h264(SessionDesc_t *sdp,char *control)
{
	//int ret;
	MediaDesc_t *media= NULL;
	// 720p
	//unsigned char sps[10]={0x67,0x42,0x00,0x1f,0x95,0xa8,0x14,0x01,0x6e,0x40};//baseline
	//unsigned char pps[4]={0x68,0xce,0x3c,0x80};//baseline
	// 360p
	//unsigned char sps[10]={0x67,0x42,0x00,0x1e,0x95,0xa8,0x28,0x0b,0xfe,0x54};//baseline
	//unsigned char pps[4]={0x68,0xce,0x3c,0x80};//baseline
	//unsigned char sps[9]={0x67,0x4d,0x00,0x1e,0x95,0xb8,0x2c,0x04,0x99};//main
	//unsigned char pps[4]={0x68,0xee,0x3c,0x80};//main
	
	//unsigned char sps_e[64],pps_e[64];
	media=(MediaDesc_t *)&sdp->media[sdp->media_num];
	sdp->spec_flag |= (1 << SDP_SPEC_MEDIA_DESC);
	strcpy(media->media_n.type,SDP_MEDIA_TYPE_VIDEO);
	media->media_n.port = 0;
	strcpy(media->media_n.protocal,SDP_MEDIA_PROTOCAL_RTP_AVP);
	media->media_n.format = SDP_PAYLOAD_TYPE_DYNAMIC;
	//media->spec_flag |= (1 << SDP_SPEC_CONNECTION);
	//strcpy(media->conn_info.nettype,SDP_NETTYPE_INTERNET);
	//strcpy(media->conn_info.addrtype,SDP_ADDR_TYPE_IPV4);
	//strcpy(media->conn_info.addr,"0.0.0.0");

	// attributions
	media->spec_flag |= (1 << SDP_SPEC_ATTRIBUTE);
	strcpy(media->attri[media->attr_num].name,sdpAttrs[SDP_ATTR_RTP_MAP]);
	sprintf(media->attri[media->attr_num].value,"%d H264/%d",
		SDP_PAYLOAD_TYPE_DYNAMIC,SDP_MEDIA_H264_FREQ);
	media->attr_num++;
	strcpy(media->attri[media->attr_num].name,sdpAttrs[SDP_ATTR_CONTROL]);
	sprintf(media->attri[media->attr_num].value,"%s",control);
	media->attr_num++;
#if false
	ret=BASE64_encode(sps,sizeof(sps),sps_e,sizeof(sps_e));
	sps_e[ret]=0;
	ret=BASE64_encode(pps,sizeof(pps),pps_e,sizeof(pps_e));
	pps_e[ret]=0;
	strcpy(media->attri[media->attr_num].name,sdpAttrs[SDP_ATTR_FMTP]);
	sprintf(media->attri[media->attr_num].value,"96 profile-level-id=%02x%02x%02x; packetization-mode=1; sprop-parameter-sets=%s,%s",
		sps[1],sps[2],sps[3],sps_e,pps_e);
	media->attr_num++;
#endif

	sdp->media_num++;
	return 0;
}

int SDP_add_g711(SessionDesc_t *sdp,char *control)
{
	MediaDesc_t *media= NULL;
	media=(MediaDesc_t *)&sdp->media[sdp->media_num];
	sdp->spec_flag |= (1 << SDP_SPEC_MEDIA_DESC);
	strcpy(media->media_n.type,SDP_MEDIA_TYPE_AUDIO);
	media->media_n.port = 0;//random port by peer
	strcpy(media->media_n.protocal,SDP_MEDIA_PROTOCAL_RTP_AVP);
	media->media_n.format = SDP_PAYLOAD_TYPE_ALAW;
	media->spec_flag |= (1 << SDP_SPEC_CONNECTION);
	strcpy(media->conn_info.nettype,SDP_NETTYPE_INTERNET);
	strcpy(media->conn_info.addrtype,SDP_ADDR_TYPE_IPV4);
	strcpy(media->conn_info.addr,"0.0.0.0");

	// attributions
	media->spec_flag |= (1 << SDP_SPEC_ATTRIBUTE);
	strcpy(media->attri[media->attr_num].name,sdpAttrs[SDP_ATTR_RTP_MAP]);
	sprintf(media->attri[media->attr_num].value,"%d PCMA/%d",
		SDP_PAYLOAD_TYPE_ALAW,SDP_MEDIA_G711_FREQ);
	media->attr_num++;
	strcpy(media->attri[media->attr_num].name,sdpAttrs[SDP_ATTR_CONTROL]);
	sprintf(media->attri[media->attr_num].value,"%s",control);
	media->attr_num++;
	
	sdp->media_num++;
	return 0;
}

int SDP_encode(SessionDesc_t *sdp)
{
	int i,j,n;
	char *ptr=sdp->buffer;
	if(sdp == NULL || ptr == NULL){
		printf("SDP-ERROR: sdp or buffer is null\r\n");
		return -1;
	}
	//
	for(i=0;i<SDP_SPEC_MEDIA_DESC;i++){
		if(i == SDP_SPEC_ATTRIBUTE){
			for(j=0;j<sdp->attr_num;j++){
				sdp_add_attr_line(sdp->attri[j].name,
					sdp->attri[j].value,&ptr);
			}
		}else{
			if(sdp->spec_flag & (1<<i)){
				sdp_add(sdp,i,&ptr,-1);
			}
		}
	}
	if(sdp->spec_flag & (1 << SDP_SPEC_MEDIA_DESC)){
		for(n=0;n<sdp->media_num;n++){
			sdp_add(sdp,SDP_SPEC_MEDIA_DESC,&ptr,n);
			for(i=0;i<SDP_SPEC_MEDIA_DESC;i++){
				if(i == SDP_SPEC_ATTRIBUTE){
					for(j=0;j<sdp->media[n].attr_num;j++){
						sdp_add_attr_line(sdp->media[n].attri[j].name,
							sdp->media[n].attri[j].value,&ptr);
					}
				}else{
					if(sdp->media[n].spec_flag & (1<<i)){
						sdp_add(sdp,i,&ptr,n);
					}
				}
			}
		}
	}
	printf("SDP-INFO: sdp setup output:\r\n:%s\r\n",sdp->buffer);
	return 0;
}

int sdp_decode_attr(Attribute_t *attr)
{
	int i=0;
	int ret;
	char tmp[SDP_MAX_ATTR_VALUE_SIZE];
	char tmp2[512],tmp3[512];
	char *ptr=tmp,*q;
	unsigned int itmp1,itmp2;
	//int len=strlen(attr->value);
	
	strcpy(tmp,attr->value);
	
	if(strcmp(attr->name,sdpAttrs[SDP_ATTR_FMTP])==0)
	{
		memset(attr->value,0,SDP_MAX_ATTR_VALUE_SIZE);
		//csp modify
		//sscanf(ptr,"fmtp:%d",&attr->fmtp.type);
		sscanf(ptr,"fmtp:%hd",&attr->fmtp.type);
		if((q=strstr(ptr,"profile-level-id"))!=NULL)
		{
			q+=strlen("profile-level-id=");
			sscanf(q,"%06x",(int *)(&attr->fmtp.profile_levle_id));
		}
		if((q=strstr(ptr,"sprop-parameter-sets"))!=NULL)
		{
			q+=strlen("sprop-parameter-sets=");
			while((*q) && ((*q) != ';'))
			{
				//printf("q:%s\n",q);
				i = 0;
				//printf("q:%s\n",q);
				while((*q) && ((*q) != ',') && ((*q) != ';'))
				{
					tmp2[i++]=*q++;
				}
				tmp2[i]=0;
				//printf("tmp2:%s \n",tmp2);
				ret=BASE64_decode(tmp2,strlen(tmp2),tmp3,sizeof(tmp3));
				if(ret > 0)
				{
					if((tmp3[0] & 0x1f) == 7)
					{//sps
						attr->fmtp.sps[0] = attr->fmtp.sps[1]=attr->fmtp.sps[2]=0;
						attr->fmtp.sps[3] = 1;
						memcpy(&attr->fmtp.sps[4],tmp3,ret);
						attr->fmtp.sps_size = ret+4;
					}
					else if((tmp3[0] & 0x1f) == 8)
					{//pps
						attr->fmtp.pps[0] = attr->fmtp.pps[1]=attr->fmtp.pps[2]=0;
						attr->fmtp.pps[3] = 1;
						memcpy(&attr->fmtp.pps[4],tmp3,ret);
						attr->fmtp.pps_size = ret+4;
					}
				}
				q++;
			}
		}
		if((q=strstr(ptr,"packetization-mode"))!=NULL)
		{
			q+=strlen("sprop-parameter-sets=");
			//csp modify
			//sscanf(q,"%d",&attr->fmtp.packet_mode);
			sscanf(q,"%hd",&attr->fmtp.packet_mode);
		}
		//printf("SDP-INFO: fmtp:%d packet-mode:%d\r\n",attr->fmtp.type,attr->fmtp.packet_mode);
		//VLOG_Hex(VLOG_DEBUG,attr->fmtp.profile_levle_id,4);
		//VLOG_Hex(VLOG_DEBUG,attr->fmtp.sps,attr->fmtp.sps_size);
		//VLOG_Hex(VLOG_DEBUG,attr->fmtp.pps,attr->fmtp.pps_size);
	}
	else if(strcmp(attr->name,sdpAttrs[SDP_ATTR_RTP_MAP])==0)
	{
		//rtpmap:96 H264/90000
		sscanf(attr->value,"%d %[^/]/%u",&itmp1,tmp2,&itmp2);
		attr->rtpmap.payload_type = itmp1;
		attr->rtpmap.freq = itmp2;
		strcpy(attr->rtpmap.codec_type,tmp2);
	}
	
	return 0;
}

SessionDesc_t* SDP_decode(char *src)
{
	int media_flag=FALSE;
	char tmp[1024],*psrc=src,*q;//char tmp[256],*psrc=src,*q;//csp modify 20140221
	MediaDesc_t *media=NULL;
	SessionDesc_t *sdp=NULL;
	int descode;
	
	if(src[0] != 'v' || src[1] != '=')
	{
		printf("SDP-ERROR: invalid sdp format\r\n");
		return NULL;
	}
	
	sdp = (SessionDesc_t *)calloc(1,sizeof(SessionDesc_t));
	if(sdp == NULL)
	{
		printf("SDP-ERROR: malloc for sdp failed\r\n");
		return NULL;
	}
	q = psrc;
	sdp->attr_num = 0;
	
	do
	{
		if(*q == 0)
		{
			//printf("haha+++\n");
			break;
		}
		memset(tmp,0,sizeof(tmp));//csp modify 20140221
		descode = sdp_read_line(q,tmp);
		if(descode < 0)
		{
			//return NULL;
		}
		//printf("SDP-DEBUG: descode:%d = %s\n",descode,tmp);
		switch(descode)
		{
			case SDP_SPEC_VERSION:
				strcpy(sdp->version,tmp);
				break;
			case SDP_SPEC_ORIGIN:
				 //<username> <session id> <session version> <net type> <address type> <unicast-address>
				 sscanf(tmp,"%s %d %d %s %s %s",sdp->originer.user_name,&sdp->originer.session_id,
				 	&sdp->originer.session_ver,sdp->originer.nettype,sdp->originer.addrtype,
				 	sdp->originer.addr);
				 break;
			case SDP_SPEC_SESSION_NAME:
				strcpy(sdp->session_name,tmp);
				break;
			case SDP_SPEC_SESSION_INFO:
				if(media_flag)
					strcpy(sdp->media[sdp->media_num-1].media_info,tmp);
				else
					strcpy(sdp->session_info,tmp);
				break;
			case SDP_SPEC_URI:
				strcpy(sdp->uri,tmp);
				break;
			case SDP_SPEC_EMAIL:
				strcpy(sdp->email,tmp);
				break;
			case SDP_SPEC_PHONE:
				strcpy(sdp->phone,tmp);
				break;
			case SDP_SPEC_CONNECTION:
				// c=* <nettype> <addrtype> <connect-address>
				if(media_flag)
					sscanf(tmp,"%s %s %s",sdp->media[sdp->media_num-1].conn_info.nettype,
						sdp->media[sdp->media_num-1].conn_info.addrtype,
					   	sdp->media[sdp->media_num-1].conn_info.addr);
				else
					sscanf(tmp,"%s %s %s",sdp->conn_info.nettype,sdp->conn_info.addrtype,
				   		sdp->conn_info.addr);
				break;
			case SDP_SPEC_BANDWIDTH:
				strcpy(sdp->bandwidth,tmp);
				break;
			case SDP_SPEC_TIMING:
				sscanf(tmp,"%d %d",&sdp->time_active.start,&sdp->time_active.end);
				break;
			case SDP_SPEC_REPEAT_TIME:
				strcpy(sdp->repeat_time,tmp);
				break;
			case SDP_SPEC_TIME_ZONE:
				strcpy(sdp->zone,tmp);
				break;
			case SDP_SPEC_ENCRYPT_KEY:
				if(media_flag)
					strcpy(sdp->media[sdp->media_num-1].encryt_key,tmp);
				else
					strcpy(sdp->encrypt_key.key,tmp);
				break;
			case SDP_SPEC_ATTRIBUTE:
				if(media_flag)
				{
					media = (MediaDesc_t *)(&sdp->media[sdp->media_num-1]);
					sscanf(tmp,"%[^:\r\n]:%[^\r\n]",
						media->attri[media->attr_num].name,
						media->attri[media->attr_num].value);
					sdp_decode_attr(&media->attri[media->attr_num]);
					media->attr_num++;
				}
				else
				{
					sscanf(tmp,"%[^:\r\n]:%[^\r\n]",sdp->attri[sdp->attr_num].name,
						sdp->attri[sdp->attr_num].value);
					sdp_decode_attr(&sdp->attri[sdp->attr_num]);
					sdp->attr_num++;
				}
				break;
			case SDP_SPEC_SSRC:
				if(media_flag)
				{
					media = (MediaDesc_t *)(&sdp->media[sdp->media_num-1]);
					sscanf(tmp,"%u",&media->ssrc);
				}
				break;
			case SDP_SPEC_MEDIA_DESC:
				media = (MediaDesc_t *)(&sdp->media[sdp->media_num]);
				sscanf(tmp,"%s %d %s %d",media->media_n.type,&media->media_n.port,
					media->media_n.protocal,&media->media_n.format);
				media_flag = TRUE;
				sdp->media_num++;
				break;
			default:
				break;
		}
		//
		psrc = q;
	}while((q=sdp_next_line(psrc)) != NULL);
	
	//printf("SDP-INFO decode success , media num:%d\r\n",sdp->media_num);
	
	return sdp;
}

int SDP_get_media_attr(SessionDesc_t *sdp,char *media_type,int attr,void *out)
{
	int i;
	int focus=FALSE;
	MediaDesc_t *media=NULL;
	for(i=0;i<sdp->media_num;i++)
	{
		if(strcmp(sdp->media[i].media_n.type,media_type) == 0)
		{
			focus = TRUE;
			break;
		}
	}
	if(focus)
	{
		focus = FALSE;
		media = (MediaDesc_t *)&sdp->media[i];
		for(i=0;i<media->attr_num;i++)
		{
			if(strcmp(media->attri[i].name,sdpAttrs[attr]) == 0)
			{
				memcpy(out,&media->attri[i],sizeof(Attribute_t));
				focus = TRUE;
				//csp modify 20140221
				break;
			}
		}
	}
	if(focus)
	{
		//printf("SDP-INFO: get media:%s attr:%s succss\n",media_type,sdpAttrs[attr]);
		return 0;
	}
	else
	{
		#if 0//csp modify 20140221
		printf("SDP-WARN: get media:%s attr:%s failed\n",media_type,sdpAttrs[attr]);
		return -1;
		#else
		//a=rtpmap:8 PCMA/8000/1
		//a=rtpmap:97 G726-16/8000
		if(strcasecmp(media_type, "audio") == 0 && attr == SDP_ATTR_RTP_MAP)
		{
			//printf("SDP-WARN: get media:%s attr:%s failed\n",media_type,sdpAttrs[attr]);
			Attribute_t *p = (Attribute_t *)out;
			p->rtpmap.payload_type = 8;
			strcpy(p->rtpmap.codec_type, "PCMA");
			p->rtpmap.freq = 8000;
			return 0;
		}
		else
		{
			printf("SDP-WARN: get media:%s attr:%s failed\n",media_type,sdpAttrs[attr]);
			return -1;
		}
		#endif
	}
}

unsigned int SDP_get_ssrc(SessionDesc_t *sdp)
{
	if(sdp == NULL)
	{
		return 0xFFFFFFFF;
	}
	if(sdp->media_num == 0)
	{
		return 0xFFFFFFFF;
	}
	return sdp->media[0].ssrc;
}

int SDP_get_h264_info(SessionDesc_t *sdp,int *payloadtype,char *ip,int *port)
{
	int i,j;
	MediaDesc_t *m=NULL;
	if(sdp == NULL)
	{
		return 0xFFFFFFFF;
	}
	if(sdp->media_num == 0)
	{
		return 0xFFFFFFFF;
	}
	for(i=0;i<sdp->media_num;i++)
	{
		m = (MediaDesc_t *)&sdp->media[i];
		for(j=0;j<m->attr_num;j++)
		{
			if(strcmp(m->attri[j].name,"rtpmap")==0)
			{
				if(strstr(m->attri[j].value,"H264/90000")!=NULL)
				{
					sscanf(m->attri[j].value,"%d",payloadtype);
					*port = m->media_n.port;
					printf("SDP-DEBUG: payloadtype:%d,dest port:%d\n",*payloadtype,*port);
					return 0;
				}
				if(strcmp(m->attri[j].rtpmap.codec_type,"H264")==0)
				{
					*payloadtype = m->attri[j].rtpmap.payload_type;
					*port = m->media_n.port;
					strcpy(ip,sdp->conn_info.addr);
				}
			}
		}
	}
	return -1;
}

int SDP_cleanup(SessionDesc_t *sdp)
{
	if(sdp)
	{
		if(sdp->buffer)
		{
			free(sdp->buffer);
			sdp->buffer = NULL;
		}
		free(sdp);
		sdp = NULL;
	}
	//printf("SDP-INFO destroy success\n");
	return 0;
}

