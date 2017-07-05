#ifndef _IPCAMERA_H_
#define _IPCAMERA_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRO_TYPE_ONVIF		0x01//onvif
#define PRO_TYPE_HAIXIN		0x02//海芯威视
#define PRO_TYPE_JUAN		0x04//九安
#define PRO_TYPE_KLW		0x08//科力维
#define PRO_TYPE_FULLHAN	0x10//富瀚
#define PRO_TYPE_ANNI		0x20//安尼
#define PRO_TYPE_RIVER		0x40//黄河
#define PRO_TYPE_XM			0x80//雄迈
//yaogang modify 20151222
#define PRO_TYPE_NVR		0X100//搜索并添加NVR

//#define IPC_PROTOCOL_TEST //test record disk error by yaogang 20170222
#ifdef IPC_PROTOCOL_TEST
#define PRO_TYPE_IPC_TEST	0X200//test record disk error by yaogang 20170222
#endif

#define SUBSTREAM_BUFSIZE (300*1024)
#define MAINSTREAM_BUFSIZE (800*1024)



#define STREAM_TYPE_MAIN	0//主码流
#define STREAM_TYPE_SUB		1//次码流
#define STREAM_TYPE_THIRD	2//子码流

#define TRANS_TYPE_UDP		0
#define TRANS_TYPE_TCP		1

#define IPC_TYPE_D1			0
#define IPC_TYPE_720P		1
#define IPC_TYPE_1080P		2

typedef struct ipc_unit
{
	unsigned char channel_no;
	unsigned char enable;
	unsigned char stream_type;
	unsigned char trans_type;
	unsigned int protocol_type;
	unsigned int dwIp;
	unsigned short wPort;
	unsigned char force_fps;
	unsigned char frame_rate;
	char user[32];
	char pwd[32];
	char name[32];
	char uuid[64];
	char address[64];//onvif使用
	char ipc_type;
	//char reserved[47];
	//yaogang modify for yuetian private
	volatile char main_audio_sw;//主码流音频开关,未使用
	volatile char sub_audio_sw;//次码流音频开关,未使用
	char reserved[45];
	//char reserved[39];
	//volatile int main_audio_sw;//主码流音频开关
	//volatile int sub_audio_sw;//次码流音频开关
	unsigned int net_mask;
	unsigned int net_gateway;
	unsigned int dns1;
	unsigned int dns2;
}ipc_unit;

typedef struct ipc_node
{
	ipc_unit ipcam;
	struct ipc_node *next;
}ipc_node;

typedef struct
{
	char name[32];
	char refresh;
}ipc_osd_param;

typedef struct
{
	unsigned char flag;
	char reserved[127];
}md_para_t;

typedef struct
{
	md_para_t para;
	char refresh;
}ipc_md_param;

typedef struct
{
	int mWidth;
	int mHeight;
} Resv;

typedef struct
{
	int Min;
	int Max;
} Range;

typedef struct
{
	Resv mResv;
	unsigned int  RateType;     //流模式(0为定码流，1为变码流)
	float Quality;
	int FrameInterval;

	int FrameRate;
	int BitRate;

	int GovLength;
} VideoEncoderParam;

//csp modify 20140812
typedef struct
{
	VideoEncoderParam para;
	char refresh;
}ipc_venc_param;


typedef struct
{
	ipc_md_param md;
	ipc_osd_param osd;
	ipc_venc_param mainenc;//csp modify 20140812
	ipc_venc_param subenc;//csp modify 20140812
	int bReboot;//yaogang modify 20140918
}ipc_param;


typedef enum
{
	MEDIA_PT_H264 = 1,
	MEDIA_PT_G711,
	MEDIA_PT_G726,
	MEDIA_PT_RAWAUDIO,
	MEDIA_PT_PCMU
}media_type_e;

typedef enum
{
	REAL_FRAME_TYPE_NONE = 0,/* yg add */
	REAL_FRAME_TYPE_P = 1, /*PSLICE types*/
	REAL_FRAME_TYPE_I = 5  /*ISLICE types*/
}real_frame_type_e;

typedef struct
{
	int chn;
	media_type_e media_type;
	real_frame_type_e frame_type;
	unsigned long long pts;//us
	int rsv;
	unsigned int len;
	unsigned char *data;
	unsigned short width;
	unsigned short height;
	unsigned char mdevent;
}real_stream_s;

typedef enum
{
	REAL_STREAM_STATE_START = 1,
	REAL_STREAM_STATE_STOP,
	REAL_STREAM_STATE_LINK,
	REAL_STREAM_STATE_LOST
}real_stream_state_e;

typedef int (*RealStreamCB)(real_stream_s *stream, unsigned int dwContext);
typedef int (*StreamStateCB)(int chn, real_stream_state_e msg);

