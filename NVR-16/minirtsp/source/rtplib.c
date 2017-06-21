/******************************************************************************

  Copyright (C), 2013-2020, Juan Optical & Electronical Tech Co., Ltd. All Rights Reserved.

 ******************************************************************************
  File Name    : rtplib.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : A transport protocal for real time application  utils , reference to rtp( rfc3550)
  	rtp payload format for H264 ( rfc3984),
  	rtp payload format for g711(rfc5391)
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "sock.h"

#include "rtpbuf.h"
#include "rtplib.h"
#include "vlog.h"
#include "portmanage.h"

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;

#define RTP_CLIENT 0
#define RTP_SERVER 1

/*
int rtp_init_transport(int cast_type,int protocal,int chn_port)
{
	int sock=-1;
	if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_UNICAST){
		sock=SOCK_udp_init(chn_port,RTSP_SOCK_TIMEOUT);
	}else if(protocal == RTP_TRANSPORT_TCP && cast_type==RTP_UNICAST){
		VLOG(VLOG_DEBUG,"rtsp over tcp");
	}else if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_MULTICAST){
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}else{
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}
	return sock;
}
*/

Rtp_t *RTP_client_new(int protocal,/* udp or tcp */
	int interleaved, /* TRUE or FALSE */
	int sock,
	char *dstip,
	int dstport,
	int buffer_time)
{
	Rtp_t *rtp=NULL;
	RtpPacket_t *p=NULL;
	RtpHeader_t *rtpHeader=NULL;
	RtspInterHeader_t *inHeader=NULL;
	
	rtp = (Rtp_t *)malloc(sizeof(Rtp_t));
	if(rtp == NULL)
	{
		VLOG(VLOG_ERROR,"malloc rtp failed");
		return NULL;
	}
	memset(rtp,0,sizeof(rtp));
	p = &rtp->packet;
	rtpHeader = &rtp->header;
	inHeader = &rtp->interHeader;
	
	rtp->role = RTP_CLIENT;
	rtp->buffertime = buffer_time;
	rtp->timestamp = 0;
	rtp->seq = 0;
	rtp->raw_data = FALSE;
	rtp->protocal = protocal;
	rtp->interleaved = interleaved;
	//
	rtp->base_seq = 0;
	rtp->cycle_cnt = 0;
	rtp->packet_cnt = 0;
	rtp->octet_cnt = 0;
	rtp->fraction_lost = 0;
	rtp->comulative_lost = 0;
	// init for packets' buffer
	p->cnt = 0;
	p->malloc_size = 0;
	p->buffer = NULL;
	p->buf_size[0] = 0;
	p->iFrameCnt = 0;
	
	memset(rtpHeader,0,sizeof(RtpHeader_t));
	memset(inHeader,0,sizeof(RtspInterHeader_t));
	// init transport
	rtp->sock = sock;
	strcpy(rtp->peername,dstip);
	rtp->peer_chn_port = dstport;
	
	//VLOG(VLOG_CRIT,"[CLIENT]rtp over %s init done,dst %s:%d sock:%d",interleaved ? "RTSP" : "UDP",rtp->peername,rtp->peer_chn_port,rtp->sock);
	
	return rtp;
}

Rtp_t *RTP_server_new(uint32_t ssrc,
	int payload_type,
	int protocal,	 /* udp or tcp */
	int interleaved, /* TRUE or FALSE */
	int sock,
	char *dstip,
	int dstport)
{
	Rtp_t *rtp=NULL;
	RtpPacket_t *p=NULL;
	RtpHeader_t *rtpHeader=NULL;
	RtspInterHeader_t *inHeader=NULL;
	
	rtp = (Rtp_t *)malloc(sizeof(Rtp_t));
	if(rtp == NULL)
	{
		VLOG(VLOG_ERROR,"malloc rtp failed");
		return NULL;
	}
	memset(rtp,0,sizeof(Rtp_t));
	p=&rtp->packet;
	rtpHeader=&rtp->header;
	inHeader=&rtp->interHeader;
	
	rtp->role = RTP_SERVER;
	rtp->timestamp = 0xffffffff;
	rtp->seq = 0;
	rtp->raw_data = FALSE;
	rtp->interleaved = interleaved;
	//
	rtp->base_seq = 1;
	rtp->cycle_cnt = 0;
	rtp->packet_cnt = 0;
	rtp->octet_cnt = 0;
	rtp->fraction_lost = 0;
	rtp->comulative_lost = 0;
	// init for packets' buffer
	p->cnt = 0;
	p->malloc_size = 0;
	p->buffer = NULL;
	
	// init rtp header
	rtpHeader->version = RTP_VERSION;
	rtpHeader->padding = FALSE;
	rtpHeader->extension = FALSE;
	rtpHeader->csrc_cnt = 0;
	rtpHeader->marker = FALSE;
	rtpHeader->payload_type = 0;
	rtpHeader->sequence_number = 0;
	rtpHeader->timestamp = 0;
	rtpHeader->ssrc = htonl(ssrc);
	//init rtsp interleaved header
	if(interleaved == TRUE)
	{
		inHeader->magic = '$';
		inHeader->channel = 0;
		inHeader->length = 0;
	}
	else
	{
		memset(inHeader,0,sizeof(RtspInterHeader_t));
	}
	// init transport
	rtp->sock = sock;
	strcpy(rtp->peername,dstip);
	rtp->peer_chn_port=dstport;
	
	VLOG(VLOG_CRIT,"[SERVER]rtp ovr %s init done,sock:%d dst %s:%d",
		interleaved ? "RTSP" : "UDP",rtp->sock,rtp->peername,rtp->peer_chn_port);
	
	return rtp;
}

