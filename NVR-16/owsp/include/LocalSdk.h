#ifndef _LOCAL_SDK_H_
#define _LOCAL_SDK_H_

// #ifndef MEDIA_TYPE
// #define MEDIA_TYPE
//图像编码类型
#define  MEDIA_TYPE_H264		(BYTE)98//H.264//可能是109?
#define  MEDIA_TYPE_MP4			(BYTE)97//MPEG-4
#define  MEDIA_TYPE_H261		(BYTE)31//H.261
#define  MEDIA_TYPE_H263		(BYTE)34//H.263
#define  MEDIA_TYPE_MJPEG		(BYTE)26//Motion JPEG
#define  MEDIA_TYPE_MP2			(BYTE)33//MPEG2 video

//语音编码类型
#define	 MEDIA_TYPE_MP3			(BYTE)96//mp3
#define  MEDIA_TYPE_PCMU		(BYTE)0//G.711 ulaw
#define  MEDIA_TYPE_PCMA		(BYTE)8//G.711 Alaw
#define	 MEDIA_TYPE_G7231		(BYTE)4//G.7231
#define	 MEDIA_TYPE_G722		(BYTE)9//G.722
#define	 MEDIA_TYPE_G728		(BYTE)15//G.728
#define	 MEDIA_TYPE_G729		(BYTE)18//G.729
#define	 MEDIA_TYPE_RAWAUDIO	(BYTE)19//raw audio
#define  MEDIA_TYPE_ADPCM		(BYTE)20//adpcm
#define  MEDIA_TYPE_ADPCM_HISI	(BYTE)21//adpcm-hisi
// #endif//MEDIA_TYPE

/* ========================== structure for each encoded frame ============================= */
typedef struct FrameHeadr
{
	unsigned char mediaType;			//encoded (video/audio) media type:
	unsigned char *pData;				//encoded data buffer
	unsigned char play_index;
	unsigned int preBufSize;			//pre buffer size, normally it is 12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
	unsigned int dataSize;				//actual buffer size pointed by pData
	unsigned char frameRate;			//frame rate, used on receive part. 
	unsigned int frameID;				//fram id，used on receive part. 
	unsigned int timeStamp;				//time stamp, used on receive part. 
	union
	{
		struct{
			int keyFrame;				//I(1) or P(0)
			unsigned short videoWidth;	//video width
			unsigned short videoHeight;	//video height
		} videoParam;
		unsigned char audioMode;		//8, 16, or 32 bit
	};
} FrameHeadr;
typedef FrameHeadr* pFrameHeadr;

#pragma pack( push, 1 )//按字节对齐

enum NETDVR_RETURN_CODE	//
{
	NETDVR_SUCCESS	= 0,
	NETDVR_REC_STATUS_STARTED,
	NETDVR_REC_STATUS_STOPPED,
	NETDVR_ERROR = 2000,
	NETDVR_ERR_UNKNOWN,
	NETDVR_ERR_PARAM,
	NETDVR_ERR_RET_PARAM,						//???
	NETDVR_ERR_NOINIT,
	NETDVR_ERR_COMMAND,
	NETDVR_ERR_NEW_CONNECTION,					//???
	NETDVR_ERR_SEND,
	NETDVR_ERR_OUTOFMEMORY,
	NETDVR_ERR_RESOURCE,
	NETDVR_ERR_FILENOTEXIST,
	NETDVR_ERR_BAUDLIMIT,						//max = 5
	NETDVR_ERR_CREATESOCKET,					//create socket error
	NETDVR_ERR_SOCKET,							//socket error
	NETDVR_ERR_CONNECT,							//connect error
	NETDVR_ERR_BIND,							//bind error -1985
	NETDVR_ERR_LISTEN,		
	NETDVR_ERR_NETSND,							//send error
	NETDVR_ERR_NETRCV,							//recv error
	NETDVR_ERR_TIMEOUT,							//timeout 
	NETDVR_ERR_CHNERROR,						// > channel limit -1980
	NETDVR_ERR_DEVICEBUSY,						//device busy
	NETDVR_ERR_WRITEFLASH,						//erase error 
	NETDVR_ERR_VERIFY,							//verify error 
	NETDVR_ERR_CONFLICT,						//system resource conflict
	NETDVR_ERR_BUSY,							//system busy-1975
	NETDVR_ERR_USER_SAME,						//user name conflict
	NETDVR_ERR_LINKLIMIT,
	NETDVR_ERR_DATABASE,
	/* === error code for login === */
	NETDVR_ERR_NOUSER,
	NETDVR_ERR_PASSWD,
	NETDVR_ERR_MACADDR, 
	NETDVR_ERR_RELOGIN,
	NETDVR_ERR_NOLOGIN,
	/* === net player === */
	NETDVR_ERR_NETDVR_PLAYER_FULL,
	/* === updateing ==== */
	NETDVR_ERR_UPDATING,
	/* === remote file download error === */
	NETDVR_ERR_DOWNLOAD,
	NETDVR_ERR_FILEOPEN,
	NETDVR_ERR_USERSTOPPED,
	NETDVR_ERR_SERIAL_REOPEN,
	NETDVR_ERR_GET_LOCALMACADDR,
	NETDVR_ERR_SDK_CHECKFAILED,
	NETDVR_ERR_RERECIVE,
	NETDVR_ERR_SENDUPDATEFILE,
};

/* ========================== structure for the DVR information ============================= */
struct NETDVR_DeviceInfo_t
{
	unsigned int	deviceIP; 						//设备IP  
	unsigned short	devicePort;						//设备端口 
	char			device_name[32];				//设备名称
	char			device_mode[32];				//设备型号
	unsigned char	maxChnNum;						//最大视频通道数
	unsigned char	maxAudioNum;					//最大音频通道数
	unsigned char	maxSubstreamNum;				//最大子码流数
	unsigned char	maxPlaybackNum;					//最大回放数
	unsigned char	maxAlarmInNum;					//最大报警输入数
	unsigned char	maxAlarmOutNum;					//最大报警输出数
	unsigned char	maxHddNum;						//最大硬盘数(实际不一定接上这么多硬盘)
	unsigned char	reserved[58];					//预留
}PACK_NO_PADDING;

/* ========================== structure for client's login information ============================= */
struct NETDVR_loginInfo_t
{
	char username[12];								//username for login
	char loginpass[12];								//password for login
	char macAddr[18];								//client mac address
	unsigned int ipAddr;							//client ip address
}PACK_NO_PADDING ;

struct NETDVR_VideoProperty_t 
{
	unsigned char	videoEncType;					//视频编码类型
	unsigned short	max_videowidth;
	unsigned short	max_videoheight;
	unsigned char	reserved[3];					//预留
}PACK_NO_PADDING ;

struct NETDVR_AudioProperty_t
{
	unsigned char	audioEnctype;					//预览音频编码类型
	unsigned char	audioBitPerSample;				//预览音频位数
	unsigned short	audioSamplePerSec;				//预览音频采样率
	unsigned short	audioFrameSize;					//预览音频每帧大小
	unsigned short	audioFrameDurTime;				//预览音频每帧间隔
	unsigned char	reserved[4];					//预留
}PACK_NO_PADDING ;

struct NETDVR_VOIPProperty_t 
{
	unsigned char	VOIPBitPerSample;				//语音对讲位数
	unsigned short	VOIPSamplePerSec;				//语音对讲采样率
	unsigned short	VOIPFrameSize;					//语音对讲每帧大小
	unsigned char	reserved[3];					//预留
}PACK_NO_PADDING ;

struct NETDVR_MDProperty_t 
{
	unsigned char	MDCol;							//移动侦测列数
	unsigned char	MDRow;							//移动侦测行数
	unsigned char	reserved[4];					//预留
}PACK_NO_PADDING ;

enum NETDVR_OVERLAP {
	NETDVR_OVERLAP_NO = 0,					//not overlap when disk(s) is(are) full
	NETDVR_OVERLAP_YES = 1,					//overlap when disk(s) is(are) full
};