typedef int (*RealStream_Start)(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
typedef int (*RealStream_Startbyurl)(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp);
typedef int (*RealStream_Stop)(int chn);
typedef int (*RealStream_GetLinkStatus)(int chn);
//抓图接口
typedef int (*SNAP_CALLBACK)(char *SnapData,	/*抓拍数据指针*/
                                  unsigned int DataSize,		/*抓拍数据长度*/
                                  int width,					/*抓拍返回的图像宽高*/
                                  int height,			
                                  int count,				 /*抓拍的第几张*/
                                  void* pUserData);		/*当前用作通道*/
//注册抓图回调

typedef int (*RealStream_Snapshot_RegisterCB)(int chn, int StreamType, SNAP_CALLBACK pSnapCB); // 0: MainStream 1: SubStream


typedef struct
{
	RealStream_Start Start;
	RealStream_Startbyurl Startbyurl;
	RealStream_Stop Stop;
	RealStream_GetLinkStatus GetLinkStatus;
	RealStream_Snapshot_RegisterCB Snapshot_RegisterCB;
}real_stream_op_set;

int IPC_Init(unsigned int chn_num);
int IPC_DeInit();

int IPC_SetTimeZone(int nTimeZone, int syncflag, int syncing);
int IPC_GetTimeZone();

int IPC_Set(int chn, ipc_unit *ipcam);
int IPC_Get(int chn, ipc_unit *ipcam);

int IPC_GetStreamResolution(int chn, int *w, int *h);
int IPC_SetStreamResolution(int chn, int w, int h);

//return value : 1 - Link; 0 - Lost
int IPC_GetLinkStatus(int chn);

unsigned int IPC_get_alarm_IPCExt(void);
unsigned int  IPC_get_alarm_IPCCover(void);

void IPC_set_alarm_IPCExt(int chn);
void  IPC_set_alarm_IPCCover(int chn);


int IPC_Start(int chn);
int IPC_Stop(int chn);

int IPC_RegisterCallback(RealStreamCB pStreamCB, StreamStateCB pStateCB);

//搜索网络摄像机
int IPC_Search(ipc_node** head, unsigned int protocol_type, unsigned char check_conflict);

//释放网络摄像机
int IPC_Free(ipc_node* head);

typedef struct
{
	unsigned int ip_address;
	unsigned int net_mask;
	unsigned int net_gateway;
	unsigned int dns1;
	unsigned int dns2;
}ipc_neteork_para_t;

int IPC_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int IPC_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);


typedef struct
{
	unsigned short hue;					//hue:0~255
	unsigned short saturation;			//satuature:0~255
	unsigned short contrast;			//contrast:0~255
	unsigned short brightness;			//brightness:0~255
}video_image_para_t;



typedef enum
{
	EM_PTZ_CMD_START_TILEUP = 0,
	EM_PTZ_CMD_START_TILEDOWN,
	EM_PTZ_CMD_START_PANLEFT,
	EM_PTZ_CMD_START_PANRIGHT,
	EM_PTZ_CMD_START_LEFTUP,
	EM_PTZ_CMD_START_LEFTDOWN,
	EM_PTZ_CMD_START_RIGHTUP,
	EM_PTZ_CMD_START_RIGHTDOWN,
	EM_PTZ_CMD_START_ZOOMWIDE,
	EM_PTZ_CMD_START_ZOOMTELE,
	EM_PTZ_CMD_START_FOCUSNEAR,
	EM_PTZ_CMD_START_FOCUSFAR,
	EM_PTZ_CMD_START_IRISSMALL,
	EM_PTZ_CMD_START_IRISLARGE,
	//
	EM_PTZ_CMD_STOP_TILEUP = 30,
	EM_PTZ_CMD_STOP_TILEDOWN,
	EM_PTZ_CMD_STOP_PANLEFT,
	EM_PTZ_CMD_STOP_PANRIGHT,
	EM_PTZ_CMD_STOP_LEFTUP,
	EM_PTZ_CMD_STOP_LEFTDOWN,
	EM_PTZ_CMD_STOP_RIGHTUP,
	EM_PTZ_CMD_STOP_RIGHTDOWN,
	EM_PTZ_CMD_STOP_ZOOMWIDE,
	EM_PTZ_CMD_STOP_ZOOMTELE,
	EM_PTZ_CMD_STOP_FOCUSNEAR,
	EM_PTZ_CMD_STOP_FOCUSFAR,
	EM_PTZ_CMD_STOP_IRISSMALL,
	EM_PTZ_CMD_STOP_IRISLARGE,
	//
	EM_PTZ_CMD_PRESET_SET = 100,
	EM_PTZ_CMD_PRESET_CLEAR,
	EM_PTZ_CMD_PRESET_GOTO,
	//
	EM_PTZ_CMD_AUTOSCAN_ON = 110,
	EM_PTZ_CMD_AUTOSCAN_OFF,
	EM_PTZ_CMD_SETLIMIT_LEFT,
	EM_PTZ_CMD_SETLIMIT_RIGHT,
	//
	EM_PTZ_CMD_AUTOPAN_ON = 120,
	EM_PTZ_CMD_AUTOPAN_OFF,
	//
	EM_PTZ_CMD_START_TOUR = 130,
	EM_PTZ_CMD_STOP_TOUR,
	EM_PTZ_CMD_ADD_TOUR,
	EM_PTZ_CMD_DELETE_TOUR,
	EM_PTZ_CMD_START_TOUR_AUTO,
	EM_PTZ_CMD_STOP_TOUR_AUTO,
	//
	EM_PTZ_CMD_START_PATTERN = 140,
	EM_PTZ_CMD_STOP_PATTERN,
	EM_PTZ_CMD_SET_START,
	EM_PTZ_CMD_SET_STOP,
	//
	EM_PTZ_CMD_AUX_ON = 150,
	EM_PTZ_CMD_AUX_OFF,
	//
	EM_PTZ_CMD_LIGHT_ON = 160,
	EM_PTZ_CMD_LIGHT_OFF,
	//
	EM_PTZ_CMD_SETSPEED = 253,
	EM_PTZ_CMD_NULL = 254,
}EMPTZCMDTYPE;

