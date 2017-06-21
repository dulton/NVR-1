#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include "custommp4.h"
#include "IpcTest.h"

#define MAIN_STREAM_FILE 	"/root/myusb/main_stream.ifv"	//从该文件取主码流帧
#define SUB_STREAM_FILE		"/root/myusb/sub_stream.ifv"	//从该文件取主码流帧

#ifdef WIN32
#define NETSNDRCVFLAG	0
#else
#include <netinet/tcp.h>
#define NETSNDRCVFLAG	MSG_NOSIGNAL
#define INVALID_SOCKET	(-1)
#define INVALID_VAL	(-1)
#define SOCKET_ERROR	(-1)
#endif

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef unsigned long long	u64;


#if 0
typedef struct
{
	int init_flag;
	int thread_running;
	u32 client_count;
	u32 bit_stream_chn;
	RealStreamCB pStreamCB;
	pthread_t thread_pid[2];
} manager_info;


static manager_info *g_manager_info = NULL;

#define PFrameInterval	(20) //只改变P帧之间的间隔时间(ms)
void* ThreadPROC(void* pParam)
{
	char *pbuf = NULL;
	u32 buf_size = 0;
	const char *pfile_name = NULL;
	custommp4_t *pfile = NULL;
	u32 file_residue_video_frame_nums = 0;
	u8 read_frame_cnt = 0;
	real_stream_s stream;
	u8 media_type = 0;
	u64 pts = 0;
	u8 key = 0;
	u32 start_time = 0;
	int readlen = 0;
	struct timeval cur_tm; //当前时间
	u64 pre_frame_pts = 0; //上一帧发送的时间
	u32 sleep_us = 0; //两帧之间的间隔
	int chn = (int)pParam;
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		goto ProcOver;
	}
	
	if(chn < (int)(g_anni_client_count/2))
	{
		buf_size = 768*1024;
		pfile_name = MAIN_STREAM_FILE;//主码流
	}
	else
	{
		buf_size = 200*1024;
		pfile_name = SUB_STREAM_FILE;//子码流
	}
	
	pbuf = (char *)malloc(buf_size);
	if(pbuf == NULL)
	{
		goto ProcOver;
	}

	if(chn < (int)(g_anni_client_count/2))
	printf("%s running, chn: %d, file name: %s\n", __func__, chn, pfile_name);
	
	while (g_init_flag)
	{
		pthread_mutex_lock(&g_annic_info[chn].lock);
		if(g_annic_info[chn].eventLoopWatchVariable)
		{
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d eventLoopWatchVariable == 1, will be exit\n", __func__, chn);
			
			pthread_mutex_unlock(&g_annic_info[chn].lock);
			break;
		}
		pthread_mutex_unlock(&g_annic_info[chn].lock);

		if (file_residue_video_frame_nums == 0)
		{
			
			//线程刚运行，还没有传输
			//或者文件传输完
			//关闭文件再次打开
			if (pfile)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer over, reopen it\n", __func__, chn);
				
				custommp4_close(pfile);
				pfile = NULL;
			}
			else
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer start\n", __func__, chn);
			}

			pfile = custommp4_open(pfile_name, O_R, 0);//该文件必须是从头开始录的
			if (pfile == NULL)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, pfile_name);

				break;
			}

			file_residue_video_frame_nums = custommp4_video_length(pfile);
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d, file_residue_video_frame_nums: %d\n", 
				__func__, chn, file_residue_video_frame_nums);			
		}


		//read frame from file,  and send it
		
		//组合sps pps I帧版本，解决AVI播放卡顿
		readlen = custommp4_read_one_media_frame2(pfile, (u8 *)pbuf, buf_size, &start_time, &key, &pts, &media_type, &read_frame_cnt);
		if (readlen <= 0)
		{
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, custommp4_read failed,errcode=%d,errstr=%s\n",
					__func__, chn, errno, strerror(errno));
						
			break;
		}

		if(0 == media_type) //video frame
		{
			file_residue_video_frame_nums -= read_frame_cnt;//总帧数递减
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, read_frame_cnt: %d, file_residue_video_frame_nums: %d\n", 
					__func__, chn, read_frame_cnt, file_residue_video_frame_nums);
			*/
			//处理帧间隔延时
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			if (pre_frame_pts == 0)//start transfer
			{
				sleep_us = 40*1000; //I frame
			}
			else
			{
				if (key)
				{
					sleep_us = 40*1000; //I frame
				}
				else
				{
					sleep_us = PFrameInterval*1000; //P frame
				}

				//精确延时
				if (pre_frame_pts < pts)//normal
				{
					if (pre_frame_pts+sleep_us > pts)
					{
						sleep_us = pre_frame_pts + sleep_us - pts;
					}
					else
					{
						sleep_us = 0;
					}
				}
				else //u64 rewind
				{
					sleep_us = 30*1000;//简单处理
				}
			}
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d pre_frame_pts: %llu, sleep_us: %u, pts: %llu\n",
					__func__, chn, pre_frame_pts, sleep_us, pts);
			*/
			usleep(sleep_us);
			
			//延时结束，发送该帧
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d send frame, type: %c, len: %d, pts_ms: %llu\n",
					__func__, chn, key? 'I':'P', readlen, pts/1000);
			*/
			//frame callback
			memset(&stream, 0, sizeof(stream));
			stream.chn = chn;
			stream.data = (u8 *)pbuf;
			stream.len = readlen;
			stream.pts = pts;
			stream.media_type = MEDIA_PT_H264;
			if(key)
			{
				stream.frame_type = REAL_FRAME_TYPE_I;
			}
			else
			{
				stream.frame_type = REAL_FRAME_TYPE_P;
			}

			if(chn < (int)(g_anni_client_count/2))
			{
				stream.width = 1920;
				stream.height = 1080;//主码流
			}
			else
			{
				stream.width = 704;
				stream.height = 576;//子码流
			}
			
			g_annic_info[chn].pStreamCB(&stream, g_annic_info[chn].dwContext);

			pre_frame_pts = stream.pts;
		}
	}

