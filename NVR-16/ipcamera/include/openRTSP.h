#ifndef _OPENRTSP_H_
#define _OPENRTSP_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int RTSPC_Init(unsigned int max_client_num);
int RTSPC_DeInit();

int RTSPC_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int RTSPC_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);

int RTSPC_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int RTSPC_GetLinkStatus(int chn);

#ifdef __cplusplus
}
#endif

#endif

