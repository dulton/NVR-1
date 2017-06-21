#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//add by liu 
#include <time.h>

#include "common_basetypes.h"

#include "ipcamera.h"
#include "openRTSP.h"
#include "I13.h"
#include "klw.h"
#include "xm.h"
#include "fullhan.h"
#include "Anni.h"
#include "river.h"
#include "IpcTest.h"

#include "vvv_devs.h"
#include "vvv_common_str.h"


//#define TEST_WIFI_AP

#ifdef __cplusplus
extern "C" {
#endif

#include "yzonvif.h"

#ifndef WIN32
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#endif

unsigned int GetLocalIp();

#ifdef __cplusplus
}
#endif

#include "ls_discovery.h"
#include <iostream>
using namespace std;

extern int Onvif_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
extern int Onvif_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

extern int Onvif_CMD_Open(int chn);
extern int Onvif_CMD_Close(int chn);
//zcm modify
extern int Onvif_CMD_SetImageParam(int chn, video_image_para_t *para);
extern int Onvif_CMD_GetImageParam(int chn, video_image_para_t *para);

extern int Onvif_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
extern int Onvif_CMD_SetTime(int chn, time_t t, int force);
extern int Onvif_CMD_Reboot(int chn);
extern int Onvif_CMD_RequestIFrame(int chn);

extern int IPC_URL(ipc_unit *ipcam, unsigned char stream_type, char *rtspURL, int *w, int *h);

//static pthread_mutex_t onvif_lock = PTHREAD_MUTEX_INITIALIZER;

#define ONVIF_LOCK()	//pthread_mutex_lock(&onvif_lock)
#define ONVIF_UNLOCK()	//pthread_mutex_unlock(&onvif_lock)

typedef struct
{
	ipc_unit cam;
	ipc_unit newcam;
	unsigned char changed;
	ipc_cmd_op_set ops;
	ipc_param param;
	pthread_mutex_t lock;
	pthread_mutex_t set_param_lock;
}ipc_chn_info;

typedef struct
{
	real_stream_op_set ops;
	int video_width;
	int video_height;
	unsigned int frame_count;
	unsigned int last_frame_count;
	unsigned int link_failed_count;
	time_t last_t;//csp modify 20140423
	pthread_mutex_t lock;
	//add by liu
	unsigned int streamKbps;
}real_stream_info;

//add by liu
typedef struct
{
	unsigned long preTime;
	unsigned int preStreamLen;
	unsigned long totalStreamLen;
	unsigned int streamKbps;
}cnt_stream_info;

//yaogang modify 20140918
//static cnt_stream_info g_stream_cnt[16];
static cnt_stream_info *g_stream_cnt;

static unsigned int g_chn_count = 0;//16: 0-15

static ipc_chn_info *g_chn_info = NULL;
static real_stream_info *g_stream_info = NULL;

static unsigned char g_init_flag = 0;

static RealStreamCB g_pStreamCB = NULL;
static StreamStateCB g_pStateCB = NULL;

static pthread_t g_pid;

//add by liu
typedef struct recordStreamInfo{
	unsigned int preKbps;
	unsigned char Samenum;
}RECSTRINFO;
RECSTRINFO g_recordStreamInfo[32];

int is_ipcamera_same(ipc_unit *cam1, ipc_unit *cam2)
{
	if(cam1 == NULL || cam2 == NULL)
	{
		return 0;
	}
	if(cam1->channel_no != cam2->channel_no)
	{
		return 0;
	}
	if(cam1->enable != cam2->enable)
	{
		return 0;
	}
	if(cam1->protocol_type != cam2->protocol_type)
	{
		return 0;
	}
	if(cam1->trans_type != cam2->trans_type)
	{
		return 0;
	}
	if(cam1->dwIp != cam2->dwIp)
	{
		return 0;
	}
	if(cam1->wPort != cam2->wPort)
	{
		return 0;
	}
	if(strcmp(cam1->user, cam2->user))
	{
		return 0;
	}
	if(strcmp(cam1->pwd, cam2->pwd))
	{
		return 0;
	}
	//yaogang modify 20140918
	if ((cam1->protocol_type == PRO_TYPE_ONVIF) \
		&&(cam2->protocol_type == PRO_TYPE_ONVIF))
	{
		if (strcmp(cam1->address, cam2->address))
			return 0;
	}
	//yaogang modify 20140918
	if (strlen(cam1->uuid) && strlen(cam2->uuid))
	{
		if(strcmp(cam1->uuid, cam2->uuid))
		{
			printf("yg is_ipcamera_same cam1.uuid: %s\n", cam1->uuid);
			printf("yg is_ipcamera_same cam2.uuid: %s\n", cam2->uuid);
			return 0;
		}
	}
	
	return 1;
}
void *camSetParamFxn(void *arg)

{
	printf("camSetParamFxn, pid:%ld\n", pthread_self());
	
	unsigned int count = 0;
	
	while(g_init_flag)
	{
		count++;
		
		int i = 0;
		for(i = 0; i < (int)g_chn_count; i++)
		{
			if(!g_chn_info[i].newcam.enable)
			{
				continue;
			}
			if (g_chn_info[i].param.bReboot)
			{
				Cmd_Reboot Reboot = g_chn_info[i].ops.Reboot;
				if(Reboot != NULL)
				{
					if (0 == Reboot(i))
					{
						g_chn_info[i].param.bReboot = 0;
					}
					else
					{
						printf("camSetParamFxn bReboot failed\n");
					}
				}
				else
				{
					g_chn_info[i].param.bReboot = 0;
				}
				g_chn_info[i].param.bReboot = 0;
			}
			
			if(g_chn_info[i].param.osd.refresh)
			//if(0)
			{
				Cmd_SetOSD SetOSD = g_chn_info[i].ops.SetOSD;
				if(SetOSD != NULL)
				{
					if (0 == SetOSD(i, g_chn_info[i].param.osd.name))
					{
						g_chn_info[i].param.osd.refresh = 0;
					}
					else
					{
						printf("camSetParamFxn SetOSD failed\n");
					}
				}
				else
				{
					g_chn_info[i].param.osd.refresh = 0;
				}
			}
			
			//csp modify 20140812
			if(g_chn_info[i].param.mainenc.refresh)
			{
				Cmd_SetVENC SetVENC = g_chn_info[i].ops.SetVENC;
				if(SetVENC != NULL)
				{
					if (0 == SetVENC(i, 0, &g_chn_info[i].param.mainenc.para))
					{
						g_chn_info[i].param.mainenc.refresh = 0;
					}
					else
					{
						printf("camSetParamFxn main SetVENC failed\n");
					}
				}
				else
				{
					g_chn_info[i].param.mainenc.refresh = 0;
				}
			}
			
			//csp modify 20140812
			if(g_chn_info[i].param.subenc.refresh)
			{
				Cmd_SetVENC SetVENC = g_chn_info[i].ops.SetVENC;
				if(SetVENC != NULL)
				{
					if (0 == SetVENC(i, 1, &g_chn_info[i].param.subenc.para))
					{
						g_chn_info[i].param.subenc.refresh = 0;
					}
					else
					{
						printf("camSetParamFxn sub SetVENC failed\n");
					}
				}
				else
				{
					g_chn_info[i].param.subenc.refresh = 0;
				}
			}
			
			if(g_chn_info[i].param.md.refresh)
			//if(0)
			{
				Cmd_SetMD SetMD = g_chn_info[i].ops.SetMD;
				if(SetMD != NULL)
				{
					if (0 == SetMD(i, &g_chn_info[i].param.md.para))
					{
						g_chn_info[i].param.md.refresh = 0;
					}
					else
					{
						printf("camSetParamFxn SetMD failed\n");
					}
				}
				else
				{
					g_chn_info[i].param.md.refresh = 0;
				}
			}

			/*
			if(g_chn_info[i].param.md.para.nEnable && (count & 1) && g_chn_info[i].newcam.protocol_type == PRO_TYPE_ONVIF)
			//if(0)
			{
				OnvifDevice *pOnvifDevice = &g_chn_info[i].odev;
				
				if(pOnvifDevice->mIsOk == 1 && pOnvifDevice->EventSubscribeXAddrs[0] != 0 && pOnvifDevice->AnalyticsXAddrs[0] != 0 && pOnvifDevice->mIsMotionSupot)
				{
					int ret = EventServiceEnable(pOnvifDevice);
					if(ret < 0)
					{
						pOnvifDevice->mIsOk = 2;
						printf("chn%d enable event server error\n", i);
					}
					else
					{
						//printf("chn%d enable event server ok\n", i);
						
						ret = EventPullMsg(pOnvifDevice);
						if(ret == 0)
						{
							if(pOnvifDevice->mIsMotion)
							{
								printf("\nchn%d motion detect\n", i);
							}
							else
							{
								//printf("\nchn%d has no motion\n", i);
							}
						}
						else
						{
							printf("chn%d pull msg error\n", i);
						}
					}
				}
			}
			*/
		}
		
		sleep(3);
	}
	
	return 0;
}

void *videoConnFxn(void *arg)
{
	printf("videoConnFxn, pid:%ld\n", pthread_self());
	
	unsigned int loop = 0;
	unsigned int n = 0;
	
	while(g_init_flag)
	{
		unsigned char reconn = 0;
		
		int i = 0;
		for(i = 0; i < (int)g_chn_count; i++)
		{
			ipc_unit ipcam;
			unsigned char changed = 0;
			
			pthread_mutex_lock(&g_chn_info[i].lock);
			
			ipcam = g_chn_info[i].newcam;
			changed = g_chn_info[i].changed;
			
			if(changed)
			{
				g_chn_info[i].changed = 0;
				if(!is_ipcamera_same(&ipcam, &g_chn_info[i].cam))
				{
					printf("videoConnFxn, chn%d param changed......\n", i);
					g_chn_info[i].cam = ipcam;
				}
				else
				{
					printf("videoConnFxn, chn%d param same......\n", i);
					changed = 0;
				}
			}
			
			pthread_mutex_unlock(&g_chn_info[i].lock);
			
			if(changed)
			{
				//printf("videoConnFxn, main chn%d stop\n",i);
				printf("%s stop chn%d & chn%d, pthread_id: %u\n", __func__, i, i+g_chn_count, pthread_self());
				IPC_Stop(i);
				
				//printf("videoConnFxn, sub chn%d stop\n",i);
				
				IPC_Stop(i+g_chn_count);
				
				//printf("videoConnFxn, chn%d stop over\n",i);
				
				reconn = 1;
			}
		}
		
		usleep(100*1000);
		
		if(!reconn)
		{
			//if(++loop != 30)
			if(++loop != 50)
			{
				continue;
			}
			n++;
		}
		loop = 0;
		
		//int j = 0;
		//for(j = 0; j < 2; j++)
		{
			for(i = 0; i < (int)g_chn_count; i++)
			{
				ipc_unit *ipcam = &g_chn_info[i].cam;
				
				int j = 0;
				for(j = 0; j < 2; j++)
				{
					int channel = i + (j * g_chn_count);
					
					unsigned char byCurStreamOpen = 0;
					if(ipcam->enable)
					{
						byCurStreamOpen = 1;
					}
					
					if(!byCurStreamOpen)
					{
						g_stream_info[channel].link_failed_count = 0;
						
						if(IPC_GetLinkStatus(channel))
						{
							printf("chn%d stop-1...\n",channel);
							IPC_Stop(channel);
						}
					}
					else
					{
						if(!IPC_GetLinkStatus(channel))
						{
							g_stream_info[channel].link_failed_count++;
							
							if(g_stream_info[channel].link_failed_count > 20)
							{
								//g_stream_info[channel].link_failed_count = 0;
								g_stream_info[i].link_failed_count = 0;
								g_stream_info[i+g_chn_count].link_failed_count = 0;
								
								pthread_mutex_lock(&g_stream_info[channel].lock);
								
								if(ipcam->protocol_type == PRO_TYPE_ONVIF)
								{
									char rtspURL[128] = {0};
									int w = 0;
									int h = 0;
									if(IPC_URL(ipcam, (channel < (int)g_chn_count)?STREAM_TYPE_MAIN:STREAM_TYPE_SUB, rtspURL, &w, &h) == 0)
									{
										g_stream_info[channel].video_width = 0;//w;
										g_stream_info[channel].video_height = 0;//h;
										
										pthread_mutex_unlock(&g_stream_info[channel].lock);
										
										printf("##########chn%d onvif ipcamera reboot##########\n",i);
										
										IPC_CMD_Reboot(i);
									}
									else
									{
										pthread_mutex_unlock(&g_stream_info[channel].lock);
									}
								}
								else
								{
									g_stream_info[channel].video_width = 0;
									g_stream_info[channel].video_height = 0;
									
									pthread_mutex_unlock(&g_stream_info[channel].lock);
									
									printf("##########chn%d private ipcamera reboot##########\n",i);
									
									IPC_CMD_Reboot(i); //yaogang
								}
							}
							else
							{
								//struct timeval tv1;
								//gettimeofday(&tv1,NULL);
								
								//if (channel == 16)
									//printf("videoConnFxn chn%d IPC_Start-1, tv1:%u.%u\n", channel, tv1.tv_sec, tv1.tv_usec);
								//if (channel < 16)
									IPC_Start(channel);
								//gettimeofday(&tv1,NULL);

								//if (channel == 16)
									//printf("videoConnFxn chn%d IPC_Start-2, tv1:%u.%u\n", channel, tv1.tv_sec, tv1.tv_usec);
							}
						}
						else
						{
							g_stream_info[channel].link_failed_count = 0;
							
							if(n%3 == 0)
							{
								if(g_stream_info[channel].frame_count == g_stream_info[channel].last_frame_count)
								{
									if(g_stream_info[channel].frame_count)
									{
										printf("chn%d stop-2...\n",channel);
										IPC_Stop(channel);
									}
									else
									{
										if(n%30 == 0)
										{
											printf("chn%d stop-3...\n",channel);
											IPC_Stop(channel);
										}
									}
								}
								
								g_stream_info[channel].last_frame_count = g_stream_info[channel].frame_count;
							}
						}
					}
				}
				
				usleep(1);
			}
		}
	}
	
	return 0;
}

