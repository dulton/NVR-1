#ifndef _ANNI_H_
#define _ANNI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#include "AnniNetSdk.h"
#include "ipcamera.h"

int ANNI_Free();
int ANNI_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict);

int Anni_Init(unsigned int max_client_num);
int Anni_DeInit();

int Anni_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int Anni_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

int Anni_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int Anni_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int Anni_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int Anni_GetLinkStatus(int chn);

int Anni_CMD_Open(int chn);
int Anni_CMD_Close(int chn);

int Anni_CMD_SetImageParam(int chn, video_image_para_t *para);
int Anni_CMD_GetImageParam(int chn, video_image_para_t *para);

int Anni_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int Anni_CMD_SetTime(int chn, time_t t, int force);

int Anni_CMD_Reboot(int chn);

int Anni_CMD_RequestIFrame(int chn);

#endif

