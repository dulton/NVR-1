#ifndef _FULLHAN_H_
#define _FULLHAN_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int FH_Init(unsigned int max_client_num);
int FH_DeInit();

int FH_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int FH_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

int FH_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int FH_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int FH_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int FH_GetLinkStatus(int chn);

int FH_CMD_Open(int chn);
int FH_CMD_Close(int chn);

int FH_CMD_SetImageParam(int chn, video_image_para_t *para);
int FH_CMD_GetImageParam(int chn, video_image_para_t *para);

int FH_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int FH_CMD_SetTime(int chn, time_t t, int force);

int FH_CMD_Reboot(int chn);

int FH_CMD_RequestIFrame(int chn);

#ifdef __cplusplus
}
#endif

#endif