int IPC_Init(unsigned int chn_num)
{
	if(chn_num <= 0)
	{
		return -1;
	}
	
	if(g_init_flag)
	{
		return 0;
	}
	
	g_chn_count = chn_num;
	
	g_chn_info = (ipc_chn_info *)malloc(g_chn_count*sizeof(ipc_chn_info));
	if(g_chn_info == NULL)
	{
		return -1;
	}
	memset(g_chn_info,0,g_chn_count*sizeof(ipc_chn_info));
	
	g_stream_info = (real_stream_info *)malloc(g_chn_count*2*sizeof(real_stream_info));
	if(g_stream_info == NULL)
	{
		free(g_chn_info);
		g_chn_info = NULL;
		return -1;
	}
	memset(g_stream_info,0,g_chn_count*2*sizeof(real_stream_info));
//yaogang modify 20140918
	g_stream_cnt = (cnt_stream_info *)malloc(g_chn_count*2*sizeof(cnt_stream_info));
	if(g_stream_cnt == NULL)
	{
		free(g_stream_info);
		g_stream_info = NULL;
		free(g_chn_info);
		g_chn_info = NULL;
		return -1;
	}
	memset(g_stream_cnt, 0, g_chn_count*2*sizeof(cnt_stream_info));

	
	int i = 0;
	//add by liu
	for(i=0;i<(int)g_chn_count;i++)
	{
		memset(&g_stream_cnt[i],0,sizeof(cnt_stream_info));
		memset(&g_recordStreamInfo[i],0,sizeof(RECSTRINFO));
	}
	
	for(i = 0; i < (int)g_chn_count; i++)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_chn_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	for(i = 0; i < (int)g_chn_count; i++)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_chn_info[i].set_param_lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	for(i = 0; i < (int)(g_chn_count*2); i++)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_stream_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	RTSPC_Init(g_chn_count*2);
	//I13_Init(g_chn_count*2);
	KLW_Init(g_chn_count*2);

	XM_Init(g_chn_count*2);
	//FH_Init(g_chn_count*2);
	//Anni_Init(g_chn_count*2);
	//RIVER_Init(g_chn_count*2);
#ifdef IPC_PROTOCOL_TEST
	IpcTest_Init(g_chn_count*2);
#endif

	
	
	g_init_flag = 1;
	
	pthread_create(&g_pid, NULL, videoConnFxn, NULL);
	pthread_create(&g_pid, NULL, camSetParamFxn, NULL);
	
	return 0;
}

int IPC_DeInit()
{
	return 0;
}

static unsigned char g_tz_index = 27;
static unsigned char g_sync_time = 0;

int IPC_SetTimeZone(int nTimeZone, int syncflag, int syncing)
{
	//printf("IPC_SetTimeZone-1\n");
	
	g_sync_time = syncflag;
	
	if(g_tz_index != nTimeZone)
	{
		g_tz_index = nTimeZone;
	}
	
	//printf("IPC_SetTimeZone-2,g_sync_time=%d,syncing=%d\n",g_sync_time,syncing);
	
	//csp modify 20140406
	if(g_sync_time && syncing)
	{
		//printf("IPC_SetTimeZone-3\n");
		
		int i;
		for(i=0;i<(int)g_chn_count;i++)
		{
			//printf("IPC_SetTimeZone-4,chn=%d\n",i);
			
			IPC_CMD_SetTime(i, 0, 0);
		}
	}
	
	return g_tz_index;
}

int IPC_GetTimeZone()
{
	return g_tz_index;
}

int IPC_Set(int chn, ipc_unit *ipcam)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	//printf("chn%d set ipcam, locking...\n",chn);
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	//printf("chn%d set ipcam, start setting\n",chn);
	
	g_chn_info[chn].newcam = *ipcam;
	g_chn_info[chn].changed = 1;

	memset(&g_chn_info[chn].ops, 0, sizeof(g_chn_info[chn].ops));
	/*
	if(ipcam->protocol_type == PRO_TYPE_HAIXIN || ipcam->protocol_type == PRO_TYPE_JUAN)
	{
		g_chn_info[chn].ops.Open = I13_CMD_Open;
		g_chn_info[chn].ops.Close = I13_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = I13_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = I13_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = I13_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = I13_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.Reboot = I13_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = I13_CMD_RequestIFrame;
	}
	*/
	//else if(ipcam->protocol_type == PRO_TYPE_KLW)
	if(ipcam->protocol_type == PRO_TYPE_KLW)
	{
		g_chn_info[chn].ops.Open = KLW_CMD_Open;
		g_chn_info[chn].ops.Close = KLW_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = KLW_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = KLW_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = KLW_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = KLW_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = KLW_CMD_SetMD;//
		g_chn_info[chn].ops.SetOSD = KLW_CMD_SetOSD;//
		g_chn_info[chn].ops.SetVENC= KLW_CMD_SetVENC;//
		g_chn_info[chn].ops.GetVENC= KLW_CMD_GetVENC;//
		g_chn_info[chn].ops.Reboot = KLW_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = KLW_CMD_RequestIFrame;
		g_chn_info[chn].ops.GetAudioSwitchStatus = KLW_CMD_GetAudioSwitchStatus;
		g_chn_info[chn].ops.SetAudioSwitchStatus = KLW_CMD_SetAudioSwitchStatus;
		//g_chn_info[chn].ops.Snapshot_RegisterCB = KLW_CMD_Snapshot_RegisterCB;
	}
	else if(ipcam->protocol_type == PRO_TYPE_XM)
	{
		g_chn_info[chn].ops.Open = XM_CMD_Open;
		g_chn_info[chn].ops.Close = XM_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = XM_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = XM_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = XM_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = XM_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.Reboot = XM_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = XM_CMD_RequestIFrame;
	}
#ifdef IPC_PROTOCOL_TEST
	else if(ipcam->protocol_type == PRO_TYPE_IPC_TEST)
	{
		memset(&g_chn_info[chn].ops, 0, sizeof(g_chn_info[chn].ops));
	}
#endif
	/*
	else if(ipcam->protocol_type == PRO_TYPE_FULLHAN)
	{
		g_chn_info[chn].ops.Open = FH_CMD_Open;
		g_chn_info[chn].ops.Close = FH_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = FH_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = FH_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = FH_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = FH_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.Reboot = FH_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = FH_CMD_RequestIFrame;
	}
	else if(ipcam->protocol_type == PRO_TYPE_ANNI)
	{
		g_chn_info[chn].ops.Open = Anni_CMD_Open;
		g_chn_info[chn].ops.Close = Anni_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = Anni_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = Anni_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = Anni_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = Anni_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.Reboot = Anni_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = Anni_CMD_RequestIFrame;
	}
	else if(ipcam->protocol_type == PRO_TYPE_RIVER)
	{
		g_chn_info[chn].ops.Open = RIVER_CMD_Open;
		g_chn_info[chn].ops.Close = RIVER_CMD_Close;
		g_chn_info[chn].ops.SetImageParam = RIVER_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = RIVER_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = RIVER_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = RIVER_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.Reboot = RIVER_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = RIVER_CMD_RequestIFrame;
	}
	*/
	else
	{
		g_chn_info[chn].ops.Open = Onvif_CMD_Open;
		g_chn_info[chn].ops.Close = Onvif_CMD_Close;
		//zcm modify
		g_chn_info[chn].ops.SetImageParam = Onvif_CMD_SetImageParam;
		g_chn_info[chn].ops.GetImageParam = Onvif_CMD_GetImageParam;
		g_chn_info[chn].ops.PtzCtrl = Onvif_CMD_PtzCtrl;
		g_chn_info[chn].ops.SetTime = Onvif_CMD_SetTime;
		g_chn_info[chn].ops.SetMD = NULL;
		g_chn_info[chn].ops.SetOSD = NULL;
		g_chn_info[chn].ops.SetVENC = NULL;
		g_chn_info[chn].ops.GetVENC = NULL;
		g_chn_info[chn].ops.Reboot = Onvif_CMD_Reboot;
		g_chn_info[chn].ops.RequestIFrame = Onvif_CMD_RequestIFrame;
		//g_chn_info[chn].ops.Snapshot_RegisterCB = NULL;
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

int IPC_Get(int chn, ipc_unit *ipcam)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	*ipcam = g_chn_info[chn].newcam;
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

int IPC_GetStreamResolution(int chn, int *w, int *h)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		return -1;
	}
	
	if(w == NULL || h == NULL)
	{
		return -1;
	}
	
	*w = g_stream_info[chn].video_width;
	*h = g_stream_info[chn].video_height;
	
	return 0;
}

int IPC_SetStreamResolution(int chn, int w, int h)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		return -1;
	}
	
	g_stream_info[chn].video_width = w;
	g_stream_info[chn].video_height = h;
	
	return 0;
}

unsigned int ipc_ext_alarm;
unsigned int ipc_cover_alarm;

unsigned int IPC_get_alarm_IPCExt(void)
{
	unsigned int tmp;

	tmp = ipc_ext_alarm;
	ipc_ext_alarm = 0;
	
	return tmp;	
}
unsigned int  IPC_get_alarm_IPCCover(void)
{
	unsigned int tmp;

	tmp = ipc_cover_alarm;
	ipc_cover_alarm = 0;
	
	return tmp;
}
void IPC_set_alarm_IPCExt(int chn)
{
	ipc_ext_alarm |= (1<<chn);
}
void  IPC_set_alarm_IPCCover(int chn)
{
	ipc_cover_alarm |= (1<<chn);
}

int IPC_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_stream_info[chn].lock);
	
	if(g_stream_info[chn].ops.GetLinkStatus != NULL)
	{
		status = g_stream_info[chn].ops.GetLinkStatus(chn);
	}
	
	pthread_mutex_unlock(&g_stream_info[chn].lock);
	
	return status;
}

int DoStreamStateCallBack(int chn, real_stream_state_e msg)
{
	ipc_unit  ipcam;
	int main_chn = chn<g_chn_count ? chn : (chn-g_chn_count);
	
	if(g_pStateCB != NULL)
	{
		if(msg == REAL_STREAM_STATE_STOP || msg == REAL_STREAM_STATE_LOST)
		{
			if(chn < (int)(g_chn_count*2))
			{
				g_stream_info[chn].video_width = 0;
				g_stream_info[chn].video_height = 0;
			}
			#if 0
			//yaogang modify 20141012
			if (IPC_Get(main_chn, &ipcam) == 0)
			{
 				if (ipcam.protocol_type == PRO_TYPE_KLW)
				{
					printf("yg StateCB klw Stop chn%d\n", chn);
					//IPC_Stop(chn);
					//IPC_Stop(chn+16);
					//IPC_Stop(main_chn);
					//IPC_Stop(main_chn+g_chn_count);
				}
			}
			#endif
		}
		
		(g_pStateCB)(chn, msg);
	}
	
	return 0;
}

//add by liu chn:0-15
int Get_StreamKbps(unsigned int chn)
{
	unsigned int getKbps;
	if(chn < 0 || chn >= (unsigned int)(g_chn_count*2))
	{
		return -1;
	}
	//yaogang modify 20141128 
	/*
	修改原因:代码的两个地方都要获取同一把锁，造成了等待。
	1、已连接的IPC，拔下网线，videoConnFxn() 线程每隔一段时间调用IPC_Start()，
	试图重新连接IPC，而在IPC_Start()中要获得g_stream_info[chn].lock
	2、双击桌面上一个通道，
	会调用EventPreviewFreshDeal()--Get_StreamKbps()获得g_stream_info[chn].lock，等待。。。
	*/
	//pthread_mutex_lock(&g_stream_info[chn].lock);
	getKbps = g_stream_info[chn].streamKbps;
	//pthread_mutex_unlock(&g_stream_info[chn].lock);
//yaogang modify 判断通道断开的算法已修改
/*
	if(0 == getKbps)
		return 0;

	if(0 == g_recordStreamInfo[chn].Samenum)
	{
		g_recordStreamInfo[chn].preKbps = getKbps;
		g_recordStreamInfo[chn].Samenum = 1;
		return (int)getKbps;
	}
	if(getKbps == g_recordStreamInfo[chn].preKbps)
	{
		g_recordStreamInfo[chn].Samenum++;
		if(g_recordStreamInfo[chn].Samenum >= 20) //说明通道已断开
		{
			pthread_mutex_lock(&g_stream_info[chn].lock);
			g_stream_info[chn].streamKbps = 0;
			pthread_mutex_unlock(&g_stream_info[chn].lock);
			getKbps = 0;
			g_recordStreamInfo[chn].preKbps = 0;
			g_recordStreamInfo[chn].Samenum = 0;
		}
	}
	else
	{
		g_recordStreamInfo[chn].Samenum = 0;

	}
*/
	return (int)getKbps;
}