enum NETDVR_DVRSTATUS {
	NETDVR_DVRSTATUS_HIDDEN = 0,			//don't display the DVR status 
	NETDVR_DVRSTATUS_DISPLAY = 1,			//display the DVR status
};

/* ========================== keybord lock time============================= */
enum NETDVR_LOCKTIME {
	NETDVR_LOCKTIME_NEVER = 0,				//never lock
	NETDVR_LOCKTIME_MIN_1 = 60,				//lock time equals 1 minute
	NETDVR_LOCKTIME_MIN_2 = 120,			//lock time equals 2 minutes
	NETDVR_LOCKTIME_MIN_5 = 300,			//lock time equals 5 minutes
	NETDVR_LOCKTIME_MIN_10 = 600,			//lock time equals 10 minutes
	NETDVR_LOCKTIME_MIN_20 = 1200,			//lock time equals 20 minutes
	NETDVR_LOCKTIME_MIN_30 = 1800,			//lock time equals 30 minutes
};

enum NETDVR_SWITCHTIME {
	NETDVR_SWITCHTIME_NEVER = 0,			//never switch
	NETDVR_SWITCHTIME_SEC_5 = 5,			//switch time equals 5 seconds
	NETDVR_SWITCHTIME_SEC_10 = 10,			//switch time equals 10 seconds
	NETDVR_SWITCHTIME_SEC_20 = 20,			//switch time equals 20 seconds
	NETDVR_SWITCHTIME_SEC_30 = 30,			//switch time equals 30 seconds
	NETDVR_SWITCHTIME_MIN_1 = 60,			//switch time equals 1 minute
	NETDVR_SWITCHTIME_MIN_2 = 120,			//switch time equals 2 minute
	NETDVR_SWITCHTIME_MIN_5 = 300,			//switch time equals 5 minute
};

enum NETDVR_VIDEOFORMAT {
	NETDVR_VIDEOFORMAT_PAL = 0,				//PAL video format
	NETDVR_VIDEOFORMAT_NTSC = 1,			//NTSC video format
};
//typedef enum NETDVR_VIDEOFORMAT video_format_t;



/* ========================== VGASOLUTION param structure =========================== */

struct NETDVR_VGAPROPERTY
{
	unsigned short		width;						//分辨率宽
	unsigned short		height;						//分辨率高
	unsigned char		flashrate;					//分辨率刷新率
}PACK_NO_PADDING ;

struct NETDVR_VGARESOLLIST
{
	struct NETDVR_VGAPROPERTY vgapro[16];
};

enum NETDVR_TRANSPARENCY {
	NETDVR_TRANSPARENCY_NO = 0,				//no transparency
	NETDVR_TRANSPARENCY_LOW = 1,			//low transparency
	NETDVR_TRANSPARENCY_MID = 2,			//middle transparency
	NETDVR_TRANSPARENCY_HIGH = 3,			//high transparency
};

enum NETDVR_LANGUAGE {
	NETDVR_LANGUAGE_SC = 0,					//simplified chinese
	NETDVR_LANGUAGE_EN = 1,					//english
	NETDVR_LANGUAGE_TC = 2,					//Traditional Chinese
};

/* ========================== sys param structure =========================== */
struct NETDVR_systemParam_t
{
	unsigned short	device_id;					//the dvr device id for 遥控器
	char			device_name[32];			//device name of dvr
	unsigned char	disable_pw;					//禁用密码
	enum NETDVR_OVERLAP flag_overlap;			//flag_overlap=1, then overlap the disk when the disk is full. or if flag_overlap=0, then not overlap
	enum NETDVR_DVRSTATUS flag_statusdisp;		//decide whether to dispay system status or not
	enum NETDVR_LOCKTIME lock_time;				//lock time for keybord
	enum NETDVR_SWITCHTIME switch_time;			//switch time for preview mode
	enum NETDVR_VIDEOFORMAT video_format;		//PAL or NTSC 重启生效
	unsigned char vga_solution;					//vga分辨率
	enum NETDVR_TRANSPARENCY transparency;		//菜单透明度
	enum NETDVR_LANGUAGE language;				//语言
	
	char reserved[32];
}PACK_NO_PADDING ;

enum NETDVR_BAUDRATE {
	NETDVR_BAUDRATE_115200 = 0,	
	NETDVR_BAUDRATE_57600 = 1,
	NETDVR_BAUDRATE_38400 = 2,
	NETDVR_BAUDRATE_19200 = 3,
	NETDVR_BAUDRATE_9600 = 4,
	NETDVR_BAUDRATE_4800 = 5,
	NETDVR_BAUDRATE_2400 = 6,
	NETDVR_BAUDRATE_1200 = 7,
	NETDVR_BAUDRATE_300 = 8,
};

enum NETDVR_DATABITSEL {
	NETDVR_DATABITSEL_8 = 0,				//data bit: 8 bits
	NETDVR_DATABITSEL_7 = 1,				//data bit: 7 bits
	NETDVR_DATABITSEL_6 = 2,				//data bit: 6 bits
};

enum NETDVR_STOPBITSEL {
	NETDVR_STOPBITSEL_1 = 0,				//stop bit: 1 bit
	NETDVR_STOPBITSEL_2 = 1,				//stop bit: 2 bits
};

enum NETDVR_CHECK_TYPE {
	NETDVR_CHECK_NONE = 0,					//no check
	NETDVR_CHECK_ODD = 1,					//odd check
	NETDVR_CHECK_EVEN = 2,					//even check
};

enum NETDVR_FLOWCONTROL {
	NETDVR_FLOWCONTROL_NO = 0,				//no flow control
	NETDVR_FLOWCONTROL_HARDWARE = 1,		//flow control by hardware
	NETDVR_FLOWCONTROL_XON_XOFF = 2,		//flow control by Xon/Xoff
};

enum NETDVR_PROTOCOLTYPE {
	NETDVR_PROTOCOLTYPE_PELCO_D = 0,		//protocol type : Pelco-d
	NETDVR_PROTOCOLTYPE_PELCO_P = 1,		//protocol type : Pelco-p
	NETDVR_PROTOCOLTYPE_B01 = 2,			//protocol type : B01
	NETDVR_PROTOCOLTYPE_SAMSUNG = 3,		//protocol type : Samsung
};

#define MAX_PRESET_NUM 128					//preset limit 0-MAX_PRESET_NUM
#define MAX_CRUISE_PATH_NUM 16				//max cruise paths of each PTZ
#define MAX_CRUISE_POS_NUM 16				//max cruise positions of each cruise path
#define MAX_CRUISE_SPEED 9					//max cruise speed
#define MAX_DWELL_TIME 255					//max dwell time

enum NETDVR_YTTRACKCONTROL {
	NETDVR_YT_COM_STARTRECORDTRACK = 0,
	NETDVR_YT_COM_STOPRECORDTRACK = 1,
	NETDVR_YT_COM_STARTTRACK = 2,
	NETDVR_YT_COM_STOPTRACK = 3,	
};

enum NETDVR_YTPRESETPOINTCONTROL
{
	NETDVR_YT_COM_ADDPRESETPOINT = 0,
	NETDVR_YT_COM_DELPRESETPOINT = 1,
	NETDVR_YT_COM_TOPRESETPOINT = 2,
};

struct NETDVR_cruisePos_t
{
	unsigned char preset_no;				//1 ~ MAX_PRESET_NUM
	unsigned char dwell_time;				//1 ~ 255 secondes
	unsigned char cruise_speed;				//1 ~ MAX_CRUISE_SPEED(1 : the slowest speed)
	unsigned char cruise_flag;				//1-add 2-del
}PACK_NO_PADDING ;

struct NETDVR_cruisePath_t
{
	unsigned char chn;
	unsigned char path_no;										//1 ~ MAX_CRUISE_PATH_NUM	
	struct NETDVR_cruisePos_t cruise_pos[MAX_CRUISE_POS_NUM];	//cruise poisitions of cruise path
}PACK_NO_PADDING ;