int RTP_destroy(Rtp_t *rtp)
{
	CircleBuffer_t *buffer=NULL;
	unsigned short port;
	
	if(rtp == NULL)
	{
        VLOG(VLOG_WARNING,"rtp is null");
		return RTP_RET_OK;
	}
	
	if(rtp->interleaved == FALSE)
	{
		SOCK_getsockport(rtp->sock,&port);
		PORT_MANAGE_free_port(port);
		SOCK_close(rtp->sock);
	}
	
	if(rtp->packet.buffer)
	{
		if(rtp->role == RTP_SERVER || rtp->packet.cnt == RTP_NORTPBUF_MAGIC)
		{
			free(rtp->packet.buffer);
		}
		else
		{
			buffer = (CircleBuffer_t *)rtp->packet.buffer;
			buffer->Destroy(buffer);
		}
		rtp->packet.buffer = NULL;
		rtp->packet.cnt = 0;
	}
	
	free(rtp);
	//VLOG(VLOG_CRIT,"rtp destroy success");
	
	return RTP_RET_OK;
}

void RTP_set_type(Rtp_t *rtp, int type)
{
	if(rtp) rtp->payload_type = type;
}

int rtp_alaw_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	
	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->start_flag = TRUE;
	info->stop_flag = TRUE;
	
	ptr += sizeof(RtpHeader_t);
	info->frame_pos = (uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t);
	
	return RTP_RET_OK;
}

int rtp_h264_singlenalu_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader = (RtpHeader_t *)ptr;
	Nalu_t nalu;
	
	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->start_flag = TRUE;
	info->stop_flag = TRUE;
	
	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	if((nalu.type == H264_IDR) || (nalu.type == H264_SPS) || (nalu.type == H264_PPS))
	{
		info->key_flag = TRUE;
	}
	else
	{
		info->key_flag = FALSE;
	}
	// add start code
	ptr -= 4;
	ptr[0] = ptr[1] = ptr[2] = 0;
	ptr[3] = 1;
	info->frame_pos=(uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t) + 4;
	
	return RTP_RET_OK;
}

int rtp_h264_rawdata_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader = (RtpHeader_t *)ptr;
	Nalu_t nalu;
	
	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = rtpHeader->marker;
	
	ptr += sizeof(RtpHeader_t);
	if(((ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 0) && (ptr[3] == 1))
		|| ((ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 1)))
	{ 
		info->start_flag = 1;
		if(ptr[2] == 1)
		{
			nalu.padding = ptr[3];
			ptr--;// add a zero byte
			*ptr = 0;
			size++;
		}
		else
		{
			nalu.padding = ptr[4];
		}
		if((nalu.type == H264_IDR) || (nalu.type == H264_SPS) || (nalu.type == H264_PPS))
		{
			info->key_flag = 1;
		}
	}
	else
	{
		//info->key_flag = 0;
	}
	info->frame_pos=(uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t);
	
	return RTP_RET_OK;
}

int rtp_h264_fua_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader = (RtpHeader_t *)ptr;
	Nalu_t nalu;
	FUHeader_t fuh;
	
	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = rtpHeader->marker;
	
	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	fuh.padding = ptr[1];
	info->start_flag = fuh.start_bit;
	info->stop_flag =fuh.stop_bit;
	if((fuh.type == H264_IDR) || (fuh.type == H264_SPS) || (fuh.type == H264_PPS))
	{ 
		info->key_flag = TRUE;
	}
	else
	{
		info->key_flag = FALSE;
	}
	if(info->start_flag == 1)
	{
		nalu.type = fuh.type;
		*(ptr+1)= nalu.padding;
	}
	info->frame_size = size - sizeof(RtpHeader_t) - ((info->start_flag == 1) ? 1 : 2);
	info->frame_pos = (uint32_t)ptr + ((info->start_flag == 1) ? 1 : 2);
	if(info->start_flag == 1)
	{
		info->frame_size += 4;
		info->frame_pos -= 4;
		ptr = (char *)info->frame_pos;
		ptr[0] = 0;
		ptr[1] = 0;
		ptr[2] = 0;
		ptr[3] = 1;
	}
	
	return RTP_RET_OK;
}

int rtp_h264_fub_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:FU-B");
	return RTP_RET_FAIL;
}

int rtp_h264_stapa_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:STAP-A");
	return RTP_RET_FAIL;
}

int rtp_h264_stapb_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:STAP-B");
	return RTP_RET_FAIL;
}

int rtp_h264_mtap16_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:MTAP16");
	return RTP_RET_FAIL;
}

int rtp_h264_mtap24_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:MTAP24");
	return RTP_RET_FAIL;
}