int DoRealStreamCallBack(real_stream_s *stream, unsigned int dwContext)
{
	if(stream == NULL || stream->chn != (int)dwContext)
	{
		printf("DoRealStreamCallBack: param error-1\n");
		return -1;
	}
	
	int chn = stream->chn;
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		printf("DoRealStreamCallBack: param error-2\n");
		return -1;
	}
	
	//add by liu
	//yaogang modify 20140819 主子码流都要统计
	//if(chn < g_chn_count) //统计主码流
	if (stream->media_type == MEDIA_PT_H264)
	{
		if(0 == g_stream_cnt[chn].preTime)
		{
			g_stream_cnt[chn].preTime = time(NULL);
			g_stream_cnt[chn].preStreamLen += stream->len;
			g_stream_cnt[chn].totalStreamLen += stream->len;
		}
		else
		{
			int nowTime = 0;
			double diftime = 0;
			nowTime = time(NULL);
			 
			g_stream_cnt[chn].totalStreamLen += stream->len;
			//得到两次机器时间差，单位为秒
			diftime = difftime(nowTime,g_stream_cnt[chn].preTime);
			if(diftime >= 7.0)
			{
				int StreamLen = 0;
				StreamLen = g_stream_cnt[chn].totalStreamLen - g_stream_cnt[chn].preStreamLen;
				g_stream_cnt[chn].streamKbps = (unsigned int)((double)StreamLen*8/(1024*diftime));
				g_stream_cnt[chn].preTime = nowTime;
				g_stream_cnt[chn].preStreamLen = g_stream_cnt[chn].totalStreamLen;
				//printf("DoRealStreamCallBack: stream[%d]-Kbps:%d\n",chn,g_stream_cnt[chn].streamKbps);

				pthread_mutex_lock(&g_stream_info[chn].lock);
				g_stream_info[chn].streamKbps = g_stream_cnt[chn].streamKbps;
				g_stream_info[chn].video_height = stream->height;
				g_stream_info[chn].video_width = stream->width;
				pthread_mutex_unlock(&g_stream_info[chn].lock);

				//yaogang modify 20140819
				/*
				if (stream->width == 0)
				{
					printf("yg DoRealStreamCallBack chn%d\n", chn);
				}
				*/
			}
		}
	}
	
	#if 0
	if(chn == 0)
	{
		if(stream->media_type == MEDIA_PT_H264)
		{
			time_t t = time(NULL);
			printf("DoRealStreamCallBack: chn%d video frame comming at %s",chn,ctime(&t));
		}
		else
		{
			time_t t = time(NULL);
			printf("DoRealStreamCallBack: chn%d audio frame comming at %s",chn,ctime(&t));
		}
	}
	#endif
	
	//if(chn == 3 || chn == 11) printf("DoRealStreamCallBack: chn%d here...\n", chn);
	
	g_stream_info[chn].frame_count++;
	
	//csp modify 20140423
	if((g_stream_info[chn].frame_count % 250) == 0)
	{
		//printf("chn%d span %lds\n",chn,time(NULL)-g_stream_info[chn].last_t);
		g_stream_info[chn].last_t = time(NULL);
	}
	
	if(g_pStreamCB != NULL)
	{
		return (g_pStreamCB)(stream, dwContext);
	}
	
	return 0;
}

int IPC_URL(ipc_unit *ipcam, unsigned char stream_type, char *rtspURL, int *w, int *h)
{
	//printf("IPC_URL-1\n");
	//struct timeval tv1;
	
	if(ipcam == NULL || rtspURL == NULL || w == NULL || h == NULL)
	{
		return -1;
	}
	
	*w = 0;
	*h = 0;
	
#ifdef TEST_WIFI_AP
	if(stream_type == 0)
	{
		strcpy(rtspURL, "rtsp://192.168.1.101/main.264");
		*w = 720;//1280;
		*h = 576;//720;
		return 0;
	}
	else if(stream_type == 1)
	{
		strcpy(rtspURL, "rtsp://192.168.1.101/sub.264");
		*w = 720;//1280;
		*h = 576;//720;
		return 0;
	}
#endif
	
	//printf("IPC_URL-2\n");
	
	int bFind = FALSE;
	
	ONVIF_LOCK();
	
#if 1
	//printf("IPC_URL-3\n");
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	//printf("IPC_URL-4\n");
	
	struct in_addr serv;
	serv.s_addr = ipcam->dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam->address, strlen(ipcam->address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam->user,ipcam->pwd);
	
	//printf("IPC_URL-4.1\n");
	
	vector<media_profile *> *pMediaProfileVector = ((device_ptz*)receiver_onvif)->get_profiles();

	//printf("IPC_URL-4.2\n");
	
	if(pMediaProfileVector == NULL || pMediaProfileVector->empty())
	{
		//printf("IPC_URL-4.2.1\n");
		
		delete receiver_onvif;
		ONVIF_UNLOCK();
		return -1;
	}

	//printf("IPC_URL-4.3\n");
	
	#if 0
	if(pMediaProfileVector != NULL)
	{
		vector<media_profile *>::iterator media_it = pMediaProfileVector->begin();
		
		int idx = 0;
		
		while(media_it != pMediaProfileVector->end())
		{
			printf("stream%d token:%s w:%d h:%d\n",idx,(*media_it)->stream_token,(*media_it)->resolution_width,(*media_it)->resolution_height);
			idx++;
			media_it++;
		}
	}
	#endif
	//gettimeofday(&tv1,NULL);
	//printf("%s 3 tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
	
	vector<std::string *> *pVector = ((device_ptz*)receiver_onvif)->get_stream_uri();

	//gettimeofday(&tv1,NULL);
	//printf("%s 4 tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
	//printf("IPC_URL-4.4\n");
	
	if(pVector == NULL || pVector->empty())
	{
		//printf("IPC_URL-4.4.1\n");
		
		delete receiver_onvif;
		ONVIF_UNLOCK();
		return -1;
	}
	
	//printf("IPC_URL-5,ipaddr=%s,address=%s\n",ipaddr,ipcam->address);
	
	vector<std::string *>::iterator uri_it = pVector->begin();
	vector<media_profile *>::iterator media_it = pMediaProfileVector->begin();
	
	int idx = 0;
	while(uri_it != pVector->end())
	{
		//printf("IPC_URL-5.1\n");
		
		//cout<<"stream"<<idx<<":"<<(*(*uri_it))<<endl;
		
		if(idx == stream_type)
		{
			strcpy(rtspURL, (*uri_it)->c_str());
			//printf("ipc address:%s,stream type:%d,url:%s,w:%d,h:%d\n",ipcam->address,stream_type,rtspURL,(*media_it)->resolution_width,(*media_it)->resolution_height);
			
			if(strstr(rtspURL, "jpeg") || strstr(rtspURL, "jpg"))
			{
				idx--;
			}
			else if(strncasecmp(rtspURL, "rtsp://", strlen("rtsp://")) != 0)
			{
				idx--;
			}
			else
			{
				bFind = TRUE;
				
				if(media_it != pMediaProfileVector->end())
				{
					*w = (*media_it)->resolution_width;
					*h = (*media_it)->resolution_height;
					//printf("ipc stream type:%d,url:%s,w:%d,h:%d\n",stream_type,rtspURL,*w,*h);
				}
				
				break;
			}
		}
		
		idx++;
		uri_it++;
		media_it++;
	}

	//printf("IPC_URL-6\n");
	
	delete receiver_onvif;
#else
	DeviceAddrs device; //one device address which we want to get its media address and ptz address
	device.addr = ipcam->address;
	
	MediaPtzAddr media_ptz;
	memset(&media_ptz, 0, sizeof(media_ptz));
	
	printf("############IPC [%s] [%s:%s] get media addr...\n", device.addr, ipcam->user, ipcam->pwd);
	
	if(!GetMediaPtzAddr(ipcam->user, ipcam->pwd, &device, &media_ptz))
	{
		printf("media profile:%s\n", media_ptz.media_addr);
		//printf("ptz profile:%s\n", media_ptz.ptz_addr);
		
		int j = 0;
		
		/* get profiletoken */
		ProfileToken profile;
		memset(&profile, 0, sizeof(profile));
		GetProfileToken(ipcam->user, ipcam->pwd, &media_ptz, &profile);
		printf("token num:%d\n", profile.len);
		for(j = 0; j < profile.len; j++)
		{
			//printf("token[%d] : %s\n", j, profile.token[j]);
		}
		
		/* get stream uri */
		//media->addr and profileToken are needed
		MediaUri uri;
		memset(&uri, 0, sizeof(uri));
		GetStreamUri(ipcam->user, ipcam->pwd, &media_ptz, &profile, &uri);
		printf("url num:%d\n", uri.len);
		for(j = 0; j < uri.len; j++)
		{
			//printf("url[%d] : %s\n", j, uri.uri[j]);
			
			if(j == stream_type)
			{
				strcpy(rtspURL, uri.uri[j]);
				printf("ipc stream type:%d,url:%s\n",stream_type,rtspURL);
				
				if(strncasecmp(rtspURL, "rtsp://", strlen("rtsp://")) == 0)
				{
					bFind = TRUE;
				}
				
				break;
			}
		}
		
		DestoryProfileToken(&profile);
		DestoryMediaUri(&uri);
		
		if(!bFind)
		{
			//printf("IPC [%s] get media addr failed\n", device.addr);
		}
	}
	else
	{
		printf("IPC [%s] get media addr error\n", device.addr);
	}
	
	DestoryMediaAddr(&media_ptz);
	
	printf("@@@@@@@@@@@@IPC [%s] get media addr over:%s\n", device.addr, bFind?"success":"failed");
#endif
	
	ONVIF_UNLOCK();
	
	if(!bFind)
	{
		return -1;
	}
	
	return 0;
}

int IPC_Start(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	//printf("IPC_Start: chn=%d\n", chn);
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		printf("%s chn%d invalid\n", __func__, chn);
		return -1;
	}
	
	//printf("IPC_Start chn%d\n", chn);
	
	ipc_unit *ipcam = NULL;
	int main_chn = chn<(int)g_chn_count ? chn : chn-(int)g_chn_count;

	//printf("%s lock1 main_chn%d\n", __func__, main_chn);
	//pthread_mutex_lock(&g_chn_info[main_chn].lock);
	//printf("%s lock2 main_chn%d\n", __func__, main_chn);
	ipcam = &g_chn_info[main_chn].cam;
	if(!ipcam->enable)
	{
		//printf("%s unlock1 main_chn%d\n", __func__, main_chn);
		//pthread_mutex_unlock(&g_chn_info[main_chn].lock);
		//printf("%s unlock2 main_chn%d\n", __func__, main_chn);
		//printf("%s chn%d, main_chn%d, ipcam->enable == 0\n", __func__, chn, main_chn);
		return -1;
	}
	//printf("%s unlock1 main_chn%d\n", __func__, main_chn);
	//pthread_mutex_unlock(&g_chn_info[main_chn].lock);
	//printf("%s unlock2 main_chn%d\n", __func__, main_chn);
	
	//printf("IPC_Start: chn%d enable:%d protocol_type:%d\n", chn, ipcam->enable, ipcam->protocol_type);

	int ret = -1;
	
	//printf("IPC_Start: chn%d protocol_type:%d\n", chn, ipcam->protocol_type);
	//printf("%s lock1 chn%d\n", __func__, chn);
	pthread_mutex_lock(&g_stream_info[chn].lock);
	//printf("%s lock2 chn%d\n", __func__, chn);
	//printf("IPC_Start - 10\n");
	
	g_stream_info[chn].frame_count = 0;
	g_stream_info[chn].last_frame_count = 0;
	
	if(ipcam->protocol_type == PRO_TYPE_ONVIF)
	{
		g_stream_info[chn].ops.Start = RTSPC_Start;
		g_stream_info[chn].ops.Startbyurl = RTSPC_Startbyurl;
		g_stream_info[chn].ops.Stop = RTSPC_Stop;
		g_stream_info[chn].ops.GetLinkStatus = RTSPC_GetLinkStatus;
		g_stream_info[chn].ops.Snapshot_RegisterCB = NULL;
		
		char rtspURL[128] = {0};
		int w = 0;
		int h = 0;
		
		//printf("IPC_Start chn%d - 11\n", chn);
		//struct timeval tv1;
		//gettimeofday(&tv1,NULL);
		//if (chn == 16)
			//printf("%s chn%d 1 tv1:%u.%u\n", __func__, chn, tv1.tv_sec, tv1.tv_usec);
		
		if(IPC_URL(ipcam, (chn < (int)g_chn_count)?STREAM_TYPE_MAIN:STREAM_TYPE_SUB, rtspURL, &w, &h) == 0)
		{
			//gettimeofday(&tv1,NULL);
			//if (chn == 16)
				//printf("%s chn%d 2 tv1:%u.%u\n", __func__, chn, tv1.tv_sec, tv1.tv_usec);
			//printf("IPC_Start chn%d - 12\n", chn);
			//yaogang modify 20140918
			g_stream_info[chn].video_width = w;
			g_stream_info[chn].video_height = h;
			
			IPC_CMD_SetTime(chn, 0, 1);

			//gettimeofday(&tv1,NULL);
			//if (chn == 16)
				//printf("%s chn%d 3 tv1:%u.%u\n", __func__, chn, tv1.tv_sec, tv1.tv_usec);
			//printf("IPC_Start chn%d - 13\n", chn);
			
			ret = RTSPC_Startbyurl(chn, DoRealStreamCallBack, chn, rtspURL, ipcam->user, ipcam->pwd, ipcam->trans_type);
			//gettimeofday(&tv1,NULL);
			//if (chn == 16)
				//printf("%s chn%d 4 tv1:%u.%u\n", __func__, chn, tv1.tv_sec, tv1.tv_usec);
			//printf("IPC_Start chn%d - 14\n", chn);
		}
		else
		{
			//printf("IPC_Start - 15\n");
		}
	}
	/*
	else if(ipcam->protocol_type == PRO_TYPE_HAIXIN || ipcam->protocol_type == PRO_TYPE_JUAN)
	{
		g_stream_info[chn].ops.Start = I13_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = I13_Stop;
		g_stream_info[chn].ops.GetLinkStatus = I13_GetLinkStatus;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		//csp modify 20140613
		char info[16];
		memset(info, 0, sizeof(info));
		if(ipcam->protocol_type == PRO_TYPE_JUAN)
		{
			strcpy(info, "JUAN");
		}
		
		//csp modify 20140613
		//ret = I13_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
		ret = I13_Start(chn, DoRealStreamCallBack, chn, info, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
	}
	*/
	else if(ipcam->protocol_type == PRO_TYPE_KLW)
	{
		g_stream_info[chn].ops.Start = KLW_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = KLW_Stop;
		g_stream_info[chn].ops.GetLinkStatus = KLW_GetLinkStatus;
		g_stream_info[chn].ops.Snapshot_RegisterCB = KLW_Snapshot_RegisterCB;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		ret = KLW_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
		if (ret)
		{
			printf("%s chn%d KLW_Start failed\n", __func__, chn);
		}
	}
	else if(ipcam->protocol_type == PRO_TYPE_XM)
	{
		g_stream_info[chn].ops.Start = XM_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = XM_Stop;
		g_stream_info[chn].ops.GetLinkStatus = XM_GetLinkStatus;
		
		//IPC_CMD_SetTime(chn, 0, 1);
		
		printf("IPC_Start call XM_Start\n");
		
		ret = XM_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
	}
