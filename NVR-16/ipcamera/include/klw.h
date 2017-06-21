#ifndef _KLW_H_
#define _KLW_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int KLW_Init(unsigned int max_client_num);
int KLW_DeInit();

int KLW_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int KLW_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int KLW_GetAudioSwitchStatus(int chn);
int KLW_SetAudioSwitchStatus(int chn);


int KLW_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int KLW_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int KLW_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int KLW_GetLinkStatus(int chn);
int KLW_Snapshot_RegisterCB(int chn, int StreamType, SNAP_CALLBACK pSnapCB);// 0: MainStream 1: SubStream





int KLW_CMD_Open(int chn);
int KLW_CMD_Close(int chn);

int KLW_CMD_SetImageParam(int chn, video_image_para_t *para);
int KLW_CMD_GetImageParam(int chn, video_image_para_t *para);

int KLW_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int KLW_CMD_SetTime(int chn, time_t t, int force);

int KLW_CMD_SetMD(int chn, md_para_t *para);
int KLW_CMD_SetOSD(int chn, char *name);
int KLW_CMD_GetVENC(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流

int KLW_CMD_SetVENC(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流

int KLW_CMD_Reboot(int chn);

int KLW_CMD_RequestIFrame(int chn);
int KLW_CMD_GetAudioSwitchStatus(int chn);
int KLW_CMD_SetAudioSwitchStatus(int chn, int bswitch);




#ifdef __cplusplus
}
#endif

#endif