typedef int (*Cmd_Open)(int chn);
typedef int (*Cmd_Close)(int chn);
typedef int (*Cmd_SetImageParam)(int chn, video_image_para_t *para);
typedef int (*Cmd_GetImageParam)(int chn, video_image_para_t *para);
typedef int (*Cmd_PtzCtrl)(int chn, EMPTZCMDTYPE cmd, void* data);
typedef int (*Cmd_SetTime)(int chn, time_t t, int force);
typedef int (*Cmd_SetMD)(int chn, md_para_t *para);
typedef int (*Cmd_SetOSD)(int chn, char *name);
typedef int (*Cmd_SetVENC)(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流
typedef int (*Cmd_GetVENC)(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流
typedef int (*Cmd_GetAudioSwitchStatus)(int chn);
typedef int (*Cmd_SetAudioSwitchStatus)(int chn, int bswitch);
typedef int (*Cmd_Reboot)(int chn);
typedef int (*Cmd_RequestIFrame)(int chn);

//typedef int (*Cmd_Snapshot_RegisterCB)(int chn, SNAP_CALLBACK pSnapCB);


typedef struct
{
	Cmd_Open Open;
	Cmd_Close Close;
	Cmd_SetImageParam SetImageParam;
	Cmd_GetImageParam GetImageParam;
	Cmd_PtzCtrl PtzCtrl;
	Cmd_SetTime SetTime;
	Cmd_SetMD SetMD;
	Cmd_SetOSD SetOSD;
	Cmd_SetVENC SetVENC;
	Cmd_GetVENC GetVENC;
	Cmd_Reboot Reboot;
	Cmd_RequestIFrame RequestIFrame;
	Cmd_GetAudioSwitchStatus GetAudioSwitchStatus;
	Cmd_SetAudioSwitchStatus SetAudioSwitchStatus;
	//Cmd_Snapshot_RegisterCB Snapshot_RegisterCB;
}ipc_cmd_op_set;

int IPC_CMD_Open(int chn);
int IPC_CMD_Close(int chn);

int IPC_CMD_SetImageParam(int chn, video_image_para_t *para);
int IPC_CMD_GetImageParam(int chn, video_image_para_t *para);

int IPC_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int IPC_CMD_SetTime(int chn, time_t t, int force);

int IPC_CMD_SetMD(int chn, md_para_t *para);
//yaogang modiy 20140918
int IPC_CMD_SetOSD(int chn, char *name);

int IPC_CMD_Reboot(int chn);

int IPC_CMD_RequestIFrame(int chn);
int IPC_CMD_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int IPC_CMD_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int IPC_CMD_SetVideoEncoderParam(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流
int IPC_CMD_GetVideoEncoderParam(int chn, int stream, VideoEncoderParam *para);//0-主码流,1-次码流
int IPC_CMD_SetAudioSwitchStatus(int chn, int bswitch);// 0:关1:开
int IPC_CMD_GetAudioSwitchStatus(int chn);// 0:关1:开

/*
chn: IPC通道
StreamType: 抓图码流，0:主码流,1:次码流,2:第三码流,3:第四码流
*/
int IPC_Snapshot_RegisterCB(int chn, int StreamType, SNAP_CALLBACK pSnapCB);


//add by liu
int Get_StreamKbps(unsigned int chn);

#ifdef __cplusplus
}
#endif

#endif