#ifdef IPC_PROTOCOL_TEST
	else if(ipcam->protocol_type == PRO_TYPE_IPC_TEST)
	{
		g_stream_info[chn].ops.Start = IpcTest_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = IpcTest_Stop;
		g_stream_info[chn].ops.GetLinkStatus = IpcTest_GetLinkStatus;
		g_stream_info[chn].ops.Snapshot_RegisterCB = NULL;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		ret = IpcTest_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
		if (ret)
		{
			printf("%s chn%d IpcTest_Start failed\n", __func__, chn);
		}
		
	}
#endif

	/*
	else if(ipcam->protocol_type == PRO_TYPE_FULLHAN)
	{
		g_stream_info[chn].ops.Start = FH_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = FH_Stop;
		g_stream_info[chn].ops.GetLinkStatus = FH_GetLinkStatus;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		ret = FH_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
	}
	else if(ipcam->protocol_type == PRO_TYPE_ANNI)
	{
		g_stream_info[chn].ops.Start = Anni_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = Anni_Stop;
		g_stream_info[chn].ops.GetLinkStatus = Anni_GetLinkStatus;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		//printf("IPC_Start call Anni_Start\n");
		
		ret = Anni_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
	}
	else if(ipcam->protocol_type == PRO_TYPE_RIVER)
	{
		g_stream_info[chn].ops.Start = RIVER_Start;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = RIVER_Stop;
		g_stream_info[chn].ops.GetLinkStatus = RIVER_GetLinkStatus;
		
		IPC_CMD_SetTime(chn, 0, 1);
		
		//printf("IPC_Start call RIVER_Start\n");
		
		ret = RIVER_Start(chn, DoRealStreamCallBack, chn, NULL, ipcam->dwIp, ipcam->wPort, ipcam->user, ipcam->pwd, ipcam->trans_type);
	}
	*/
	else
	{
		g_stream_info[chn].ops.Start = NULL;
		g_stream_info[chn].ops.Startbyurl = NULL;
		g_stream_info[chn].ops.Stop = NULL;
		g_stream_info[chn].ops.GetLinkStatus = NULL;
	}
	//printf("%s unlock1 chn%d\n", __func__, chn);
	pthread_mutex_unlock(&g_stream_info[chn].lock);
	//printf("%s unlock2 chn%d\n", __func__, chn);
	
	if(ret == 0)
	{
		if(g_pStateCB != NULL)
		{
			//printf("%s REAL_STREAM_STATE_START chn%d\n", __func__, chn);
			(g_pStateCB)(chn, REAL_STREAM_STATE_START);
		}
	}
	
	//printf("IPC_Start over chn%d\n", chn);
	
	return ret;
}

int IPC_Stop(int chn)
{
	//printf("%s chn%d, pthread_id: %u\n", __func__, chn, pthread_self());
	if(!g_init_flag)
	{
		printf("%s: g_init_flag == 0\n", __FUNCTION__);
		return -1;
	}
	
	//printf("IPC_Stop: chn%d\n", chn);
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		printf("%s: chn%d invalid\n", __FUNCTION__, chn);
		return -1;
	}
	
	//printf("IPC_Stop: chn%d lock...\n", chn);
	//printf("%s 1 chn%d\n", __func__, chn);
	pthread_mutex_lock(&g_stream_info[chn].lock);
	//printf("%s 2 chn%d\n", __func__, chn);
	//printf("IPC_Stop: chn%d stop...\n", chn);
	
	if(g_stream_info[chn].ops.Stop != NULL)
	{
		g_stream_info[chn].ops.Stop(chn);
	}
	//printf("%s 3 chn%d\n", __func__, chn);
	g_stream_info[chn].ops.Start = NULL;
	g_stream_info[chn].ops.Startbyurl = NULL;
	g_stream_info[chn].ops.Stop = NULL;
	g_stream_info[chn].ops.GetLinkStatus = NULL;
	g_stream_info[chn].ops.Snapshot_RegisterCB = NULL;
	
	g_stream_info[chn].frame_count = 0;
	g_stream_info[chn].last_frame_count = 0;
	//yaogang modify 20141010
	g_stream_info[chn].streamKbps = 0;
	g_stream_info[chn].video_width= 0;
	g_stream_info[chn].video_height= 0;
	
	//printf("%s unlock1 chn%d\n", __func__, chn);
	pthread_mutex_unlock(&g_stream_info[chn].lock);
	//printf("%s unlock2 chn%d\n", __func__, chn);
	//printf("IPC_Stop: chn%d over...\n", chn);
	
	if(g_pStateCB != NULL)
	{
		//printf("chn%d, REAL_STREAM_STATE_STOP\n", chn);
		(g_pStateCB)(chn, REAL_STREAM_STATE_STOP);
	}
	
	//printf("IPC_Stop over: chn%d\n", chn);
	
	return 0;
}

int IPC_RegisterCallback(RealStreamCB pStreamCB, StreamStateCB pStateCB)
{
	g_pStreamCB = pStreamCB;
	g_pStateCB = pStateCB;
	return 0;
}

int IPC_Find(ipc_node* head, ipc_node *pNode)
{
	if(head == NULL || pNode == NULL)
	{
		return 0;
	}
	
	if(pNode->ipcam.protocol_type == PRO_TYPE_ONVIF)
	{
		ipc_node *p = head;
		while(p)
		{
			//csp modify 20140427
			//if(p->ipcam.protocol_type != PRO_TYPE_ONVIF && p->ipcam.dwIp == pNode->ipcam.dwIp)
			if((p->ipcam.protocol_type != PRO_TYPE_ONVIF) && (p->ipcam.dwIp == pNode->ipcam.dwIp)/* && (strcmp(p->ipcam.uuid, pNode->ipcam.uuid) == 0)*/)
			{
				//csp modify 20140427
				//return 1;
				return 2;
			}
			p = p->next;
		}
	}
	else
	{
		ipc_node *p = head;
		while(p)
		{
			if(p->ipcam.dwIp == pNode->ipcam.dwIp)
			{
				//csp modify 20140427
				//if((p->ipcam.protocol_type == pNode->ipcam.protocol_type) && (strcmp(p->ipcam.uuid, pNode->ipcam.uuid) == 0))
				if((strcmp(p->ipcam.uuid, pNode->ipcam.uuid) == 0))
				{
					return 2;
				}
				else
				{
					return 1;
				}
			}
			p = p->next;
		}
	}
	
	return 0;
}

#define FHNP_61_SEARCH_CLMCAST_IP			"239.1.2.255"
#define FHNP_61_SEARCH_CLMCAST_PORT			51167
#define FHNP_61_SEARCH_MCAST_IP				"239.1.1.255"
#define FHNP_61_SEARCH_MCAST_PORT			51168
#define FHNP_61_SEARCH_FLAG_VERSION			0x00010001 // 版本号(用于扩展，若今后交互内容发生变化，则增加版本号，以便区分不同版本)
#define FHNP_61_SEARCH_FLAG_PRIVATE			0x00010001 // 私有标识
/* 命令码 */
#define FHNP_61_SEARCH_CMD_REQUEST			0x01 // 设备搜索
#define FHNP_61_SEARCH_CMD_REQUEST_ACK		0x02 // 设备搜索反馈
#define FHNP_61_SEARCH_CMD_MODIFY_NET		0x03 // 修改网络地址
#define FHNP_61_SEARCH_CMD_MODIFY_NET_ACK	0x04 // 修改网络地址反馈
#define FHNP_61_SEARCH_DType_Fh61Cam		0x01 //!< FH8610 CAM

/* 网络消息头结构 */
typedef struct 
{
	unsigned int crc; //!< CRC是对buffer(buflen长度)数据块的验证
	unsigned int version_flag; //!< FHNP_61_SEARCH_FLAG_VERSION
	unsigned int private_flag; //!< FHNP_61_SEARCH_FLAG_PRIVATE
	//unsigned int cmd;  //!<  参考FHNP_61_SEARCH_CMD_xxx,  例如: FHNP_61_SEARCH_CMD_REQUEST
	unsigned short cmd;  //!<  参考FHNP_61_SEARCH_CMD_xxx,  例如: FHNP_61_SEARCH_CMD_REQUEST
	unsigned int dev_type; //!< must be FHNP_61_SEARCH_DType_Fh61Cam or 0xff
	int buflen; //!< 按实际数据长度填充
	char buffer[1024]; //!< 按实际数据长度发送
} PACK_NO_PADDING FHNP_61_Search_MSG_t;

/*! 网络信息 */
typedef struct
{
	char mac[18]; //!< MAC
	char ip[16];    //!< IP
	char mask[16]; //!< 子网掩码
	char gateway[16]; //!< 网关
	unsigned short port; //!< service PORT
} PACK_NO_PADDING FHNP_61_Search_DevNetInfo_t;

/*! 设备版本信息 */
typedef struct 
{
	char software_version[32]; //!< 软件版本
	char hardware_version[32]; //!< 硬件版本
} PACK_NO_PADDING FHNP_61_Search_DevVersion_t;

/*! 设备私有信息 */
typedef struct
{
	char device_name[32]; //!< 设备名称32个长度
	char serial_num[48]; //!< 序列号48个长度
} PACK_NO_PADDING FHNP_61_Search_DevPriInfo_t;

/* 设备信息 */
typedef struct
{
	FHNP_61_Search_DevNetInfo_t  netinfo;
	FHNP_61_Search_DevVersion_t  version;
	FHNP_61_Search_DevPriInfo_t  private_info;
} PACK_NO_PADDING FHNP_61_Search_DevInfo_t;

#ifdef __cplusplus
extern "C" {
#endif

//#include "crc32.h"

#ifdef __cplusplus
}
#endif

const unsigned short CRCtbl[256] = 
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

unsigned int GetCRC(unsigned char* pBuf, int iBufLen)
{
	unsigned short CRC = 0xFFFF;    
	unsigned short mid = 0;
	unsigned int i = 0;
	
	if(!pBuf || iBufLen < 0)
	{
		return 0;
	}
	for(i = 0; i < (unsigned int)iBufLen; i++)
	{
		mid = CRC & 0xFF;
		CRC = (CRC >> 8) ^ CRCtbl[(mid ^ pBuf[i]) % 256];  
	}
	return (int)CRC;
}