ProcOver:
	printf("%s quit, chn: %d\n", __func__, chn);
	
	if (pfile)
	{
		custommp4_close(pfile);
		pfile = NULL;
	}

	if(pbuf)
	{
		free(pbuf);
		pbuf = NULL;
	}
	
	return 0;
}


int IpcTest_Init(unsigned int max_client_num)
{
	printf("%s max_client_num: %d\n", __func__, max_client_num);
	
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	g_manager_info = (manager_info *)malloc(sizeof(manager_info));
	memset(g_manager_info, 0, sizeof(manager_info));

	
	g_manager_info->thread_running;
	g_manager_info->client_count = max_client_num;
	g_manager_info->bit_stream_chn = 0;
	g_manager_info->pStreamCB = NULL;

	int i;
	for (i=0; i<2; ++i)
	{
		g_manager_info->thread_pid[i] = INVALID_VAL;		
	}

	for (i=0; i<2; ++i)
	{
		g_manager_info->thread_pid[i];		
	}
		//g_annic_info[i].eventLoopWatchVariable = 0;
		
	pthread_mutex_init(&g_annic_info[i].lock, NULL);
	
	init_flag;
	
	return 0;
}

int IpcTest_DeInit()
{
	printf("%s\n", __func__);
	
	return 0;
}


int IpcTest_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s\n", __func__);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	pnw->ip_address = ipcam->dwIp;
	pnw->net_mask = ipcam->net_mask;
	pnw->net_gateway = ipcam->net_gateway;
	pnw->dns1 = ipcam->dns1;
	pnw->dns2 = ipcam->dns2;
	
	return 0;
	
}

int IpcTest_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s do nothing!\n", __func__);
	return 0;
}

int IpcTest_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	printf("%s chn: %d\n", __func__, chn);

	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	if(IpcTest_GetLinkStatus(chn))
	{
		IpcTest_Stop(chn);
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	g_annic_info[chn].pStreamCB = pCB;
	g_annic_info[chn].dwContext = dwContext;
	
	/*
	g_annic_info[chn].pfilename = pfile_name;
	
	g_annic_info[chn].pfile = custommp4_open(g_annic_info[chn].pfilename, O_R, 0);//该文件必须是从头开始录的
	if (g_annic_info[chn].pfile == NULL)
	{
		printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, g_annic_info[chn].pfilename);

		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	*/
	
	if(pthread_create(&g_annic_info[chn].pid, NULL, ThreadPROC, (void *)chn) != 0)
	{
		printf("error: %s chn: %d, pthread_create failed\n", __func__, chn);
		
		g_annic_info[chn].pid = INVALID_VAL;
		//custommp4_close(g_annic_info[chn].pfile);
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	g_annic_info[chn].connected = 1;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);

	return 0;	
}

int IpcTest_Stop(int chn)
{
	printf("%s chn: %d\n", __func__, chn);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	if (!g_annic_info[chn].connected)
	{
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		return 0;
	}
	
	if (g_annic_info[chn].pid != INVALID_VAL)
	{
		g_annic_info[chn].eventLoopWatchVariable = 1;
		
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		pthread_join(g_annic_info[chn].pid, NULL);
		
		pthread_mutex_lock(&g_annic_info[chn].lock);
	}	
	
	g_annic_info[chn].pid = INVALID_VAL;
	g_annic_info[chn].eventLoopWatchVariable = 0;
	g_annic_info[chn].connected = 0;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}