struct NETDVR_ptzParam_t 
{
	unsigned char		chn;						//摄像头所在视频通道
	unsigned short		address;					//云台解码器地址
	enum NETDVR_BAUDRATE baud_ratesel;
	enum NETDVR_DATABITSEL data_bitsel;
	enum NETDVR_STOPBITSEL stop_bitsel;
	enum NETDVR_CHECK_TYPE check_type;
	enum NETDVR_FLOWCONTROL flow_control;
	enum NETDVR_PROTOCOLTYPE protocol;
	unsigned int copy2Chnmask;						//当前参数复制到其他通道 按位
	char reserved[32];
}PACK_NO_PADDING ;

enum NETDVR_PTZCONTROL {
	NETDVR_PTZ_COM_STOP = 0,
	NETDVR_PTZ_COM_MOVEUP = 1,
	NETDVR_PTZ_COM_MOVEDOWN = 2,
	NETDVR_PTZ_COM_MOVELEFT = 3,
	NETDVR_PTZ_COM_MOVERIGHT = 4,
	NETDVR_PTZ_COM_ROTATION = 5,
	NETDVR_PTZ_COM_ZOOMADD = 6,
	NETDVR_PTZ_COM_ZOOMSUBTRACT = 7,
	NETDVR_PTZ_COM_FOCUSADD = 8,
	NETDVR_PTZ_COM_FOCUSSUBTRACT = 9,
	NETDVR_PTZ_COM_APERTUREADD = 10,
	NETDVR_PTZ_COM_APERTURESUBTRACT = 11,
	NETDVR_PTZ_COM_LIGHTINGOPEN = 12,
	NETDVR_PTZ_COM_LIGHTINGCLOSE = 13,
	NETDVR_PTZ_COM_WIPERSOPEN = 14,
	NETDVR_PTZ_COM_WIPERSCLOSE = 15,
	NETDVR_PTZ_COM_FAST = 16,
	NETDVR_PTZ_COM_NORMAL = 17,
	NETDVR_PTZ_COM_SLOW = 18,
	NETDVR_PTZ_COM_AUXILIARYOPEN = 19,
	NETDVR_PTZ_COM_AUXILIARYCLOSE = 20,
};

struct NETDVR_PtzCtrl_t
{
	unsigned char	chn;						//通道
	enum NETDVR_PTZCONTROL  cmd;				//命令
	unsigned char	aux;						//辅助功能
}PACK_NO_PADDING ;

#define NETDVR_VIDEO_LOST_DECT 0			//dect video lost
#define NETDVR_VIDEO_LOST_UNDECT 1			//not dect video lost

#define NETDVR_VIDEO_MD_CLOSE 0				//not dect video motion
#define NETDVR_VIDEO_MD_SEN_LOWEST 1		//dect video motion in a lowest sensitivity
#define NETDVR_VIDEO_MD_SEN_LOWER 2			//dect video motion in a lower sensitivity
#define NETDVR_VIDEO_MD_SEN_LOW 3			//dect video motion in a low sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGH 4			//dect video motion in a high sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGHER 5		//dect video motion in a higher sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGHEST 6		//dect video motion in a highest sensitivity



struct NETDVR_videoParam_t
{
	unsigned char		channel_no;					//通道号
	char				channelname[32];			//通道名
	unsigned char		flag_name;					//名称位置显示
	unsigned short		chnpos_x;					//名称x坐标
	unsigned short		chnpos_y;					//名称y坐标
	unsigned char		flag_time;					//时间位置显示
	unsigned short		timepos_x;					//时间x坐标
	unsigned short		timepos_y;					//时间y坐标
	unsigned char		flag_mask;					//遮盖
	
	//遮盖区域;R8016每一个视频通道支持2个遮盖区域;海思系列支持4个
	struct net_maskAREA_t
	{
		unsigned short	 	x;
		unsigned short		y;
		unsigned short		width;
		unsigned short		height;
	}maskinfo[4];
	
	//handler
	unsigned char		flag_safechn;				//安全通道标记，不起作用
	unsigned int		copy2chnmask;				//复制到其他通道，每一位一个通道
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

typedef enum NETDVR_FLOW_TYPE {
	NETDVR_FLOW_VIDEO = 0,						//video flow
	NETDVR_FLOW_MUTI,							//multiple flow(both video and audio)
} flow_type_t;

typedef enum NETDVR_BITRATE_TYPE {
	NETDVR_BITRATE_FIXED = 0,					//fixed bit rate
	NETDVR_BITRATE_VARIABLE,					//variable bit rate
} bitrate_type_t;

typedef enum NETDVR_BITRATE {
	NETDVR_BITRATE_64 = 64,					//64kbps
	NETDVR_BITRATE_128 = 128,					//128kbps
	NETDVR_BITRATE_256 = 256,					//256kbps
	NETDVR_BITRATE_384 = 384,					//384kbps
	NETDVR_BITRATE_512 = 512,					//512kbps
	NETDVR_BITRATE_768 = 768,					//768kbps
	NETDVR_BITRATE_1024 = 1024,				//1Mbps
	NETDVR_BITRATE_1536 = 1536,				//1.5Mbps
	NETDVR_BITRATE_2048 = 2048,				//2Mbps
} bitrate_t;

typedef enum NETDVR_VIDEO_QUALITY {
	NETDVR_VIDEO_QUALITY_BEST = 0,				//best video quality
	NETDVR_VIDEO_QUALITY_BETTER,				//better video quality
	NETDVR_VIDEO_QUALITY_GOOD,					//good video quality
	NETDVR_VIDEO_QUALITY_NORMAL,				//normal video quality
	NETDVR_VIDEO_QUALITY_BAD,					//bad video quality
	NETDVR_VIDEO_QUALITY_WORSE,				//worse video quality
} video_quality_t;

typedef enum NETDVR_FRAMERATE {
	NETDVR_FRAMERATE_25 = 25,					//25f/s
	NETDVR_FRAMERATE_20 = 20,					//20f/s
	NETDVR_FRAMERATE_15 = 15,					//15f/s
	NETDVR_FRAMERATE_10 = 10,					//10f/s
	NETDVR_FRAMERATE_5 = 5,					//5f/s
	NETDVR_FRAMERATE_2 = 2,					//2f/s
	NETDVR_FRAMERATE_1 = 1,					//1f/s
} framerate_t;

typedef enum NETDVR_PRERECORD_TIME {
	NETDVR_PRERECORD_TIME_0 = 0,				//do not pre-record
	NETDVR_PRERECORD_TIME_5 = 5,					//pre-record time: 5s
	NETDVR_PRERECORD_TIME_10 = 10,					//pre-record time: 10s
	NETDVR_PRERECORD_TIME_15 = 15,					//pre-record time: 15s
	NETDVR_PRERECORD_TIME_20 = 20,					//pre-record time: 20s
	NETDVR_PRERECORD_TIME_25 = 25,					//pre-record time: 25s
	NETDVR_PRERECORD_TIME_30 = 30,					//pre-record time: 30s
} prerecord_time_t;

typedef enum NETDVR_POSTRECORD_TIME {
	NETDVR_POSTRECORD_TIME_5 = 5,					//post-record time: 5s
	NETDVR_POSTRECORD_TIME_10 = 10,					//post-record time: 10s
	NETDVR_POSTRECORD_TIME_30 = 30,					//post-record time: 30s
	NETDVR_POSTRECORD_TIME_60 = 60,					//post-record time: 60s
	NETDVR_POSTRECORD_TIME_120 = 120,				//post-record time: 120s
	NETDVR_POSTRECORD_TIME_300 = 300,				//post-record time: 300s
	NETDVR_POSTRECORD_TIME_600 = 600,				//post-record time: 600s
} postrecord_time_t;

struct NETDVR_recordParam_t
{
	unsigned char		channelno;					//通道号
	flow_type_t			code_type;					//see flow_type_t
	bitrate_type_t		bit_type;					//see bitrate_type_t
	bitrate_t			bit_max;					//see bitrate_t
	unsigned short		intraRate;					//关键帧间隔
	unsigned char		qi;							//关键帧量化因子
	unsigned char		minQ;						//最小量化因子
	unsigned char		maxQ;						//最大量化因子
	video_quality_t		quality;					//see video_quality_t
	framerate_t			frame_rate;					//see framerate_t
	prerecord_time_t	pre_record;					//see prerecord_time_t
	postrecord_time_t	post_record;				//see postrecord_time_t
	