int FH_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict)
{
	if(head == NULL || tail == NULL)
	{
		return -1;
	}
	
	int count = 0;
	
	int i = 0;
	
	struct sockaddr_in local, remote, from;
	
	int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock == -1)
	{
		printf("Search IPNC:socket error\n");
		return -1;
	}
	
	int reuseflag = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
	{
		printf("Search IPNC:port mulit error\n");
		close(sock);
		return -1;
	}
	
	local.sin_family = AF_INET;
	local.sin_port   = htons(51167);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
		printf("Search IPNC:bind error\n");
		close(sock);
		return -1;
	}
	
	struct ip_mreq mreq;
	memset(&mreq,0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr("239.1.2.255");
	mreq.imr_interface.s_addr = GetLocalIp();
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
	{
		printf("Search IPNC:join mulit error\n");
		close(sock);
		return -1;
	}
	
	int optval = 0;
	if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		printf("setsockopt(IP_MULTICAST_LOOP) failed\n");
		//close(sock);
		//return 0;
	}
	
	remote.sin_family      = AF_INET;
	remote.sin_port        = htons(51168);
	remote.sin_addr.s_addr = inet_addr("239.1.1.255");
	
	FHNP_61_Search_MSG_t msg;
	msg.version_flag = FHNP_61_SEARCH_FLAG_VERSION;
	msg.private_flag = FHNP_61_SEARCH_FLAG_PRIVATE;
	msg.cmd = FHNP_61_SEARCH_CMD_REQUEST;
	msg.dev_type = 0xff;
	msg.buflen = 0;
	memset(msg.buffer, 0, sizeof(msg.buffer));
	msg.crc = GetCRC((unsigned char *)msg.buffer, msg.buflen);
	
	for(i = 0; i < 1; i++)
	{
		if(sendto(sock, (char *)&msg, sizeof(msg)-sizeof(msg.buffer), 0, (struct sockaddr *)&remote, sizeof(remote)) == SOCKET_ERROR)
		{
			printf("Search IPNC:sendto failed with: %d\n", errno);
			close(sock);
			return -1;
		}
	}

	socklen_t len = sizeof(struct sockaddr_in);
	
	unsigned int cc = 0;
	
	unsigned char conflict_flag = 0;
	
	for(i = 0; i < 100; i++)
	{
		fd_set r;
		struct timeval t;
		
		t.tv_sec = 0;
		t.tv_usec = 10*1000;
		
		FD_ZERO(&r);
		FD_SET(sock, &r);
		
		int ret = select(sock + 1, &r, NULL, NULL, &t);
		
		if(ret <= 0)
		{
			if(cc++ >= 10)
			{
				break;
			}
			continue;
		}
		
		cc = 0;
		
		if(ret > 0)
		{
			FHNP_61_Search_MSG_t resp;
			ret = recvfrom(sock, (char *)&resp, sizeof(resp), 0, (struct sockaddr *)&from, &len);
			//printf("FH_Search recvfrom:%d\n",ret);
			if(ret < 0)
			{
				printf("FH_Search recvfrom failed-1\n");
				break;
			}
			if(ret == 0)
			{
				printf("FH_Search recvfrom failed-2\n");
				break;
			}
			if(ret != (sizeof(resp)-sizeof(resp.buffer)+sizeof(FHNP_61_Search_DevInfo_t)))
			{
				printf("FH_Search recvfrom failed-3\n");
				break;
			}
			if(resp.buflen != sizeof(FHNP_61_Search_DevInfo_t))
			{
				printf("FH_Search recvfrom failed-4\n");
				break;
			}
			
			//printf("cmd=0x%x,dev_type=0x%x,private_flag=0x%08x,crc=(0x%x,0x%x)\n",resp.cmd,resp.dev_type,resp.private_flag,resp.crc,crc32(0, resp.buffer, resp.buflen));
			
			if(resp.cmd == FHNP_61_SEARCH_CMD_REQUEST_ACK && resp.dev_type == FHNP_61_SEARCH_DType_Fh61Cam && resp.private_flag == FHNP_61_SEARCH_FLAG_PRIVATE && resp.crc == GetCRC((unsigned char *)resp.buffer, resp.buflen))
			{
				FHNP_61_Search_DevInfo_t *pDevInfo = (FHNP_61_Search_DevInfo_t *)resp.buffer;
				
				char *ip = pDevInfo->netinfo.ip;
				char *id = pDevInfo->private_info.serial_num;
				char *mac = pDevInfo->netinfo.mac;
				char *name = pDevInfo->private_info.device_name;
				unsigned short port = pDevInfo->netinfo.port;
				
				char *mask = pDevInfo->netinfo.mask;
				char *gate = pDevInfo->netinfo.gateway;
				char *fdns = (char *)"8.8.8.8";
				char *sdns = (char *)"4.4.4.4";
				
				ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
				if(pNode == NULL)
				{
					printf("Not enough space to save new ipc info.\n");
					//if(*head)
					//{
					//	IPC_Free(*head);
					//	*head = NULL;
					//	*tail = NULL;
					//}
					close(sock);
					return count;//return -1;
				}
				
				memset(pNode, 0, sizeof(*pNode));
				pNode->next = NULL;
				
				
				
				strncpy(pNode->ipcam.address, mac, sizeof(pNode->ipcam.address)-1);
				strcpy(pNode->ipcam.user, "admin");
				strcpy(pNode->ipcam.pwd, "admin");
				pNode->ipcam.channel_no = 0;
				pNode->ipcam.enable = 0;
				pNode->ipcam.ipc_type = IPC_TYPE_D1;
				pNode->ipcam.protocol_type = PRO_TYPE_FULLHAN;
				pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
				pNode->ipcam.trans_type = TRANS_TYPE_TCP;
				pNode->ipcam.force_fps = 0;
				pNode->ipcam.frame_rate = 30;
				pNode->ipcam.dwIp = inet_addr(ip);
				pNode->ipcam.wPort = (port);
				strcpy(pNode->ipcam.uuid, id);
				strcpy(pNode->ipcam.name, name);
				pNode->ipcam.net_mask = inet_addr(mask);
				pNode->ipcam.net_gateway = inet_addr(gate);
				if(fdns != NULL)
				{
					pNode->ipcam.dns1 = inet_addr(fdns);
					if(pNode->ipcam.dns1 == (unsigned int)(-1))
					{
						pNode->ipcam.dns1 = inet_addr("8.8.8.8");
					}
				}
				if(sdns != NULL)
				{
					pNode->ipcam.dns2 = inet_addr(sdns);
					if(pNode->ipcam.dns2 == (unsigned int)(-1))
					{
						pNode->ipcam.dns2 = inet_addr("4.4.4.4");
					}
				}
				
				int rtn = IPC_Find(*head, pNode);
				if(rtn == 1)
				{
					printf("ipc conflict : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
					conflict_flag = 1;
				}
				else if(rtn == 2)
				{
					printf("ipc repeat : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
					free(pNode);
					pNode = NULL;
					continue;
				}
				
				count++;
				printf("fullhan-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
				
				if(*head == NULL)
				{
					(*head) = pNode;
					(*tail) = pNode;
				}
				else
				{
					(*tail)->next = pNode;
					(*tail) = pNode;
				}
			}
			else
			{
				printf("FH_Search recvfrom failed-5\n");
			}
		}
	}
	
	close(sock);
	
	return count;
}

#ifdef IPC_PROTOCOL_TEST
int Ipctest_Search(ipc_node** phead)
{
	ipc_node *head = NULL;
	ipc_node *tail = NULL;
	const char *ip = "192.168.1.20";
	unsigned int ip_start = inet_addr(ip);
	unsigned int ip_le = 0;
	int i;
	int count = 0;
	struct in_addr in;
	
	for (i=0; i<16; ++i)
	{
		ip_le = htonl(ip_start) + i;

		ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
		if(pNode == NULL)
		{
			printf("%s Not enough space to save new ipc info.\n", __func__);
			
			break;
		}
		
		memset(pNode, 0, sizeof(*pNode));
		pNode->next = NULL;
		
		//strncpy(pNode->ipcam.address, pstruDevInfo->aszMAC, sizeof(pNode->ipcam.address)-1);
		strcpy(pNode->ipcam.user, "admin");
		strcpy(pNode->ipcam.pwd, "admin");
		pNode->ipcam.channel_no = 0;
		pNode->ipcam.enable = 0;
		pNode->ipcam.ipc_type = IPC_TYPE_720P;
		pNode->ipcam.protocol_type = PRO_TYPE_IPC_TEST;
		pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
		pNode->ipcam.trans_type = TRANS_TYPE_TCP;
		pNode->ipcam.force_fps = 0;
		pNode->ipcam.frame_rate = 30;
		pNode->ipcam.dwIp = htonl(ip_le);
		pNode->ipcam.wPort = 80;
		//strcpy(pNode->ipcam.uuid, pstruDevInfo->aszDevID);
		
		strcpy(pNode->ipcam.name, "IpcTestName");
		pNode->ipcam.net_mask = inet_addr("255.255.255.0");
		pNode->ipcam.net_gateway = inet_addr("192.168.1.1");
		pNode->ipcam.dns1 = inet_addr("8.8.8.8");
		pNode->ipcam.dns2 = inet_addr("4.4.4.4");

		in.s_addr = pNode->ipcam.dwIp;
		printf("%s add ipc ip: %s\n", __func__, inet_ntoa(in));
		
		if(head == NULL)
		{
			head = pNode;
			tail = pNode;
		}
		else
		{
			tail->next = pNode;
			tail = pNode;
		}

		count++;
	}	

	if (count)
		*phead = head;

	return count;
}

#endif


static pthread_mutex_t g_klw_search_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned char g_klw_search_start = 0;
static ipc_node *g_klw_head = NULL;
static ipc_node *g_klw_tail = NULL;

int CallBackCmd(const void* pvHandle,/*句柄*/
                char* pszRNPCode,    /*码字*/
                VVV_SEARCH_CMD_RES_S* pstruResponseInfo,/*反馈信息*/
                void* pvUserData)  /*用户数据*/
{
    printf("\n----\n");
    printf("-->respcode:%s %s %s \n",pszRNPCode,pstruResponseInfo->pszCmd, pstruResponseInfo->pszResultInfo);
    return 0;    
}

int CallBackSearch(const void* pvHandle,char* pszRNPCode,VVV_SEARCH_DEVINFO_S* pstruDevInfo,void* pvUserData)
{
	if(pszRNPCode == NULL || pstruDevInfo == NULL)
	{
		printf("CallBackSearch: param error\n");
		return -1;
	}
	
	printf("%s, ip=%s, port = %s, mask = %s\n", pszRNPCode, pstruDevInfo->aszIP, pstruDevInfo->aszHttpPort, pstruDevInfo->aszMASK);
	
	pthread_mutex_lock(&g_klw_search_lock);
	
	if(g_klw_search_start)
	{
		ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
		if(pNode == NULL)
		{
			printf("Not enough space to save new ipc info.\n");
			pthread_mutex_unlock(&g_klw_search_lock);
			return -1;
		}
		
		memset(pNode, 0, sizeof(*pNode));
		pNode->next = NULL;
		
		strncpy(pNode->ipcam.address, pstruDevInfo->aszMAC, sizeof(pNode->ipcam.address)-1);
		strcpy(pNode->ipcam.user, "admin");
		strcpy(pNode->ipcam.pwd, "admin");
		pNode->ipcam.channel_no = 0;
		pNode->ipcam.enable = 0;
		pNode->ipcam.ipc_type = IPC_TYPE_720P;
		pNode->ipcam.protocol_type = PRO_TYPE_KLW;
		pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
		pNode->ipcam.trans_type = TRANS_TYPE_TCP;
		pNode->ipcam.force_fps = 0;
		pNode->ipcam.frame_rate = 30;
		pNode->ipcam.dwIp = inet_addr(pstruDevInfo->aszIP);
		pNode->ipcam.wPort = atoi(pstruDevInfo->aszHttpPort);
		strcpy(pNode->ipcam.uuid, pstruDevInfo->aszDevID);
		
		//printf("yg search ipcam uuid: %s\n", pNode->ipcam.uuid);
		
		strcpy(pNode->ipcam.name, pstruDevInfo->aszDevName);
		pNode->ipcam.net_mask = inet_addr(pstruDevInfo->aszMASK);
		pNode->ipcam.net_gateway = inet_addr(pstruDevInfo->aszGTW);
		pNode->ipcam.dns1 = inet_addr("8.8.8.8");
		pNode->ipcam.dns2 = inet_addr("4.4.4.4");
		
		if(g_klw_head == NULL)
		{
			g_klw_head = pNode;
			g_klw_tail = pNode;
		}
		else
		{
			g_klw_tail->next = pNode;
			g_klw_tail = pNode;
		}
	}
	
	pthread_mutex_unlock(&g_klw_search_lock);
	
	return 0;
}

int KLW_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict)
{
	if(head == NULL || tail == NULL)
	{
		printf("KLW_Search: param error\n");
		return -1;
	}
	
	int count = 0;
	
	unsigned char conflict_flag = 0;
	int handle, ret;
	ipc_node *pNode = NULL;

	//nodeid, 是一个随机生成的字符串, 这里为了调试方便, 写了一个固定的值, 如果输入NULL, 系统随机生成一个id
	ret = devs_init(devs_node_control, NULL);
	if (0 != ret)
	{
		printf("devs_init failed ret: %d\n", ret);
		return -1;
	}
	
	pthread_mutex_lock(&g_klw_search_lock);
	
	g_klw_search_start = 1;
	g_klw_head = g_klw_tail = NULL;
	
	pthread_mutex_unlock(&g_klw_search_lock);
	
	//初始化v1的设备搜索, 兼容以前设备的搜索
	handle = -1;
	struct devs_callback cbfuns;

	ret = devs_init_handle(devs_proto_mcast_v10, &handle, devs_sendmode_allnic);
	if (0 != ret)
	{
		printf("devs_init_handle failed ret: %d\n", ret);
		goto out1;
	}

	memset(&cbfuns, 0, sizeof(struct devs_callback));

	cbfuns.pfn_search_req_v2 = NULL;
	cbfuns.pfn_cmd_req_v2 = NULL;

	cbfuns.pfn_search_resp = CallBackSearch;
	cbfuns.pfn_cmd_resp = CallBackCmd;
	 
	ret = devs_regcb(handle, &cbfuns);
	if (0 != ret)
	{
		printf("devs_regcb failed ret: %d\n", ret);
		goto out2;
	}

	//开启设备搜索服务
	ret = devs_start(handle);
	if (0 != ret)
	{
		printf("devs_start failed ret: %d\n", ret);
		goto out2;
	}

	//发送设备搜索包, 这个要手工去发送
	ret = devs_search(handle);
	if (0 != ret)
	{
		printf("devs_start failed ret: %d\n", ret);
		goto out2;
	}
	
	usleep(3*1000*1000);

	pNode = g_klw_head;
	while(pNode)
	{
		int rtn = IPC_Find(*head, pNode);
		if(rtn == 1)
		{
			printf("ipc conflict : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
			conflict_flag = 1;
		}
		else if(rtn == 2)
		{
			printf("ipc repeat : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
			ipc_node *p = pNode;
			pNode = pNode->next;
			free(p);
			p = NULL;
			continue;
		}
		
		count++;
		
		printf("klw-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
		
		if(*head == NULL)
		{
			(*head) = pNode;
			(*tail) = pNode;
		}
		else
		{
			(*tail)->next = pNode;
			(*tail) = pNode;
		}
		
		pNode = pNode->next;
		(*tail)->next = NULL;
	}

out2:
	devs_deinit_handle(handle);
	
out1:
	devs_deinit();
	
	pthread_mutex_lock(&g_klw_search_lock);
	
	g_klw_search_start = 0;
	g_klw_head = g_klw_tail = NULL;
	
	pthread_mutex_unlock(&g_klw_search_lock);
	
	return count;
}

/*
wsdd:type:tdn:NetworkVideoTransmitter
wsdd:uuid:urn:uuid:5f5a69c2-e0ae-504f-829b-000389111004
http://192.168.1.189:8080/onvif/device_service

wsdd:type:tdn:NetworkVideoTransmitter
wsdd:uuid:urn:uuid:11223344-5566-7788-99aa-003e0b04015d
http://192.168.1.11:8899/onvif/device_service
*/
//排序
void IPC_Sort(ipc_node** head)
{
	ipc_node **ppstart = head;
	ipc_node **ppmin = NULL;
	ipc_node **pptmp = NULL;
	ipc_node *ptmp = NULL;

	while (*ppstart)
	{
		ppmin = ppstart;
		pptmp = &((*ppstart)->next);

		while (*pptmp)
		{
			if(ntohl((*ppmin)->ipcam.dwIp) > ntohl((*pptmp)->ipcam.dwIp))
			{
				ppmin = pptmp;
			}

			pptmp = &((*pptmp)->next);
		}

		if (ppmin != ppstart)
		{
			//取出最小节点
			ptmp = *ppmin;
			*ppmin = ptmp->next;

			//插入队头
			ptmp->next = *ppstart;
			*ppstart = ptmp;
		}

		ppstart = &((*ppstart)->next);		
	}
	
}

//搜索网络摄像机
int IPC_Search(ipc_node** extern_head, unsigned int protocol_type, unsigned char check_conflict)
{
	if(extern_head == NULL)
	{
		return -1;
	}
	
	*extern_head = NULL;
	
	ipc_node *head_tmp = NULL;
	ipc_node *tail_tmp = NULL;
	
	//yaogang modify 20151117
	ipc_node *head1 = NULL;//链表头
	ipc_node *tail1 = NULL;

	ipc_node *tmp = NULL;
	
	int count = 0;
	int rtn = 0;
	
	
	/*
	//csp modify 20140411
	if(protocol_type & PRO_TYPE_FULLHAN)
	{
		int rtn = FH_Search(head, &tail, check_conflict);
		if(rtn > 0)
		{
			count += rtn;
		}
	}
	*/
	//csp modify 20140427
	if(protocol_type & PRO_TYPE_KLW)
	{
		//yaogang modify 20151222
		head_tmp = tail_tmp = NULL;
		rtn = 0;
		
		rtn = KLW_Search(&head_tmp, &tail_tmp, check_conflict);
		if(rtn > 0)
		{
			count += rtn;

			IPC_Sort(&head_tmp);
			
			tmp = head_tmp;			
			while(tmp)//排序之后的尾部
			{
				//in.s_addr = tmp->ipcam.dwIp;
				//printf("yao: %d, ip: %s\n", yao++, inet_ntoa(in));

				tail_tmp = tmp;
				tmp = tmp->next;
			}

			if (head1 == NULL)
			{
				head1 = head_tmp;
			}
			else
			{
				tail1->next = head_tmp;
			}
			tail1 = tail_tmp;
		}
	}

	//xm
	if(protocol_type & PRO_TYPE_XM)
	{
		//yaogang modify 20151222
		head_tmp = tail_tmp = NULL;
		rtn = 0;
		
		rtn = XM_Search(&head_tmp, &tail_tmp, check_conflict);
		if(rtn > 0)
		{
			count += rtn;

			IPC_Sort(&head_tmp);
			
			tmp = head_tmp;			
			while(tmp)//排序之后的尾部
			{
				//in.s_addr = tmp->ipcam.dwIp;
				//printf("yao: %d, ip: %s\n", yao++, inet_ntoa(in));

				tail_tmp = tmp;
				tmp = tmp->next;
			}

			if (head1 == NULL)
			{
				head1 = head_tmp;
			}
			else
			{
				tail1->next = head_tmp;
			}
			tail1 = tail_tmp;
		}
	}

	
//Step2:
	if(protocol_type & PRO_TYPE_ONVIF)
	{
		//yaogang modify 20170503
		head_tmp = tail_tmp = NULL;
		rtn = 0;
	#if 1
		//printf("ONVIF search-1......\n");
		//fflush(stdout);
		
		//ONVIF_LOCK();
		
		//printf("ONVIF search-2\n");
		//fflush(stdout);
		
		onvif_wsdiscovery onvif;
		onvif.init(0);
		
		//printf("ONVIF search-3\n");
		//fflush(stdout);
		
		onvif.send_once_wsdiscovery_multi();
		
		//printf("ONVIF search-4\n");
		//fflush(stdout);
		
		//ONVIF_UNLOCK();
		
		//printf("ONVIF search-5\n");
		//fflush(stdout);
		
		for(list<discovery_device *>::iterator it = onvif.receivers.begin(); it != onvif.receivers.end(); it++)
		{
			//printf("ONVIF search-6\n");
			//fflush(stdout);
			
			int len = 0;
			
			char *ip = NULL;
			char *port = NULL;
			
			char default_port[] = {"80"};
			
			char address[256];
			memset(address, 0, sizeof(address));
			strncpy(address, (*it)->get_xaddrs(len), sizeof(address)-1);
			
			ip = strstr(address, "http://");
			if(ip == NULL)
			{
				printf("error address(%s) for resolve ip\n",address);
				//if(*head)
				//{
				//	IPC_Free(*head);
				//	*head = NULL;
				//}
				//return -1;
				continue;
			}
			ip += strlen("http://");
			
			//海康最新型号的摄像机支持ipv4和ipv6
			//http://192.168.1.99/onvif/device_service http://[fe80::4619:b7ff:fe11:fe78]/onvif/device_service
			char *ptr = strstr(ip, " http://");
			if(ptr != NULL)
			{
				*ptr = 0;
			}
			
			port = strstr(ip, ":");
			if(port == NULL)
			{
				printf("error address(%s) for resolve port\n",address);
				//if(*head)
				//{
				//	IPC_Free(*head);
				//	*head = NULL;
				//}
				//return -1;
				port = default_port;
				if(strstr(ip, "/") != NULL)
				{
					*strstr(ip, "/") = '\0';
				}
			}
			else
			{
				*port = '\0';
				port += strlen(":");
				if(strstr(port, "/") != NULL)
				{
					*strstr(port, "/") = '\0';
				}
			}
			
			ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
			if(pNode == NULL)
			{
				printf("Not enough space to save new ipc info.\n");
				//yaogang modify 20170503
				#if 0
				if(*head)
				{
					IPC_Free(*head);
					*head = NULL;
				}
				return -1;
				#else
				
				break;
				
				#endif
			}
			
			memset(pNode, 0, sizeof(*pNode));
			pNode->next = NULL;
			
			//strncpy(pNode->ipcam.address, (*it)->get_xaddrs(len), sizeof(pNode->ipcam.address)-1);
			char onvifaddress[256];
			memset(onvifaddress, 0, sizeof(onvifaddress));
			strncpy(onvifaddress, (*it)->get_xaddrs(len), sizeof(onvifaddress)-1);
			ptr = strstr(onvifaddress+strlen("http://"), " http://");
			if(ptr != NULL)
			{
				*ptr = 0;
			}
			strncpy(pNode->ipcam.address, onvifaddress, sizeof(pNode->ipcam.address)-1);
			strcpy(pNode->ipcam.user, "admin");
			strcpy(pNode->ipcam.pwd, "admin");
			strcpy(pNode->ipcam.name, "onvif-nvt-ipc");
			//strcpy(pNode->ipcam.uuid, "5f5a69c2-e0ae-504f-829b-000389111004");
			pNode->ipcam.channel_no = 0;
			pNode->ipcam.enable = 0;
			pNode->ipcam.ipc_type = IPC_TYPE_720P;
			pNode->ipcam.protocol_type = PRO_TYPE_ONVIF;
			pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
			pNode->ipcam.trans_type = TRANS_TYPE_TCP;
			pNode->ipcam.force_fps = 0;
			pNode->ipcam.frame_rate = 30;
			pNode->ipcam.dwIp = inet_addr((*it)->get_ip(len));//inet_addr(ip);
			pNode->ipcam.wPort = atoi(port);
			strcpy(pNode->ipcam.uuid, (*it)->get_uuid(len));
			
			#if 0
			device_ip dev_ip;
			memset(&dev_ip,0,sizeof(dev_ip));
			((device_ptz*)(*it))->handle_mngmt_get_ip(&dev_ip);
			pNode->ipcam.net_mask = (unsigned int)(-1) - ((1 << (32 - dev_ip.prefix)) - 1);
			pNode->ipcam.net_mask = htonl(pNode->ipcam.net_mask);
			//printf("IPC_Search prefix=%d,net_mask=0x%08x\n",dev_ip.prefix,pNode->ipcam.net_mask);
			
			device_gateway dev_gateway;
			memset(&dev_gateway,0,sizeof(dev_gateway));
			((device_ptz*)(*it))->handle_mngmt_get_default_gateway(&dev_gateway);
			pNode->ipcam.net_gateway = inet_addr(dev_gateway.gateway);
			
			device_dns dev_dns;
			memset(&dev_dns,0,sizeof(dev_dns));
			((device_ptz*)(*it))->handle_mngmt_get_dns(&dev_dns);
			pNode->ipcam.dns1 = inet_addr(dev_dns.dns);
			pNode->ipcam.dns2 = 0;
			#endif
			
			if(IPC_Find(head_tmp, pNode))
			{
				printf("ipc conflict : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
				free(pNode);
				pNode = NULL;
				continue;
			}
			
			rtn++;
			
			printf("ONVIF-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
			
			if(head_tmp == NULL)
			{
				head_tmp = pNode;
				tail_tmp = pNode;
			}
			else
			{
				tail_tmp->next = pNode;
				tail_tmp = pNode;
			}
		}

		if(rtn > 0)
		{
			count += rtn;

			IPC_Sort(&head_tmp);
			
			tmp = head_tmp;			
			while(tmp)//排序之后的尾部
			{
				//in.s_addr = tmp->ipcam.dwIp;
				//printf("yao: %d, ip: %s\n", yao++, inet_ntoa(in));

				tail_tmp = tmp;
				tmp = tmp->next;
			}

			if (head1 == NULL)
			{
				head1 = head_tmp;
			}
			else
			{
				tail1->next = head_tmp;
			}
			tail1 = tail_tmp;
		}
	#else
		/*define container to store the device address */
		DeviceAddrsStack deviceStack;
		memset(&deviceStack, 0, sizeof(deviceStack));
		
		ONVIF_LOCK();
		
		if(!discovery(&deviceStack))
		{
			ONVIF_UNLOCK();
			
			printf("discovery %d devices\n",deviceStack.len);
			
			for(i = 0; i < deviceStack.len; ++i)
			{
				//printf("ipc%d	:	%s\n", i, deviceStack.stack[i]);
				
				char *ip = NULL;
				char *port = NULL;
				
				char address[64];
				memset(address, 0, sizeof(address));
				strncpy(address, deviceStack.stack[i], sizeof(address)-1);
				
				ip = strstr(address, "http://");
				if(ip == NULL)
				{
					printf("error address for resolve ip\n");
					if(*head)
					{
						IPC_Free(*head);
						*head = NULL;
					}
					return -1;
				}
				ip += strlen("http://");
				
				port = strstr(ip, ":");
				if(port == NULL)
				{
					printf("error address for resolve port\n");
					if(*head)
					{
						IPC_Free(*head);
						*head = NULL;
					}
					return -1;
				}
				*port = '\0';
				port += strlen(":");
				*strstr(port, "/") = 0;
				
				ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
				if(pNode == NULL)
				{
					printf("Not enough space to save new ipc info.\n");
					if(*head)
					{
						IPC_Free(*head);
						*head = NULL;
					}
					return -1;
				}
				
				memset(pNode, 0, sizeof(*pNode));
				pNode->next = NULL;
				
				strncpy(pNode->ipcam.address, deviceStack.stack[i], sizeof(pNode->ipcam.address)-1);
				strcpy(pNode->ipcam.user, "admin");
				strcpy(pNode->ipcam.pwd, "admin");
				//strcpy(pNode->ipcam.name, "onvif-nvt-ipc");
				//strcpy(pNode->ipcam.uuid, "5f5a69c2-e0ae-504f-829b-000389111004");
				pNode->ipcam.channel_no = 0;
				pNode->ipcam.enable = 0;
				pNode->ipcam.ipc_type = IPC_TYPE_720P;
				pNode->ipcam.protocol_type = PRO_TYPE_ONVIF;
				pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
				pNode->ipcam.trans_type = TRANS_TYPE_TCP;
				pNode->ipcam.force_fps = 0;
				pNode->ipcam.frame_rate = 30;
				pNode->ipcam.dwIp = inet_addr(ip);
				pNode->ipcam.wPort = atoi(port);
				strcpy(pNode->ipcam.uuid, deviceStack.uuid[i]);
				
				printf("STD-ipc%d : [%s,0x%08x,%d,%s]\n", i, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
				
				if(*head == NULL)
				{
					*head = pNode;
					tail = pNode;
				}
				else
				{
					tail->next = pNode;
					tail = pNode;
				}
			}
		}
		else
		{
			ONVIF_UNLOCK();
		}
		
		DestoryDevice(&deviceStack);
	#endif
	}

#ifdef IPC_PROTOCOL_TEST
	//PRO_TYPE_IPC_TEST  search start
	if(protocol_type & PRO_TYPE_IPC_TEST)
	{
		//yaogang modify 20151222
		head_tmp = tail_tmp = NULL;
		rtn = 0;

		rtn = Ipctest_Search(&head_tmp);
		if(rtn > 0)
		{
			count += rtn;

			IPC_Sort(&head_tmp);
			
			tmp = head_tmp;			
			while(tmp)//排序之后的尾部
			{
				//in.s_addr = tmp->ipcam.dwIp;
				//printf("yao: %d, ip: %s\n", yao++, inet_ntoa(in));

				tail_tmp = tmp;
				tmp = tmp->next;
			}

			if (head1 == NULL)
			{
				head1 = head_tmp;
			}
			else
			{
				tail1->next = head_tmp;
			}
			tail1 = tail_tmp;
		}
	}
#endif	
	
	if(head1 == NULL)
	{
		printf("No ipc on this network.\n");
		return -1;
	}

	*extern_head = head1;
	
	return 0;
}

//释放网络摄像机
int IPC_Free(ipc_node* head)
{
	if(head == NULL)
	{
		return 0;
	}
	
	ipc_node *p1 = head;
	
	while(p1)
	{
		ipc_node *p2 = p1->next;
		
		//printf("free ipc : [%s,0x%08x,%d]\n", p1->ipcam.address, p1->ipcam.dwIp, p1->ipcam.wPort);
		
		free(p1);
		
		p1 = p2;
	}
	
	return 0;
}

int IPC_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
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
	
	//printf("IPC_GetNetworkParam-IPC_GetNetworkParam\n");
	
	if(ipcam->protocol_type == PRO_TYPE_ONVIF)
	{
		return Onvif_GetNetworkParam(ipcam, pnw);
	}
	/*
	else if(ipcam->protocol_type == PRO_TYPE_HAIXIN || ipcam->protocol_type == PRO_TYPE_JUAN)
	{
		return I13_GetNetworkParam(ipcam, pnw);
	}
	*/
	else if(ipcam->protocol_type == PRO_TYPE_KLW)
	{
		//printf("IPC_GetNetworkParam-KLW_GetNetworkParam\n");
		return KLW_GetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_XM)
	{
		return XM_GetNetworkParam(ipcam, pnw);
	}
	/*
	else if(ipcam->protocol_type == PRO_TYPE_FULLHAN)
	{
		return FH_GetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_ANNI)
	{
		return Anni_GetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_RIVER)
	{
		return RIVER_GetNetworkParam(ipcam, pnw);
	}
	*/
	return -1;
}

int IPC_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
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
	
	if(ipcam->protocol_type == PRO_TYPE_ONVIF)
	{
		return Onvif_SetNetworkParam(ipcam, pnw);
	}
	/*
	else if(ipcam->protocol_type == PRO_TYPE_HAIXIN || ipcam->protocol_type == PRO_TYPE_JUAN)
	{
		return I13_SetNetworkParam(ipcam, pnw);
	}
	*/
	else if(ipcam->protocol_type == PRO_TYPE_KLW)
	{
		return KLW_SetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_XM)
	{
		return XM_SetNetworkParam(ipcam, pnw);
	}
	/*
	else if(ipcam->protocol_type == PRO_TYPE_FULLHAN)
	{
		return FH_SetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_ANNI)
	{
		return Anni_SetNetworkParam(ipcam, pnw);
	}
	else if(ipcam->protocol_type == PRO_TYPE_RIVER)
	{
		return RIVER_SetNetworkParam(ipcam, pnw);
	}
	*/
	return -1;
}

int IPC_CMD_Open(int chn)
{
	return 0;
}

int IPC_CMD_Close(int chn)
{
	return 0;
}

int IPC_CMD_GetAudioSwitchStatus(int chn)// 0:关1:开
{
	int status = 0;// 0:关1:开
	
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count))
	{
		return 0;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.GetAudioSwitchStatus != NULL)
	{
		status = g_chn_info[chn].ops.GetAudioSwitchStatus(chn);
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);

	return status;
}

	
int IPC_CMD_SetAudioSwitchStatus(int chn, int bswitch)
{
	int status = 0;// 0:关1:开
	
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count))
	{
		return 0;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetAudioSwitchStatus != NULL)
	{
		status = g_chn_info[chn].ops.SetAudioSwitchStatus(chn, bswitch);
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);

	return status;
}

int IPC_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
	
	if(para == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetImageParam != NULL)
	{
		g_chn_info[chn].ops.SetImageParam(chn, para);
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

int IPC_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
	
	if(para == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.GetImageParam != NULL)
	{
		g_chn_info[chn].ops.GetImageParam(chn, para);
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

static unsigned int g_speed = 0;

int IPC_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(cmd == EM_PTZ_CMD_SETSPEED)
	{
		g_speed = *((unsigned int *)data);
	}
	
	if(cmd < EM_PTZ_CMD_STOP_TILEUP)
	{
		unsigned int speed = *((unsigned int *)data);
		if(speed == 0)
		{
			if(g_chn_info[chn].newcam.protocol_type != PRO_TYPE_HAIXIN && g_chn_info[chn].newcam.protocol_type != PRO_TYPE_JUAN)
			{
				*((unsigned int *)data) = g_speed;
			}
		}
	}
	
	if(g_chn_info[chn].ops.PtzCtrl != NULL)
	{
		g_chn_info[chn].ops.PtzCtrl(chn, cmd, data);
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

int IPC_CMD_SetTime(int chn, time_t t, int force)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	//csp modify 20140403
	if(!g_sync_time)
	{
		//printf("IPC_CMD_SetTime: chn%d not need sync time\n",chn);
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		//printf("IPC_CMD_SetTime: chn%d error\n",chn);
		return -1;
	}

	//yaogang 当RTC故障后时间变成20130101，不同步IPC
	if(t == 0)
	{
		t = time(NULL);// + 1;//???
	}
		
	struct tm now;
	localtime_r(&t,&now);

	printf("%s chn%d-- %04d.%02d.%02d.%02d.%02d.%02d\n", __func__, chn,
		now.tm_year+1900,
		now.tm_mon+1,
		now.tm_mday,
		now.tm_hour,
		now.tm_min,
		now.tm_sec);
	
	if ( (now.tm_year+1900 < 2015) ||
		((now.tm_year+1900 == 2015) && (now.tm_mon+1 <=6)) )//不同步
	{
		printf("%s RTC exception\n", __func__);
		return 0;
	}
	
	
	//end
	
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetTime != NULL)
	{
		printf("chn%d set ipc time...\n",chn);
		g_chn_info[chn].ops.SetTime(chn, t, force);
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	return 0;
}

int IPC_CMD_SetMD(int chn, md_para_t *para)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		printf("IPC_CMD_SetMD: chn%d error\n",chn);
		return -1;
	}
	
	if(para == NULL)
	{
		return -1;
	}
	
#if 1
	pthread_mutex_lock(&g_chn_info[chn].set_param_lock);
	
	g_chn_info[chn].param.md.para = *para;
	g_chn_info[chn].param.md.refresh = 1;
	
	pthread_mutex_unlock(&g_chn_info[chn].set_param_lock);
#else
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetMD != NULL)
	{
		printf("chn%d set md param...\n",chn);
		g_chn_info[chn].ops.SetMD(chn, para);
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
#endif
	return 0;
}
int IPC_CMD_SetOSD(int chn, char *name)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		printf("IPC_CMD_SetOSD: chn%d error\n",chn);
		return -1;
	}
	
	if(name == NULL)
	{
		return -1;
	}
