
#ifndef _DMSNETSDK_H_
#define _DMSNETSDK_H_

#include "DMSType.h"

#define DMS_NET_GET_PICCFG                  0x020200        //获取图象压缩参数
#define DMS_NET_SET_PICCFG                  0x020201        //设置图象压缩参数
#define DMS_NET_GET_SUPPORT_STREAM_FMT      0x020202        //获取系统支持的图像能力
#define DMS_NET_CMD_IFRAME_REQUEST          0xA010000B		//请求视频关键帧

#define DMS_MAX_STREAMNUM			3
#define DMS_NAME_LEN					32
#define DMS_MAX_IP_LENGTH           16

#define DMS_MAX_PRESET					255
#define DMS_MAX_CRUISE_GROUP_NUM	5
#define DMS_MAX_CRUISE_POINT_NUM		32
#define DMS_PHONENUMBER_LEN			32

#define	ENCODE_FORMAT_MGPE4				0x0
#define	ENCODE_FORMAT_H264				0x4
#define ENCODE_FORMAT_MJPEG				0x5

#define	JBNET_FLAG			9000
#define SERVER_PACK_FLAG	0x03404324
#define CPUTYPE_3518        3518

#define DMS_PASSWD_LEN		32
#define NETCMD_BUF_SIZE			2048

#define     BRD_SET_SERVER_INFO     0x30000001	//设置服务器信息
#define     BRD_GET_SERVER_INFO     0x40000001	//获取服务器信息
#define		CMD_RESTORE				0x00000002	//恢复出厂设置


#define		CMD_UPDATEFLASH				0x00000003
#define		CMD_GET_NETWORK				0x10000009
#define		CMD_SET_NETWORK				0x00000010
#define		CMD_REQUEST_IFRAME			0x00000063

#define MULTI_BROADCAST_SREACH_RECV_PORT		0x8888
#define MULTI_BROADCAST_SREACH_SEND_PORT		0x9888

#define MULTI_BROADCAST_SREACH_IPADDR			"224.188.188.188"

#define NETCMD_BASE				5000
#define NETCMD_KEEP_ALIVE			NETCMD_BASE+1
#define NETCMD_CHANNEL_CHECK_OK		NETCMD_BASE+2
#define NETCMD_NOT_USER				NETCMD_BASE+3
#define NETCMD_MAX_LINK				NETCMD_BASE+4
#define NETCMD_NOT_JBDATA			NETCMD_BASE+5
#define NETCMD_NOT_LOGON			NETCMD_BASE+6
#define NETCMD_LOGON_OK				NETCMD_BASE+7
#define NETCMD_MULTI_OPEN			NETCMD_BASE+8
#define NETCMD_PLAY_CLOSE			NETCMD_BASE+9
#define NETCMD_UPDATE_FILE			NETCMD_BASE+10
#define NETCMD_READ_FILE_LEN			NETCMD_BASE+11
#define NETCMD_READ_FILE_DATA			NETCMD_BASE+12
#define NETCMD_READ_FILE_CLOSE			NETCMD_BASE+13

#define		CMD_GET_NETWORK				0x10000009		//JB_SERVER_NETWORK
#define		CMD_SET_NETWORK				0x00000010		//JB_SERVER_NETWORK

/************************************************************************/
/* 云台控制命令宏定义                                                   */
/************************************************************************/
#define DMS_PTZ_CMD_UP              1
#define DMS_PTZ_CMD_DOWN            2
#define DMS_PTZ_CMD_LEFT            3
#define DMS_PTZ_CMD_RIGHT           4
#define DMS_PTZ_CMD_UP_LEFT			31
#define DMS_PTZ_CMD_UP_RIGHT		32
#define DMS_PTZ_CMD_DOWN_LEFT		33
#define DMS_PTZ_CMD_DOWN_RIGHT		34

