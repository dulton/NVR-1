#ifndef _IPC_TEST_H_
#define _IPC_TEST_H_

#include "ipcamera.h"

#ifdef __cplusplus
extern "C" {
#endif

int IpcTest_Init(unsigned int max_client_num);
int IpcTest_DeInit();

int IpcTest_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int IpcTest_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);


int IpcTest_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);

int IpcTest_Stop(int chn);

//return value : 1 - Link; 0 - Lost
int IpcTest_GetLinkStatus(int chn);



#ifdef __cplusplus
}
#endif

#endif
