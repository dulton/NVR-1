#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <sys/time.h>

#include "openRTSP.h"

#include "sock.h"
#include "rtspdef.h"
#include "vlog.h"
#include "rtsplib.h"
#include "rtspsession.h"
#include "portmanage.h"

#include "rtspclient.h"

static unsigned char g_rtspc_loop = 0;

#define FLAG_RUN				0		//正在运行
#define FLAG_DESTORY			1		//正在销毁
#define FLAG_FREE				2		//已经销毁，可以复用

#define RTSP_FLAG_DISCONNECT	1
#define RTSP_FLAG_THREADOVER	2

typedef struct
{
	unsigned int flagState;
	unsigned int dwRTSPFlag;
	Rtsp_t *rtspClient;
	char rtsp_url[64];
	char username[32];
	char password[32];
	unsigned char stream_over_tcp;
	RealStreamCB pStreamCB;
	unsigned int dwStreamContext;
	pthread_mutex_t lock;
}rtsp_client_info;

#define MAX_RTSP_CLIENT_NUM	72

static rtsp_client_info g_rtspc_info[MAX_RTSP_CLIENT_NUM];

static unsigned int g_rtsp_client_count = MAX_RTSP_CLIENT_NUM;

#if defined(_WIN32) || defined(_WIN64) || defined(NOCROSS) || defined(IPCAM_SOLUTION)
enum
{
	AVENC_AUDIO = 0,
	AVENC_IDR,
	AVENC_PSLICE,
	AVENC_FRAME_TYPE_CNT,
};
#endif

void* file_new(const char *name)
{
	FILE *f = fopen(name,"wb+");
	if(f == NULL)
	{
		printf("open file failed\n");
		return NULL;
	}
	VLOG(VLOG_CRIT,"create file:%s success",name);
	return (void *)f;
}

int file_write(FILE *f,char *buf,int size)
{
	if(f == NULL)
	{
		return -1;
	}
	
	if(fwrite(buf,size,1,f) != 1)
	{
		printf("write file failed\n");
		return -1;
	}
	
	return 0;
}

#ifdef _TEST_
extern "C" int IPC_GetStreamResolution(int chn, int *w, int *h)
{
	*w = 1280;
	*h = 720;
	return 0;
}

extern int DoStreamStateCallBack(int chn, real_stream_state_e msg)
{
	return 0;
}
#endif

static inline int rtspc_decode(void *decoder,int type,void *payload,int size,int chn,int stream,uint64_t ts,RealStreamCB FrameHandle)
{
	RealStreamCB pStreamCB = FrameHandle;
	if(pStreamCB == NULL)
	{
		return -1;
	}
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	real_stream_s realstream;
	realstream.pts = (unsigned long long)tv.tv_sec * (unsigned long long)1000000 + (unsigned long long)tv.tv_usec;
	realstream.rsv = 0;
	stream.mdevent = 0;
	realstream.chn = chn;
	realstream.len = size;
	realstream.data = (unsigned char*)payload;
	
	//if(chn == 2) printf("chn%d rtsp data,type=%d,ts=%llu,pts=%llu,size=%d\n",chn,type,ts,realstream.pts,size);
	
	if(type == AVENC_AUDIO)
	{
		
	}
	else
	{
	    realstream.media_type = MEDIA_PT_H264;
		realstream.frame_type = (real_frame_type_e)type;//(type==AVENC_IDR)?REAL_FRAME_TYPE_I:REAL_FRAME_TYPE_P;
		
		int w = 0;
		int h = 0;
		if(IPC_GetStreamResolution(chn,&w,&h) < 0)
		{
			w = h = 0;
		}
		realstream.width = w;
		realstream.height = h;
		
		pStreamCB(&realstream,chn);
	}
	
	return 0;
}

extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);