#define DMS_PTZ_CMD_FOCUS_SUB       5       //Focus Far
#define DMS_PTZ_CMD_FOCUS_ADD       6       //Focus Near
#define DMS_PTZ_CMD_ZOOM_SUB        7       //Zoom Wide
#define DMS_PTZ_CMD_ZOOM_ADD        8       //Zoom Tele
#define DMS_PTZ_CMD_IRIS_SUB        9       //Iris Close
#define DMS_PTZ_CMD_IRIS_ADD        10      //Iris Open
#define DMS_PTZ_CMD_STOP            11
#define DMS_PTZ_CMD_PRESET          12      //预置
#define DMS_PTZ_CMD_CALL            13      //调用

#define DMS_PTZ_CMD_AUTO_STRAT      14      //自动      //STRAT shoule be START 
#define DMS_PTZ_CMD_AUTO_STOP       15
#define DMS_PTZ_CMD_LIGHT_OPEN      16      //灯光
#define DMS_PTZ_CMD_LIGHT_CLOSE     17      
#define DMS_PTZ_CMD_BRUSH_START     18      //雨刷
#define DMS_PTZ_CMD_BRUSH_STOP      19      
#define DMS_PTZ_CMD_TRACK_START     20      //轨迹
#define DMS_PTZ_CMD_TRACK_STOP      21
#define DMS_PTZ_CMD_TRACK_RUN       22
#define DMS_PTZ_CMD_PRESET_CLS      23      //清除预置点

#define DMS_PTZ_CMD_ADD_POS_CRU		24		/* 将预置点加入巡航序列 */
#define DMS_PTZ_CMD_DEL_POS_CRU		25		/* 将巡航点从巡航序列中删除 */
#define DMS_PTZ_CMD_DEL_PRE_CRU		26		/* 将预置点从巡航序列中删除 */
#define DMS_PTZ_CMD_MOD_POINT_CRU		27		/* 修改巡航点*/
#define DMS_PTZ_CMD_START_CRU		28		/* 开始巡航 */
#define DMS_PTZ_CMD_STOP_CRU		29		/* 停止巡航 */
#define DMS_PTZ_CMD_CRU_STATUS		30		/* 巡航状态 */

typedef struct tagDMS_NET_PTZ_CONTROL_V0
{
	DWORD		dwSize;
	DWORD		dwChannel;
	DWORD       dwPTZCommand;
	int         nPTZParam;  //根据具体命令字而定
	BYTE        byReserve[32];
}DMS_NET_PTZ_CONTROL_V0;

typedef struct
{
	BYTE 	byPointIndex;	//巡航组中的下标,如果值大于DMS_MAX_CRUISE_POINT_NUM表示添加到末尾
	BYTE 	byPresetNo;	//预置点编号
	BYTE 	byRemainTime;	//预置点滞留时间 1~60，默认 10
	BYTE 	bySpeed;		//到预置点速度 1~8，默认 4
}DMS_NET_CRUISE_POINT;

