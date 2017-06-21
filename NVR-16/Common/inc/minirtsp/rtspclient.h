#ifndef __RTSP_CLIENT_H__
#define __RTSP_CLIENT_H__

#define RTSPC_STATE_INIT				(0)
#define RTSPC_STATE_BUFFER_FILLING		(1)
#define RTSPC_STATE_BUFFER_AVAILABLE	(2)

#ifdef __cplusplus
extern "C" {
#endif

//extern void *RTSPC_NETWORK_proc(void *param);
//extern void *RTSPC_DECODE_proc(void *param);
//extern int RTSPC_daemon(void **rtsp,char *url,char *user,char *pwd,int bInterleaved,int buffer_time,int chn,int stream,int *trigger);
//extern int RTSPC_daemon2(void **rtsp,char *url,char *user,char *pwd,int bInterleaved,int buffer_time,int chn,int stream,int *trigger);
//extern int RTSPC_test(int argc,char *argv[]);

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