	unsigned int		copy2chnmask;
	unsigned char		supportdeinter;				//只读，是否支持deinter设置 1是 0否 (待用)
	unsigned char		deinterval;					//deinter强度 0-4 禁用，弱，中，强，超强
	unsigned char		supportResolu;				//只读，是否支持设置录像分辨率
	unsigned char		resolutionpos;				//分辨率选项值, 取值见NETDVR_VideoResolu_list_t
	unsigned char		reserved1[12];				//保留字段	
}PACK_NO_PADDING ;

//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
#define NETDVR_VIDEORESOLU_BEGIN	1 //从1开始
#define NETDVR_VIDEORESOLU_QCIF		1 //QCIF
#define NETDVR_VIDEORESOLU_CIF		2 //CIF
#define NETDVR_VIDEORESOLU_HD1		3 //HD1
#define NETDVR_VIDEORESOLU_D1		4 //D1
#define NETDVR_VIDEORESOLU_END		NETDVR_VIDEORESOLU_D1 //在NETDVR_VIDEORESOLU_D1这里结束，可以通过判断是否在BEGIN和END之间来确认是否正确
struct  NETDVR_VideoResolu_list_t
{
	unsigned char chn;	//通道
	unsigned char type;	// 0 主码流 1 子码流
	unsigned char videoresolu[8];	//分辨率列表 未使用的填0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
}PACK_NO_PADDING ;

/* ========================== 子码流参数 ============================= */
typedef enum NETDVR_SUBFLAG 
{
	NETDVR_SUBFLAG_CIF = 0,
	NETDVR_SUBFLAG_QCIF = 1,	
}subflag_t;

typedef enum NETDVR_SUBBITTYPE {
	NETDVR_SUBBITRATE_FIXED = 0,					//fixed bit rate
	NETDVR_SUBBITRATE_VARIABLE,					//variable bit rate
} subbittype_t;

typedef enum NETDVR_SUBVIDEOQUALITY {
	NETDVR_SUBVIDEOQUALITY_BEST = 0,				//best video quality
	NETDVR_SUBVIDEOQUALITY_BETTER,				//better video quality
	NETDVR_SUBVIDEOQUALITY_GOOD,					//good video quality
	NETDVR_SUBVIDEOQUALITY_NORMAL,				//normal video quality
	NETDVR_SUBVIDEOQUALITY_BAD,					//bad video quality
	NETDVR_SUBVIDEOQUALITY_WORSE,				//worse video quality
} subvideoquality_t;

typedef enum NETDVR_SUBFRAMERATE {
	NETDVR_SUBFRAMERATE_25 = 25,					//25f/s
	NETDVR_SUBFRAMERATE_20 = 20,					//20f/s
	NETDVR_SUBFRAMERATE_15 = 15,					//15f/s
	NETDVR_SUBFRAMERATE_10 = 10,					//10f/s
	NETDVR_SUBFRAMERATE_5 = 5,						//5f/s
	NETDVR_SUBFRAMERATE_2 = 2,						//2f/s
	NETDVR_SUBFRAMERATE_1 = 1,						//1f/s
} subframerate_t;

typedef enum NETDVR_SUBBITRATE {
	NETDVR_SUBBITRATE_64 = 64,					//64kbps
	NETDVR_SUBBITRATE_128 = 128,					//128kbps
	NETDVR_SUBBITRATE_256 = 256,					//256kbps
	NETDVR_SUBBITRATE_384 = 384,					//384kbps
	NETDVR_SUBBITRATE_512 = 512,					//512kbps
	NETDVR_SUBBITRATE_768 = 768,					//768kbps
	NETDVR_SUBBITRATE_1024 = 1024,				//1Mbps
	NETDVR_SUBBITRATE_1536 = 1536,				//1.5Mbps
	NETDVR_SUBBITRATE_2048 = 2048,				//2Mbps
} subbitrate_t;

struct NETDVR_SubStreamParam_t								
{
	unsigned char		chn;
	subflag_t			sub_flag;					//区分多种子码流列问 0 cif 1 qcif
	subbittype_t		sub_bit_type;				//子码流位率类型
	unsigned short		sub_intraRate;				//关键帧间隔
	unsigned char		sub_qi;						//关键帧量化因子
	unsigned char		sub_minQ;					//最小量化因子
	unsigned char		sub_maxQ;					//最大量化因子
	subvideoquality_t	sub_quality;				//子码流图像质量
	subframerate_t 		sub_framerate;				//子码流的帧率
	subbitrate_t 		sub_bitrate;				//子码流的位率
	unsigned int		copy2chnmask;				//复制到其他通道。每一位一个通道
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

typedef enum NETDVR_ALARMINTYPE {
	NETDVR_ALARMIN_HIGH = 1,				//high level alarm input
	NETDVR_ALARMIN_LOW,						//low level alarm input
} alarmintype_t;

typedef enum NETDVR_ALARMOUTYPE {
	NETDVR_ALARMOUT_NO = 1,					//alarm output type:normal open
	NETDVR_ALARMOUT_NC,						//alarm output type:normal close
} alarmouttype_t;

typedef enum NETDVR_DELAY_TIME {
	NETDVR_DELAY_5 = 5,					//5s
	NETDVR_DELAY_10=10,					//10s
	NETDVR_DELAY_30=30,					//30s
	NETDVR_DELAY_60=60,					//60s
	NETDVR_DELAY_120=120,				//120s
	NETDVR_DELAY_300=300,				//300s
	NETDVR_DELAY_MANUAL=0xffff, 		//manual
} delay_t;

struct AlarmInPtz								//PTZ联动
{
	unsigned char		flag_preset;			//预置点
	unsigned char		preset;
	unsigned char		flag_cruise;			//巡航点
	unsigned char		cruise;
	unsigned char		flag_track;				//轨迹
}PACK_NO_PADDING ;

struct NETDVR_alarmInParam_t
{
	unsigned char		inid;						//报警输入量
	unsigned char		flag_deal;					//1: deal with input alarm ; 0: for no
	alarmintype_t		typein;						//input alarm type:低电平or高电平报警 see alarmintype_t
	unsigned int		triRecChn;					//触发通道录像，每一位一通道
	unsigned int		triAlarmoutid;				//触发报警输出，按位
	unsigned char		flag_buzz;					//触发蜂鸣器
	unsigned char		flag_email;					//触发emaill报警
	unsigned char		flag_mobile;				//触发手机报警
	delay_t				delay;						//报警输出延时
	unsigned int		copy2AlarmInmask;			//当前参数复制到其他报警输入通道，按位
	struct AlarmInPtz	alarmptz[32];				//触发PTZ联动
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

struct NETDVR_alarmOutParam_t
{
	unsigned char		outid;						//报警输出通道 
	alarmouttype_t		typeout;					//报警输出类型:常开型or常闭型 see alarmouttype_t
	unsigned int		copy2AlarmOutmask;			//复制到其他报警输出，按位
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

struct NETDVR_AlarmNoticeParam_t									
{
	char			alarm_email[32];				//报警email地址
	char			alarm_mobile[32];				//报警手机号码
	unsigned char	reserved[32];					//保留字段
}PACK_NO_PADDING ;

struct NETDVR_networkParam_t
{
	char				mac_address[18];			//mac地址
	unsigned int		ip_address;					//ip地址
	unsigned short		server_port;				//设备端口
	unsigned int		net_mask;					//掩码
	unsigned int		net_gateway;				//网关
	unsigned int		dns;						//dns
	unsigned char		flag_multicast;				//组播启用标记
	unsigned int		multicast_address;			//组播地址
	unsigned short		http_port;					//http端口
	unsigned char		flag_pppoe;					//pppoe启用标记
	char				pppoe_user_name[64];		//pppoe用户名
	char				pppoe_passwd[64];			//pppoe密码
	unsigned char		flag_dhcp;					//dhcp启用标志
	unsigned char		ddnsserver;					//ddns服务商
	unsigned char		flag_ddns;					//ddns启用标志
	char				ddns_domain[64];			//ddns域名
	char				ddns_user[64];				//ddns用户名
	char				ddns_passwd[64];			//ddns密码
	unsigned int		centerhost_address;			//中心服务器地址
	unsigned short		centerhost_port;			//中心服务器端口
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

typedef enum NETDVR_TimeFormat_T
{
	NETDVR_TF_YYYYMMDD = 0,
	NETDVR_TF_MMDDYYYY = 1,
}timeFormat_t;

//系统时间
struct NETDVR_SysTime_t
{
	unsigned int		systemtime;					//系统时间
	timeFormat_t		format;						//时间格式 选项值
	unsigned char		flag_time;					//预览时间位置显示
	unsigned short		timepos_x;					//预览时间x坐标
	unsigned short		timepos_y;					//预览时间y坐标
}PACK_NO_PADDING ;

/* ========================== Motion Detect structures =========================== */

#define NETDVR_MD_MIN_SENSE	0						
#define NETDVR_MD_MAX_SENSE	5

//移动侦测
struct NETDVR_motionDetection_t 
{
	unsigned char	chn;
	unsigned int 	trigRecChn;						//触发通道录像 按位
	unsigned int 	trigAlarmOut;					//触发报警输出 按位
	unsigned char	flag_buzz;						//触发蜂鸣器
	unsigned char	flag_email;						//触发emaill报警
	unsigned char	flag_mobile;					//触发手机报警
	unsigned char	sense;							//移动侦测灵敏度:0-不检测;1-5灵敏度,值越大越灵敏
	delay_t			delay;							//延时
	unsigned char	block[44*36];					//移动侦测区域:每一个字节代表一个区域,1代表选中,0代表不选中
	unsigned char	reserved[16];					//预留
}PACK_NO_PADDING ;

//视频丢失
struct NETDVR_VideoLostParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//触发通道录像 按位
	unsigned int 		trigAlarmOut;				//触发报警输出 按位
	unsigned char 		flag_buzz;					//触发蜂鸣器
	unsigned char		flag_email;					//触发emaill报警
	unsigned char		flag_mobile;				//触发手机报警
	delay_t 			delay;						//延时
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

//视频遮挡
struct NETDVR_VideoBlockParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//触发通道录像 按位
	unsigned int 		trigAlarmOut;				//触发报警输出 按位
	unsigned char 		flag_buzz;					//触发蜂鸣器
	unsigned char		flag_email;					//触发emaill报警
	unsigned char		flag_mobile;				//触发手机报警
	delay_t				delay;						//延时
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

/* ========================== remote HDD INFO structures=========================== */
struct NETDVR_hddInfo_t 
{
	unsigned char		hdd_index;					//硬盘序号
	unsigned char		hdd_exist;  				//1:接上硬盘; 0:硬盘没有接上
	unsigned int		capability;					//MB
	unsigned int		freesize;					//MB
	unsigned char		reserved[2];				//预留
}PACK_NO_PADDING ;

/* ========================== remote System Version info structures=========================== */
struct NETDVR_SysVerInfo_t
{
	char devicename[32];
	char devicemodel[32];
	char deviceser[32];
	char version[64];
}PACK_NO_PADDING ;

/*=========================remote pic adjust============================*/
enum NETDVR_PICADJUST {
	NETDVR_PIC_BRIGHTNESS = 0,					
	NETDVR_PIC_CONTRAST,
	NETDVR_PIC_HUE,
	NETDVR_PIC_SATURATION,
};

struct NETDVR_PICADJUST_T
{
	unsigned char			channel_no;				//通道号
	enum NETDVR_PICADJUST	flag;					//调节标志:0-3
	unsigned char			val;					//调节值
	unsigned int			copy2chnmask;			//复制到其他通道。每一位一个通道
}PACK_NO_PADDING ;

typedef enum NETDVR_WEEKDAY {
	NETDVR_WEEKDAY_1 = 0,							//Monday
	NETDVR_WEEKDAY_2,							//Tuesday
	NETDVR_WEEKDAY_3,							//Wednesday
	NETDVR_WEEKDAY_4,							//Thursday
	NETDVR_WEEKDAY_5,							//Friday
	NETDVR_WEEKDAY_6,							//Saturday
	NETDVR_WEEKDAY_7,							//Sunday
} weekday_t;


/* ======================================================= */
struct RecTimeField_t
{
	unsigned int	starttime;						//起始时间
	unsigned int	endtime;						//终止时间
	unsigned char	flag_sch;						//定时录像
	unsigned char 	flag_md;						//移动侦测录像
	unsigned char	flag_alarm;						//报警录像
	unsigned char	reserved[4];					//保留字段
}PACK_NO_PADDING ;

//录像布防
struct NETDVR_RecordSCH_t
{
	unsigned char		chn;						//通道
	enum NETDVR_WEEKDAY	weekday;					//星期
	