typedef enum
{
    DMS_PT_PCMU = 0,
    DMS_PT_1016 = 1,
    DMS_PT_G721 = 2,
    DMS_PT_GSM = 3,
    DMS_PT_G723 = 4,
    DMS_PT_DVI4_8K = 5,
    DMS_PT_DVI4_16K = 6,
    DMS_PT_LPC = 7,
    DMS_PT_PCMA = 8,
    DMS_PT_G722 = 9,
    DMS_PT_S16BE_STEREO,
    DMS_PT_S16BE_MONO = 11,
    DMS_PT_QCELP = 12,
    DMS_PT_CN = 13,
    DMS_PT_MPEGAUDIO = 14,
    DMS_PT_G728 = 15,
    DMS_PT_DVI4_3 = 16,
    DMS_PT_DVI4_4 = 17,
    DMS_PT_G729 = 18,
    DMS_PT_G711A = 19,
    DMS_PT_G711U = 20,
    DMS_PT_G726 = 21,
    DMS_PT_G729A = 22,
    DMS_PT_LPCM = 23,
    DMS_PT_CelB = 25,
    DMS_PT_JPEG = 26,
    DMS_PT_CUSM = 27,
    DMS_PT_NV = 28,
    DMS_PT_PICW = 29,
    DMS_PT_CPV = 30,
    DMS_PT_H261 = 31,
    DMS_PT_MPEGVIDEO = 32,
    DMS_PT_MPEG2TS = 33,
    DMS_PT_H263 = 34,
    DMS_PT_SPEG = 35,
    DMS_PT_MPEG2VIDEO = 36,
    DMS_PT_AAC = 37,
    DMS_PT_WMA9STD = 38,
    DMS_PT_HEAAC = 39,
    DMS_PT_PCM_VOICE = 40,
    DMS_PT_PCM_AUDIO = 41,
    DMS_PT_AACLC = 42,
    DMS_PT_MP3 = 43,
    DMS_PT_ADPCMA = 49,
    DMS_PT_AEC = 50,
    DMS_PT_X_LD = 95,
    DMS_PT_H264 = 96,
    DMS_PT_D_GSM_HR = 200,
    DMS_PT_D_GSM_EFR = 201,
    DMS_PT_D_L8 = 202,
    DMS_PT_D_RED = 203,
    DMS_PT_D_VDVI = 204,
    DMS_PT_D_BT656 = 220,
    DMS_PT_D_H263_1998 = 221,
    DMS_PT_D_MP1S = 222,
    DMS_PT_D_MP2P = 223,
    DMS_PT_D_BMPEG = 224,
    DMS_PT_MP4VIDEO = 230,
    DMS_PT_MP4AUDIO = 237,
    DMS_PT_VC1 = 238,
    DMS_PT_JVC_ASF = 255,
    DMS_PT_D_AVI = 256,
    DMS_PT_MAX = 257,
    
    DMS_PT_AMR = 1001, /* add by mpp */
    DMS_PT_MJPEG = 1002,
	DMS_PT_H264_HIGHPROFILE = 1003,
	DMS_PT_H264_MAINPROFILE = 1004,
	DMS_PT_H264_BASELINE = 1005,

}DMS_PAYLOAD_TYPE_E;
typedef enum _PROTOCOL_TYPE
{
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1,
    PROTOCOL_MULTI = 2
}PROTOCOL_TYPE;

typedef struct _ACCEPT_HEAD
{
	unsigned long	nFlag;  //9000
	unsigned long	nSockType;//0
}ACCEPT_HEAD,*PACCEPT_HEAD;
/*视频操作（打开，关闭）请求结构体*/
typedef struct _OPEN_HEAD
{
	unsigned long	nFlag;    //协议版本标识
	unsigned long	nID;      //用户ID,在用户登录的时候由前端返回
	PROTOCOL_TYPE   nProtocolType; //网络协议
	unsigned long	nStreamType; //视频流类型，主码流，子码流
	unsigned long	nSerChannel; //视频通道号
	unsigned long	nClientChannel; //标识请求客户端的显示窗口号
}OPEN_HEAD,*POPEN_HEAD;

typedef struct _USER_INFO
{
	char	szUserName[DMS_NAME_LEN];
	char	szUserPsw[DMS_PASSWD_LEN];
	char	MacCheck[36];
}USER_INFO,*PUSER_INFO;
/*网络命令消息头*/
typedef struct _COMM_HEAD
{
	unsigned long	nFlag;
	unsigned long	nCommand;
	unsigned long	nChannel;
	unsigned long	nErrorCode;
	unsigned long	nBufSize;
}COMM_HEAD,*PCOMM_HEAD;

typedef struct _COMM_NODE
{
    COMM_HEAD   commHead;
    char        pBuf[NETCMD_BUF_SIZE];
}COMM_NODE,*PCOMM_NODE;