#if 0
int RTP_handle_packet(Rtp_t *rtp,void *payload,int payload_size)
{
	int ret,size;
	char buf[1024*16],*ptr=NULL,*pbuf=NULL;
	RtpHeader_t *rtpHeader=NULL;
	Nalu_t nalu;
	uint32_t expect_size = 0;
	CircleBuffer_t *buffer=NULL;
	RtpFrameInfo_t info;
	
	if(rtp->interleaved == TRUE)
	{
		ptr = (char *)payload+sizeof(RtspInterHeader_t);
		size = payload_size-sizeof(RtspInterHeader_t);
	}
	else
	{
		ret = SOCK_recvfrom(rtp->sock,rtp->peername,&rtp->peer_chn_port,buf,sizeof(buf),0);
		if(ret == RTP_RET_FAIL)
		{
			return RTP_RET_FAIL;
		}
		ptr = buf;
		size = ret;
	}
	
	pbuf = ptr;
	rtpHeader = (RtpHeader_t *)ptr;
	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	
	if(rtpHeader->padding == TRUE)
	{
		size -= pbuf[size -1];
	}
	
	// update received packets and octect and max seq number
	rtp->packet_cnt++;
	rtp->octet_cnt += size;
	if(rtp->interleaved == TRUE) rtp->octet_cnt += 4;
	if(ntohs(rtpHeader->sequence_number) > rtp->seq)
	{
		rtp->seq = ntohs(rtpHeader->sequence_number);
	}
	// malloc circle buffer
	if(rtp->packet.buffer == NULL)
	{
		if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU)
		{
			expect_size = (uint32_t)(RTP_AUDIO_RECV_BUFFER_SIZE);
		}
		else
		{
			expect_size = (uint32_t)(RTP_VIDEO_RECV_BUFFER_SIZE);
		}
		rtp->packet.buffer = (CircleBuffer_t *)CIRCLEBUFFER_new(expect_size,rtp->buffertime);
		if(rtp->packet.buffer == NULL)
		{
			return RTP_RET_FAIL;
		}
		rtp->packet.cnt = 1;
		rtp->packet.buf_size[0] = expect_size;
		rtp->packet.malloc_size = expect_size;
	}
	buffer = (CircleBuffer_t *)rtp->packet.buffer;
	
	if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU)
	{
		ret = rtp_alaw_decode(pbuf,size,&info);
	}
	else if(rtpHeader->payload_type == RTP_DEFAULT_VIDEO_TYPE)
	{
		if(rtp->raw_data == TRUE)
		{
			ret = rtp_h264_rawdata_decode(pbuf,size,&info);
		}
		else if((rtp->raw_data == FALSE) && 
			(((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==0) && (ptr[3] == 1))
			|| ((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==1))))
		{
			VLOG(VLOG_DEBUG,"h264 format : raw data");
			//rtp->raw_data = TRUE;
			ret = rtp_h264_rawdata_decode(pbuf,size,&info);
		}
		else if(nalu.type <= H264_FILLER_DATA)
		{
			ret = rtp_h264_singlenalu_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_FU_A)
		{
			ret = rtp_h264_fua_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_FU_B)
		{
			ret = rtp_h264_fub_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_STAP_A)
		{
			ret = rtp_h264_stapa_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_STAP_B)
		{
			ret = rtp_h264_stapb_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_MTAP16)
		{
			ret = rtp_h264_mtap16_decode(pbuf,size,&info);
		}
		else if(nalu.type == RTP_MTAP24)
		{
			ret = rtp_h264_mtap24_decode(pbuf,size,&info);
		}
		else
		{
			VLOG(VLOG_ERROR,"unknown nalu type:%d",nalu.type);
			return RTP_RET_FAIL;
		}
	}
	else
	{
		VLOG(VLOG_ERROR,"unsupport payload type:%d interleaved:%d",rtpHeader->payload_type,rtp->interleaved);
		return RTP_RET_FAIL;
	}
	if(ret == RTP_RET_FAIL)
		return RTP_RET_FAIL;
	
#ifdef RTSP_BUFFER_ENTER_KEYFRAME_FIRST
	if((buffer->IsAvailable(buffer) == FALSE) && (buffer->GetUsedSize(buffer) == 0) && (info.type == RTP_DEFAULT_VIDEO_TYPE))
	{
		if(info.key_flag == FALSE)
		{
			VLOG(VLOG_WARNING,"RTP: recv a packet,size:%d ts:%u seq:%u,$$ignore$$",size,info.timestamp,info.seq);
			return RTP_RET_OK;
		}
	}
#endif
	
	ret = buffer->AddRtpFrame(buffer,&info);
	if(ret == RTP_RET_FAIL)
	{
		return RTP_RET_FAIL;
	}
	else if(ret == CBUFFER_RET_NODATA)
	{
		buffer->Flush(buffer);
		ret = buffer->AddRtpFrame(buffer,&info);
		if(ret != RTP_RET_OK) return RTP_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"RTP:recv a packet,type:%d size:%d ts:%u seq:%u",info.type,size,info.timestamp,info.seq);
	return RTP_RET_OK;
}
#endif