#if 1
	pthread_mutex_lock(&g_chn_info[chn].set_param_lock);
	
	strcpy(g_chn_info[chn].param.osd.name, name);
	g_chn_info[chn].param.osd.refresh = 1;
	
	pthread_mutex_unlock(&g_chn_info[chn].set_param_lock);
#else
	
	//printf("IPC_CMD_SetOSD: chn=%d name=%s\n",chn,name);
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetOSD != NULL)
	{
		//printf("yg IPC_CMD_SetOSD chn%d set IPC name: %s\n",chn, name);
		g_chn_info[chn].ops.SetOSD(chn, name);
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
	
#endif
	return 0;
}

int IPC_CMD_GetVideoEncoderParam(int chn, int stream, VideoEncoderParam *para)//0-主码流,1-次码流
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		printf("IPC_CMD_GetVideoEncoderParam: chn%d error\n",chn);
		return -1;
	}
	
	if(para == NULL)
	{
		return -1;
	}
	
	//printf("IPC_CMD_SetOSD: chn=%d name=%s\n",chn,name);
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.GetVENC != NULL)
	{
		//printf("yg IPC_CMD_GetVideoEncoderParam chn%d\n",chn);
		g_chn_info[chn].ops.GetVENC(chn, stream, para);
	}
	else
	{
		printf("IPC protocol not support GETVENC.\n");
		return -1;
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
	return 0;
}