void *RTSPC_NETWORK_proc3(void *param)
{
	int ret;
	
	int error_occur = true;
	int first_connect = 1;
	
	int timeout_cnt = 0;
	int TIMEOUTCNT = 1000;
	
	int max_sock = 0;
	fd_set read_set;
	struct timeval timeout;
	
	int connect_status;
	int connect_len;
	
	char url[256] = {0};
	unsigned char *ptr = NULL;
	
	ThreadArgs_t *args = (ThreadArgs_t *)param;
	Rtsp_t *r = (Rtsp_t *)args->data;
	r->trigger = true;
	
	int chn = args->RParam;
	int streamtype = (chn >= (int)(g_rtsp_client_count/2)) ? 1 : 0;
	if(!(streamtype == 0 || streamtype == 1))
	{
		VLOG(VLOG_ERROR,"chn:%d stream:%d\n",chn,streamtype);
		r->trigger = false;
	}
	
#ifdef NOCROSS
	char filename_1[32];
	char filename_2[32];
	sprintf(filename_1,"test_%d.264",chn);
	sprintf(filename_2,"test_%d.711a",chn);
	FILE *f1 = NULL;//(FILE *)file_new(filename_1);
	FILE *f2 = NULL;//(FILE *)file_new(filename_2);
	void *dec264 = (void *)f1;
	void *decg711 = (void *)f2;
#else
	void *dec264 = NULL;
	void *decg711 = NULL;
#endif
	
#if 1//csp modify 20140221
	#define BFSIZE (64*1024)
	char buf[BFSIZE];
	int used = 0;
#endif
	
	//pthread_detach(pthread_self());//csp modify
	
	//printf("chn%d stream%d network proc start...\n",chn,streamtype);
	
	while((r->toggle == RTSPC_RUNNING) && (r->trigger))
	{
		if(error_occur == true)
		{
			RTSP_cleanup(r);
			
			if(first_connect)
			{
				MSLEEP(10);
				first_connect = 0;
			}
			else
			{
				MSLEEP(500);
			}
			
			timeout_cnt = 0;
			
			//printf("\nconnect rtsp server...\n");
			
			snprintf(url,sizeof(url),"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
			ret = RTSP_connect_server2(r,url,r->user,r->pwd,r->b_interleavedMode,r->buffer_time);
			if(ret == RTSP_RET_FAIL)
			{
				VLOG(VLOG_ERROR,"RTSP reconnect to %s failed,retry later...",url);
				error_occur = true;
				r->trigger = false;//csp modify
				continue;
			}
			else
			{
				//printf("\nconnect rtsp server success\n");
				error_occur = false;
			}
		}
		
		if(r->state != RTSP_STATE_PLAYING)
		{
			MSLEEP(10);//csp modify
			continue;
		}
		
		timeout.tv_sec = 0;
		//timeout.tv_usec = 200*1000;
		timeout.tv_usec = 1;//10;
		
		FD_ZERO(&read_set);
		max_sock = 0;
		
		if(r->b_interleavedMode == true)
		{
			if(r->sock != -1)
			{
				FD_SET(r->sock,&read_set);
				max_sock = r->sock;
			}
		}
		else
		{
			if(r->rtp_video)
			{
				if(r->rtp_video->sock != -1)
				{
					FD_SET(r->rtp_video->sock,&read_set);
					if(r->rtp_video->sock > max_sock)
					{
						max_sock = r->rtp_video->sock;
					}
				}
			}
			if(r->rtp_audio)
			{
				if(r->rtp_audio->sock != -1)
				{
					FD_SET(r->rtp_audio->sock,&read_set);
					if(r->rtp_audio->sock > max_sock)
					{
						max_sock = r->rtp_audio->sock;
					}
				}
			}
			
			#if 0//lcy modify trouble is here
			if(r->rtcp_video)
			{
				printf("r->rtcp_video->sock = %d\n",r->rtcp_video->sock);
				if(r->rtcp_video->sock != -1)
				{
					FD_SET(r->rtcp_video->sock,&read_set);
					if(r->rtcp_video->sock > max_sock)
					{
						max_sock = r->rtcp_video->sock;
					}
				}
			}
			if(r->rtcp_audio)
			{
				printf("r->rtcp_audio->sock = %d\n",r->rtcp_audio->sock);
				if(r->rtcp_audio->sock != -1)
				{
					FD_SET(r->rtcp_audio->sock,&read_set);
					if(r->rtcp_audio->sock > max_sock)
					{
						max_sock = r->rtcp_audio->sock;
					}
				}
			}
			#endif
		}
		
		ret = select(max_sock+1,&read_set,NULL,NULL,&timeout);
		if(ret < 0)
		{
			VLOG(VLOG_ERROR,"select failed");
			error_occur = true;
			r->trigger = false;//csp modify
			continue;
		}
		else if(ret == 0)
		{
			connect_len = sizeof(connect_status);
			if(getsockopt(r->sock,SOL_SOCKET,SO_ERROR,(char *)&connect_status,(socklen_t*)&connect_len) < 0)
			{
				VLOG(VLOG_ERROR,"rtspc: socket disconnect,errno:%d",errno);
				error_occur = true;
				r->trigger = false;//csp modify
				continue;
			}
			else
			{
				if(connect_status != 0)
				{
					VLOG(VLOG_ERROR,"rtspc: socket disconnect,errno:%d",errno);
					error_occur = true;
					r->trigger = false;//csp modify//???
					continue;
				}
			}
			timeout_cnt++;
			if(r->data_available == false)
			{
				TIMEOUTCNT = 3000;
			}
			else
			{
				TIMEOUTCNT = 1500;
			}
			if(timeout_cnt > TIMEOUTCNT)
			{
				VLOG(VLOG_ERROR,"rtspc: select timeout");
				error_occur = true;
				r->trigger = false;//csp modify
				continue;
			}
			else
			{
				usleep(4*1000);
				continue;//csp modify 20140227
			}
		}
		else
		{
			timeout_cnt = 0;
			
			if(r->b_interleavedMode == true)
			{
				#if 1//csp modify 20140221
				ret = recv(r->sock, buf+used, sizeof(buf)-used, 0);
				if(ret <= 0)
				{
					printf("rtsp recv error occur\n");
					error_occur = true;
					r->trigger = false;//csp modify
					continue;
				}
				ret += used;
				used = 0;
				int offset = 0;
				while(offset < ret)
				{
					for( ; offset < ret; offset++)
					{
						if(buf[offset] == RTSP_INTERLEAVED_MAGIC)
						{
							break;
						}
					}
					if(offset >= ret)
					{
						used = 0;
						break;
					}
					if(offset == ret - 1)
					{
						buf[0] = RTSP_INTERLEAVED_MAGIC;
						used = 1;
						break;
					}
					Rtp_t *rtp = NULL;
					Rtcp_t *rtcp = NULL;
					bool bDataFlag = 0;
					if(!bDataFlag && r->rtp_video)
					{
						if(buf[offset+1] == r->rtp_video->peer_chn_port)
						{
							rtp = r->rtp_video;
							bDataFlag = 1;
						}
					}
					if(!bDataFlag && r->rtcp_video)
					{
						if(buf[offset+1] == r->rtcp_video->chn_port_c)
						{
							rtcp = r->rtcp_video;
							bDataFlag = 1;
						}
					}
					if(!bDataFlag && r->rtp_audio)
					{
						if(buf[offset+1] == r->rtp_audio->peer_chn_port)
						{
							rtp = r->rtp_audio;
							bDataFlag = 1;
						}
					}
					if(!bDataFlag && r->rtcp_audio)
					{
						if(buf[offset+1] == r->rtcp_audio->chn_port_c)
						{
							rtcp = r->rtcp_audio;
							bDataFlag = 1;
						}
					}
					if(!bDataFlag)
					{
						if(buf[offset+1] == RTSP_INTERLEAVED_MAGIC)
						{
							offset++;
						}
						else
						{
							offset += 2;
						}
						continue;
					}
					if(offset + (int)sizeof(RtspInterHeader_t) > ret)
					{
						used = ret - offset;
						memmove(buf, buf+offset, used);
						break;
					}
					int rtn = RTP_RET_OK;
					RtspInterHeader_t interHeader;
					memcpy(&interHeader, buf+offset, sizeof(RtspInterHeader_t));
					int packetsize = ntohs(interHeader.length);
					if(rtp)
					{
						if(packetsize <= (int)sizeof(RtpHeader_t))
						{
							used = 0;
							
							printf("rtp_handle_packet error1 occur\n");
							error_occur = true;
							r->trigger = false;//csp modify
							
							break;
						}
						if(offset + (int)sizeof(RtspInterHeader_t) + (int)sizeof(RtpHeader_t) > ret)
						{
							used = ret - offset;
							memmove(buf, buf+offset, used);
							break;
						}
						RtpHeader_t rtpHeader;
						memcpy(&rtpHeader,buf+offset+sizeof(RtspInterHeader_t),sizeof(RtpHeader_t));
						int extra = 0;
						if(rtpHeader.payload_type == RTP_DEFAULT_VIDEO_TYPE)
						{
							extra = 2;
						}
						//if(packetsize <= (int)sizeof(RtpHeader_t) + 4*rtpHeader.csrc_cnt + extra)
						if(packetsize < (int)sizeof(RtpHeader_t) + 4*rtpHeader.csrc_cnt + extra)
						{
							used = 0;
							
							printf("rtp_handle_packet error2 occur,packetsize=%d,csrc_cnt=%d\n",packetsize,rtpHeader.csrc_cnt);
							error_occur = true;
							r->trigger = false;//csp modify
							
							break;
						}
						/*if(packetsize == (int)sizeof(RtpHeader_t) + 4*rtpHeader.csrc_cnt + extra)
						{
							used = 0;
							break;
						}*/
						if(offset + (int)sizeof(RtspInterHeader_t) + (int)sizeof(RtpHeader_t) + 4*rtpHeader.csrc_cnt + extra > ret)
						{
							used = ret - offset;
							memmove(buf, buf+offset, used);
							break;
						}
						rtn = RTP_handle_packet_bufandnet(rtp, buf+offset+sizeof(RtspInterHeader_t), packetsize, ret-offset-sizeof(RtspInterHeader_t));
						if(rtn == RTSP_RET_FAIL)
						{
							used = 0;
							
							printf("rtp_handle_packet3 error occur\n");
							error_occur = true;
							r->trigger = false;//csp modify
							
							break;
						}
						if(r->rtp_audio)
						{
							if(r->rtp_audio->packet.iFrameCnt)
							{
								rtspc_decode(decg711,AVENC_AUDIO,r->rtp_audio->packet.buffer,
									r->rtp_audio->packet.buf_size[0],chn,streamtype,
									r->rtp_audio->timestamp,
									(RealStreamCB)args->LParam);
								r->rtp_audio->packet.buf_size[0] = 0;
								r->rtp_audio->packet.iFrameCnt = 0;
							}
						}
						if(r->rtp_video)
						{
							if(r->rtp_video->packet.iFrameCnt)
							{
								ptr = (unsigned char *)r->rtp_video->packet.buffer;
								rtspc_decode(dec264,((ptr[4] & 0x1f) == H264_IDR) ? REAL_FRAME_TYPE_I : REAL_FRAME_TYPE_P,
									r->rtp_video->packet.buffer,
									r->rtp_video->packet.buf_size[0],chn,streamtype,
									r->rtp_video->timestamp,
									(RealStreamCB)args->LParam);
								r->rtp_video->packet.buf_size[0] = 0;
								r->rtp_video->packet.iFrameCnt = 0;
								r->data_available = true;
							}
						}
					}
					else if(rtcp)
					{
						if(packetsize <= (int)sizeof(rtcp_common_t))
						{
							used = 0;
							
							printf("rtcp_handle_packet error occur\n");
							error_occur = true;
							r->trigger = false;//csp modify
							
							break;
						}
						//printf("chn%d handle rtcp packet\n",chn);
						rtn = RTCP_handle_packet_bufandnet(rtcp, buf+offset+sizeof(RtspInterHeader_t), packetsize, ret-offset-sizeof(RtspInterHeader_t));
						if(rtn == RTSP_RET_FAIL)
						{
							used = 0;
							
							printf("rtcp_handle_packet error occur\n");
							error_occur = true;
							r->trigger = false;//csp modify
							
							break;
						}
					}
					else
					{
						offset += 2;
						continue;
					}
					offset += (sizeof(RtspInterHeader_t)+packetsize);
					if(offset >= ret)
					{
						used = 0;
						break;
					}
				}
				#else
				ret = RTSP_read_message5(r, RTP_HANDLE, RTCP_HANDLE2);
				if(ret == RTSP_RET_FAIL)
				{
					printf("RTSP_read_message5 error occur\n");
					error_occur = true;
					r->trigger = false;//csp modify
					continue;
				}
				#endif
			}
			else
			{
				if(r->rtp_video)
				{
					if(FD_ISSET(r->rtp_video->sock,&read_set))
					{
						if((ret = RTP_handle_packet_nortpbuf(r->rtp_video,NULL,0)) == RTSP_RET_FAIL)
						{
							error_occur = true;
							r->trigger = false;//csp modify
							continue;
						}
					}
				}
				
				#if 0//lcy modify
				if(r->rtp_audio)
				{
					if(FD_ISSET(r->rtp_audio->sock,&read_set))
					{
						if((ret = RTP_handle_packet_nortpbuf(r->rtp_audio,NULL,0)) == RTSP_RET_FAIL)
						{
							error_occur = true;
							r->trigger = false;//csp modify
							continue;
						}
					}
				}
				#endif
				
				#if RTSP_ENABLE_RTCP == TRUE
				if(r->rtcp_audio)
				{
					if(FD_ISSET(r->rtcp_audio->sock,&read_set))
					{
						RTCP_handle_packet(r->rtcp_audio,NULL,0);
					}
				}
				if(r->rtcp_video)
				{
					if(FD_ISSET(r->rtcp_video->sock,&read_set))
					{
						RTCP_handle_packet(r->rtcp_video,NULL,0);
					}
				}
				#endif
			}
			
			#if 1//csp modify 20140221
			if(r->b_interleavedMode == false)
			{
				if(r->rtp_audio)
				{
					if(r->rtp_audio->packet.iFrameCnt)
					{
						rtspc_decode(decg711,AVENC_AUDIO,r->rtp_audio->packet.buffer,
							r->rtp_audio->packet.buf_size[0],chn,streamtype,
							r->rtp_audio->timestamp,
							(RealStreamCB)args->LParam);
						r->rtp_audio->packet.buf_size[0] = 0;
						r->rtp_audio->packet.iFrameCnt = 0;
					}
				}
				if(r->rtp_video)
				{
					if(r->rtp_video->packet.iFrameCnt)
					{
						ptr = (unsigned char *)r->rtp_video->packet.buffer;
						rtspc_decode(dec264,((ptr[4] & 0x1f) == H264_IDR) ? REAL_FRAME_TYPE_I : REAL_FRAME_TYPE_P,
							r->rtp_video->packet.buffer,
							r->rtp_video->packet.buf_size[0],chn,streamtype,
							r->rtp_video->timestamp,
							(RealStreamCB)args->LParam);
						r->rtp_video->packet.buf_size[0] = 0;
						r->rtp_video->packet.iFrameCnt = 0;
						r->data_available = true;
					}
				}
			}
			#else
			if(r->rtp_audio)
			{
				if(r->rtp_audio->packet.iFrameCnt)
				{
					rtspc_decode(decg711,AVENC_AUDIO,r->rtp_audio->packet.buffer,
						r->rtp_audio->packet.buf_size[0],chn,streamtype,
						r->rtp_audio->timestamp,
						(RealStreamCB)args->LParam);
					r->rtp_audio->packet.buf_size[0] = 0;
					r->rtp_audio->packet.iFrameCnt = 0;
				}
			}
			if(r->rtp_video)
			{
				if(r->rtp_video->packet.iFrameCnt)
				{
					ptr = (unsigned char *)r->rtp_video->packet.buffer;
					rtspc_decode(dec264,((ptr[4] & 0x1f) == H264_IDR) ? REAL_FRAME_TYPE_I : REAL_FRAME_TYPE_P,
						r->rtp_video->packet.buffer,
						r->rtp_video->packet.buf_size[0],chn,streamtype,
						r->rtp_video->timestamp,
						(RealStreamCB)args->LParam);
					r->rtp_video->packet.buf_size[0] = 0;
					r->rtp_video->packet.iFrameCnt = 0;
					r->data_available = true;
				}
			}
			#endif
		}
		
		#if RTSP_ENABLE_RTCP == TRUE
		if(r->rtcp_audio)
		{
			//printf("chn%d process audio rtcp\n",chn);
			RTCP_process(r->rtcp_audio);
		}
		if(r->rtcp_video)
		{
			//printf("chn%d process video rtcp\n",chn);
			RTCP_process(r->rtcp_video);
		}
		#endif
	}
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	RTSP_destroy(r);
	free(param);
	
	memset(g_rtspc_info[chn].rtsp_url, 0, sizeof(g_rtspc_info[chn].rtsp_url));
	memset(g_rtspc_info[chn].username, 0, sizeof(g_rtspc_info[chn].username));
	memset(g_rtspc_info[chn].password, 0, sizeof(g_rtspc_info[chn].password));
	g_rtspc_info[chn].stream_over_tcp = 0;
	g_rtspc_info[chn].pStreamCB = NULL;
	g_rtspc_info[chn].dwStreamContext = 0;
	g_rtspc_info[chn].dwRTSPFlag = 0;
	g_rtspc_info[chn].rtspClient = NULL;
	g_rtspc_info[chn].flagState = FLAG_FREE;
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
	
	printf("chn%d network proc exit!!!\n",chn);
	
	pthread_detach(pthread_self());
	
	return NULL;
}

int RTSPC_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	char rtspURL[64];
	memset(rtspURL, 0, sizeof(rtspURL));
	
	struct in_addr serv;
	serv.s_addr = dwIp;
	
	if(wPort == 554)
	{
		sprintf(rtspURL, "rtsp://%s", inet_ntoa(serv));
	}
	else
	{
		sprintf(rtspURL, "rtsp://%s:%d", inet_ntoa(serv), wPort);
	}
	
	if(streamInfo)
	{
		strcat(rtspURL, "/");
		strcat(rtspURL, streamInfo);
	}
	
	return RTSPC_Startbyurl(chn, pCB, dwContext, rtspURL, user, pwd, rtsp_over_tcp);
}