	struct RecTimeField_t recTimeFieldt[4];
	/*struct RecTimeField_t
	{
		unsigned int	starttime;					//起始时间
		unsigned int	endtime;					//终止时间
		unsigned char	flag_sch;					//定时录像
		unsigned char 	flag_md;					//移动侦测录像
		unsigned char	flag_alarm;					//报警录像
		unsigned char	reserved[4];				//保留字段
	}recTimeFieldt[4];*/
	
	unsigned char		copy2Weekday;				//复制到一周的其他天 按位 
	unsigned int		copy2Chnmask;				//复制到其他通道 按位
	unsigned char		reserved[16];				//保留字段
}PACK_NO_PADDING ;

//手动录像
struct NETDVR_ManualRecord_t
{
	unsigned int  chnRecState;					//通道手动录像状态 按位	
}PACK_NO_PADDING ;

/* ==================== user control structure ======================== */
struct NETDVR_userInfo_t 
{
	char	name[12];
	char	password[12];
	
	char	mac_address[18];
	
	/* 1:open， 0:close */
	unsigned char		rcamer;						//remote yuntai
	unsigned char		rrec;						//remote record
	unsigned char		rplay;						//remote playback
	unsigned char		rsetpara;					//remote set param
	unsigned char		rlog;						//remote get log
	unsigned char		rtool;						//remote use tool
	unsigned char		rpreview;					//remote preview
	unsigned char		ralarm;						//remote alarm
	unsigned char		rvoip;						//voip
	unsigned char		lcamer;						//local yuntai
	unsigned char		lrec;						//local record
	unsigned char		lplay;						//local palyback
	unsigned char		lsetpara;					//local set param
	unsigned char		llog;						//local log
	unsigned char		ltool;						//local tool
}PACK_NO_PADDING ;

struct NETDVR_UserNumber_t 
{
	struct NETDVR_userInfo_t userinfo[8];
}PACK_NO_PADDING ;

struct NETDVR_AlarmUploadState_t
{
	//0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,
	//5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问
	unsigned char	type;		
	unsigned char	state;			//1报警 2恢复
	unsigned char	id;				//通道,硬盘号,报警输入号,取决于type 
	unsigned short	reserved1;		//预留
	unsigned int	reserved2;		//预留	
}PACK_NO_PADDING ;

enum NETDVR_REC_INDEX_MASK {
	NETDVR_REC_INDEX_TIMER = 0x1,
	NETDVR_REC_INDEX_MD = 0x2,
	NETDVR_REC_INDEX_ALARM = 0x4,
	NETDVR_REC_INDEX_HAND = 0x8,
	/*定时、移动侦测、报警、手动录像可以进行或操作*/
	NETDVR_REC_INDEX_ALL = 0x10,//全部类型录像
};

//录像文件搜索参数
struct NETDVR_recfileSearchCond_t
{
	unsigned short chn_mask;				//录像通道掩码位:某一为1对应该通道选中,需要搜索该通道的录像数据
	unsigned short type;					//NETDVR_REC_INDEX_MASK
	unsigned int start_time;				//从"1970-01-01 00:00:00"开始的秒数
	unsigned int end_time;					//从"1970-01-01 00:00:00"开始的秒数
	//unsigned short startID;
	//unsigned short max_return;
	unsigned char reserved[4];
}PACK_NO_PADDING ;

#if 0
#define MAX_FRAME_NUM_PER_SEQ  (30)
typedef struct{
	unsigned int timestamp;
	unsigned int video_frame_num:8;
	unsigned int audio_frame_num:8;
	unsigned int reserved:16;
	unsigned short video_frame_len[MAX_FRAME_NUM_PER_SEQ];
}key_info_t;
#define MAX_KEY_NUM_PER_SEG  (5000)
typedef struct{
	/*媒体信息1*/
	unsigned int video_type:8;
	unsigned int video_resolution:8;
	unsigned int is_cbr:1;
	unsigned int bitrate:15; /*kbps, VBR表示最大码率, CBR表示平均码率*/
	/*媒体信息2*/
	unsigned int is_pal:1;
	unsigned int frame_rate:7; /*0 --表示全帧率*/
	unsigned int audio_type:8;
	unsigned int audio_frame_len:16; /*音频定长数据，bytes*/
	/*视频帧序列信息*/
	unsigned int key_num:16;
	unsigned int reserved:16;
	key_info_t  key_info[MAX_KEY_NUM_PER_SEG];
}replay_desc_t;
#else
typedef struct{
	/*媒体信息1*/
	unsigned int video_type:8;
	unsigned int video_resolution:8;
	unsigned int is_cbr:1;
	unsigned int bitrate:15; /*kbps, VBR表示最大码率, CBR表示平均码率*/
	
	/*媒体信息2*/
	unsigned int is_pal:1;
	unsigned int frame_rate:7; /*0 --表示全帧率*/
	unsigned int audio_type:8;
	unsigned int audio_frame_len:16; /*音频定长数据，bytes*/
	
	/*视频帧序列信息*/
	//unsigned int key_num:16;
	//unsigned int reserved:16;
	//key_info_t  key_info[MAX_KEY_NUM_PER_SEG];
	
	unsigned int video_frames;//视频总帧数
	unsigned int audio_frames;//音频总帧数
}PACK_NO_PADDING replay_desc_t;

/*typedef struct{
	unsigned int timestamp;//时间戳
	unsigned int key:8;//1:I帧;0:P帧
	unsigned int frame_len:24;//帧长度
}video_desc_t;

typedef struct{
	unsigned int timestamp;//时间戳
}audio_desc_t;*/

typedef struct{
	unsigned int timestamp;//时间戳
	unsigned int media:1;//0:视频;1:音频
	unsigned int key:7;//0:P帧;非0:关键帧
	unsigned int frame_len:24;//帧长度
}PACK_NO_PADDING media_desc_t;
#endif

struct NETDVR_recFileInfo_t
{
	unsigned char chn;						//录像通道:0-15
	unsigned char type;						//NETDVR_REC_INDEX_MASK
	unsigned int start_time;				//start time of the record file:从"1970-01-01 00:00:00"开始的秒数
	unsigned int end_time;					//end time of the record file:从"1970-01-01 00:00:00"开始的秒数
	unsigned char image_format;				//frame type:3(Pal-cif) ; 4(Pal-D1); 8(NTSC-cif); 9(NTSC-D1)
	unsigned char stream_flag;				//stream flag:0 for video stream ; 1 for video and audio stream
	unsigned int size;						//录像文件大小
	unsigned int offset;					//录像文件在128M文件容器中的偏移(回放时要传入该参数)
	char filename[64];						//录像文件名
	//struct NETDVR_recFileInfo_t *pnext;	//poiter reference to the next record file information
}PACK_NO_PADDING ;

//录像文件搜索结果
struct NETDVR_recFileSearchResult_t
{
	//unsigned short sum;					//totals of remote indexed files.
	//unsigned short startID;				//if no file is indexed, startID will be 0, or it'll be a value based on index condition's startID(struct NETDVR_fileSearchCondition_t)
	//unsigned short endID;					//if startID isn't 0, then (endID - startID + 1) files is indexed.
	//struct NETDVR_recFileInfo_t *precInfo;//if the first file of all indexed files.
	struct NETDVR_recFileInfo_t *precArray;	//储存录像文件信息的数组
	unsigned short arraySize;				//储存录像文件信息的数组的大小
	unsigned short realSize;				//实际搜索到的文件记录的条数
}PACK_NO_PADDING ;

struct NETDVR_progressParam_t
{
	unsigned int curr_pos;
	unsigned int total_size;
}PACK_NO_PADDING ;

//南瑞服务器
struct NETDVR_NRServer_t
{
	unsigned int nrserverip; //服务器地址
	unsigned short serverport; //服务器端口
	unsigned short databindport; //数据绑定端口
	unsigned char reserved[16]; //预留
}PACK_NO_PADDING ;

//报警
struct NETDVR_AlarmVal_t
{
	unsigned char alarmid;		//alarmid 0开始
	unsigned char  val;			//取值 0未触发 1触发
	unsigned char reserved[2];	//预留
}PACK_NO_PADDING ;

struct NETDVR_TimePlayCond_t
{
	unsigned char chn;						//录像通道:0-15
	unsigned short type;					//NETDVR_REC_INDEX_MASK
	unsigned int start_time;				//从"1970-01-01 00:00:00"开始的秒数
	unsigned int end_time;					//从"1970-01-01 00:00:00"开始的秒数
}PACK_NO_PADDING ;

//解码视频格式
typedef enum NETDVR_FMT_TYPE {
	NETDVR_FMT_RGB24 = 2,					//rgb24 format
		NETDVR_FMT_RGB32 = 4,					//rgb32 format
		NETDVR_FMT_YV12 = 6,					//yv12 format
		NETDVR_FMT_I420 = 8,					//i420 format
		NETDVR_FMT_YUY2 = 10,					//yuy2 format(snapshot is not supported currently)
} fmt_type_t;

//解码码流帧结构
typedef struct FrameHeadrDec
{
	unsigned char mediaType;			//original encoded (video/audio) media type:
	char reserved1[3];					//reserved
	void *data;							//decoded data buf
	unsigned int data_size;				//decoded data length
	char reserved2[32];					//reserved for extensible development
	union
	{
		struct{
			fmt_type_t fmt;				//decoded format
			unsigned short width;		//video width
			unsigned short height;		//video height
		} video_param;
		unsigned char audio_mode;		//8, 16, or 32 bit
	};
} FrameHeadrDec;

struct NETDVR_xwServer_t
{
	u8 		flag_server;
	u32 	ip_server;
	u16 	port_server;
	u16 	port_download;
	char	device_serial[32];
	char	device_passwd[32];
	u8		flag_verifytime;
}PACK_NO_PADDING ;
#pragma pack( pop )

/* =============== callback function type for user register  ============= */
typedef void (*PFUN_MSGHASAUDIO_T)(unsigned char b_has_audio, unsigned int dwContent);
typedef void (*PFUN_PROGRESS_T)(struct NETDVR_progressParam_t progress, unsigned int dwContent);
typedef void (*pFrameCallBack)(pFrameHeadr pFrmHdr, unsigned int dwContext);
typedef void (*PFUN_MSG_T)(unsigned int dwContent);
typedef void (*PFUN_ALARMSTATE_T)(struct NETDVR_AlarmUploadState_t alarmstate, unsigned int dwContent);
typedef FrameHeadrDec* pFrameHeadrDec;
typedef void (* pDecFrameCallBack)(pFrameHeadrDec pFrmHdrDec, unsigned int dwContext);

#ifdef __cplusplus
extern "C"{
#endif

/* =============== NETDVR user function definetion  ============= */

int DVRSDK_startup(int *pHandle);
int DVRSDK_cleanup(int Handle);

int DVRSDK_openVideoChannel(int Handle, unsigned char chn, pFrameCallBack pCBFun, unsigned int dwContent);
int DVRSDK_closeVideoChannel(int Handle, unsigned char chn);

int DVRSDK_openSubVideoChannel(int Handle, unsigned char chn, pFrameCallBack pCBFun, unsigned int dwContent);
int DVRSDK_closeSubVideoChannel(int Handle, unsigned char chn);

int DVRSDK_openAudioChannel(int Handle, unsigned char chn, pFrameCallBack pCBFun, unsigned int dwContent);
int DVRSDK_closeAudioChannel(int Handle, unsigned char chn);

//回放或下载搜索到的文件
//flag: 0 - 回放 ; 1 - 下载 回放与下载唯一的区别在于回放时DVR端的应用程序会根据录像文件的时间戳作延时控制而下载不需要延时
//p_player_index:输入参数,保存回放句柄
//回放结束时SDK默认处理为调用DVRSDK_stopFilePlay函数,您也可以在中途调用DVRSDK_stopFilePlay停止回放
int DVRSDK_startFilePlay(int Handle, int *p_player_index, const struct NETDVR_recFileInfo_t *pFileInfo, unsigned char flag, pFrameCallBack pCBFun, unsigned int dwContent, PFUN_MSG_T p_over_func, unsigned int dwOverContent);
//停止文件回放
int DVRSDK_stopFilePlay(int Handle, int player_index);

//按时间回放或下载
//回放结束时SDK默认处理为调用DVRSDK_stopTimePlay函数,您也可以在中途调用DVRSDK_stopTimePlay停止回放
int DVRSDK_startTimePlay(int Handle, int *p_player_index, const struct NETDVR_TimePlayCond_t *prfs, unsigned char flag, pFrameCallBack pCBFun, unsigned int dwContent, PFUN_MSG_T p_over_func, unsigned int dwOverContent);
//停止时间回放
int DVRSDK_stopTimePlay(int Handle, int player_index);

typedef void (*pDowloadDataCBFunc)(int Handle, unsigned char* databuf, unsigned int datalen, unsigned int content);
int NETDVR_startFileDownload(int Handle, const struct NETDVR_recFileInfo_t *pFileInfo, pDowloadDataCBFunc pcbfUNC, unsigned int content);
int NETDVR_stopFileDownload(int Handle);

//录像文件搜索
int NETDVR_recFilesSearch(int Handle, const struct NETDVR_recfileSearchCond_t *prfs, struct NETDVR_recFileSearchResult_t *pdesc);

//注册报警上传回调函数
int NETDVR_regCBAlarmState(int Handle, PFUN_ALARMSTATE_T p_cb_func, unsigned int dwContent);

//启动或停止报警上传
//uploadflag:1-启动;0-停止
int NETDVR_SetAlarmUpload(int Handle, const unsigned char uploadflag);

//设置、获取中心服务器参数
//特定客户需求
int NETDVR_setxwServerParams(int Handle, const struct NETDVR_xwServer_t *pxwServ);
int NETDVR_getxwServerParams(int Handle, struct NETDVR_xwServer_t *pxwServ);

//设置、获取系统通用参数
int NETDVR_setSystemParams(int Handle, const struct NETDVR_systemParam_t *pSysPara);
int NETDVR_getSystemParams(int Handle, struct NETDVR_systemParam_t *pSysPara);

//得到VGA分辨率列表,目前我们的DVR设备一般支持800*600/1024*768/1280*1024三种分辨率
int NETDVR_GetVGAResolutonList(int Handle, struct NETDVR_VGARESOLLIST *pvgasol);

//设置、获取云镜控制参数
int NETDVR_setPtzParams(int Handle, const struct NETDVR_ptzParam_t *ptzParam);
int NETDVR_getPtzParams(int Handle, unsigned char chn, struct NETDVR_ptzParam_t *p_ptz_param);

//设置、获取巡航参数
int NETDVR_SetCruiseParam(int Handle, const struct NETDVR_cruisePath_t *p_cruise_path);
int NETDVR_GetCruiseParam(int Handle, unsigned char chn, unsigned char pathnum, struct NETDVR_cruisePath_t *p_cruise_path);

//控制云台
int NETDVR_PtzControl(int Handle, const struct NETDVR_PtzCtrl_t *p_para);

//开始、停止云台巡航
int NETDVR_startYuntaiCruise(int Handle, unsigned char chn, unsigned char path_no);
int NETDVR_stopYuntaiCruise(int Handle, unsigned char chn, unsigned char path_no);

//开始记录、结束记录、开始运行、停止运行云台轨迹
int NETDVR_SetYTTrack(int Handle, unsigned char chn, enum NETDVR_YTTRACKCONTROL yt_cmd);

//设置、删除、访问云台预置点
int NETDVR_SetYTPresetPoint(int Handle, unsigned char chn, unsigned char preset_pos, enum NETDVR_YTPRESETPOINTCONTROL yt_com);

//设置、获取图像参数
int NETDVR_setVideoParams(int Handle, const struct NETDVR_videoParam_t *p_para);
int NETDVR_getVideoParams(int Handle, unsigned char chn, struct NETDVR_videoParam_t *p_para);

//获取、设置图像亮度、对比度、色调、饱和度
int NETDVR_setPicAdjust(int Handle, const struct NETDVR_PICADJUST_T *p_para);
int NETDVR_getPicAdjust(int Handle, unsigned char chn, enum NETDVR_PICADJUST type, struct NETDVR_PICADJUST_T *p_para);

//设置、获取移动侦测参数
int NETDVR_getMotionDection(int Handle, unsigned char chn, struct NETDVR_motionDetection_t *p_para);
int NETDVR_setMotionDection(int Handle, const struct NETDVR_motionDetection_t *p_para);

//设置、获取视频丢失参数
int NETDVR_getVideoLost(int Handle, unsigned char chn, struct NETDVR_VideoLostParam_t *p_para);
int NETDVR_setVideoLost(int Handle, const struct NETDVR_VideoLostParam_t *p_para);

//设置、获取视频遮挡报警参数
int NETDVR_GetVideoBlockParam(int Handle, unsigned char chn, struct NETDVR_VideoBlockParam_t *p_para);
int NETDVR_SetVideoBlockParam(int Handle, const struct NETDVR_VideoBlockParam_t *p_para);

//设置、获取录像参数(主码流):位率、帧率、图像质量等
int NETDVR_setRecordParams(int Handle, const struct NETDVR_recordParam_t *p_para);
int NETDVR_getRecordParams(int Handle, unsigned char chn, struct NETDVR_recordParam_t *p_para);

//获取录像布防参数(录像布防包括定时录像、遮挡报警录像和移动侦测录像三种录像方式的布防)
int NETDVR_GetRecordSCH(int Handle, unsigned char chn, enum NETDVR_WEEKDAY day, struct NETDVR_RecordSCH_t *p_para);
//设置录像布防参数(布防哪一个通道和星期几在p_para所指向的结构体里指定)
int NETDVR_SetRecordSCH(int Handle, const struct NETDVR_RecordSCH_t *p_para);

//设置、获取子码流编码参数(子码流):位率、帧率、图像质量等
int NETDVR_GetSubStreamParam(int Handle, unsigned char chn, struct NETDVR_SubStreamParam_t *p_para);
int NETDVR_SetSubStreamParam(int Handle, const struct NETDVR_SubStreamParam_t *p_para);

//设置、获取报警输入参数
int NETDVR_setAlarmInParams(int Handle, const struct NETDVR_alarmInParam_t *p_para);
int NETDVR_getAlarmInParams(int Handle, unsigned char in_id, struct NETDVR_alarmInParam_t *p_para);

//设置、获取报警输出参数
int NETDVR_setAlarmOutParams(int Handle, const struct NETDVR_alarmOutParam_t *p_para);
int NETDVR_getAlarmOutParams(int Handle, unsigned char out_id, struct NETDVR_alarmOutParam_t *p_para);
//清除报警(包括蜂鸣器和所有报警输出)
int NETDVR_clearAlarms(int Handle);

//设置、获取报警email地址和短信报警手机号(短信只支持移动飞信服务,目前屏蔽此功能)
int NETDVR_GetAlarmNoticeParam(int Handle, struct NETDVR_AlarmNoticeParam_t *p_para);
int NETDVR_SetAlarmNoticeParam(int Handle, const struct NETDVR_AlarmNoticeParam_t *p_para);

//设置、获取网络参数
int NETDVR_setNetworkParams(int Handle, const struct NETDVR_networkParam_t *p_para);
int NETDVR_getNetworkParams(int Handle, struct NETDVR_networkParam_t *p_para);

//设置MAC地址
int NETDVR_setmacaddress(int Handle, const struct NETDVR_networkParam_t *p_para);

//获取所有用户信息(一共支持8个用户)
int NETDVR_GetUserInfo(int Handle, struct NETDVR_UserNumber_t *p_para);
//添加一个用户(只有admin用户有这个权限)
int NETDVR_AddUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para);
//修改用户密码(所有用户都有这个权限)
int NETDVR_EditUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para);
//删除用户(只有admin用户有这个权限/admin用户不能删除自己)
int NETDVR_DelUserInfo(int Handle, const char *username);

//得到DVR设备的常用信息
int NETDVR_GetDeviceInfo(int Handle, struct NETDVR_DeviceInfo_t *pDeviceInfo);

//得到DVR设备的视频属性:宽、高、编码方式(h264)
int NETDVR_GetVideoProperty(int Handle, struct NETDVR_VideoProperty_t *pVideoPro);
//得到DVR设备的音频属性:采样率、bit、编码方式等
int NETDVR_GetAudioProperty(int Handle, struct NETDVR_AudioProperty_t *pVideoPro);
//得到DVR设备的语音对讲属性:采样率、bit、编码方式等
int NETDVR_GetVoipProperty(int Handle, struct NETDVR_VOIPProperty_t *pVoipPro);
//得到DVR设备的移动侦测属性:移动侦测区域分多少行、多少列
int NETDVR_GetMDProperty(int Handle, struct NETDVR_MDProperty_t *pMDPro);

//获取系统版本信息
int NETDVR_remoteGetSysVerInfo(int Handle, struct NETDVR_SysVerInfo_t *p_para);

//设置、获取系统时间
int NETDVR_getSystemTime(int Handle, struct NETDVR_SysTime_t *p_para);
int NETDVR_setSystemTime(int Handle, const struct NETDVR_SysTime_t *p_para);

//获取手动录像状态
int NETDVR_GetRecordState(int Handle, struct NETDVR_ManualRecord_t *p_para);
//开启手动录像(每一位代表一个通道:1代表开启,0代表停止)
int NETDVR_SetRecordState(int Handle, const struct NETDVR_ManualRecord_t *p_para);

//获取系统第hddindex个SATA口的硬盘信息,hddindex从0开始计数
int NETDVR_remoteGetHddInfo(int Handle, unsigned char hddindex, struct NETDVR_hddInfo_t *p_hddinfo);

//语音对讲
//以下voipindex只支持 0
int NETDVR_startVOIP(int Handle, int voipindex);
int NETDVR_stopVOIP(int Handle, int voipindex);
int NETDVR_VOIPRegRcvCB(int Handle, int voipindex, pDecFrameCallBack rcvfunc, unsigned int dwContext);
int NETDVR_VOIPSetSendMode(int Handle, int voipindex, unsigned char flagmode);
//flagmode：0: win32下取声卡数据发送  1:自定义数据发送
int NETDVR_VOIPSendData(int Handle, int voipindex, FrameHeadrDec voipdata);
//NETDVR_VOIPSetSendMode 填1的时候有效

//南瑞服务器
int NETDVR_setNRServer(int Handle, const struct NETDVR_NRServer_t *p_para);
int NETDVR_getNRServer( int Handle, struct NETDVR_NRServer_t * p_para);

//报警
int NETDVR_setAlarmOutVal(int Handle, const struct NETDVR_AlarmVal_t *p_para);//获取报警输入值
int NETDVR_getAlarmInVal( int Handle, unsigned char alarm_in_id,struct NETDVR_AlarmVal_t * p_para);//设置报警输出值

//透明串口
typedef void(*pSerialDataCallBack)(int lSerialPort, unsigned char bySerialChannel, char *pRecvDataBuffer, unsigned int dwBufSize, unsigned int dwContent);
//lSerialPort 1－232串口；2－485串口 
int NETDVR_SerialStart(int Handle, int lSerialPort,	pSerialDataCallBack cbSerialDataCallBack, unsigned int dwContent);
//byChannel, 使用485串口时有效，从1开始；232串口作为透明通道时该值设置为0 
//dwBufSize 最大值4096
int NETDVR_SerialSend(int Handle, int lSerialPort, unsigned char byChannel, char* pSendBuf, unsigned int dwBufSize);
int NETDVR_SerialStop(int Handle, int lSerialPort);

/*
NETDVR_regCBMsgHasAudio: register a callback function to deal with no audio(file has no audio) notification message for one dvr's play reciever
NETDVR_regCBMsgProgress: register a callback function to deal with progress notification message for one dvr's play reciever
*/
int NETDVR_regCBMsgHasAudio(int Handle, int player_index, PFUN_MSGHASAUDIO_T p_cb_func, unsigned int dwContent);
int NETDVR_regCBMsgProgress(int Handle, int player_index, PFUN_PROGRESS_T p_cb_func, unsigned int dwContent);
/*
NETDVR_pausePlay: pause a play
NETDVR_resumePlay: resume a paused play
NETDVR_singleFramePlay: single frame play
NETDVR_fastPlay: play faster(when playrate = 8,  reset playrate to 1)
NETDVR_slowPlay: play slower(when playrate = -8,  reset playrate to 1)
NETDVR_setPlayRate: set play rate to a new value(play_rate [-8, 8])(-8 means 1/8)
*/
//暂停播放
int NETDVR_pausePlay(int Handle, int player_index);
//恢复播放
int NETDVR_resumePlay(int Handle, int player_index);
//帧进
int NETDVR_singleFramePlay(int Handle, int player_index);
//快放
int NETDVR_fastPlay(int Handle, int player_index);
//慢放
int NETDVR_slowPlay(int Handle, int player_index);
//播放速率
int NETDVR_setPlayRate(int Handle, int player_index, int play_rate);

/*
NETDVR_playPrevious: play previous segments when play by file; play previous file when play by time
NETDVR_playNext: play next segments when play by file; play next file when play by time
NETDVR_playSeek: play to a new position
NETDVR_playMute: play in mute or not depending on b_mute
NETDVR_playProgress: let DVR send play progress depending on b_send_progress
*/
//上一段
int NETDVR_playPrevious(int Handle, int player_index);
//下一段
int NETDVR_playNext(int Handle, int player_index);
//定位播放位置
int NETDVR_playSeek(int Handle, int player_index, unsigned int new_time);
//回放静音
int NETDVR_playMute(int Handle, int player_index, int b_mute);
//开启进度回调
int NETDVR_playProgress(int Handle, int player_index, int b_send_progress);

//恢复出厂设置
int NETDVR_restoreFactorySettings(int Handle);
//重启设备
int NETDVR_reboot(int Handle);
//关机(通过面板软关机)
int NETDVR_shutdown(int Handle);

//获得视频分辨率
int NETDVR_getVideoResoluList( int Handle, unsigned char chn, unsigned char vidoetype, struct NETDVR_VideoResolu_list_t* pList );

int NETDVR_sendExtendCmd(int Handle, unsigned short wCommand, const void *pInData, int nInDataLen, void* pOutData, int nMaxOutDatalen);

#ifdef __cplusplus
}
#endif

#endif