int IPC_CMD_SetVideoEncoderParam(int chn, int stream, VideoEncoderParam *para)//0-主码流,1-次码流
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		printf("IPC_CMD_SetVideoEncoderParam: chn%d error\n",chn);
		return -1;
	}
	
	if(para == NULL)
	{
		return -1;
	}

#if 1
	pthread_mutex_lock(&g_chn_info[chn].set_param_lock);
	
	if(stream)
	{
		g_chn_info[chn].param.subenc.para = *para;
		g_chn_info[chn].param.subenc.refresh = 1;
	}
	else
	{
		g_chn_info[chn].param.mainenc.para = *para;
		g_chn_info[chn].param.mainenc.refresh = 1;
	}
	
	pthread_mutex_unlock(&g_chn_info[chn].set_param_lock);
#else
	//printf("IPC_CMD_SetOSD: chn=%d name=%s\n",chn,name);
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.SetVENC != NULL)
	{
		//printf("yg IPC_CMD_SetVideoEncoderParam chn%d\n",chn);
		g_chn_info[chn].ops.SetVENC(chn, stream, para);
	}
	else
	{
		printf("IPC protocol not support SETVENC.\n");
		return -1;
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
#endif
	return 0;
}


int IPC_CMD_Reboot(int chn)
{
	return 0;
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		return -1;
	}
#if 1
	pthread_mutex_lock(&g_chn_info[chn].set_param_lock);

	g_chn_info[chn].param.bReboot= 1;
	
	pthread_mutex_unlock(&g_chn_info[chn].set_param_lock);
#else
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	if(g_chn_info[chn].ops.Reboot != NULL)
	{
		g_chn_info[chn].ops.Reboot(chn);
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
#endif

	return 0;
}

int IPC_CMD_RequestIFrame(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		return -1;
	}
	
	int idx = chn;
	
	if(chn >= (int)(g_chn_count))
	{
		chn -= g_chn_count;
	}
	
	//printf("chn%d IPC_CMD_RequestIFrame try lock\n",chn);
	
	//pthread_mutex_lock(&g_chn_info[chn].lock);
	
	//printf("chn%d IPC_CMD_RequestIFrame get lock\n",chn);
	
	if(g_chn_info[chn].ops.RequestIFrame != NULL)
	{
		g_chn_info[chn].ops.RequestIFrame(idx);
	}
	
	//pthread_mutex_unlock(&g_chn_info[chn].lock);
	
	//printf("chn%d IPC_CMD_RequestIFrame over\n",chn);
	
	return 0;
}

int Onvif_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
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
	
	if(ipcam->net_mask)
	{
		pnw->ip_address = ipcam->dwIp;
		pnw->net_mask = ipcam->net_mask;
		pnw->net_gateway = ipcam->net_gateway;
		pnw->dns1 = ipcam->dns1;
		pnw->dns2 = ipcam->dns2;
		
		return 0;
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam->dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam->address, strlen(ipcam->address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam->user,ipcam->pwd);
	
	device_ip dev_ip;
	memset(&dev_ip,0,sizeof(dev_ip));
	((device_ptz*)receiver_onvif)->handle_mngmt_get_ip(&dev_ip);
	if(ipcam->dwIp)
	{
		pnw->ip_address = ipcam->dwIp;
	}
	else
	{
		pnw->ip_address = inet_addr(dev_ip.ip);
	}
	pnw->net_mask = (unsigned int)(-1) - ((1 << (32 - dev_ip.prefix)) - 1);
	pnw->net_mask = htonl(pnw->net_mask);
	printf("Onvif_GetNetworkParam prefix=%d,net_mask=0x%08x\n",dev_ip.prefix,pnw->net_mask);
	
	device_gateway dev_gateway;
	memset(&dev_gateway,0,sizeof(dev_gateway));
	((device_ptz*)receiver_onvif)->handle_mngmt_get_default_gateway(&dev_gateway);
	pnw->net_gateway = inet_addr(dev_gateway.gateway);
	
	device_dns dev_dns;
	memset(&dev_dns,0,sizeof(dev_dns));
	((device_ptz*)receiver_onvif)->handle_mngmt_get_dns(&dev_dns);
	pnw->dns1 = inet_addr(dev_dns.dns);
	pnw->dns2 = 0;
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

int Onvif_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
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
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam->dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam->address, strlen(ipcam->address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam->user,ipcam->pwd);
	
	int prefix_length = 0;
	
	struct in_addr host;
	host.s_addr = pnw->ip_address;
	strcpy(ipaddr, inet_ntoa(host));
	
	int i = 0;
	int bits = 32;//sizeof(pnw->net_mask)*8;
	unsigned int val = ntohl(pnw->net_mask);
	for(i=bits-1;i>=0;i--)
	{
		if((val>>i)&0x1)
		{
			prefix_length++;
		}
		else
		{
			break;
		}
	}
	printf("Onvif_SetNetworkParam:prefix_length=%d\n",prefix_length);
	((device_ptz*)receiver_onvif)->handle_mngmt_set_ip_prefix(ipaddr, strlen(ipaddr), prefix_length);
	
	host.s_addr = pnw->net_gateway;
	strcpy(ipaddr, inet_ntoa(host));
	((device_ptz*)receiver_onvif)->handle_mngmt_set_default_gateway(ipaddr, strlen(ipaddr));
	
	host.s_addr = pnw->dns1;
	strcpy(ipaddr, inet_ntoa(host));
	((device_ptz*)receiver_onvif)->handle_mngmt_set_dns(ipaddr, strlen(ipaddr));
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

int Onvif_CMD_Open(int chn)
{
	return 0;
}

int Onvif_CMD_Close(int chn)
{
	return 0;
}

int Onvif_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	int ret;
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!IPC_GetLinkStatus(chn))
	{
		printf("Onvif_CMD_PtzCtrl: chn%d vlost!!!\n",chn);
		return -1;
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}

	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	if(((device_ptz*)receiver_onvif)->handle_image_setting_support())//先得到一些前提条件
	{
		//那么是设置主码流还是子码流?  0 为主码流
		//由chn  来看
		ret = ((device_ptz*)receiver_onvif)->handle_image_set_brightness(0,para->brightness);
		ret = ((device_ptz*)receiver_onvif)->handle_image_set_color_saturation(0,para->saturation);
		ret = ((device_ptz*)receiver_onvif)->handle_image_set_contrast(0,para->contrast);
		//ret = ((device_ptz*)receiver_onvif)->			// not support hue
	}

	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}