typedef struct tagDMS_RANGE
{
    int         nMin;
    int         nMax;
}DMS_RANGE, *LDMS_RANGE;

typedef struct tagDMS_SIZE
{
    int           nWidth;
    int           nHeight;
}DMS_SIZE, *LDMS_SIZE;

typedef struct  _tagDMS_SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;    
	WORD wDayOfWeek;
	WORD wDay;    
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
}DMS_SYSTEMTIME;

typedef struct tagDMS_NET_SUPPORT_STREAM_FMT
{
    DWORD       dwSize;                     //struct size
    DWORD       dwChannel;
    DWORD       dwVideoSupportFmt[DMS_MAX_STREAMNUM][4];        // Video Format.
    DMS_RANGE   stVideoBitRate[DMS_MAX_STREAMNUM];
    DMS_SIZE    stVideoSize[DMS_MAX_STREAMNUM][10];     // Video Size(height,width)
    DWORD       dwAudioFmt[4];              //Audio Format
    DWORD       dwAudioSampleRate[4];       //Audio Sample Rate
    BOOL        bSupportAudioAEC;           //b Support Audio Echo Cancellation
    BYTE        byStreamCount;              //max is DMS_MAX_STREAMNUM
    BYTE        byReserve[31];
}DMS_NET_SUPPORT_STREAM_FMT;

//视频通道压缩参数
typedef struct tagDMS_NET_COMPRESSION_INFO
{
    DWORD   dwCompressionType;      //
    DWORD   dwFrameRate;            //帧率 (1-25/30) PAL为25，NTSC为30
    DWORD   dwStreamFormat;         //视频分辨率 (0为CIF,1为D1,2为HALF-D1,3为QCIF)
    WORD    wHeight;            //不做实际用途，只用于显示
    WORD    wWidth;             //不做实际用途，只用于显示
    DWORD   dwRateType;         //流模式(0为定码流，1为变码流)
    DWORD   dwBitRate;          //码率 (16000-4096000)
    DWORD   dwImageQuality;     //编码质量(0-4),0为最好
    DWORD   dwMaxKeyInterval;   //关键帧间隔(1-100)
    
    WORD    wEncodeAudio;       //是否编码音频
    WORD    wEncodeVideo;       //是否编码视频

    WORD    wFormatTag;        /* format type */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */

    BYTE    byReseved[16];
}DMS_NET_COMPRESSION_INFO, *LPDMS_NET_COMPRESSION_INFO;

typedef struct tagDMS_NET_CHANNEL_PIC_INFO
{
    DWORD   dwSize;
    DWORD   dwChannel;
    char    csChannelName[DMS_NAME_LEN];    //通道名称

    DMS_NET_COMPRESSION_INFO   stRecordPara; /* 录像 */
    DMS_NET_COMPRESSION_INFO   stNetPara;    /* 网传 */
    DMS_NET_COMPRESSION_INFO   stPhonePara;  /* 手机监看 */
    DMS_NET_COMPRESSION_INFO   stEventRecordPara; /*事件触发录像压缩参数*/
}DMS_NET_CHANNEL_PIC_INFO, *LPDMS_NET_CHANNEL_PIC_INFO;