int RTP_handle_packet_bufandnet(Rtp_t *rtp,char *payload,int payload_size,int received_size)
{
#define BUFFERSIZE (16*1024)
#define RTP_LOOP_RECV(BUF,SIZE,FLAG) \
		ret = SOCK_recv2(rtp->sock, BUF, SIZE, FLAG);\
		if(ret < 0) return RTP_RET_FAIL
	
	int ret;
	char *ptr=NULL;
	char tmp[128];
	RtpHeader_t rtpHeader;
	Nalu_t nalu;
	FUHeader_t fuh;
	unsigned int expect_size = 0;
	int uncompleted_size = (received_size >= payload_size) ? 0 : (payload_size - received_size);
	
	memcpy(&rtpHeader, payload, sizeof(RtpHeader_t));
	
	//check the RTP header
	if(rtpHeader.version != 2)
	{
		VLOG(VLOG_ERROR,"rtp version is wrong!!!");
		//return RTP_RET_OK;
		return RTP_RET_FAIL;
	}
	if(rtpHeader.payload_type != RTP_DEFAULT_VIDEO_TYPE)
	{
		VLOG(VLOG_ERROR,"rtp payload type is wrong!!!");
		//return RTP_RET_OK;
		return RTP_RET_FAIL;
	}
	
	int skip_size = sizeof(RtpHeader_t)+4*rtpHeader.csrc_cnt;
	
	rtp->seq = ntohs(rtpHeader.sequence_number);
	rtp->payload_type = rtpHeader.payload_type;
	rtp->timestamp = ntohl(rtpHeader.timestamp);
	
	// check rtp buffer
	if(rtp->packet.buffer == NULL)
	{
		if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
		{
			expect_size = RTP_AUDIO_RECV_BUFFER_SIZE;
		}
		else
		{
			expect_size = RTP_VIDEO_RECV_BUFFER_SIZE;
		}
		rtp->packet.buffer = (void *)malloc(expect_size);
		if(rtp->packet.buffer == NULL)
		{
			VLOG(VLOG_ERROR,"malloc for rtp buffer failed!\n");
			exit(0);
		}
		rtp->packet.cnt = RTP_NORTPBUF_MAGIC; //note:: when nortpbuf, use it to markup
		rtp->packet.malloc_size = expect_size;
		rtp->packet.buf_size[0] = 0;
	}
	
	//receive frame data
	if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
	{
		if(payload_size-skip_size > rtp->packet.malloc_size)
		{
			VLOG(VLOG_ERROR,"audio rtp payload size is wrong!!!");
			return RTP_RET_FAIL;
		}
		// read audio frame data
		ptr = (char *)rtp->packet.buffer;
		memcpy(ptr, payload+skip_size, payload_size-skip_size-uncompleted_size);
		if(uncompleted_size)
		{
			RTP_LOOP_RECV(ptr+payload_size-skip_size-uncompleted_size, uncompleted_size, 0);
		}
		rtp->packet.buf_size[0] = payload_size-skip_size;
		rtp->packet.iFrameCnt = 1;
	}
	else if(rtpHeader.payload_type == RTP_DEFAULT_VIDEO_TYPE)
	{
		//read video frame data
		// --peek 2 bytes firstly
		memcpy(tmp, payload+skip_size, 2);
		skip_size += 2;
		if(skip_size > payload_size)
		{
			printf("video data is error\n");
			return RTP_RET_FAIL;
		}
		nalu.padding = tmp[0];
		// check and read the real frame data
		if(nalu.type <= H264_FILLER_DATA)
		{
			if(payload_size-skip_size+6 > rtp->packet.malloc_size)
			{
				VLOG(VLOG_ERROR,"video rtp payload size is wrong!!!");
				return RTP_RET_FAIL;
			}
			ptr = (char *)rtp->packet.buffer;
			ptr[0] = 0;
			ptr[1] = 0;
			ptr[2] = 0;
			ptr[3] = 1;
			ptr[4] = tmp[0];
			ptr[5] = tmp[1];
			memcpy(ptr+6, payload+skip_size, payload_size-skip_size-uncompleted_size);
			if(uncompleted_size)
			{
				RTP_LOOP_RECV(ptr+6+payload_size-skip_size-uncompleted_size, uncompleted_size, 0);
			}
			rtp->packet.buf_size[0] = payload_size-skip_size+6;
			rtp->packet.iFrameCnt = 1;
		}
		else if(nalu.type == RTP_FU_A)
		{
			//printf("ptr:%p pbuf:%p %u %d\n",ptr,pbuf,size,rtp->packet.buf_size[0]);
			fuh.padding = tmp[1];
			ptr = (char *)rtp->packet.buffer+rtp->packet.buf_size[0];
			if(fuh.start_bit == 1)
			{
				if(payload_size-skip_size+5 > rtp->packet.malloc_size)
				{
					VLOG(VLOG_ERROR,"video rtp payload size is wrong!!!");
					return RTP_RET_FAIL;
				}
				// reset buffer pos
				rtp->packet.iFrameCnt = 0;
				rtp->packet.buf_size[0] = 0;
				ptr = (char *)rtp->packet.buffer;
				//
				nalu.type = fuh.type;
				ptr[0] = 0;
				ptr[1] = 0;
				ptr[2] = 0;
				ptr[3] = 1;
				ptr[4] = nalu.padding;
				memcpy(ptr+5, payload+skip_size, payload_size-skip_size-uncompleted_size);
				if(uncompleted_size)
				{
					RTP_LOOP_RECV(ptr+5+payload_size-skip_size-uncompleted_size, uncompleted_size, 0);
				}
				rtp->packet.buf_size[0] = payload_size-skip_size+5;
			}
			else
			{
				if((rtp->packet.buf_size[0]+payload_size-skip_size) >= rtp->packet.malloc_size)
				{
					VLOG(VLOG_ERROR,"rtp video buffer exceed malloc size!!!");
					return RTP_RET_FAIL;
				}
				memcpy(ptr, payload+skip_size, payload_size-skip_size-uncompleted_size);
				if(uncompleted_size)
				{
					RTP_LOOP_RECV(ptr+payload_size-skip_size-uncompleted_size, uncompleted_size, 0);
				}
				rtp->packet.buf_size[0] += (payload_size-skip_size);
				//printf("Sock%d Packet size = %d\n",rtp->sock,rtp->packet.buf_size[0]);///////////////////////////////////////////////
			}
			if(fuh.stop_bit == 1)
			{
				rtp->packet.iFrameCnt = 1;
			}
		}
		else
		{
			VLOG(VLOG_ERROR, "unknown nalu type: %d!", nalu.type);
			#if 1//csp modify 20140221
			return RTP_RET_FAIL;
			#else
			//return RTP_RET_FAIL;
			return RTP_RET_OK;//lcy modify
			#endif
		}
	}
	else
	{
		VLOG(VLOG_ERROR, "unknown payload type: %d!", rtp->payload_type);
		return RTP_RET_FAIL;
	}
	
	rtp->packet_cnt++;
	
	//if(rtp->packet.iFrameCnt == 1)
		VLOG(VLOG_DEBUG,"RTP::recv a packet,type:%d size:%d ts:%u seq:%u",rtp->payload_type,rtp->packet.buf_size[0],rtp->timestamp,rtp->seq);
	
	return RTP_RET_OK;
}

