#ifndef _NVR_H_
#define _NVR_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int NVR_Init(unsigned int max_client_num);
int NVR_DeInit();

//req_nvr_chn :指示要申请NVR 的哪一路码流
int NVR_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char req_nvr_chn);

int NVR_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int NVR_GetLinkStatus(int chn);

int NVR_Search(ipc_node** head, unsigned char check_conflict);


#ifdef __cplusplus
}
#endif

#endif