int Onvif_CMD_GetImageParam(int chn, video_image_para_t *para)
{
/*	int ret;
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!IPC_GetLinkStatus(chn))
	{
		printf("Onvif_CMD_PtzCtrl: chn%d vlost!!!\n",chn);
		return -1;
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}


	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	if(((device_ptz*)receiver_onvif)->handle_image_setting_support())//先得到一些前提条件
	{
		current_image_config* ncurrent_image_config;
		ret = ((device_ptz*)receiver_onvif)->handle_image_current_config(0,&ncurrent_image_config);	//1 or 0 ?
		if(ret >= 0)
		{

		}
	}
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
*/
	return 0;
}

int Onvif_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!IPC_GetLinkStatus(chn))
	{
		printf("Onvif_CMD_PtzCtrl: chn%d vlost!!!\n",chn);
		return -1;
	}
	
	//printf("Onvif_CMD_PtzCtrl: chn%d start......\n",chn);
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	uint32_t stream_index = 0;
	
	if(((device_ptz*)receiver_onvif)->handle_ptz_support())
	{
		//printf("Onvif_CMD_PtzCtrl: chn%d support ptz cmd\n",chn);
		
		if(cmd == EM_PTZ_CMD_START_TILEUP)
		{
			unsigned int tiltSpeed = *((unsigned int *)data);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_UP,0,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_TILEDOWN)
		{
			unsigned int tiltSpeed = *((unsigned int *)data);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_DOWN,0,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_PANLEFT)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_LEFT,panSpeed,0,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_PANRIGHT)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			//printf("Onvif_CMD_PtzCtrl: chn%d ptz right speed=%d\n",chn,panSpeed);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_RIGHT,panSpeed,0,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_LEFTUP)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			unsigned int tiltSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_UL,panSpeed,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_LEFTDOWN)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			unsigned int tiltSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_DL,panSpeed,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_RIGHTUP)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			unsigned int tiltSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_UR,panSpeed,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_RIGHTDOWN)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			unsigned int tiltSpeed = *((unsigned int *)data);
			panSpeed = panSpeed ? (panSpeed) : (LS_PTZ_SPEED_MAX/3);
			tiltSpeed = tiltSpeed ? (tiltSpeed) : (LS_PTZ_SPEED_MAX/3);
			
			panSpeed = panSpeed * LS_PTZ_SPEED_MAX / 10;
			tiltSpeed = tiltSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_DR,panSpeed,tiltSpeed,0,0);
		}
		else if(cmd == EM_PTZ_CMD_START_ZOOMTELE)
		{
			unsigned int zoomSpeed = *((unsigned int *)data);
			zoomSpeed = zoomSpeed ? (zoomSpeed) : (1);
			//printf("Onvif_CMD_PtzCtrl: chn%d zoomin speed=%d\n",chn,zoomSpeed);
			
			zoomSpeed = zoomSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_ZOOMIN,0,0,zoomSpeed,0);
		}
		else if(cmd == EM_PTZ_CMD_START_ZOOMWIDE)
		{
			unsigned int zoomSpeed = *((unsigned int *)data);
			zoomSpeed = zoomSpeed ? (zoomSpeed) : (1);
			//printf("Onvif_CMD_PtzCtrl: chn%d zoomout speed=%d\n",chn,zoomSpeed);
			
			zoomSpeed = zoomSpeed * LS_PTZ_SPEED_MAX / 10;
			
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_ZOOMOUT,0,0,zoomSpeed,0);
		}
		else if(cmd == EM_PTZ_CMD_START_FOCUSNEAR)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_START_FOCUSFAR)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_START_IRISSMALL)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_START_IRISLARGE)
		{
			
		}
		else if(cmd >= EM_PTZ_CMD_STOP_TILEUP && cmd < EM_PTZ_CMD_PRESET_SET)
		{
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_RIGHT_STOP,0,0,0,0);
		}
		else if(cmd == EM_PTZ_CMD_PRESET_SET)
		{
			unsigned int preset = *((unsigned int *)data);
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_POS_SET,0,0,0,preset);
		}
		else if(cmd == EM_PTZ_CMD_PRESET_GOTO)
		{
			unsigned int preset = *((unsigned int *)data);
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_POS_CALL,0,0,0,preset);
		}
		else if(cmd == EM_PTZ_CMD_PRESET_CLEAR)
		{
			unsigned int preset = *((unsigned int *)data);
			((device_ptz*)receiver_onvif)->do_ptz(stream_index,LS_PTZ_CMD_POS_CLEAR,0,0,0,preset);
		}
		else if(cmd == EM_PTZ_CMD_LIGHT_ON)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_LIGHT_OFF)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_AUX_ON)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_AUX_OFF)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_AUTOPAN_ON)
		{
			
		}
		else if(cmd == EM_PTZ_CMD_AUTOPAN_OFF)
		{
			
		}
	}
	else
	{
		printf("Onvif_CMD_PtzCtrl: chn%d do not support ptz cmd\n",chn);
	}
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

#if 0
static const char* szTimeZoneInfo[] = 
{
#if 1
	"GMT-12",
	"GMT-11",
	"GMT-10",
	"GMT-09",
	"GMT-08",
	"GMT-07",
	"GMT-06",
	"GMT-05",
	"GMT-04:30",
	"GMT-04",
	"GMT-03:30",
	"GMT-03",
	"GMT-02",
	"GMT-01",
	"GMT",//"GMT+00:00",//"GMT+0",
	"GMT+01",
	"GMT+02",
	"GMT+03",
	"GMT+03:30",
	"GMT+04",
	"GMT+04:30",
	"GMT+05",
	"GMT+05:30",
	"GMT+05:45",
	"GMT+06",
	"GMT+06:30",
	"GMT+07",
	"GMT+08",//"ChinaStandardTime-8",
	"GMT+09",
	"GMT+09:30",
	"GMT+10",
	"GMT+11",
	"GMT+12",
	"GMT+13",
#else
	"GMT-12:00",
	"GMT-11:00",
	"GMT-10:00",
	"GMT-09:00",
	"GMT-08:00",
	"GMT-07:00",
	"GMT-06:00",
	"GMT-05:00",
	"GMT-04:30",
	"GMT-04:00",
	"GMT-03:30",
	"GMT-03:00",
	"GMT-02:00",
	"GMT-01:00",
	"GMT",//"GMT+00:00",//"GMT+0",
	"GMT+01:00",
	"GMT+02:00",
	"GMT+03:00",
	"GMT+03:30",
	"GMT+04:00",
	"GMT+04:30",
	"GMT+05:00",
	"GMT+05:30",
	"GMT+05:45",
	"GMT+06:00",
	"GMT+06:30",
	"GMT+07:00",
	"GMT+08:00",
	"GMT+09:00",
	"GMT+09:30",
	"GMT+10:00",
	"GMT+11:00",
	"GMT+12:00",
	"GMT+13:00",
#endif
};
#endif

static const char* szTimeZoneInfo_IEEE[] = 
{
	"GMT12",//"GMT-12:00",
	"SST11",//"GMT-11:00",
	"HAST10",//"GMT-10:00",
	"AKST9",//"GMT-09:00",
	"PST8",//"GMT-08:00",
	"MST7",//"GMT-07:00",
	"CST6",//"GMT-06:00",
	"EST5",//"GMT-05:00",
	"VET4:30",//"GMT-04:30",//加拉加斯（委内瑞拉首都）
	"AST4",//"GMT-04:00",
	"NST3:30",//"GMT-03:30",//纽芬兰
	"BRT3",//"GMT-03:00",
	"GST2",//"GMT-02:00",
	"CVT1",//"GMT-01:00",
	"GMT0",//"GMT",//"GMT+00:00",
	"CET-1",//"GMT+01:00",
	"EET-2",//"GMT+02:00",
	"AST-3",//"GMT+03:00",
	"IRT-3:30",//"GMT+03:30",//伊朗首都德黑兰
	"GMT-4",//"GMT+04:00",
	"AFT-4:30",//"GMT+04:30",//喀布尔
	"PKT-5",//"GMT+05:00",
	"IST-5:30",//"GMT+05:30",//马德拉斯，加尔各答，孟买，新德里
	"NPT-5:45",//"GMT+05:45",//加德满都
	"BDT-6",//"GMT+06:00",
	"MMT-6:30",//"GMT+06:30",//仰光
	"ICT-7",//"GMT+07:00",
	"CST-8",//"GMT+08:00",
	"JST-9",//"GMT+09:00",
	"CST-9:30",//"GMT+09:30",//阿德莱德
	"EST-10",//"GMT+10:00",
	"SBT-11",//"GMT+11:00",
	"NZST-12",//"GMT+12:00",
	"TOT-13",//"GMT+13:00",
};

static const char* GetTZInfo(int index)
{
#if 1
	if(index < 0 || index >= (int)(sizeof(szTimeZoneInfo_IEEE)/sizeof(szTimeZoneInfo_IEEE[0])))
	{
		return "CST-8";
	}
	
	return szTimeZoneInfo_IEEE[index];
#else
	if(index < 0 || index >= (int)(sizeof(szTimeZoneInfo)/sizeof(szTimeZoneInfo[0])))
	{
		return "GMT+08";//"ChinaStandardTime-8";
	}
	
	return szTimeZoneInfo[index];
#endif
}

#if 0
static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

static int GetTZOffset(int index)
{
	if(index < 0 || index >= (int)(sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0])))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}
#endif

int Onvif_CMD_SetTime(int chn, time_t t, int force)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !IPC_GetLinkStatus(chn))
		{
			//printf("Onvif_CMD_SetTime: chn%d vlost!!!\n",chn);
			return -1;
		}
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	if(t == 0)
	{
		t = time(NULL);// + 1;//???
	}
	
	int nTimeZone = IPC_GetTimeZone();
	//t += GetTZOffset(nTimeZone);
	
	//printf("Onvif_CMD_SetTime: chn%d start set,TimeZone=%s!!!\n",chn,GetTZInfo(nTimeZone));
	
	//((device_ptz*)receiver_onvif)->handle_mngmt_set_time_zone("GMT+00:00");//海芯摄像机不对
	//((device_ptz*)receiver_onvif)->handle_mngmt_set_time_zone("GMT+0");//海芯摄像机不对
	//((device_ptz*)receiver_onvif)->handle_mngmt_set_time_zone("GMT");
	//((device_ptz*)receiver_onvif)->handle_mngmt_set_time_zone(GetTZInfo(nTimeZone));
	((device_ptz*)receiver_onvif)->handle_mngmt_set_date_time(t,GetTZInfo(nTimeZone));
	//((device_ptz*)receiver_onvif)->handle_mngmt_set_date_time(t,NULL);
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

int Onvif_CMD_Reboot(int chn)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	printf("$$$$$$$$$$chn%d onvif-ipc reboot$$$$$$$$$$\n",chn);
	
	((device_ptz*)receiver_onvif)->handle_mngmt_system_reboot();
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

int Onvif_CMD_RequestIFrame(int chn)
{
	return -1;
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_chn_count*2))
	{
		return -1;
	}
	
	if(!IPC_GetLinkStatus(chn))
	{
		//printf("Onvif_CMD_RequestIFrame: chn%d vlost!!!\n",chn);
		return -1;
	}
	
	//int idx = chn;
	
	if(chn >= (int)(g_chn_count))
	{
		chn -= g_chn_count;
	}
	
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!ipcam.enable)
	{
		return -1;
	}
	
	ONVIF_LOCK();
	
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		ONVIF_UNLOCK();
		return -1;
	}
	
	struct in_addr serv;
	serv.s_addr = ipcam.dwIp;
	char ipaddr[32] = {0};
	strcpy(ipaddr, inet_ntoa(serv));
	receiver_onvif->set_ip(ipaddr, strlen(ipaddr));
	
	receiver_onvif->set_xaddrs(ipcam.address, strlen(ipcam.address));
	
	((device_ptz*)receiver_onvif)->set_username_password(ipcam.user,ipcam.pwd);
	
	//((device_ptz*)receiver_onvif)->handle_mngmt_system_reboot();
	
	delete receiver_onvif;
	
	ONVIF_UNLOCK();
	
	return 0;
}

//yaogang modify 20141225
/**********************
chn: IPC通道0-15
StreamType: 抓图码流，0:主码流,1:次码流,2:第三码流,3:第四码流
**********************/
int IPC_Snapshot_RegisterCB(int chn, int StreamType, SNAP_CALLBACK pSnapCB)
{
	int ret = 0;
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_chn_count)
	{
		printf("%s chn%d invalid\n", __func__, chn);
		return -1;
	}

	int snap_chn = StreamType ? chn+g_chn_count : chn; 

	if (!IPC_GetLinkStatus(snap_chn))
	{
		//printf("Onvif_CMD_RequestIFrame: chn%d vlost!!!\n",chn);
		printf("%s IPC_GetLinkStatus(chn%d) == 0\n", __func__, snap_chn);
		return -1;
	}
	
	if(pSnapCB == NULL)
	{
		printf("%s chn%d, pSnapCB == NULL\n", __func__, snap_chn);
		return -1;
	}
	
	//printf("%s lock1 chn%d\n", __func__, chn);
	printf("%s chn%d, 1\n", __func__, chn);
	pthread_mutex_lock(&g_stream_info[snap_chn].lock);
	//printf("%s lock2 chn%d\n", __func__, chn);
	
	if(g_stream_info[snap_chn].ops.Snapshot_RegisterCB != NULL)
	{
		ret = g_stream_info[snap_chn].ops.Snapshot_RegisterCB(snap_chn, StreamType, pSnapCB);
	}
	printf("%s chn%d, 2\n", __func__, chn);
	//printf("%s unlock1 chn%d\n", __func__, chn);
	pthread_mutex_unlock(&g_stream_info[snap_chn].lock);
	//printf("%s unlock2 chn%d\n", __func__, chn);
	return ret;
}