int RTP_handle_packet_nortpbuf(Rtp_t *rtp,void *payload,int payload_size)
{
#define BFSIZE (16*1024)
#define RTP_RECV(BUF,SIZE,FLAG) \
		if(rtp->interleaved){\
			ret = SOCK_recv2(rtp->sock, BUF, SIZE, FLAG);\
		}else{\
			memcpy(BUF, pbuf, SIZE);\
			pbuf += SIZE;\
			ret = SIZE;\
		}\
		if(ret < 0) return RTP_RET_FAIL
	
	int ret;
	char buf[BFSIZE], *ptr=NULL, *pbuf=NULL;
	char tmp[128];
	RtpHeader_t rtpHeader;
	Nalu_t nalu;
	FUHeader_t fuh;
	unsigned int expect_size = 0;
	int remind = rtp->interleaved ? payload_size : BFSIZE;
	if(rtp->interleaved == FALSE)
	{
		ret = SOCK_recvfrom(rtp->sock,rtp->peername,&rtp->peer_chn_port,buf,BFSIZE,0);
		pbuf = buf;
		remind = ret;
	}
	// receive rtp header first
	RTP_RECV((char *)&rtpHeader, sizeof(RtpHeader_t), 0);
	remind -= sizeof(RtpHeader_t);
	//check the RTP header 
	if(rtpHeader.version != 2)
	{
		VLOG(VLOG_ERROR,"rtp version is wrong!!!");
		return RTP_RET_OK;
	}
	if(rtpHeader.payload_type != RTP_DEFAULT_VIDEO_TYPE)
	{
		VLOG(VLOG_ERROR,"rtp payload type is wrong!!!");
		return RTP_RET_OK;
	}
	
	if(rtpHeader.csrc_cnt)
	{
		printf("RTP_RECV rtpHeader.csrc_cnt#######################\n");//lcy def
		RTP_RECV(tmp, 4*rtpHeader.csrc_cnt, 0);
		remind -= 4*rtpHeader.csrc_cnt;
	}
	
	rtp->seq = ntohs(rtpHeader.sequence_number);
	rtp->payload_type = rtpHeader.payload_type;
	rtp->timestamp = ntohl(rtpHeader.timestamp);
	
	// check rtp buffer 
	if(rtp->packet.buffer == NULL)
	{
		if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
		{
			expect_size = RTP_AUDIO_RECV_BUFFER_SIZE;
		}
		else
		{
			expect_size = RTP_VIDEO_RECV_BUFFER_SIZE;
		}
		rtp->packet.buffer = (void *)malloc(expect_size);
		if(rtp->packet.buffer == NULL)
		{
			VLOG(VLOG_ERROR,"malloc for rtp buffer failed!\n");
			exit(0);
		}
		rtp->packet.cnt = RTP_NORTPBUF_MAGIC; //note:: when nortpbuf, use it to markup
		rtp->packet.malloc_size = expect_size;
		rtp->packet.buf_size[0] = 0;
	}
	
	//receive frame data
	if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
	{
		// read audio frame data
		ptr = (char *)rtp->packet.buffer;
		RTP_RECV(ptr, remind, 0);
		rtp->packet.buf_size[0] = ret;
		rtp->packet.iFrameCnt = 1;
	}
	else if(rtpHeader.payload_type == RTP_DEFAULT_VIDEO_TYPE)
	{
		//read video frame data
		// --peek 2 bytes firstly
		RTP_RECV(tmp, 2, 0);
		remind -= 2;
		if(remind < 0)
		{
			printf("remind data is %d\n",remind);
			//csp modify 20140220
			//exit(0);//debug
			return RTP_RET_FAIL;
		}
		nalu.padding = tmp[0];
		// check and read the real frame data
		if(nalu.type <= H264_FILLER_DATA)
		{
			ptr = (char *)rtp->packet.buffer;
			ptr[0] = 0;
			ptr[1] = 0;
			ptr[2] = 0;
			ptr[3] = 1;
			ptr[4] = tmp[0];
			ptr[5] = tmp[1];
			RTP_RECV(ptr + 6, remind, 0);
			rtp->packet.buf_size[0] = ret + 6;
			rtp->packet.iFrameCnt = 1;
		}
		else if(nalu.type == RTP_FU_A)
		{
			//printf("ptr:%p pbuf:%p %u %d\n",ptr,pbuf,size,rtp->packet.buf_size[0]);
			fuh.padding = tmp[1];
			ptr = (char *)rtp->packet.buffer+rtp->packet.buf_size[0];
			if(fuh.start_bit == 1)
			{
				// reset buffer pos
				rtp->packet.iFrameCnt = 0;
				rtp->packet.buf_size[0] = 0;
				ptr = (char *)rtp->packet.buffer;
				//
				nalu.type = fuh.type;
				ptr[0] = 0;
				ptr[1] = 0;
				ptr[2] = 0;
				ptr[3] = 1;
				ptr[4] = nalu.padding;
				RTP_RECV(ptr + 5, remind, 0);
				rtp->packet.buf_size[0] = ret + 5;
			}
			else
			{
				if((rtp->packet.buf_size[0] + remind) >= rtp->packet.malloc_size)
				{
					VLOG(VLOG_ERROR,"rtp video buffer exceed malloc size!!!");
					return RTP_RET_FAIL;
				}
				RTP_RECV(ptr, remind, 0);
				rtp->packet.buf_size[0] += ret;
				//printf("Sock%d Packet size = %d\n",rtp->sock,rtp->packet.buf_size[0]);///////////////////////////////////////////////
			}
			if(fuh.stop_bit == 1)
			{
				rtp->packet.iFrameCnt = 1;
			}
		}
		else
		{
			VLOG(VLOG_ERROR, "unknown nalu type: %d!", nalu.type);
			#if 1//csp modify 20140221
			return RTP_RET_FAIL;
			#else
			//return RTP_RET_FAIL;
			return RTP_RET_OK;//lcy modify
			#endif
		}
	}
	else
	{
		VLOG(VLOG_ERROR, "unknown payload type: %d!", rtp->payload_type);
		return RTP_RET_FAIL;
	}
	
	rtp->packet_cnt++;
	//rtp->octet_cnt += payload_size;
	
	//if(rtp->packet.iFrameCnt == 1)
		VLOG(VLOG_DEBUG,"RTP::recv a packet,type:%d size:%d ts:%u seq:%u",rtp->payload_type,rtp->packet.buf_size[0],rtp->timestamp,rtp->seq);
	//*********************************************************************************
	
	//*********************************************************************************
	return RTP_RET_OK;
}