//return value : 1 - Link; 0 - Lost
int IpcTest_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	//status = (g_annic_info[chn].cap_fd != INVALID_SOCKET);
	status = g_annic_info[chn].connected;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return status;
}

#else
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
	//int cap_fd;//数据通道SOCKET
	//custommp4_t* pfile;
	//const char *pfilename;
	int connected;
	//int thread_running;
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	int video_width;
	int video_height;
	volatile char eventLoopWatchVariable;
	pthread_t pid;
	//pthread_cond_t CondThreadExit;
	pthread_mutex_t lock;
}client_info;


static client_info *g_annic_info = NULL;

static unsigned int g_anni_client_count = 0;

static unsigned char g_init_flag = 0;

#define PFrameInterval	(20) //只改变P帧之间的间隔时间(ms)
void* ThreadPROC(void* pParam)
{
	char *pbuf = NULL;
	u32 buf_size = 0;
	const char *pfile_name = NULL;
	custommp4_t *pfile = NULL;
	u32 file_residue_video_frame_nums = 0;
	u8 read_frame_cnt = 0;
	real_stream_s stream;
	u8 media_type = 0;
	u64 pts = 0;
	u8 key = 0;
	u32 start_time = 0;
	int readlen = 0;
	struct timeval cur_tm; //当前时间
	u64 pre_frame_pts = 0; //上一帧发送的时间
	u32 sleep_us = 0; //两帧之间的间隔
	int chn = (int)pParam;
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		goto ProcOver;
	}
	
	if(chn < (int)(g_anni_client_count/2))
	{
		buf_size = 768*1024;
		pfile_name = MAIN_STREAM_FILE;//主码流
	}
	else
	{
		buf_size = 200*1024;
		pfile_name = SUB_STREAM_FILE;//子码流
	}
	
	pbuf = (char *)malloc(buf_size);
	if(pbuf == NULL)
	{
		goto ProcOver;
	}

	if(chn < (int)(g_anni_client_count/2))
	printf("%s running, chn: %d, file name: %s\n", __func__, chn, pfile_name);
	
	while (g_init_flag)
	{
		pthread_mutex_lock(&g_annic_info[chn].lock);
		if(g_annic_info[chn].eventLoopWatchVariable)
		{
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d eventLoopWatchVariable == 1, will be exit\n", __func__, chn);
			
			pthread_mutex_unlock(&g_annic_info[chn].lock);
			break;
		}
		pthread_mutex_unlock(&g_annic_info[chn].lock);

		if (file_residue_video_frame_nums == 0)
		{
			
			//线程刚运行，还没有传输
			//或者文件传输完
			//关闭文件再次打开
			if (pfile)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer over, reopen it\n", __func__, chn);
				
				custommp4_close(pfile);
				pfile = NULL;
			}
			else
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer start\n", __func__, chn);
			}

			pfile = custommp4_open(pfile_name, O_R, 0);//该文件必须是从头开始录的
			if (pfile == NULL)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, pfile_name);

				break;
			}

			file_residue_video_frame_nums = custommp4_video_length(pfile);
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d, file_residue_video_frame_nums: %d\n", 
				__func__, chn, file_residue_video_frame_nums);			
		}


		//read frame from file,  and send it
		
		//组合sps pps I帧版本，解决AVI播放卡顿
		readlen = custommp4_read_one_media_frame2(pfile, (u8 *)pbuf, buf_size, &start_time, &key, &pts, &media_type, &read_frame_cnt);
		if (readlen <= 0)
		{
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, custommp4_read failed,errcode=%d,errstr=%s\n",
					__func__, chn, errno, strerror(errno));
						
			break;
		}

		if(0 == media_type) //video frame
		{
			file_residue_video_frame_nums -= read_frame_cnt;//总帧数递减
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, read_frame_cnt: %d, file_residue_video_frame_nums: %d\n", 
					__func__, chn, read_frame_cnt, file_residue_video_frame_nums);
			*/
			//处理帧间隔延时
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			if (pre_frame_pts == 0)//start transfer
			{
				sleep_us = 40*1000; //I frame
			}
			else
			{
				if (key)
				{
					sleep_us = 40*1000; //I frame
				}
				else
				{
					sleep_us = PFrameInterval*1000; //P frame
				}

				//精确延时
				if (pre_frame_pts < pts)//normal
				{
					if (pre_frame_pts+sleep_us > pts)
					{
						sleep_us = pre_frame_pts + sleep_us - pts;
					}
					else
					{
						sleep_us = 0;
					}
				}
				else //u64 rewind
				{
					sleep_us = 30*1000;//简单处理
				}
			}
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d pre_frame_pts: %llu, sleep_us: %u, pts: %llu\n",
					__func__, chn, pre_frame_pts, sleep_us, pts);
			*/
			usleep(sleep_us);
			
			//延时结束，发送该帧
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d send frame, type: %c, len: %d, pts_ms: %llu\n",
					__func__, chn, key? 'I':'P', readlen, pts/1000);
			*/
			//frame callback
			memset(&stream, 0, sizeof(stream));
			stream.chn = chn;
			stream.data = (u8 *)pbuf;
			stream.len = readlen;
			stream.pts = pts;
			stream.media_type = MEDIA_PT_H264;
			if(key)
			{
				stream.frame_type = REAL_FRAME_TYPE_I;
			}
			else
			{
				stream.frame_type = REAL_FRAME_TYPE_P;
			}

			if(chn < (int)(g_anni_client_count/2))
			{
				stream.width = 1920;
				stream.height = 1080;//主码流
			}
			else
			{
				stream.width = 704;
				stream.height = 576;//子码流
			}
			
			g_annic_info[chn].pStreamCB(&stream, g_annic_info[chn].dwContext);

			pre_frame_pts = stream.pts;
		}
	}