typedef struct tagJBNV_SERVER_INFO
{
	DWORD		dwSize;	
	DWORD		dwServerFlag;
	DWORD		dwServerIp;	
	char		szServerIp[16];
	WORD		wServerPort;
	WORD		wChannelNum;
	DWORD		dwVersionNum;	
	char		szServerName[32];
	DWORD		dwServerCPUType;
	BYTE		bServerSerial[48];
	BYTE		byMACAddr[6];	
	DWORD		AlarmInNum;
	DWORD		AlarmOutNum;
	DWORD		dwSysFlags;
	DWORD		dwUserRight;
	DWORD		dwNetPreviewRight;
    DWORD       dwCompany;
    char		csServerDes[60];
}JBNV_SERVER_INFO,*PJBNV_SERVER_INFO;
typedef struct tagDMS_NET_CHANNEL_INFO
{
	DWORD   dwSize;
	DWORD   dwStream1Height;		/*视频高(1)*/
	DWORD   dwStream1Width;		/*视频宽*/
	DWORD   dwStream1CodecID;		/*视频编码类型号（MPEG4为0，JPEG2000为1,H264为2）*/
	DWORD   dwStream2Height;		/*视频高(2)*/
	DWORD   dwStream2Width;		/*视频宽*/
	DWORD   dwStream2CodecID;		/*视频编码类型号（MPEG4为0，JPEG2000为1,H264为2）*/
	DWORD   dwAudioChannels;		/*音频通道数*/
	DWORD   dwAudioBits;			/*音频比特率*/
	DWORD   dwAudioSamples;		/*音频采样率*/
	DWORD   dwAudioFormatTag;		/*音频编码类型号*/
	char	csChannelName[32];		/*通道名称*/
}DMS_NET_CHANNEL_INFO,*PLDMS_NET_CHANNEL_INFO;

typedef struct tagDMS_SENSOR_INFO	//JB_SENSOR_INFO
{
	DWORD	dwSize;
	DWORD	dwIndex;
	DWORD	dwSensorType;
	char	csSensorName[32];
}DMS_SENSOR_INFO,*PLDMS_SENSOR_INFO;

typedef struct tagDMS_SERVER_NETWORK	//JB_SERVER_NETWORK
{
	DWORD		dwSize;
	DWORD		dwNetIpAddr;			//IP地址
	DWORD       dwNetMask;				//掩码
	DWORD       dwGateway;				//网关
	BYTE		bEnableDHCP;			//
	BYTE		bEnableAutoDNS;
	BYTE		bReserve;			//保留
	BYTE		bVideoStandard;			//0 - NTSC, 1 - PAL
	DWORD       dwHttpPort;				//Http端口
	DWORD       dwDataPort;				//数据端口
	DWORD		dwDNSServer;			//DNS服务器
	DWORD		dwTalkBackIp;			//服务器告警时，自动连接的对讲IP
	char        szMacAddr[6];			//网卡MAC地址
	char		szServerName[32];		//服务器名称
}DMS_SERVER_NETWORK,*PLDMS_SERVER_NETWORK;

typedef struct tagDMS_CENTER_INFO{//JBNV_NXSIGHT_SERVER_ADDR
	DWORD	dwSize;
	BOOL	bEnable;
	DWORD	dwCenterIp;
	DWORD	wCenterPort;
	char	csServerNo[64];
}DMS_CENTER_SERVER_ADDR;

typedef struct tagJBNV_SERVER_PACK
{
	char	szIp[16];		//Server Ip
	WORD	wMediaPort;		//Media Port
	WORD	wWebPort;		//Http Port
	WORD	wChannelCount;		//通道个数
	char	szServerName[32];	//
	DWORD	dwDeviceType;		//设备类型(CPUTYPE_3518...)
	DWORD	dwServerVersion;	//版本号
	WORD	wChannelStatic;		//通道个数,同wChannelCount
	WORD	wSensorStatic;		//Sensor个数
	WORD	wAlarmOutStatic;	//Alarmout个数
}JBNV_SERVER_PACK;

typedef struct tagJBNV_SERVER_PACK_EX_V2
{
    JBNV_SERVER_PACK jspack;
    BYTE            bMac[6];
    BOOL            bEnableDHCP;    
    BOOL        bEnableDNS;
    DWORD   dwNetMask;
    DWORD   dwGateway;
    DWORD   dwDNS;
    DWORD   dwComputerIP;   
    BOOL            bEnableCenter;
    DWORD   dwCenterIpAddress;
    DWORD   dwCenterPort;
    char        csServerNo[64];
    int     bEncodeAudio;
    char    csCenterIpAddress[64];
}JBNV_SERVER_PACK_EX_V2;