int RTSPC_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	//rtsp_over_tcp = 0;//for debug
	
	if(chn >= (int)g_rtsp_client_count)
	{
		return -1;
	}
	
	#if 0
	//...
	if(chn >= (int)g_rtsp_client_count/2)
	{
		return -1;
	}
	#endif
	
	char url[128];
	memset(url, 0, sizeof(url));
	//sprintf(url, "rtsp://%s:%s@%s", user, pwd, rtspURL+7);
	strcpy(url, rtspURL);
	//if(rtsp_over_tcp)
	//{
	//	strcat(url, "?tcp");
	//}
	rtspURL = url;
	
	printf("chn%d RTSPC_Startbyurl(%s) - begin (%s)\n", chn, rtspURL, rtsp_over_tcp ? "tcp" : "udp");
	fflush(stdout);
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	if(g_rtspc_info[chn].flagState == FLAG_RUN)
	{
		rtsp_client_info *p = &g_rtspc_info[chn];
		if(strcmp(rtspURL, p->rtsp_url) || 
			strcmp(user, p->username) || 
			strcmp(pwd, p->password) || 
			pCB != p->pStreamCB || 
			dwContext != p->dwStreamContext || 
			rtsp_over_tcp != p->stream_over_tcp)
		{
			printf("chn%d RTSPC_Startbyurl param change:(%s,%s)(%s,%s)(%s,%s)(0x%08x,0x%08x)(%d,%d)(%d,%d)\n", 
					chn, 
					rtspURL, p->rtsp_url, 
					user, p->username, 
					pwd, p->password, 
					(unsigned int)pCB, (unsigned int)p->pStreamCB, 
					dwContext, p->dwStreamContext, 
					rtsp_over_tcp, p->stream_over_tcp);
			pthread_mutex_unlock(&g_rtspc_info[chn].lock);
			RTSPC_Stop(chn);
			pthread_mutex_lock(&g_rtspc_info[chn].lock);
		}
		else
		{
			printf("chn%d RTSPC_Startbyurl - not change\n", chn);
			pthread_mutex_unlock(&g_rtspc_info[chn].lock);
			return 0;
		}
	}
	else if(g_rtspc_info[chn].flagState == FLAG_DESTORY)
	{
		printf("chn%d RTSPC_Startbyurl - destroying\n", chn);
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	
	ThreadArgs_t *args = (ThreadArgs_t *)malloc(sizeof(ThreadArgs_t));
	if(args == NULL)
	{
		VLOG(VLOG_ERROR,"malloc for threadargs failed");
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	
	Rtsp_t *r = RTSP_CLIENT_init(rtspURL,user,pwd,rtsp_over_tcp,0);
	if(r == NULL)
	{
		VLOG(VLOG_ERROR,"RTSP_CLIENT_init failed");
		free(args);
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	r->ThreadId = 0;
	//printf("RTSP_CLIENT_init done\n");
	
	args->data = r;
	args->LParam = (void *)pCB;
    args->RParam = chn;
	//printf("chn%d create rtsp thread...\n",chn);
	int ret = pthread_create(&r->ThreadId, NULL, RTSPC_NETWORK_proc3, (void *)args);
	if(ret != 0)
	{
		r->ThreadId = 0;
		RTSP_destroy(r);
		free(args);
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return -1;
	}
	
	strcpy(g_rtspc_info[chn].rtsp_url, rtspURL);
	if(user)
	{
		strcpy(g_rtspc_info[chn].username, user);
	}
	else
	{
		memset(g_rtspc_info[chn].username, 0, sizeof(g_rtspc_info[chn].username));
	}
	if(pwd)
	{
		strcpy(g_rtspc_info[chn].password, pwd);
	}
	else
	{
		memset(g_rtspc_info[chn].password, 0, sizeof(g_rtspc_info[chn].password));
	}
	g_rtspc_info[chn].stream_over_tcp = rtsp_over_tcp;
	g_rtspc_info[chn].pStreamCB = pCB;
	g_rtspc_info[chn].dwStreamContext = dwContext;
	g_rtspc_info[chn].dwRTSPFlag = 0;
	g_rtspc_info[chn].rtspClient = r;
	g_rtspc_info[chn].flagState = FLAG_RUN;
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	printf("chn%d RTSPC_Startbyurl - over\n", chn);
	fflush(stdout);
	
	return 0;
}

int RTSPC_Stop(int chn)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		printf("chn beyond MAX_CLIENT_NUM:%d\n",g_rtsp_client_count);
		return -1;
	}
	
	int bExit = false;
	ThreadId_t ThreadId = 0;
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	Rtsp_t *p = g_rtspc_info[chn].rtspClient;
	if(p != NULL)
	{
		if(g_rtspc_info[chn].flagState == FLAG_RUN)
		{
			g_rtspc_info[chn].flagState = FLAG_DESTORY;
			bExit = true;
			p->trigger = false;
			ThreadId = p->ThreadId;
		}
		
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		
		if(bExit)
		{
			printf("chn%d RTSPC_Stop - wait join...\n", chn);
			
			pthread_join(ThreadId, NULL);
			
			printf("chn%d RTSPC_Stop - join over!!!\n", chn);
			
			pthread_mutex_lock(&g_rtspc_info[chn].lock);
			
			memset(g_rtspc_info[chn].rtsp_url, 0, sizeof(g_rtspc_info[chn].rtsp_url));
			memset(g_rtspc_info[chn].username, 0, sizeof(g_rtspc_info[chn].username));
			memset(g_rtspc_info[chn].password, 0, sizeof(g_rtspc_info[chn].password));
			g_rtspc_info[chn].stream_over_tcp = 0;
			g_rtspc_info[chn].pStreamCB = NULL;
			g_rtspc_info[chn].dwStreamContext = 0;
			g_rtspc_info[chn].dwRTSPFlag = 0;
			g_rtspc_info[chn].rtspClient = NULL;
			g_rtspc_info[chn].flagState = FLAG_FREE;
			
			pthread_mutex_unlock(&g_rtspc_info[chn].lock);
			
			//DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);//
			
			printf("chn%d RTSPC_Stop - finish\n", chn);
		}
	}
	else
	{
		//printf("chn%d RTSPC_Stop - not opened\n", chn);
		g_rtspc_info[chn].flagState = FLAG_FREE;
		pthread_mutex_unlock(&g_rtspc_info[chn].lock);
		return 0;
	}
	
#if 0
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	//printf("chn%d RTSPC_Stop - 1\n", chn);
	
	while(g_rtspc_info[chn].rtspClient != NULL || g_rtspc_info[chn].flagState != FLAG_FREE)
	{
		usleep(1);
	}
	
	//printf("chn%d RTSPC_Stop - 2\n", chn);
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	//printf("chn%d RTSPC_Stop - over\n", chn);
#endif
	
	return 0;
}

