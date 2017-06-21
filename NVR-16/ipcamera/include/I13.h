#ifndef _I13_H_
#define _I13_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCASTADDR		"239.255.255.250"
#define MCASTPORT		8002
#define BUFSIZE			1024

int I13_Init(unsigned int max_client_num);
int I13_DeInit();

int I13_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int I13_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

int I13_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int I13_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int I13_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int I13_GetLinkStatus(int chn);

int I13_CMD_Open(int chn);
int I13_CMD_Close(int chn);

int I13_CMD_SetImageParam(int chn, video_image_para_t *para);
int I13_CMD_GetImageParam(int chn, video_image_para_t *para);

int I13_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int I13_CMD_SetTime(int chn, time_t t, int force);

int I13_CMD_Reboot(int chn);

int I13_CMD_RequestIFrame(int chn);

#ifdef __cplusplus
}
#endif

#endif