typedef struct tagJBNV_SERVER_MSG_DATA_EX_V2
{
    DWORD                   dwSize; // >= sizeof(JBNV_SERVER_PACK);
    DWORD                   dwPackFlag; // == SERVER_PACK_FLAG(0x03404324)
    JBNV_SERVER_PACK_EX_V2  jbServerPack;
}JBNV_SERVER_MSG_DATA_EX_V2;

typedef struct tagJBNV_BROADCAST_PACK{
    DWORD   dwFlag;	//9000
    DWORD   dwServerIpaddr;
    WORD    wServerPort;
    WORD    wBufSize;
    DWORD   dwCommand;
    DWORD   dwChannel;
}JBNV_BROADCAST_PACK; 


typedef struct tagJBNV_SERVER_PACK_EX
{
    JBNV_SERVER_PACK jspack;
    BYTE            bMac[6];
    BOOL            bEnableDHCP;    
    BOOL        bEnableDNS;
    DWORD   dwNetMask;
    DWORD   dwGateway;
    DWORD   dwDNS;
    DWORD   dwComputerIP;   
    BOOL            bEnableCenter;
    DWORD   dwCenterIpAddress;
    DWORD   dwCenterPort;
    char        csServerNo[64];
    int     bEncodeAudio;
}JBNV_SERVER_PACK_EX;
typedef struct tagJBNV_SERVER_MSG_DATA_EX
{
    DWORD                   dwSize; // >= sizeof(JBNV_SERVER_PACK) + 8;
    DWORD                   dwPackFlag; // == SERVER_PACK_FLAG
    JBNV_SERVER_PACK_EX     jbServerPack;
}JBNV_SERVER_MSG_DATA_EX;

typedef struct
{
    DWORD                             dwSize;
    DWORD               nCmd;
    DWORD               dwPackFlag; // == SERVER_PACK_FLAG
    DWORD               nErrorCode;
}DMS_BROADCAST_HEADER;
typedef struct tagJB_SERVER_SET_INFO{
    DWORD   dwSize;
    DWORD     dwIp;             //Server Ip
    DWORD   dwMediaPort;        //Media Port:8200
    DWORD   dwWebPort;      //Http Port:80

    DWORD   dwNetMask;
    DWORD   dwGateway;
    DWORD   dwDNS;
    DWORD   dwComputerIP;
    BOOL            bEnableDHCP;    
    BOOL            bEnableAutoDNS;
    BOOL            bEncodeAudio;

    char          szoldMac[6];
    char          szMac[6];
    char        szServerName[32];   
}JBNV_SET_SIGHT_SERVER_INFO;

typedef struct tagJB_CENTER_INFO_V2{
	DWORD	dwSize;
	BOOL	bEnable;
	DWORD	dwCenterIp; //不再用，为与之前版本兼容故保留。
	DWORD	wCenterPort;
	char	csServerNo[64];
	char csCenterIP[64];
}JBNV_NXSIGHT_SERVER_ADDR_V2;
typedef struct tagJBNV_SERVER_INFO_BROADCAST{
    DWORD   nBufSize;       /*sieze of JBNV_SET_SERVER_INFO_BROADCAST*/
    DMS_BROADCAST_HEADER          hdr;
    JBNV_SET_SIGHT_SERVER_INFO    setInfo;
    DMS_CENTER_SERVER_ADDR        nxServer;
}JBNV_SET_SERVER_INFO_BROADCAST;

typedef struct tagJBNV_SERVER_INFO_BROADCAST_V2{
    DWORD   nBufSize;       /*sieze of JBNV_SET_SERVER_INFO_BROADCAST*/
    DMS_BROADCAST_HEADER               hdr;
    JBNV_SET_SIGHT_SERVER_INFO                  setInfo;
    JBNV_NXSIGHT_SERVER_ADDR_V2           nxServer;
}JBNV_SET_SERVER_INFO_BROADCAST_V2;