int RTSPC_GetLinkStatus(int chn)
{
	if(chn >= (int)g_rtsp_client_count)
	{
		return 0;
	}
	
	pthread_mutex_lock(&g_rtspc_info[chn].lock);
	
	int ret = (g_rtspc_info[chn].rtspClient != NULL && g_rtspc_info[chn].flagState == FLAG_RUN);
	
	pthread_mutex_unlock(&g_rtspc_info[chn].lock);
	
	return ret;
}

int RTSPC_Init(unsigned int max_client_num)
{
	g_rtsp_client_count = (max_client_num > MAX_RTSP_CLIENT_NUM) ? MAX_RTSP_CLIENT_NUM : max_client_num;
	
	int i = 0;
	for(i = 0; i < MAX_RTSP_CLIENT_NUM; i++)
	{
		memset(g_rtspc_info[i].rtsp_url, 0, sizeof(g_rtspc_info[i].rtsp_url));
		memset(g_rtspc_info[i].username, 0, sizeof(g_rtspc_info[i].username));
		memset(g_rtspc_info[i].password, 0, sizeof(g_rtspc_info[i].password));
		g_rtspc_info[i].stream_over_tcp = 0;
		g_rtspc_info[i].pStreamCB = NULL;
		g_rtspc_info[i].dwStreamContext = 0;
		g_rtspc_info[i].dwRTSPFlag = 0;
		g_rtspc_info[i].rtspClient = NULL;
		g_rtspc_info[i].flagState = FLAG_FREE;
	}
	for(i = 0; i < (int)g_rtsp_client_count; i++)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_rtspc_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	RTSP_session_init();
	PORT_MANAGE_init(41000, 42000);
	
	g_rtspc_loop = 1;
	
	return 0;
}

int RTSPC_DeInit()
{
	g_rtspc_loop = 0;
	return 0;
}