#if 0
int RTP_handle_packet_nortpbuf2(Rtp_t *rtp,void *payload,int payload_size)
{
	int ret,size;
	char buf[1024*16],*ptr=NULL,*pbuf=NULL;
	RtpHeader_t *rtpHeader=NULL;
	Nalu_t nalu;
	uint32_t expect_size = 0;
	char *buffer=NULL;
	FUHeader_t fuh;
	
	if(rtp->interleaved == TRUE){
		ptr=(char *)payload+sizeof(RtspInterHeader_t);
		size = payload_size-sizeof(RtspInterHeader_t);
		//
	}else{
		ret=SOCK_recvfrom(rtp->sock,rtp->peername,&rtp->peer_chn_port,buf,sizeof(buf),0);
		if(ret == RTP_RET_FAIL){
			return RTP_RET_FAIL;
		}
		ptr = buf;
		size = ret;
	}
	
	pbuf=ptr;
	rtpHeader = (RtpHeader_t *)ptr;
	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	if(rtpHeader->padding == TRUE){
		size-=pbuf[size -1];
	}
	
	// malloc circle buffer
	if(rtp->packet.buffer == NULL){
		if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU){
			expect_size = (uint32_t)(RTP_AUDIO_RECV_BUFFER_SIZE);
		}else{
			expect_size = (uint32_t)(RTP_VIDEO_RECV_BUFFER_SIZE);
		}
		rtp->packet.buffer=(void *)malloc(expect_size);
		if(rtp->packet.buffer == NULL){
			return RTP_RET_FAIL;
		}
		rtp->packet.cnt=RTP_NORTPBUF_MAGIC; //note:: when nortpbuf, use it to markup
		rtp->packet.malloc_size = expect_size;
		rtp->packet.buf_size[0] = 0;
	}
	buffer = (char *)rtp->packet.buffer;
	
	// update received packets and octect and max seq number
	if(rtp->seq != 65535){
		if((ntohs(rtpHeader->sequence_number) != (rtp->seq+1)) && rtp->packet_cnt){
			VLOG(VLOG_ERROR,"seq number wrong,%s:%d->%d",rtp->peername,rtp->seq,ntohs(rtpHeader->sequence_number));
		}
	}else{
		if(ntohs(rtpHeader->sequence_number) != 0){
			VLOG(VLOG_ERROR,"seq number wrong,%s:%d->%d",rtp->peername,rtp->seq,ntohs(rtpHeader->sequence_number));
		}
		rtp->cycle_cnt++;
	}
	rtp->packet_cnt++;
	rtp->octet_cnt += size;
	if(rtp->interleaved == TRUE) rtp->octet_cnt += 4;
	
	//if(ntohs(rtpHeader->sequence_number) > rtp->seq ){
	rtp->seq = ntohs(rtpHeader->sequence_number);
	//}
	rtp->payload_type = rtpHeader->payload_type;
	rtp->timestamp = ntohl(rtpHeader->timestamp);
	
	if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU){
		if((size-sizeof(RtpHeader_t)) >= rtp->packet.malloc_size){
			VLOG(VLOG_ERROR,"rtp audio buffer exceed malloc size!!!");
			return RTP_RET_FAIL;
		}
		memcpy((char *)rtp->packet.buffer,pbuf+sizeof(RtpHeader_t),size-sizeof(RtpHeader_t));
		rtp->packet.buf_size[0] = size-sizeof(RtpHeader_t);
		rtp->packet.iFrameCnt = 1;
	}else if(rtpHeader->payload_type == RTP_DEFAULT_VIDEO_TYPE){
		if((rtp->raw_data==FALSE) && 
			(((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==0) && (ptr[3] == 1))
			|| ((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==1)))){			
			memcpy((char *)rtp->packet.buffer,
				pbuf+sizeof(RtpHeader_t),size-sizeof(RtpHeader_t));
			rtp->packet.buf_size[0] = size-sizeof(RtpHeader_t);
			rtp->packet.iFrameCnt = 1;
		}else if(nalu.type <= H264_FILLER_DATA){
			ptr = (char *)rtp->packet.buffer;
			ptr[0] = 0;ptr[1] = 0;ptr[2]=0;ptr[3] = 1;
			memcpy(ptr+4,pbuf+sizeof(RtpHeader_t),size-sizeof(RtpHeader_t));
			rtp->packet.buf_size[0] = size-sizeof(RtpHeader_t) + 4;
			rtp->packet.iFrameCnt = 1;
		}else if(nalu.type == RTP_FU_A){	
			//printf("ptr:%p pbuf:%p %u %d\n",ptr,pbuf,size,rtp->packet.buf_size[0]);
			fuh.padding = ptr[1];
			ptr = (char *)rtp->packet.buffer+rtp->packet.buf_size[0];
			if(fuh.start_bit == 1){
				// reset buffer pos
				rtp->packet.iFrameCnt = 0;
				rtp->packet.buf_size[0] = 0;
				ptr = (char *)rtp->packet.buffer;
				//
				nalu.type = fuh.type;
				ptr[0]= 0;
				ptr[1] = 0;
				ptr[2] = 0;
				ptr[3] = 1;
				ptr[4] = nalu.padding;
				rtp->packet.buf_size[0] += 5;
				memcpy(ptr+5,pbuf+sizeof(RtpHeader_t)+2,size-sizeof(RtpHeader_t)-2);
				rtp->packet.buf_size[0] += size-sizeof(RtpHeader_t)-2;
			}else{
				if((rtp->packet.buf_size[0] + size-sizeof(RtpHeader_t)-2) >= rtp->packet.malloc_size){
					VLOG(VLOG_ERROR,"rtp video buffer exceed malloc size!!!");
					return RTP_RET_FAIL;
				}
				memcpy(ptr,pbuf+sizeof(RtpHeader_t)+2,size-sizeof(RtpHeader_t)-2);
				rtp->packet.buf_size[0] += size-sizeof(RtpHeader_t)-2;
			}
			if(fuh.stop_bit == 1){
				rtp->packet.iFrameCnt = 1;
			}
		}else{
			VLOG(VLOG_ERROR,"unknown nalu type:%d",nalu.type);
			return RTP_RET_FAIL;
		}
	}else{
		VLOG(VLOG_ERROR,"unsupport payload type:%d interleaved:%d",rtpHeader->payload_type,rtp->interleaved);
		return RTP_RET_FAIL;
	}
	
	VLOG(VLOG_DEBUG,"RTP:recv a packet,type:%d size:%d ts:%u seq:%u",
		rtp->payload_type,size,rtp->timestamp,rtp->seq);
	return RTP_RET_OK;
}