ProcOver:
	printf("%s quit, chn: %d\n", __func__, chn);
	
	if (pfile)
	{
		custommp4_close(pfile);
		pfile = NULL;
	}

	if(pbuf)
	{
		free(pbuf);
		pbuf = NULL;
	}
	
	return 0;
}


int IpcTest_Init(unsigned int max_client_num)
{
	printf("%s max_client_num: %d\n", __func__, max_client_num);
	
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	if(g_init_flag)
	{
		return 0;
	}
	
	g_anni_client_count = max_client_num;
	
	g_annic_info = (client_info *)malloc(g_anni_client_count*sizeof(client_info));
	if(g_annic_info == NULL)
	{
		return -1;
	}
	memset(g_annic_info, 0, g_anni_client_count*sizeof(client_info));

	int i = 0;	
	for(i = 0; i < (int)g_anni_client_count; i++)
	{
		//g_annic_info[i].cap_fd = INVALID_SOCKET;
		//g_annic_info[i].pfile = NULL;
		//g_annic_info[i].pfilename = NULL;
		g_annic_info[i].pid = INVALID_VAL;
		//g_annic_info[i].eventLoopWatchVariable = 0;
		
		pthread_mutex_init(&g_annic_info[i].lock, NULL);
	}
	
	g_init_flag = 1;
	
	return 0;
}

int IpcTest_DeInit()
{
	printf("%s\n", __func__);
	
	return 0;
}


int IpcTest_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s\n", __func__);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	pnw->ip_address = ipcam->dwIp;
	pnw->net_mask = ipcam->net_mask;
	pnw->net_gateway = ipcam->net_gateway;
	pnw->dns1 = ipcam->dns1;
	pnw->dns2 = ipcam->dns2;
	
	return 0;
	
}

int IpcTest_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s do nothing!\n", __func__);
	return 0;
}

int IpcTest_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	printf("%s chn: %d\n", __func__, chn);

	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	if(IpcTest_GetLinkStatus(chn))
	{
		IpcTest_Stop(chn);
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	g_annic_info[chn].pStreamCB = pCB;
	g_annic_info[chn].dwContext = dwContext;
	
	/*
	g_annic_info[chn].pfilename = pfile_name;
	
	g_annic_info[chn].pfile = custommp4_open(g_annic_info[chn].pfilename, O_R, 0);//该文件必须是从头开始录的
	if (g_annic_info[chn].pfile == NULL)
	{
		printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, g_annic_info[chn].pfilename);

		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	*/
	
	if(pthread_create(&g_annic_info[chn].pid, NULL, ThreadPROC, (void *)chn) != 0)
	{
		printf("error: %s chn: %d, pthread_create failed\n", __func__, chn);
		
		g_annic_info[chn].pid = INVALID_VAL;
		//custommp4_close(g_annic_info[chn].pfile);
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	g_annic_info[chn].connected = 1;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);

	return 0;	
}

int IpcTest_Stop(int chn)
{
	printf("%s chn: %d\n", __func__, chn);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	if (!g_annic_info[chn].connected)
	{
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		return 0;
	}
	
	if (g_annic_info[chn].pid != INVALID_VAL)
	{
		g_annic_info[chn].eventLoopWatchVariable = 1;
		
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		pthread_join(g_annic_info[chn].pid, NULL);
		
		pthread_mutex_lock(&g_annic_info[chn].lock);
	}	
	
	g_annic_info[chn].pid = INVALID_VAL;
	g_annic_info[chn].eventLoopWatchVariable = 0;
	g_annic_info[chn].connected = 0;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}


//return value : 1 - Link; 0 - Lost
int IpcTest_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	//status = (g_annic_info[chn].cap_fd != INVALID_SOCKET);
	status = g_annic_info[chn].connected;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return status;
}

#endif