typedef struct tagPRESETPOINT
{
	unsigned char chn;       //预置点通道
	unsigned char no;        //预置点序号
	char          csName[64];//预置点名称
}CPRESETPOINT;
typedef struct
{
	BYTE byPointNum; 		//预置点数量
	BYTE byCruiseIndex;	//本巡航的序号
	BYTE byRes[2];
	DMS_NET_CRUISE_POINT struCruisePoint[DMS_MAX_CRUISE_POINT_NUM];
}DMS_NET_CRUISE_INFO;

typedef struct
{
	DWORD dwSize;
	int     nChannel;
	BYTE     byIsCruising;	//是否在巡航
	BYTE     byCruisingIndex;	//正在巡航的巡航编号
	BYTE     byPointIndex;	//正在巡航的预置点序号(数组下标)
	BYTE     byReseved;
	DMS_NET_CRUISE_INFO struCruise[DMS_MAX_CRUISE_GROUP_NUM];
}DMS_NET_CRUISE_CFG;

//yaogang modify
typedef struct _NET_DATA_HEAD 
{ 
	unsigned long  nFlag;  
	unsigned long  nSize;  
} NET_DATA_HEAD,*PNET_DATA_HEAD;  

typedef struct tagFRAME_HEADER 
{ 
	WORD    wMotionDetect; 
	WORD    wFrameIndex; 
	DWORD  dwVideoSize;  
	DWORD  dwTimeTick; 
	WORD    wAudioSize;  
	BYTE    byKeyFrame;  
	BYTE    byReserved; 
} FRAME_HEADER,*PFRAME_HEADER;  

#define PACK_SIZE (8*1024)
typedef struct tagDATA_PACKET
{
	WORD      wIsSampleHead;
	WORD      wBufSize;
	FRAME_HEADER  stFrameHeader;
	unsigned char    byPackData[PACK_SIZE];
} DATA_PACKET,*PDATA_PACKET;

typedef struct _NET_DATA_PACKET 
{ 
	NET_DATA_HEAD       stPackHead; 
	DATA_PACKET         stPackData; 
} NET_DATA_PACKET,*PNET_DATA_PACKET;

typedef struct tagCMD_SERVER_INFO{ 
	DWORD  dwMultiAddr; 
	DWORD  dwServerIndex;  
	JBNV_SERVER_INFO ServerInfo; 
} CMD_SERVER_INFO;


typedef struct tagJBNV_CHANNEL_INFO 
{ 
	DWORD  dwSize;  
	DWORD  dwStream1Height;  //视频高(1) 
	DWORD  dwStream1Width;  //视频宽 
	DWORD  dwStream1CodecID;   //视频编码类型号（MPEG4 为 0，JPEG2000 为 1,H264 为2） 
	DWORD  dwStream2Height;  //视频高(2) 
	DWORD  dwStream2Width;  //视频宽 
	DWORD  dwStream2CodecID;   //视频编码类型号（MPEG4 为 0，JPEG2000 为 1,H264 为2） 
	DWORD  dwAudioChannels;  //音频通道数 
	DWORD  dwAudioBits;    //音频比特率 
	DWORD  dwAudioSamples;  //音频采样率 
	DWORD  dwWaveFormatTag;  //音频编码类型号 
	char	csChannelName[32];  //通道名称 
} JBNV_CHANNEL_INFO,*PJBNV_CHANNEL_INFO;  
 
typedef struct tagJBNV_SENSOR_INFO 
{ 
	DWORD  dwSize;  
	DWORD  dwIndex;      //探头索引 
	DWORD  dwSensorType;      //探头类型 
	char    csSensorName[32];    //探头名称 
} JBNV_SENSOR_INFO,*PJBNV_SENSOR_INFO;  

#endif


