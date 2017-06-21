#ifndef _RIVER_H_
#define _RIVER_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int RIVER_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict);

int RIVER_Init(unsigned int max_client_num);
int RIVER_DeInit();

int RIVER_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int RIVER_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

int RIVER_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int RIVER_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int RIVER_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int RIVER_GetLinkStatus(int chn);

int RIVER_CMD_Open(int chn);
int RIVER_CMD_Close(int chn);

int RIVER_CMD_SetImageParam(int chn, video_image_para_t *para);
int RIVER_CMD_GetImageParam(int chn, video_image_para_t *para);

int RIVER_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int RIVER_CMD_SetTime(int chn, time_t t, int force);

int RIVER_CMD_Reboot(int chn);

int RIVER_CMD_RequestIFrame(int chn);

#ifdef __cplusplus
}
#endif

#endif