int RTP_handle_packet_nortpbuf_oneRecv(Rtp_t *rtp,void *payload,int payload_size)
{
#define BFSIZE (16*1024)
	//int ret;
	char buf[BFSIZE],*bpt=NULL,*ptr=NULL;//,*pbuf=NULL;
	char tmp[128];
	RtpHeader_t rtpHeader;
	Nalu_t nalu;
	FUHeader_t fuh;
	unsigned int expect_size = 0;
	
	//recieve a complete RTP packet
	bpt = buf;
	int remind = SOCK_recv2(rtp->sock,bpt,payload_size,0);
	
	// get rtp header first
	memcpy(&rtpHeader,bpt,sizeof(rtpHeader));
	bpt += sizeof(RtpHeader_t);
	remind -= sizeof(RtpHeader_t);
	
	//check the RTP header
	if(rtpHeader.version != 2)
	{
		VLOG(VLOG_ERROR,"rtp version is wrong!!!");
		return RTP_RET_OK;
	}
	if(rtpHeader.payload_type != RTP_DEFAULT_VIDEO_TYPE)
	{
		VLOG(VLOG_ERROR,"rtp payload type is wrong!!!");
		return RTP_RET_OK;
	}
	if(rtpHeader.csrc_cnt)
	{
		memcpy(tmp,ptr,4*rtpHeader.csrc_cnt);
		bpt += 4*rtpHeader.csrc_cnt;
		remind -= 4*rtpHeader.csrc_cnt;
	}
	rtp->seq = ntohs(rtpHeader.sequence_number);
	rtp->payload_type = rtpHeader.payload_type;
	rtp->timestamp = ntohl(rtpHeader.timestamp);
	
	// check rtp buffer
	if(rtp->packet.buffer == NULL)
	{
		if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
		{
			expect_size = RTP_AUDIO_RECV_BUFFER_SIZE;
		}
		else
		{
			expect_size = RTP_VIDEO_RECV_BUFFER_SIZE;
		}
		rtp->packet.buffer = (void *)malloc(expect_size);
		if(rtp->packet.buffer == NULL)
		{
			VLOG(VLOG_ERROR,"malloc for rtp buffer failed!\n");
			exit(0);
		}
		rtp->packet.cnt = RTP_NORTPBUF_MAGIC; //note:: when nortpbuf, use it to markup
		rtp->packet.malloc_size = expect_size;
		rtp->packet.buf_size[0] = 0;
	}
	
	//receive frame data
	if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU)
	{
		// read audio frame data
		ptr = (char *)rtp->packet.buffer;
		memcpy(ptr,bpt,remind);
		rtp->packet.buf_size[0] = remind;//ret;//csp ???
		rtp->packet.iFrameCnt = 1;
	}
	else if(rtpHeader.payload_type == RTP_DEFAULT_VIDEO_TYPE)
	{
		//read video frame data
		// --peek 2 bytes firstly
		memcpy(tmp,bpt,2);
		bpt += 2;
		remind -= 2;
		if(remind < 0)
		{
			printf("remind data is %d\n",remind);
			exit(0);//debug//csp ???
		}
		nalu.padding = tmp[0];
		// check and read the real frame data
		if(nalu.type <= H264_FILLER_DATA)
		{
			ptr = (char *)rtp->packet.buffer;
			ptr[0] = 0;
			ptr[1] = 0;
			ptr[2] = 0;
			ptr[3] = 1;
			ptr[4] = tmp[0];
			ptr[5] = tmp[1];
			memcpy(ptr + 6,bpt,remind);
			rtp->packet.buf_size[0] = remind + 6;
			rtp->packet.iFrameCnt = 1;
		}
		else if(nalu.type == RTP_FU_A)
		{
			//printf("ptr:%p pbuf:%p %u %d\n",ptr,pbuf,size,rtp->packet.buf_size[0]);
			fuh.padding = tmp[1];
			ptr = (char *)rtp->packet.buffer+rtp->packet.buf_size[0];
			if(fuh.start_bit == 1)
			{
				// reset buffer pos
				rtp->packet.iFrameCnt = 0;
				rtp->packet.buf_size[0] = 0;
				ptr = (char *)rtp->packet.buffer;
				//
				nalu.type = fuh.type;
				ptr[0] = 0;
				ptr[1] = 0;
				ptr[2] = 0;
				ptr[3] = 1;
				ptr[4] = nalu.padding;
				memcpy(ptr + 5,bpt,remind);
				rtp->packet.buf_size[0] = remind + 5;
			}
			else
			{
				if((rtp->packet.buf_size[0] + remind) >= rtp->packet.malloc_size)
				{
					VLOG(VLOG_ERROR,"rtp video buffer exceed malloc size!!!");
					return RTP_RET_FAIL;
				}
				memcpy(ptr,bpt,remind);
				rtp->packet.buf_size[0] += remind;
			}
			if(fuh.stop_bit == 1)
			{
				rtp->packet.iFrameCnt = 1;
			}
		}
		else
		{
			VLOG(VLOG_ERROR, "unknown nalu type: %d!", nalu.type);
			return RTP_RET_FAIL;
		}
	}
	else
	{
		VLOG(VLOG_ERROR, "unknown payload type: %d!", rtp->payload_type);
		return RTP_RET_FAIL;
	}
	
	rtp->packet_cnt++;
	//rtp->octet_cnt += payload_size;
	
	printf("ONE_RECV#########################################\n");
	
	//if(rtp->packet.iFrameCnt == 1)
	{
		VLOG(VLOG_DEBUG,"RTP::recv a packet,type:%d size:%d ts:%u seq:%u",
			rtp->payload_type,rtp->packet.buf_size[0],rtp->timestamp,rtp->seq);
	}
	
	return RTP_RET_OK;
}
#endif

