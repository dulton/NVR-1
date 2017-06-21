/******************************************************************************
* 系统名称：设备参数
* 文件名称：param.h
* 版本    ：	V2.0.0.0.2
* 说明	  ：
			接口结构参数类型定义；

			  
			  一、数据类型 定义
			  		I、 系统常量
			  		II、 常量宏定义
			  		III、球机云台命令
			  		
			  二、枚举类型定义
			  	
			  		

			   三、编码器参数设置信息结构
			   		I、   公用结构
			   		II、  系统参数
			   			1、日期时间
			   			2、配置信息
			   			3、NTP参数
			   			4、用户设置

			   		III、  视频参数
			   			1、视频设置(osd字符叠加、 视频编码、 亮色调节,   图片参数)
			   			2、设备支持的编码格式和宽高
			   			3、图像屏蔽设置
			   			4、视频输入的色彩调节和相关设置
			   			5、视频输入场景相关设置 
			   			6、视频输入的所有色彩调节和场景设置
			   			7、视频信号参数
			   		IV、  音频属性
			   			1、音频设置
			   			
			   		V、   网络设置
			   			1、网络设置结构(基本参数、有线网络、pppoe参数、 DDNS参数、中心连接)
			   			2、无线 WIFI 参数
			   			3、UPNP  参数
			   			4、EMAIL参数
			   			5、 FTP 服务器参数
			   			6、DDNS参数
			   			7、3G参数
			   		VI、  存储设置
			   			1、磁盘信息
			   			2、录像计划
			   			3、抓拍计划
			   			
			   		VII、 报警设置
			   			1、移动报警
			   			2、探头报警
			   			3、网络报警
			   			4、视频丢失报警
			   			5、报警输出控制
			   			
			   		VIII、串口设置

					IX、云台
			   			1、透明通讯
			   			2、云台控制
			   			3、云台信息
			   			
			   		X、  其它参数结构
			   			1、TW2824参数
			   			2、定时重启
			   			3、扩展信息结构(ntp, upnp, email, alarm in/out, wifi, send dest, auto reset, ftp, rstp, gps)
			   			4、用户数据
			   			5、设备所有参数结构
			   			
			   	四、 解码器参数设置信息结构
* 其他说明: 无
******************************************************************************/
#ifndef  __PARAM_H__
#define  __PARAM_H__
	
#define PACKED  __attribute__((packed, aligned(1)))
#define PACKED4 __attribute__((packed, aligned(4)))


 /**************************** ********************************************
						  一、数据类型 定义
 **************************************************************************/

 // 1， 基本数据类型定义

typedef	char				CHAR;
typedef	short				SHORT;
typedef int					INT;
typedef	long				LONG;
typedef	unsigned char		UCHAR;
typedef	unsigned short		USHORT;
typedef unsigned int		UINT;
typedef	unsigned long		ULONG;
typedef	int					BOOL;
typedef unsigned long*		HANDLE;
typedef unsigned long  		*TALKHANDLE;
typedef void*				LPVOID;

typedef unsigned char       UInt8;
typedef unsigned short      UInt16;
typedef unsigned int        UInt32;

typedef unsigned char  		 BYTE ;
typedef unsigned long 		 DWORD;


//2，系统常量
#define	IN
#define	OUT
#define	MAX_PATH					256

#define MAX_VIDEO_NUM				4	//数字视频服务器最大通道数
#define MAX_AUDIO_NUM				4	//数字视频服务器音频最大通道数
#define MAX_SENSOR_NUM				4	//探头输入路数
#define MAX_RF_SENSOR_NUM			8	//无线探头最大路数
#define MAX_OUTPUT_NUM				4	//探头输出路数
#define MAX_COM_NUM					2	//串口最大个数	
#define MAX_USER_NUM				5	//用户个数
#define MAX_LOGON_NUM				10	//最大同时登录用户数
#define MAX_LINKVIEW_NUM			16	//最大同时TCP、UDP预览用户数
#define MAX_MULTIVIEW_NUM			60	//最大多播用户数
#define DVS_NAME_LEN           	32	//数字视频服务器名称长度

#define CAM_TITLE_LEN_NEW			64	//31个汉字，63个字符
#define CAM_TITLE_LEN				16	//摄像机标题长度(8个汉字，16个字符)
#define USER_NAME_LEN 				16	//用户名的最大长度
#define USER_PASSWD_LEN   			16	//用户密码最大长度
	
#define DDNS_NAME_LEN				32	//注册主机名称长度
#define DDNS_PASS_LEN				16	//注册主机密码长度	
#define	DDNS_SERVER_NAME_LEN		32
#define PPPPOE_NAME_LEN			32	//拨号用户名长度
#define PPPPOE_PASSWD_LEN			16	//拨号用户名长度
#define YUNTAI_NAME_LEN			32	//云台协议名称长度
#define MAX_YUNTAI_PROTOCOL_NUM	36	//内置最大云台协议数
#define MAX_PTZ_PROTOCOL_NUM  		4	//最大云台协议数
#define	MAX_IP_NAME_LEN			128	//输入IP或名字的最大长度
#define	IP_STR_LEN					16	//IP长

#define	DOMAIN_NAME_LEN			32  //域名的长度
#define EMAIL_TITLE_LEN        	32	//EMAIL头和内容长
#define DD_URL_ADDR_LEN			32	//URL地址长度
#define	DD_MAX_ADDR_NUM			32	//地址薄的地址数量
#define	DD_MAX_ALARMINFO_NUM		64	//报警记录个数最大值

#ifndef NULL
#define NULL  ((void *)0)
#endif


//3，云台控制
#define	YT_UP					1	//云台上
#define	YT_DOWN					2	//云台下	
#define	YT_LEFT					3	//云台左
#define	YT_RIGHT				4	//云台右
#define	YT_FOCUSADD				5	//聚焦+
#define	YT_FOCUSSUB				6	//聚焦-
#define	YT_IRISADD				7	//光圈+
#define	YT_IRISSUB				8	//光圈-
#define	YT_ZOOMADD				9	//变倍+
#define	YT_ZOOMSUB				10	//变倍-
#define	YT_AUTOOPEN				11	//自动开
#define	YT_AUTOCLOSE			12	//自动关
#define	YT_LAMPOPEN				13	//灯光开
#define	YT_LAMPCLOSE			14	//灯光关
#define	YT_BRUSHOPEN			15	//雨刮开
#define	YT_BRUSHCLOSE			16	//雨刮关
#define	YT_WATEROPEN			17	//放水开
#define	YT_WATERCLOSE			18	//放水关
#define	YT_PRESET				19	//预置 + 号
#define	YT_CALL					20	//调用 + 号
#define	YT_STOP					21	//停止
#define	YT_UP_STOP				30	//云台上-停	
#define	YT_DOWN_STOP			31	//云台下-停	
#define	YT_LEFT_STOP			32	//云台左-停
#define	YT_RIGHT_STOP			33	//云台右-停
#define	YT_FOCUSADD_STOP		34	//聚焦+ -停
#define	YT_FOCUSSUB_STOP		35	//聚焦- -停
#define	YT_IRISADD_STOP		36	//光圈+ -停
#define	YT_IRISSUB_STOP		37	//光圈- -停
#define	YT_ZOOMADD_STOP		38	//变倍+ -停
#define	YT_ZOOMSUB_STOP		39	//变倍- -停	
#define YT_PRESET_DEL			70  //预置位删除

//-----------------------------------end--------------------------------


/********************************************************************************
								二、枚举类型定义
*********************************************************************************/

// 视频编码算法
typedef enum _VIDEO_ENCODER_E
{
    VENC_NONE  = 0x00,
    VENC_H264  = 0x01,
    VENC_MPEG4 = 0x02,
    VENC_MJPEG = 0x03,
    VENC_JPEG  = 0x04,
}VIDEO_ENCODER_E;

//音频编码算法
typedef enum _AUDIO_ENCODER_E
{
    AENC_NONE  = 0x00,
    AENC_G726  = 0x01,
    AENC_G722  = 0x02,
    AENC_G711  = 0x03,
    AENC_ADPCM = 0x04,
    AENC_MP3   = 0x05,
}AUDIO_ENCODER_E;

//音频采样率
typedef enum _AUDIO_SAMPLERATE_E
{
    AI_SAMPLE_RATE_NONE  = 0, 
    AI_SAMPLE_RATE_8000  = 8000,  /* 8kHz sampling rate      */
    AI_SAMPLE_RATE_11025 = 11025, /* 11.025kHz sampling rate */
    AI_SAMPLE_RATE_16000 = 16000, /* 16kHz sampling rate     */
    AI_SAMPLE_RATE_22050 = 22050, /* 22.050kHz sampling rate */
    AI_SAMPLE_RATE_24000 = 24000, /* 24kHz sampling rate     */
    AI_SAMPLE_RATE_32000 = 32000, /* 32kHz sampling rate     */
    AI_SAMPLE_RATE_44100 = 44100, /* 44.1kHz sampling rate   */
    AI_SAMPLE_RATE_48000 = 48000, /* 48kHz sampling rate     */
}AUDIO_SAMPLERATE_E;

//音频编码码率(G726)
typedef enum _AUDIO_BITRATE_E
{ 
    AENC_BITRATE_NONE  = 0,
    AENC_BITRATE_16000 = 16000, 
    AENC_BITRATE_24000 = 24000, 
    AENC_BITRATE_32000 = 32000, 
    AENC_BITRATE_40000 = 40000, 
}AUDIO_BITRATE_E;


//视频输入的色彩调节和相关设置，类型定义
typedef enum _VIDEO_IN_SENSOR_E
{
	//色彩
	VCT_BRIGHTNESS	=	0X00000001,//亮度
	VCT_HUE			=	0X00000002,//色度
	VCT_CONTRAST	=	0X00000004,//对比度
	VCT_SATURATION	=	0X00000008,//饱和度
	VCT_SHARPNESS	=	0X00000010,//锐度
	VCT_RED			=	0X00000020,//红色
	VCT_GREEN		=	0X00000040,//绿色
	VCT_BLUE		=	0X00000080,//蓝色
	VCT_GAMMA		=	0X00000100,//Gamma

	//白平衡
	VCT_AUTOAWB		=	0X00000200,//自动白平衡
	VCT_AWBRED		=	0X00000400,//白平衡 红
	VCT_AWBGREEN	=	0X00000800,//白平衡 绿
	VCT_AWBBLUE		=	0X00001000,//白平衡 蓝

	//增益
	VCT_AUTOAGC		=	0X00002000,//自动增益
	VCT_AGCLEVEL	=	0X00004000,//增益值

	//背光补偿
	VCT_AUTOBLC		=	0X00008000,//自动补偿
	VCT_BLCLEVEL	=	0X00010000,//补偿值
	
	//曝光
	VCT_AUTOEXPOSURE=	0X00020000,//自动曝光 
	VCT_EXPOSURETIME=	0X00040000,//手动曝光时间 
	
	//快门
	VCT_SHUTTERFIRST=	0X00080000,//快门、曝光优先
	VCT_AUTOSHUTTER	=	0X00100000,//自动快门
	VCT_SHUTTERSPEED=	0X00200000,//快门速度
	VCT_SLOWSHUTTER	=	0X00400000,//慢快门
	VCT_SLOWSHUTTERLEVEL=0X00800000,//慢快门速度

	VCT_AUTOAWBMODE =	0X01000000,//1080p Hispeed自动白平衡模式

	VCT_SENSOR_ALL	=	0xFFFFFFFF,//设置所以参数
}VIDEO_IN_SENSOR_E;

//视频输入场景设置，类型定义
typedef enum _VIDEO_IN_SCENE_E
{
	//光圈
	VCT_AUTOIRIS	=	0X00000001,//自动光圈
	VCT_IRISLEVEL	=	0X00000002,//光圈电平

	//聚焦
	VCT_AUTOFOCUS	=	0X00000004,//自动聚焦
	VCT_FOCUSLEVEL	=	0X00000008,//焦距

	//变倍
	VCT_ZOOMSPEED	=	0X00000010,//变倍速度
	VCT_AUTOPTZSPEED=	0X00000020,//景深比例变速

	//电平控制
	VCT_AUTOALC		=	0X00000040,//自动电平控制
	VCT_ALCLEVEL	=	0X00000080, //电平控制值
	
	//彩转黑
	VCT_CTB			=	0X00000100,//彩转黑

	//场景
	VCT_SCENE		=	0X00000200,//场景
	VCT_MIRROR		=	0X00000400,//镜向
	VCT_FLIP		=	0X00000800,//翻转
	VCT_AUTOFLIP	=	0X00001000,//自动翻转
	VCT_PWDFREQ1	=	0X00002000,//照明频率1
	VCT_PWDFREQ2	=	0X00004000,//照明频率2

	//红外
	VCT_IREXIST		=	0X00008000,//是否有红外
	VCT_IRCFMODE	=	0X00010000,//IRCF模式
	VCT_IRLIGHTTYPE	=	0X00020000,//红外灯类型
	//
	VCT_WDR 		=	0X00040000,//是否有宽动态
	VCT_WDRLEVEL	=	0X00080000,//宽动态的值
	VCT_LOWILLUM	=	0X00100000,//低照度
	VCT_IMAGEMODE	=	0X00200000,//图像模式
	//
	VCT_VI_SIZE     =   0X00400000,//视频输入尺寸
	VCT_CTBLEVEL	=	0X00800000,//彩转黑阀值，当彩转黑为自动时有效
	VCT_MINFOCUSLEN =   0X01000000,//设置最小聚焦距离
	VCT_IRLEVEL		=   0X02000000,//夜间启动点		
	VCT_LENSCORRECTION =0X04000000,//镜头校正		
	VCT_SMARTNR     = 	0x08000000,//智能降噪 0 ~ 255  0为关
	VCT_3DNR   	    =	0X10000000,//3D降噪		
	VCT_3DNRLEVEL	= 	0x20000000,//3D降噪值

	VCT_SCENE_ALL	=	0xFFFFFFFF,//设置所以参数
}VIDEO_IN_SCENE_E;


typedef enum _WF_AUTH_TYPE		//主认证加密类型
{
	WF_AUTH_NONE	= 0x00,
	WF_AUTH_WEP		= 0x01,
	WF_AUTH_WPA		= 0x02,
	WF_AUTH_WPA2	= 0x03,
	WF_AUTH_WPAWPA2	= 0x04,
}WF_AUTH_TYPE;


typedef enum _WF_ENC_TYPE		//辅助认证加密
{
	WF_ENC_NONE		= 0x00,		//WEP  开放系统
	WF_ENC_WEP		= 0x01,		//WEP  共享密匙
	WF_ENC_TKIP		= 0x02,		//WEP  自动选择
	WF_ENC_AES		= 0x03,
}WF_ENC_TYPE;


// 3G拨号限制模式   0-不限制 1-每天限制时间  2-每月限制时间  3-既限制每天的时间，也限制每月的时间
typedef enum T3G_TIME_CTRL_MODE
{
	NO_LIMITED    = 0x00,
	DAY_LIMITED   = 0x01,
	MONTH_LIMITED = 0x02,
	ALL_LIMITED   = 0x03,
}T3G_TIME_CTRL_MODE;// 限制模式


//网络协议类型
typedef enum _NET_PROTOCOL_TYPE
{
    NET_PROTOCOL_TCP 	= 0,		//TCP协议
    NET_PROTOCOL_UDP 	= 1,		//UDP协议
    NET_PROTOCOL_MULTI = 2			//多播协议
}NET_PROTOCOL_TYPE;
//---------------------------------------------end---------------------------------




/**************************** ********************************************
						 三、编码器参数设置结构
**************************************************************************/

//-----------------------------------------------------------//
// CMS_NET (网络命令)    CMD_GET_XX(获取参数网络命令)    CMD_SET_XX (设置参数网络命令)


//-----------------------------I、公用结构-----------------------------------------
//时间段结构：定义一天的起始时间和结束时间

typedef struct _TIME_SEGMENT_NEW 
{
	UINT 		bDetectOpen;				//检测开关
	UCHAR        nBgnHour ;					//开始时
	UCHAR        nBgnMinute ;				//开始分
	UCHAR        nEndHour ;					//结束时
	UCHAR        nEndMinute ;				//结束分
}TIME_SEGMENT_NEW;

typedef  struct  _DVS_TIME
{
	UCHAR		year ;
	UCHAR		month ;
	UCHAR		day ;
	UCHAR		week ;
	UCHAR		hour   ;
	UCHAR		minute ;
	UCHAR		second ;
	UCHAR		millisecond;							
} DVS_TIME, *PDVS_TIME;



typedef struct _DATE_TIME_ST
{
	UINT			second : 6;							//秒:  0 ~ 59
	UINT			minute : 6;							//分:  0 ~ 59
	UINT			hour : 5;							//时:  0 ~ 23
	UINT			day : 5;							//日:  1 ~ 31
	UINT			month : 4;							//月:  1 ~ 12
	UINT			year : 6;							//年:  2000 ~ 2063
}DATE_TIME_ST;
//---------------------------------------------------------------------------------------


//-----------------------------II、系统参数结构-----------------------------------

//1，日期时间结构
//CMD_NET   CMD_GET_TIME  CMD_SET_TIME
typedef  struct  _DVS_DATE
{
	UCHAR		year;									//年
	UCHAR		month;									//月
	UCHAR		day;									//日
	UCHAR		week;									//周
	UCHAR		hour;									//小时
	UCHAR		minute;									//分钟
	UCHAR		second;									//秒
}PACKED DVS_DATE;

//2，配置信息
//CMS_NET : CMD_GET_SYS_CONFIG		 CMD_SET_SYS_CONFIG      
typedef struct _DVS_CONFIG
{
   	CHAR               	sysName[DVS_NAME_LEN + 1];		//名字
	UCHAR				sysInputNum;					//路数
	UCHAR				sysType;						//机器型号	0:CIF  1:HD1  2:D1  
	UCHAR				sysVideoFormat;				//编码格式	
    UCHAR				sysLanguage;					//语言
    UCHAR				sysStandard;					//制式		0:PAL  1:NTSC
    UCHAR				AD[2];
	UINT      			sysID;  						//每台机器有个ID号  	
	UINT      			sysVer;  						//软件版本  		
}PACKED DVS_CONFIG;


//3、NTP  参数
//CMS_NET: 	CMD_GET_NTP		CMD_SET_NTP    
typedef struct _NTP_CONFIG
{
	UINT			ntpOpen;							//NTP开关
	UINT			ntpTimeZone;						//时区
	CHAR			ntpHost[DOMAIN_NAME_LEN];			//NTP Server
}NTP_CONFIG;


//4，用户设置
//CMS_NET:	CMD_GET_USER_CONFIG	  CMD_SET_USER_CONFIG    		  
typedef struct _USER_CONFIG
{
	UINT 		Level;									//权限
	CHAR 		Name[USER_NAME_LEN + 1];				//用户名
	CHAR 		Password[USER_PASSWD_LEN + 1];			//用户密码
	CHAR 		AD[2];
} PACKED USER_CONFIG;

//------------------------------end-------------------------------------

//-------------------------------III、视频参数-----------------------------

// osd  参数坐标、开关
typedef struct _OSD_CTRL_ST
{
	UINT		Show;						//0: 关  1: 开
	USHORT		X;							//x，y：显示的位置，该位置的范围为0-704，0-576，与图像的分辨率无关，x必须为4的倍数；
	USHORT		Y;
} OSD_CTRL_ST ;

//视频编码参数
typedef struct _VENC_CTRL_ST
{
	UINT        Open;						//是否开启编码（从码流）
	UINT		EncType;					//编码算法
	UINT		EncFormat;					//编码格式(D1, HD1, CIF)
	UINT		Width;						//宽度
	UINT		Height;						//高度
	
	UINT		KeyInterval;			    //I 帧间隔
	UINT		Bitrate ;					//码率
	float		FrameRate ;					//帧率float
	UINT		BitflowType; 			    //码流类型(0:VBR, 1:CBR)
	UINT		Qulity;						//质量0--5
	UINT		Profile;				 	//0: baseline 1:main profile
	UINT       Reserved[7];
} VENC_CTRL_ST ;


//视频宽高
typedef struct _VIDEO_SIZE
{
	USHORT		Width;							//宽
	USHORT		Height;							//高
}VIDEO_SIZE;


//=======================================
//抓拍图片参数
//=======================================
typedef struct _PICTURE_CONFIG					//sizeof() = 148
{
	UINT		dwSize;							//本结构长度

	UINT     	EncType;						//抓图格式(.jpg, bmp) 
	UINT     	EncFormat;						//图片其它参数(24位， 32位) 
	UINT     	Width;
	UINT     	Height;
	UCHAR		TwoFeild;						//
	UCHAR 		qulity;							//(1~5)
	UCHAR     	reseved[42];
}PICTURE_CONFIG;


//1, 视频设置(osd字符叠加、 视频编码、 亮色调节,   图片参数)
//CMS_NET:  CMD_GET_VIDEO_CONFIG_NEW	 	CMD_SET_VIDEO_CONFIG_NEW
typedef struct _VIDEO_CONFIG_NEW
{
	UINT			dwSize;						//本结构长度

	VENC_CTRL_ST 	stMainVEncCtrl;			//主码流控制
	VENC_CTRL_ST 	stMinVEncCtrl;				//从码流控制
	PICTURE_CONFIG	stPictureCfg;				//抓拍图片参数

	OSD_CTRL_ST		stDateOSD; 					// 0:不叠加日期,   1:叠加日期
	OSD_CTRL_ST		stTimeOSD; 					// 0:不叠加时间,   1:叠加时间
	OSD_CTRL_ST		stWeekOSD; 					// 0:不叠加星期,   1:叠加星期 
	OSD_CTRL_ST		stTitleOSD;					// 0:不叠加标题,   1:叠加标题 
	OSD_CTRL_ST		stBitrateOSD;				// 0:不叠加码率,   1:叠加码率 	
	
	char    		Title[CAM_TITLE_LEN_NEW];	//标题字符(64个字符  31个汉字)

    UCHAR 			Brightness;				    //亮度
	UCHAR 			Hue;						//色调
	UCHAR 			Saturation;				    //饱和度
	UCHAR 			Contrast;					//对比度

	USHORT 			Protocol;					//协议编号(0 ~ MAX_PAN_PROTOCOL_NUM - 1)
	USHORT 			YTAddr;						//解码器地址
	UCHAR			mtClear;
	UCHAR			mtRed;
	UCHAR			mtBlue;
	UCHAR			mtGamma;
	UCHAR			nOsdPrefer;   				//0: XXXX-XX-XX 年月日 
								 				//1: XX-XX-XXXX 月日年 
												//2: XXXX年XX月XX日 
												//3: XX月XX日XXXX年
							  
								
	UCHAR       	nOsdColor;	 			// 	osd颜色0白色 1 黑 2黄 3红 4 蓝
	UCHAR      	 	nScene;      
	UCHAR       	nOSDClientNum;
	UCHAR        	reseved[24];
}VIDEO_CONFIG_NEW;


//2， 设备支持的编码格式和宽高
//CMS_NET : CMD_GET_SUPPORT_AV_FMT

typedef struct _SUPPORT_AV_FMT
{
	UINT		dwSize;						//本结构长度
	
	UINT		MainVideoFmt[8];			//主码流编码格式
	UINT		MinVideoFmt[8];				//从码流编码格式
	UINT		PictureFmt[8];				//图片编码格式

	VIDEO_SIZE  MainVideoSize[8];			//主码流 宽、高	
	VIDEO_SIZE  MinVideoSize[8];			//从码流 宽、高	
	VIDEO_SIZE  PictureSize[8];				//图片 宽、高	

	UINT		AudioFmt[8];				//音频编码格式
	UINT		AudioSampleRate[8];			//音频采样率
	UINT		AudioBitrate[8];			//音频码率
	UINT		AuidoAEC;					//是否支持回声抵消

	UCHAR     	reserve[32];	
}SUPPORT_AV_FMT;


typedef struct _AREA_RECT
{
	USHORT		X;							//X坐标	
	USHORT		Y;							//Y坐标
	USHORT		W;							//宽度
	USHORT		H;							//高度
} AREA_RECT;


//3，图像屏蔽设置
//CMS_NET:	CMD_GET_VMASK_CONFIG		CMD_SET_VMASK_CONFIG 	

typedef struct _VIDEOMASK_CONFIG_NEW
{
	UCHAR 			MaskOpen;					//视频屏蔽开关		0:关         1：开
	UCHAR			AA[3];
	AREA_RECT		MaskArea[9];				//视频屏蔽区域		
}VIDEOMASK_CONFIG_NEW;


//4，视频输入的色彩调节和相关设置 size=64+32=96
//CMS_NET: CMD_GET_VI_SENSOR		CMD_SET_VI_SENSOR

typedef struct _VIDEO_IN_SENSOR_S
{
	VIDEO_IN_SENSOR_E	eValidSupport;		/*有效支持的参数，该参数设置时候无效
											在获取的时候有效，用以判断设备是否支持该参数
											*/

	VIDEO_IN_SENSOR_E	eValidSetting;		/*设置有效的参数，该参数获取时候无效
											在设置的时候有效，用以指定具体设置的参数
											*/

	//色彩
	UInt8		byBrightness;				/*亮度;			0 ~ 255*/
	UInt8		byBrightnessDefault;		/*亮度缺省值;	*/
	UInt8		byBrightnessStep;			/*亮度调节步长;	*/

	UInt8		byHue;						/*色度;			0 ~ 255*/
	UInt8		byHueDefault;				/*色度缺省值;	*/
	UInt8		byHueStep;					/*色度调节步长;	*/
	
	UInt8		byContrast;					/*对比度;	0 ~ 255*/
	UInt8		byContrastDefault;			/*对比度缺省值;	0 ~ 255*/
	UInt8		byContrastStep;			/*对比度调节步长;	0 ~ 255*/

	UInt8		bySaturation;				/*饱和度;	0 ~ 255*/
	UInt8		bySaturationDefault;		/*饱和度缺省值;*/
	UInt8		bySaturationStep;			/*饱和度调节步长;*/

	UInt8		bySharpness;				/*锐度;		0 ~ 255*/
	UInt8		bySharpnessDefault;		/*锐度缺省值;*/
	UInt8		bySharpnessStep;			/*锐度调节步长;*/

	UInt8		byRed;						/*红色;		0 ~ 255*/
	UInt8		byRedDefault;				/*红色缺省值;*/
	UInt8		byRedStep;					/*红色调节步长;*/

	UInt8		byGreen;					/*绿色;		0 ~ 255*/
	UInt8		byGreenDefault;			/*绿色缺省值;*/
	UInt8		byGreenStep;				/*绿色调节步长;*/

	UInt8		byBlue;						/*蓝色;		0 ~ 255*/
	UInt8		byBlueDefault;				/*蓝色缺省值;*/
	UInt8		byBlueStep;					/*蓝色调节步长;*/

	UInt8		byGamma;					/*gamma;	0 ~ 255*/
	UInt8		byGammaDefault;			/*gamma缺省值;*/
	UInt8		byGammaStep;				/*gamma调节步长;*/


	//白平衡
	UInt8		byAutoAwb;					/*自动白平衡;	0自动, 1 手动*/
	
	UInt8		byAwbRed;					/*白平衡 红;	0 ~ 255*/
	UInt8		byAwbRedDefault;			/*白平衡 红缺省值;*/
	UInt8		byAwbRedStep;				/*白平衡 红调节步长;*/

	UInt8		byAwbGreen;					/*白平衡 绿;	0 ~ 255*/
	UInt8		byAwbGreenDefault;			/*白平衡 绿缺省值;*/
	UInt8		byAwbGreenStep;			/*白平衡 绿调节步长;*/

	UInt8		byAwBblue;					/*白平衡 蓝;	0 ~ 255*/
	UInt8		byAwBblueDefault;			/*白平衡 蓝缺省值;*/
	UInt8		byAwBblueStep;				/*白平衡 蓝调节步长;*/

	//高清高速球   1080P   byAgcLevel --> 最大增益值
	//						  720P   byAgcLevel --> AGC
	//							     byAutoAgc  --> 最大增益值
 	//增益
	UInt8		byAutoAgc;					/*自动增益;		0自动, 1 手动*/
	UInt8		byAgcLevel;					/*增益值;		0 ~ 255*/
	UInt8		byAgcLevelDefault;			/*增益值缺省值;*/
	UInt8		byAgcLevelStep;			/*增益值调节步长;*/


	//背光补偿
	UInt8		byAutoBlc;					/*自动补偿;		0自动, 1 手动*/
	UInt8		byBlcLevel;					/*补偿值;		0 ~ 255*/
	UInt8		byBlcLevelDefault;			/*补偿值缺省值;*/
	UInt8		byBlcLevelStep;			/*补偿值调节步长;*/
	

	//曝光
	UInt8		byAutoExposure;			/*0自动 1 手动*/
	UInt16		wExpoSuretime;				/*手动曝光时间	F1.6=16
													F2.2=22
													F3.2=32
													F4.4=44
													F6.4=64
													F8.8=88
													F12.0=120
													F17.0=170
													F24.0=240
													F34.0=340
											*/


	//快门
	UInt8		byShutterFirst;			/*快门、曝光优先;	0快门优先, 1 曝光优先*/
	UInt8		byAutoShutter;				/*自动快门;		0自动, 1 固定*/
	UInt16		wShutterSpeed;				/*快门速度;		1		= 1
													1/2		= 2
													1/4		= 4
													1/8		= 8
													1/16	= 16
													1/25	= 25
													1/50	= 50
													1/100	= 100
													1/150	= 150
													1/200	= 200
													1/250	= 250
													1/300	= 300
													1/400	= 400
													1/1000	= 1000
													1/2000	= 2000
													1/4000	= 4000
													1/10000	= 10000
												*/
	UInt8		bySlowShutter;					/*慢快门;		0关,   1 开*/

	UInt8		bySlowShutterLevel;			/*慢快门速度;	0 ~ 255*/
	UInt8		bySlowShutterLevelDefault;	/*慢快门速度缺省值;*/
	UInt8		bySlowShutterLevelStep;		/*慢快门速度调节步长;*/
	
	UInt8		byAwbAutoMode;					//自动白平衡模式,仅当白平衡为自动时有效
	UInt8		byMaxAgc;						//最大增益值   (自动模拟增益)
	UInt16		wExpTimeMax;					//自动曝光快门最大值  
	UInt8		byAntiFog;						//去雾
	UInt8       byAntiFalseColor;              //去伪彩
	UInt8       byAntiDIS;                      //去抖
	UInt8       byRotate;                       //旋转

	UInt8		byRes[24];
}VIDEO_IN_SENSOR_S,*LPVIDEO_IN_SENSOR_S;


//5，视频输入场景相关设置 size=32+64=96
//CMS_NET:  CMD_GET_VI_SCENE	CMD_SET_VI_SCENE   
typedef struct _VIDEO_IN_SCENE_S
{
	VIDEO_IN_SCENE_E	eValidSupport;		/*有效支持的参数，该参数设置时候无效
											在获取的时候有效，用以判断设备是否支持该参数
											*/

	VIDEO_IN_SCENE_E	eValidSetting;		/*设置有效的参数，该参数获取时候无效
											在设置的时候有效，用以指定具体设置的参数
											*/

	//光圈
	UInt8		byAutoIris;					/*自动光圈;		0自动, 1 手动*/
	UInt8		byIrisLevel;				/*光圈电平;			0 ~ 255*/
	UInt8		byIrisLevelDefault;		/*光圈电平缺省值;*/
	UInt8		byIrisLevelStep;			/*光圈电平调节步长;*/


	//聚焦
	UInt8		byAutoFocus;				/*自动聚焦;		0自动, 1 手动*/

	UInt8		byFocusLevel;				/*焦距;				0 ～ 255*/
	UInt8		byFocusLevelDefault;		/*焦距缺省值;*/
	UInt8		byFocusLevelStep;			/*焦距调节步长;*/


	//变倍
	UInt8		byZoomSpeed;				/*变倍速度;		0 高速,  1 正常速度*/
	UInt8		byAutoPtzSpeed;			/*景深比例变速; 0关,   1 开*/


	//电平控制
	UInt8		byAutoAlc;					/*自动电平控制;	AutoALC	0自动 1 手动*/

	UInt8		byAlcLevel;					/*电平值;			ALC Level	0 ~ 255*/
	UInt8		byAlcLevelDefault;			/*电平缺省值;		ALC Level	*/
	UInt8		byAlcLevelStep;			/*电平值调节步长;	ALC Level	*/


	//彩转黑
	UInt8		byCtb;						/*彩转黑;		0关,   1 开*/


	//场景
	UInt8		byScene;					/*场景;			0 ~ 255*/
	UInt8		byMirror;					/*镜向;			0关,   1 开*/
	UInt8		byFlip;						/*翻转;			0关,   1 开*/
	UInt8		byAutoFlip;					/*自动翻转;		0关,  1 开*/
	UInt8		byPwdFreq1;					/*照明频率1;	0 60HZ,	1 50HZ*/
	UInt8		byPwdFreq2;					/*照明频率2;	0 60HZ,	1 50HZ,	2 户外*/


	//红外
	UInt8		byIRExist;					/*是否有红外	0无,   1 有*/
	UInt8		byIRCFMode;					/*IRcf模式;		0 OUT=>IN, 1 IN=>OUT*/
	UInt8		byIRLightType;				/*红外灯类型;	0 正常光,  1 850mm,   2 950mm*/
	UInt8		byWDR;						/*宽动态		0无,   1 有*/
	UInt8		byWDRLevel;					/*宽动态		0 ~ 255*/
	UInt8		byLowIllumination;			/*低照度		1默认AF
													2低照度AF优化开启
													3低照度点光源AF优化开启								
											*/
	UInt8		byImageMode;				/*图像模式		0 模式 1，  1 模式 2 */

	UInt16      u16ViWidth;        		 	/*视频输入 宽度*/
	UInt16		u16ViHeight;       			/*视频输入 高度*/
	UInt8		byCtbLevel;					/*彩转黑阀值，自动彩转黑时有效*/
	UInt8		byMinFocusLen;				/*设置最小聚焦距离*/			  
	UInt8		byIRLevel;					/*夜间启动点 */		          
	UInt8 		byLensCorrection; 			/*镜头校正	0: 关	1：开  */ 
	UInt8       bySmartNR;					//智能降噪  0 ~ 255 0: 关 
	UInt8       bySmartNRDefault;			//智能降噪  默认值 
	UInt8 		by3DNR; 					/*3D 降噪   0: 关	1：开  */  
	UInt8       by3DNRLevel;				//3D降噪值 
	UInt8		byRes[48];					//
}VIDEO_IN_SCENE_S,*LPVIDEO_IN_SCENE_S;

//6，视频输入的所有色彩调节和场景设置
//大小196(sizeof(VIDEO_IN_CFG_S))
//CMS_NET:	CMD_GET_VI_CFG		CMD_SET_VI_CFG  

typedef struct _VIDEO_IN_CFG_S
{
	UInt32				dwSize;		    /*结构大小*/
	VIDEO_IN_SENSOR_S	struViSensor;	/*视频输入设置*/
	VIDEO_IN_SCENE_S	struViScene;	/*视频输入场景相关设置*/
}VIDEO_IN_CFG_S,*LPVIDEO_IN_CFG_S;


//7，视频信号参数
//CMS_NET:	CMD_GET_VIDEO_SIGNAL_CONFIG 	 CMD_SET_VIDEO_SIGNAL_CONFIG
typedef struct __VIDEO_SIGNAL_CONFIG
{
	UCHAR				ChannelNo;						//通道号	0 ~ PORTNUM - 1
	UCHAR 				ChannelNum;						//通道数<NVS总的通道数，因为单路、四路的参数默认值是不同的>
	UCHAR 				Brightness;						//亮度
	UCHAR 				Hue;							//色调
	UCHAR 				Saturation;						//饱和度
	UCHAR 				Contrast;						//对比度
}VIDEO_SIGNAL_CONFIG;


//-------------------------------end------------------------------------


//-----------------------------IV、音频属性--------------------------
//1，音频设置
//CMS_NET   CMD_ GET_AUDIO_CONFIG  CMD_SET_AUDIO_CONFIG
typedef struct _AUDIO_CONFIG
{
	UCHAR		Open:1;									//音频是否打开
	UCHAR		MicOrLine:1;							//音频输入模式：麦克或线输入
	UCHAR		resver:6;
	UCHAR		Type;
	UCHAR		AD[2];
				//压缩格式
	UINT		Bitrate ;								//码率
	UINT		SampleBitrate ;
}PACKED AUDIO_CONFIG;
//-------------------------------end------------------------------------

//-----------------------------V、网络设置---------------------------
//1，网络设置结构(基本参数、有线网络、pppoe参数、 DDNS参数、中心连接)
//CMS_NET: CMD_GET_NET_CONFIG     CMD_SET_NET_CONFIG

typedef struct _NET_CONFIG
{	
	UINT             	IPAddr ;						//IP地址
	UINT             	SubNetMask ;					//掩码
	UINT             	GateWay ;						//网关

   	USHORT             	ComPortNo;						//设置接收客户端命令端口号        UDP	
   	USHORT             	WebPortNo;						//Webserver端口     			  TCP
	UINT             	MultiCastIPAddr ;				//多播IP地址
   	USHORT             	MultiCastPortNo;				//UDP传输起始端口<多播传输>
	
	UCHAR			   	RT8139MAC[6];      				//人工设置网卡的MAC地址		5---0有效
   	UCHAR             	DHCP;							//DHCP 开关					0	关		1:开
   	
	UCHAR             	PppoeOpen;						//PPPOE 开关     	
	CHAR				PppoeName[PPPPOE_NAME_LEN+1];	//拨号用户名
	CHAR				PppoePass[PPPPOE_PASSWD_LEN+1]; //拨号密码	
	UINT             	PppoeTimes;   					//在线时间
	UINT             	PppoeIPAddr ;					//PPPOEIP地址		报警回传IP	
	
	UINT				DdnsOpen;						//DDNS开关
	CHAR				DdnsName[DDNS_NAME_LEN+1];		//注册主机名称
	CHAR				DdnsPass[DDNS_PASS_LEN+1];		//注册主机密码	
	
	CHAR				DdnsIP[DDNS_SERVER_NAME_LEN+1];	//DDNS服务器
	CHAR				AAD[1];
	USHORT				DdnsPortNo;						//DDNS服务器端口
	USHORT				DdnsMapWebPort;					//本地WEB映射端口
	USHORT				DdnsMapDataPort;				//本地数据映射端口
	CHAR				ABD[2];

	UINT				DNSHostIP;						//DNS的IP

	INT					ConnectCenter;					//是否主动连接中心
	CHAR				ConnectCenterIP[DDNS_SERVER_NAME_LEN+1];//中心IP
	CHAR				ACD[1];
	USHORT				ConnectCenterPort;						//中心端口

	USHORT				appFunction;					//实现功能定义,以bit位表示:0-PPPOE,1-NTP,2-UPNP,3-WF,4-MAIL,5-定向发送
	UCHAR				tcpSendInterval;
	UCHAR				udpSendInterval;
	USHORT				PacketMTU;

	UCHAR				CaptureMode;					//抓拍模式
	UCHAR				CapturePort[MAX_SENSOR_NUM];	//抓拍通道(0-3bit: 分别表示1 ~ 4号通道  1:开  0: 关)
	CHAR				AED[1];
	union
	{
	  UINT       		DdnsIP2;						
 	  UINT      		DNSHostIP2;						
	};						//DDNS 2的IP,非网络字节次序
	USHORT				DdnsPortNo2;					//DDNS 2的端口

	CHAR				sysByname[DVS_NAME_LEN+1];		//域名
	UCHAR				domainSetState;					//0－成功；1－名字存在，修改新域名；2－正在申请；3－失败
}PACKED NET_CONFIG;

//2、无线 WIFI 参数
//CMS_NET:  CMD_GET_WFNET    CMD_SET_WFNET
typedef struct _WF_NET_CONFIG
{
	UINT			dwSize;							//大小
	USHORT			wfOpen;							//是否打开无线
	USHORT			dhcpOpen;						//DHCP开关
	//
	UINT			ipAddress;						//IP地址
	UINT			subMask;						//子网掩码
	UINT			gateWay;						//网关
	UINT			dnsIP1;							//DNS服务器1 IP
	UINT			dnsIP2;							//DNS服务器2 IP
	UCHAR			macAddress[6];
	//
	UCHAR			ssidLen;						//SSID名称长度
	UCHAR			pswLen;							//密码长度

	CHAR			ssidName[34];					//SSID名称
	CHAR			pswText[34];					//密码
	UINT			channel;						//通道
	UINT			bps;
	UINT			type;
	UCHAR			wfAuthType;						//主认证加密类型
	UCHAR			wfEncType;						//辅助认证加密
	UCHAR			wfPswType;
	CHAR			reserve[9];
}WF_NET_CONFIG;

//3、UPNP  参数
//CMS_NET:	CMD_GET_UPNP   CMD_SET_UPNP

typedef struct _UPNP_CONFIG
{
	USHORT			upnpOpen;						//UPNP开关
	UCHAR			upnpEthNo;						//UPNP网卡 0－有线，1－无线
	UCHAR			upnpMode;						//UPNP模式 0－指定，1－自动
	USHORT			upnpPortWeb;					//UPNP WEB端口
	USHORT			upnpPortData;					//UPNP DATA端口
	UINT			upnpHost;						//UPNP主机
	USHORT			upnpStatusWeb;					//UPNP WEB状态
	USHORT			upnpStatusData;				//UPNP DATA状态
}UPNP_CONFIG;

//4、EMAIL参数
//CMS_NET :  CMD_GET_MAIL     CMD_SET_MAIL
typedef struct _MAIL_CONFIG
{
	CHAR			smtpServer[DOMAIN_NAME_LEN];	//发送服务器
	CHAR			pop3Server[DOMAIN_NAME_LEN];	//接收服务器
	CHAR			fromMailAddr[DOMAIN_NAME_LEN];	//邮件发送地址
	CHAR			toMailAddr[DOMAIN_NAME_LEN];	//邮件接收地址
	CHAR			ccMailAddr[DOMAIN_NAME_LEN];	//邮件抄送地址
	CHAR			smtpUser[DOMAIN_NAME_LEN];		//SMTP 用户名
	CHAR			smtpPassword[DOMAIN_NAME_LEN];  //SMTP 用户密码
	CHAR			pop3User[DOMAIN_NAME_LEN];		//POP3 用户名
	CHAR			pop3Password[DOMAIN_NAME_LEN];	//POP3 用户密码
		
	UINT			motionMailOpen;					//视频移动触发MAIL开关
	UINT			motionMail[MAX_VIDEO_NUM];		//视频移动触发抓拍发送MAIL开关
	UINT			sensorMailOpen;					//探头报警触发MAIL开关
	UINT			sensorMail[MAX_SENSOR_NUM];		//探头报警触发抓拍发送MAIL开关
	UINT			viloseMailOpen;		

	UINT			timeMailOpen;					//定时发送(/分)
	UINT			timeVideoMail[MAX_VIDEO_NUM];	//视频定时抓拍发送MAIL开关
	
	CHAR			subjectMail[EMAIL_TITLE_LEN];	//邮件头
	CHAR			textMail[EMAIL_TITLE_LEN];		//邮件内容
		
	UINT			sendMailCount;					//状态: 发送的email记数	
	UINT			sendNow;						//立即发送
	USHORT			smtpPort;						//SMTP 端口
	USHORT			popPort;						//POP3 端口
	USHORT			protocolType;					//SSL
	CHAR			res[6];
}MAIL_CONFIG;


//5， FTP 服务器参数
//CMS_NET: CMD_GET_FTP_CONFIG  CMD_SET_FTP_CONFIG

typedef struct __FTP_CONFIG
{
    char			ftpURL[64];
    char			ftpPath[64];
    UINT			ftpPort;
    char			ftpUser[32];
    char			ftpPsw[32];

    char			ftpURL2[64];
    char			ftpPath2[64];
    UINT			ftpPort2;
    char			ftpUser2[32];
    char			ftpPsw2[32];
	
	USHORT			StartPort;				
	USHORT			EndPort;	
	UCHAR           retransimission;
    char			res[27];
}FTP_CONFIG;


//6，DDNS设置
//CMS_NET: CMD_GET_DDNS  CMD_SET_DDNS
typedef struct _DDNS_SET
{
	INT					bDdnsStart;
	CHAR				szDdnsIP[DDNS_SERVER_NAME_LEN+1];
	UINT				dwDdnsPort;
	CHAR				szDdnsName[DVS_NAME_LEN+1];
	UINT				dwMapWebPort;
	UINT				dwMapDataPort;
}PACKED4 DDNS_SET;

//流媒体服务  rtsp
typedef struct _MEDIA_SERVER
{
	UCHAR				bRtspOpen; 		//RTSP  服务开关
	UCHAR				bMmsOpen;		//no
	USHORT				rtspPort;		//被动RTSP  服务端口
	UCHAR				bRtspAuth;		//是否认证
	UCHAR				bRtspPassive;	//0: 主动连服务器  1: 被动等待客户端连接
	USHORT				maxPacketSize;	//RTSP MTU
	UINT				rtspActiveIp;	//主动连接时RTSP的IP地址
	USHORT				rtspActiveport;//主动连接时RTSP的端口
	USHORT				mmsPort;       //no
}MEDIA_SERVER;


//GPS 参数
//CMS_NET: CMD_GET_GPS_CONFIG    CMD_SET_GPS_CONFIG
typedef struct _GPS_INFO
{
	UCHAR		bGpsOpen;
	UCHAR		gpsStatus;
	USHORT		serverPort;
	UINT		serverIP;
	UCHAR		bGpsOSD;
	UCHAR		gpsPlatform;
	UCHAR		phoneNum[11];
	UCHAR       	reportTime;	//ljm 2009-09-25 
	UCHAR		res[2];
	//BYTE		res[3];
}GPS_INFO;


//3G参数的数据结构
typedef struct _3G_NET_CONFIG_S
{
	unsigned char       nSize;
	unsigned char       n3gOpen;			//3g 开关
	unsigned char       n3gType;			//3g 类型 0 evdo 1 wcdma 2 td
	unsigned char       n3gSecType;			//3g 验证类型  0 自动选择 1 pap 2 chap
	unsigned char       n3gParam;		    //标记要用哪些参数 
	char        		sz3gUsr[32];		//3g 拨号用户名			
	char        		sz3gPsw[32];		//3g 拨号密码
	char        		sz3gApn[32];		//3g apn 
	char        		sz3gTelNum[16];    //3g 拨号号码
	char        		sz3gLocalIp[16];
	char        		sz3gRemoteIp[16];
	char        		sz3gPara1[16];
	char        		sz3gPara2[16];
	unsigned char       res[16]; 
}PACKED T3G_NET_CONFIG_S;//200字节


typedef struct _SS_3G_INFO_S
{
	unsigned char 	  moduleExits;   		// 1 module exits else 0
	unsigned char     moduleStatus;  		// 1 ok else 0
	unsigned char     simExits;      		// 1 umi exits  else 0
	unsigned char     sigal;         		// 
	unsigned char     serviceStatus; 		// srv status 
	// 0 no srv status
	// 1 a restrict srv status
	// 2 a good srv
	// 3 a restrict district srv
	// 4 sleep status
	unsigned char     roamStatus;    		// 0 not roam 1 roam
	unsigned char     networkType ;  	    // 0 not service
	// 1 AMPS
	// 2 CDMA
	// 3 GSM/GPRS
	// 4 HDR
	// 5 WCDMA
	// 6 GPS
	// 7 GSM/WCDMA
	// 8 CDMA/HDR HYBIRD
	unsigned char     refresh      ;
	unsigned char     initModule   ;
	unsigned char     initDail	   ;
	unsigned char	  c3gstatus	   ;		// 3G状态
	unsigned char     res          ;		// 拨号失败次数,可以不使用
	char      		  softWareVer[128];   	// software version
	char      		  hardWareVer[128];  	// hardware version
	char      		  moduleType[32] ;   	// module type
}PACKED SS_3G_INFO_S, *PSS_3G_INFO_S;

//3G  时间
typedef struct _T3G_TIME_CTRL_S// 20101125 结构调整,实现字节对齐
{
	BYTE		t3gtimectrlopen;		// 3G拨号限时是否开启 0-关闭  1-开启
	BYTE		t3gdaylimitedend;		// 标志位-每天限时拨号     0-未到  1-已到
	BYTE		t3gmonthlimitedend;		// 标志位-每月限时拨号   0-未到  1-已到
	BYTE		t3gdaylimited;			// 限制每天拨号时长   0-不限制
	T3G_TIME_CTRL_MODE	t3gtimectrlmode;// 3G时间限制模式
	DWORD		t3gdialusedtime;		// 已经拨号时间
	DWORD		t3gmonthlimited;		// 限制每月拨号时长   0-不限制
	BYTE		t3openlimitedmonth;		// 初次设定限时的月份
	BYTE		t3openlimitedday;// 初次设定限时的号数
	BYTE		revers[2];// 字节对齐
	TIME_SEGMENT_NEW	t3gtimesegment[4];// 定义每天4个可设定时间段
}PACKED T3G_TIME_CTRL_S;// 50个字节 = 1+4+4+1+1+1+4+1+1+4*(4+4)


//7、3G 总参数
//CMS_NET  CMD_GET_3G_CONFIG   CMD_SET_3G_CONFIG
typedef struct _T3Ginfo_S
{
	T3G_NET_CONFIG_S  t_3ginfo;
	T3G_TIME_CTRL_S   t_3gTime;
	SS_3G_INFO_S	  t_3ginfo1;
}T3Ginfo_S;

//---------------------------end-----------------------------------------



//----------------------------------VI、存储设置-------------------------------

//1，磁盘信息
//1、CMS_NET:  CMD_GET_DISK_INFO   
typedef struct _DISK_INFO
{
	UINT	DiskNum;						//磁盘个数
	UCHAR	DiskType[8];					//磁盘类型
	UINT	TotalSize[8];					//磁盘总空间大小
	UINT	FreeSize[8];					//磁盘剩余空间
	UCHAR	Reserved[8];					//BYTE	Reserved[4];	2008.9.23
}DISK_INFO;

//2，磁盘操作
typedef struct _DISK_MANAGE
{
	UCHAR	Cmd;
	UCHAR	Data;
	UCHAR	Reserved[126];
}DISK_MANAGE;

//3，磁盘格式化进度
typedef struct _DISK_FORMAT_PROCESS
{
	UINT	DiskType;							//磁盘类型
	UINT	Status;								//格式化状态
	UINT	Process;							//进度()
	UINT	Reserved[5];
}DISK_FORMAT_PROCESS;

//定时录像
typedef struct _CH_TIMER_RECORD_CONFIG		//sizeof() = 140
{
	USHORT					KeepTime;			//保留天数(天)
	USHORT					RecordSvaeMode;	//bit0 本地存储  bit1 ftp上传  bit2 email
	UINT					reserve[2];

	TIME_SEGMENT_NEW		tSchTable1[8] ;		//某个通道的定时录像时间段1 (64)
	TIME_SEGMENT_NEW		tSchTable2[8] ;		//某个通道的定时录像时间段2 (64)
}CH_TIMER_RECORD_CONFIG;	


//4、录像计划
//CMS_NET : CMD_SET_RECORD_CONFIG				CMD_GET_RECORD_CONFIG	(unsupport)
typedef struct _TIMER_RECORD_CONFIG			//sizeof() = 592
{
	UCHAR					PackageTime;	  						//打包时间(分钟)
	UCHAR 					OverWrite;        						//自动覆盖
	UCHAR     				MainVEnc;								//录像采用 0: 从码流  1: 主码流
	UCHAR     				CheckDisk;								//系统启动时进行磁盘扫描
	USHORT					nSaveDays;	
	UCHAR     				reserve[24];

	CH_TIMER_RECORD_CONFIG	ChRecordCfg[MAX_VIDEO_NUM];				//通道的录像参数(140 * 4 = 560)
}TIMER_RECORD_CONFIG;


//定时抓拍
typedef struct _CH_TIMER_SHOOT_CONFIG		//sizeof() = 140
{
	USHORT 					KeepTime;								//保留天数(天)
	USHORT					ShootSaveMode;							//bit0 本地存储  bit1 ftp上传  bit2 email

	UINT					ShootNum;								//连续抓拍张数()
	float					ShootInterval;							//连续抓拍时间间隔(秒)  0: 连续抓拍

	TIME_SEGMENT_NEW		tSchTable1[8] ;							//某个通道的定时抓拍时间段1 (40)
	TIME_SEGMENT_NEW		tSchTable2[8] ;							//某个通道的定时抓拍时间段2 (40)
}CH_TIMER_SHOOT_CONFIG;

//5、抓拍计划
//CMS_NET: CMD_GET_SHOOT_CONFIG   CMD_SET_SHOOT_CONFIG
typedef struct _TIMER_SHOOT_CONFIG			//sizeof() = 592
{
	UCHAR 					OverWrite;								//自动覆盖
	UCHAR     				reserve[31];

	CH_TIMER_SHOOT_CONFIG	ChShootCfg[MAX_VIDEO_NUM];				//通道的录像参数(92 * 4 = 368)
}TIMER_SHOOT_CONFIG;

//------------------------------------end------------------------------------------


//----------------------------------VII、报警设置-----------------------------
//报警联动操作
typedef struct _ALARM_LINKAGE 
{
	UINT 		Output;						//报警联动输出				     bit.0---bit.3	
	UINT 		AutoClearTime;		        //报警自动清除时间	

	UINT 		Record_PC;					//PC端报警联动录像通道			 bit.0---bit.3
	UINT 		Record_Dev;					//设备端报警联动录像通道		 bit.0---bit.3
	UINT		RecordSvaeMode;				//bit0 本地存储  bit1 ftp上传  bit2 email
	UINT		RecordTime;					//报警录像时间(秒)

	UINT		ShootSaveMode;				//bit0 本地存储  bit1 ftp上传  bit2 email
	UINT		ShootChns;					//抓拍通道(bit.0 ~ bit.3)
	UINT		ShootNum;					//连续抓拍张数()
	float		ShootInterval;				//连续抓拍时间间隔(秒)  0: 连续抓拍

	UINT		EMail;						//发送email
	UINT		Reserved;					//备用
} ALARM_LINKAGE;

//1，移动侦测设置
//CMS_NET: CMD_GET_VMOTION_CONFIG_NEW  CMD_SET_VMOTION_CONFIG_NEW
typedef struct _VIDEOMOTION_CONFIG_NEW
{
	UINT			dwSize;						//本结构长度

	UINT 			DetectLevel;			    //移动检测灵敏度
	UINT 			DetectOpen;				    //移动检测开关

	AREA_RECT		DetectArea[9];				//移动检测区域			

	TIME_SEGMENT_NEW  tSchTable1[8] ;		    //移动检测时间表1				 0: 每天
	TIME_SEGMENT_NEW  tSchTable2[8] ;		    //移动检测时间表1				 0: 每天

	ALARM_LINKAGE	stAlarmLink;				//报警联动

	UCHAR			reseved[128];
}VIDEOMOTION_CONFIG_NEW ;


//2，探头报警侦测设置
//CMS_NET: CMD_GET_SENSOR_ALARM_NEW 	CMD_SET_SENSOR_ALARM_NEW

typedef struct _SENSOR_CONFIG_NEW 
{
	UINT				dwSize;						//本结构长度

	UINT				SensorType;					//0:常开探头	1:常闭探头
	UINT 				DetectOpen;				    //检测开关

	TIME_SEGMENT_NEW  	tSchTable1[8] ;		    	//探头检测时间1				     0: 每天
	TIME_SEGMENT_NEW 	tSchTable2[8] ;		    	//探头检测时间1				     0: 每天

	ALARM_LINKAGE		stAlarmLink;				//报警联动

	UCHAR				reseved[32];
}SENSOR_CONFIG_NEW;


//3，网络断线报警设置
//CMS_NET: CMD_GET_NET_ALARM_CONFIG  CMD_SET_NET_ALARM_CONFIG

typedef struct _NET_ALARM_CONFIG 
{
	UINT			dwSize;						//本结构长度

	UINT 			DetectOpen;				    //检测开关
	
	ALARM_LINKAGE	stAlarmLink;				//报警联动

	UCHAR			reseved[32];
} NET_ALARM_CONFIG;


//4，视频丢失设置
//CMS_NET:	CMD_GET_VLOST_CONFIG_NEW  CMD_SET_VLOST_CONFIG_NEW
typedef struct _VIDEOLOSE_CONFIG_NEW
{
	UINT			dwSize;						//本结构长度

	UINT			DetectOpen;					//视频丢失检测

	ALARM_LINKAGE	stAlarmLink;				//报警联动

	UCHAR			reseved[32];
} VIDEOLOSE_CONFIG_NEW;



//无线探头报警 (不支持)
typedef struct _RF_ALARM_CONFIG
{
	UINT			dwSize;						//本结构长度

	UINT			SensorType;					//0:常开探头	1:常闭探头
	UINT 			DetectOpen;				    //检测开关

	TIME_SEGMENT_NEW  tSchTable1[8] ;		    //探头检测时间1				     0: 每天
	TIME_SEGMENT_NEW  tSchTable2[8] ;		    //探头检测时间1				     0: 每天

	ALARM_LINKAGE	stAlarmLink;				//报警联动

	UCHAR			reseved[32];
}RF_ALARM_CONFIG;


//5，输出继电器控制
//CMS_NET:	CMD_SET_OUTPUT_CTRL
typedef struct __OUTPUT_CTRL
{
	UCHAR				ChannelNo;						//继电器号	0 ~ MAX_OUTPUT_NUM - 1
	UCHAR				Status;							//0：关		1: 开	
}OUTPUT_CTRL;

//
typedef struct _ALARM_INOUT_NAME
{
	CHAR			alarmInName[MAX_SENSOR_NUM][CAM_TITLE_LEN+2];	//探头名
	CHAR			alarmOutName[MAX_OUTPUT_NUM][CAM_TITLE_LEN+2];	//报警输出名
}ALARM_INOUT_NAME;



typedef  struct  _ALARM_INFO
{
	UCHAR	SensorAlarm[MAX_SENSOR_NUM];				//探头报警
	UCHAR	SensorAlarmOut[MAX_SENSOR_NUM];				//探头报警输出
	UCHAR	MotionAlarm[MAX_VIDEO_NUM];					//视频移动
	UCHAR	MotionAlarmOut[MAX_VIDEO_NUM];				//视频移动报警输出
	UCHAR	ViLoseAlarm[MAX_VIDEO_NUM];					//视频丢失
	UCHAR	ViLoseAlarmOut[MAX_VIDEO_NUM];				//视频丢失报警输出
} ALARM_INFO;








//----------------------------------end--------------------------------------------



//---------------------------------VIII、串口设置--------------------------------

//1，串口设置
//CMS_NET:  CMD_GET_COM_CONFIG    CMD_SET_COM_CONFIG
typedef struct __COM_CONFIG
{
	UINT 				Baudrate;						//串口波特率				300---115200
	UCHAR				Databit;						//串口数据位长度			5=5位 6=6位 7=7位 8=8位	其他=8位
	UCHAR				Stopbit;						//停止位位数				1=1位 2=2位 其他=1位
	UCHAR				CheckType;						//校验						0=无  1=奇  2=偶  3=恒1  4=恒0 
	UCHAR				Flowctrl; 						//软流控/硬流控/无流控		备用
}COM_CONFIG;


//----------------------------------云台-------------------------------------
typedef struct _SERIAL_PROTOCOL_S
{
	ULONG			baudrate : 20;						//波特率(bps) 300 ~ 115200
	ULONG			dataBits : 4;						//数据有几位  5: 5位 6: 6位 7: 7位 8: 8位
	ULONG			stopBits : 2;						//停止位:     1: 1位 2: 2位
	ULONG			parity   : 3;						//校验        0: 无  1: 奇  2:偶   3: 恒1  4:恒0 
	ULONG			flowControl : 2;					//流控        0: 无  1: 软流控     2:硬流控
}SERIAL_PROTOCOL_S;


typedef struct _DEV_SERIAL_CHN_S
{
	UINT				dwSize;
	UINT				bExist;							//是否存在: 只读
	CHAR				szName[64];	
	SERIAL_PROTOCOL_S	stSerialProtocol;
	UCHAR				byRes[16];
}DEV_SERIAL_CHN_S;


//1，透明通讯
//CMS_NET : CMD_SET_COMM_SENDDATA
typedef struct __COMM_CTRL
{
	UCHAR				COMMNo;							//串口号	0 ~ 1
	UCHAR				AD[3];
	
	DEV_SERIAL_CHN_S	COMConfig;						//串口配置
	USHORT				DataLength;						//数据长度
	UCHAR				Data[256];						//数据
	UCHAR				AE[2];
}PACKED COMM_CTRL;

//2，云台控制
//CMS_NET: CMD_SET_PAN_CTRL
typedef struct __PAN_CTRL
{
	UCHAR				ChannelNo;						//通道号	0 ~ PORTNUM - 1
	UCHAR				COMMNo;							//串口号	0 ~ 1
	UCHAR				PanType;						//云台协议序号 0 ~ YUNTAI_PROTOCOL_NUM - 1
	UCHAR				PanCmd;							//云台控制命令
	UCHAR				Data1;							//数据1		（预置、调用号）
	UCHAR				Data2;							//数据2		（备用）
}PAN_CTRL;

//3、云台信息
//CMS_NET:  CMD_GET_YUNTAI_CONFIG
typedef struct _YUNTAI_INFO
{
	CHAR 		Name[YUNTAI_NAME_LEN + 1];				//云台名称
	CHAR 		AD[3];
	COM_CONFIG	ComSet;									//通讯协议<波特率...>
}PACKED YUNTAI_INFO;

//------------------------------end---------------------------------


//----------------------------IX、其它参数-----------------------

//1，TW2824参数
//CMS_NET: CMD_SET_TW2824
typedef struct _TW2824_CTRL
{
	UCHAR				Page;							//页
	UCHAR 				RegNo;							//寄存器
	UCHAR 				Data;							//数据
	UCHAR				AD[1];
	UINT				nReserved[8];					//保留
}TW2824_CTRL;

typedef struct _COMM_DATA_NOTIFY
{
	HANDLE					hLogonServer;
	UINT					dwClientID;
	UINT					dwServerIP;
	UINT					dwServerPort;
	COMM_CTRL				commCtrl;
}COMM_DATA_NOTIFY;


//2，调试信息开关
typedef struct _DEBUG_CONFIG
{
	UCHAR				Status;							//0：关		1: 开
}DEBUG_CONFIG;


//3，日志查询
//CMD_FING_LOG
typedef struct _FIND_LOG_S
{
	UINT		nSize;					//结构大小
	UINT		nSelectMode;			//查询方式:0－全部，1－按类型，2－按时间，3－按时间和类型
	UINT		nType;					//报警类型
    DVS_TIME	stStartTime;
	DVS_TIME	stEndTime;
}FIND_LOG_S;

typedef struct _LOG_ITEM_S
{
	UINT		nSize;					//结构大小
	UINT		nAlarmType;				//报警类型
	UINT        nChn;					//通道号、断线报警的IP地址
	DVS_TIME	tAlarmTime;	
	char		szNote[12];
}LOG_ITEM_S;



//4，登录的用户用户信息

typedef struct _USER_DETAIL
{
	INT					bLogon;							//TRUE-登陆用户,FALSE-预览用户
	UINT 				Level;							//权限
	CHAR 				Name[USER_NAME_LEN+1];			//用户名
	CHAR 				Password[USER_PASSWD_LEN+1];	//用户密码
	NET_PROTOCOL_TYPE	protocolType;					//协议
	UINT				clientIP;						//IP
	UINT				clientPort;						//端口
}PACKED4 USER_DETAIL;

typedef struct _USER_INFO
{
	UINT				dwLogonNum;
	UINT				dwPreviewNum;
	USER_DETAIL			userDetail[MAX_LOGON_NUM+MAX_LINKVIEW_NUM+MAX_MULTIVIEW_NUM];
}PACKED4 USER_INFO;


//视频定向发送设置
typedef struct _SEND_DEST
{	
	UINT			dwSet[MAX_VIDEO_NUM];			//发送开关
	UINT			dwStreamSend[MAX_VIDEO_NUM];	//发送开关
	UINT			dwStreamDestIP[MAX_VIDEO_NUM];	//目的IP,网络字节顺序
	UINT			dwStreamDestPort[MAX_VIDEO_NUM];//目的端口
	CHAR			res[16];
}SEND_DEST;
//自动重新注册
typedef struct _AUTO_RESET_TIME
{
	UCHAR			open;				//bit 1:hour,bit 2:day,bit 3:week	
	UCHAR			second;
	UCHAR			minute;
	UCHAR			hour;
	UCHAR			week;				//0 星期日，1 星期一，2 星期二 

	UCHAR			res[3];
}AUTO_RESET_TIME;


//5, 扩展信息结构(修改定义， 大小和原结构相同， 所以命令也相同)
//CMS_NET: CMD_GET_EXTINFO   CMD_SET_EXTINFO

typedef struct _EXTINFO_CONFIG_NEW
{
	UINT				nSize;					//结构大小(1328)

	NTP_CONFIG			ntpConfig;				//ntp
	
	UPNP_CONFIG			upnpConfig;				//upnp
		
	MAIL_CONFIG			mailConfig;				//MAIL
	
	ALARM_INOUT_NAME	alarmInOutName;		//ALARM IN/OUT NAME
	
	WF_NET_CONFIG		wfConfig;				//WF NETWORK
	
	SEND_DEST			sendDest;				//SEND DEST
	
	AUTO_RESET_TIME		autoResetTime;			//AUTO RESET 

	FTP_CONFIG			ftpCfg;					//FTP 服务器配置

	MEDIA_SERVER		mediaServer;			//流媒体服务

	GPS_INFO			gpsInfo;				//GPS 信息,

	UCHAR               reportWanIp;            //用于上报公网IP地址
	UCHAR               reportIpInterval;       //
	char				reserve[6];			

	//char				reserve[16];  
	//char				reserve[40];  
}EXTINFO_CONFIG_NEW;


//用户数据
typedef struct __USERDATA_CONFIG
{
	int		nDataLen;
	char	userData[252];
}USERDATA_CONFIG;


//6, 设备所有参数（sizeof = 10248）
//CMS_NET: CMD_GET_ALL_CONFIGPARAM

typedef struct _DVS_Parameter_ST
{
	UINT						dwSize;						//本结构长度

    char                		sysName[DVS_NAME_LEN + 4];

	UCHAR						sysInputNum;		        //路数
	UCHAR						sysType;			        //机器型号	0:CIF  1:HD1  2:D1  
	UCHAR						sysVideoFormat;		        //编码格式	0:D1,1:HD1,2:CIF,3:VGA 4:HVGA 5:CVGA 6:QCIF 7:QVGA
    UCHAR						sysLanguage;		        //语言

    UINT						sysStandard;		        //制式		0:PAL  1:NTSC

    UINT      					sysID;  	            //每台机器有个ID
	UINT      					sysVer;  	            //软件版本  

	VIDEO_CONFIG_NEW	    	sysVideo[MAX_VIDEO_NUM];	
	VIDEOMOTION_CONFIG_NEW		sysVideoMotion[MAX_VIDEO_NUM];
	VIDEOMASK_CONFIG_NEW    	sysVideoMask[MAX_VIDEO_NUM];	
	VIDEOLOSE_CONFIG_NEW    	sysVideoLose[MAX_VIDEO_NUM];	
	AUDIO_CONFIG		    	sysAudio[MAX_AUDIO_NUM];
	NET_CONFIG					sysNet;
	COM_CONFIG					sysCom[MAX_COM_NUM];
	SENSOR_CONFIG_NEW	   	 	sysSensor[MAX_SENSOR_NUM];
	USER_CONFIG					sysUser[MAX_USER_NUM];

    EXTINFO_CONFIG_NEW			sysExtConfig;				//扩展参数

	DISK_INFO					DiskInfo;					//存储设备信息

	TIMER_RECORD_CONFIG			TimerRecordCfg;
	TIMER_SHOOT_CONFIG			TimerShootCfg;				//定时抓拍参数

	RF_ALARM_CONFIG				rfAlarmCfg[MAX_RF_SENSOR_NUM];	//无线探头报警
	NET_ALARM_CONFIG			netAlarmCfg;					//网络断线报警

	USERDATA_CONFIG				userData;					//用户数据

	UINT						YunTaiProtocolNum;
	YUNTAI_INFO 				YunTaiInfo[MAX_PTZ_PROTOCOL_NUM];	

	UCHAR						reserved[256];				//备用

} DVS_Parameter_New;


/*
typedef struct _DVS_Parameter_New
{
	UINT					dwSize;						//本结构长度
    char                    sysName[DVS_NAME_LEN + 4];
	UCHAR					sysInputNum;		        //路数
	UCHAR					sysType;			        //机器型号	0:CIF  1:HD1  2:D1  
	UCHAR					sysVideoFormat;		        //编码格式	0:D1,1:HD1,2:CIF,3:VGA 4:HVGA 5:CVGA 6:QCIF 7:QVGA
    UCHAR					sysLanguage;		        //语言
    UINT					sysStandard;		        //制式		0:PAL  1:NTSC
    UINT      				sysID;  	                //每台机器有个ID
	UINT      				sysVer;  	                //软件版本  s
	VIDEO_CONFIG_NEW	    sysVideo[MAX_VIDEO_NUM];	
	VIDEOMOTION_CONFIG_NEW	sysVideoMotion[MAX_VIDEO_NUM];
	VIDEOMASK_CONFIG_NEW    sysVideoMask[MAX_VIDEO_NUM];	
	VIDEOLOSE_CONFIG_NEW    sysVideoLose[MAX_VIDEO_NUM];	
	AUDIO_CONFIG		    sysAudio[MAX_AUDIO_NUM];
	NET_CONFIG			    sysNet;
	COM_CONFIG			    sysCom[MAX_COM_NUM];
	SENSOR_CONFIG_NEW	    sysSensor[MAX_SENSOR_NUM];
	USER_CONFIG			    sysUser[MAX_USER_NUM];

    EXTINFO_CONFIG_NEW		sysExtConfig;				//扩展参数
	DISK_INFO				DiskInfo;					//存储设备信息
	TIMER_RECORD_CONFIG		TimerRecordCfg;
	TIMER_SHOOT_CONFIG		TimerShootCfg;				//定时抓拍参数
	RF_ALARM_CONFIG			rfAlarmCfg[MAX_RF_SENSOR_NUM];	//无线探头报警
	NET_ALARM_CONFIG		netAlarmCfg;					//网络断线报警
	USERDATA_CONFIG			userData;					//用户数据
	UINT					YunTaiProtocolNum;
	YUNTAI_INFO 			YunTaiInfo[MAX_PTZ_PROTOCOL_NUM];	
	UCHAR					reserved[256];				//备用

} DVS_Parameter_New;

*/
//=======================================
//其它扩展参数(如平台设置其它参数)
//=======================================
typedef struct __EXT_DATA_CONFIG
{
	UINT		nSize;					//结构大小
	UINT		nType;	
       UINT		nDataLen;
    char		szData[512];
}EXT_DATA_CONFIG;



//备用1
typedef struct _CONFIG_ENCPAR
{ 
    UINT u32PicWidth;   
    UINT u32PicHeight;     
    UINT u32CapSel; 
    UINT u32StaticParam; 
    UINT u32TargetBitrate; 
    UINT u32TargetFramerate; 
    UINT u32Gop;             
    UINT u32MaxQP;        
    UINT u32PreferFrameRate; 
    UINT u32Strategy; 
    UINT u32VencMode;        
    UINT u32BaseQP;         
    UINT u32MaxBitRate;   
}CONFIG_ENCPAR; 

//备用2
typedef struct _PING_CONFIG
{
	BOOL	bPingOpen;
	BOOL	bDisconnectReboot;
	BOOL	bDisconnectAlarm;
	UINT	dwPingNum;
	UINT   dwPingPeriod;
	char	szPingServer[64];
}PING_CONFIG;

//---------------------------end------------------------------------




/**************************** ********************************************
						 四、解码器参数设置结构
**************************************************************************/

//---------------------------------------------------------//
//1，系统时间
typedef  struct  _DD_NVD_DATE
{
	UCHAR		year;									//年
	UCHAR		month;									//月
	UCHAR		day;									//日
	UCHAR		week;									//周
	UCHAR		hour;									//小时
	UCHAR		minute;									//分
	UCHAR		second;									//秒
}DD_NVD_DATE, *PDD_NVD_DATE;

//2，系统配置
typedef struct _DD_SYSTEM_PARAM 
{
   	CHAR               	Name[DVS_NAME_LEN + 1];			//名字
	UCHAR				Type;							//机器型号	0:CIF  1:HD1  2:D1  
    UCHAR				Language;						//语言
    UCHAR				Standard;						//制式		0:PAL  1:NTSC
	UINT      			ID;  	        				//每台机器有个ID号  	
	UINT      			RS485Addr;       				//RS485地址  		
	UINT      			RS485Func;       				//RS485功能  			
	UINT      			RS485Baud;       				//RS485波特率  			
	UINT      			Version;         				//软件版本  
	UCHAR      			RemoteID;         				//遥控ID号  	
	UCHAR				Audio;							//监听开关	
	UCHAR				TimeOn;							//时间显示开关				
	UCHAR				DispMode;						//显示模式(1单画面、4四画面)
	UCHAR				ImageQulity;					//解码图像质量
	UCHAR				Reserve[31];					//备用
}PACKED DD_SYSTEM_PARAM;

//3，网络参数
typedef struct _DD_NET_CONFIG
{
	UINT             	IPAddr ;						//IP地址
	UINT             	SubNetMask ;					//掩码
	UINT             	GateWay ;						//网关

   	USHORT             	ComPortNo;						//设置接收客户端命令端口号        UDP	
   	USHORT             	WebPortNo;						//Webserver端口     			  TCP
	UINT             	MultiCastIPAddr ;				//多播IP地址
   	USHORT             	MultiCastPortNo;				//UDP传输起始端口<多播传输>
	
	UCHAR			   	RT8139MAC[6];      				//人工设置网卡的MAC地址		5---0有效
   	UCHAR             	DHCP;							//DHCP 开关					0	关		1:开
	UCHAR				AD[1];
	
   	USHORT             	PppoeOpen;						//PPPOE 开关         	
	UINT             	PppoeIPAddr ;					//PPPOEIP地址 （报警回传IP）
	CHAR				PppoeName[PPPPOE_NAME_LEN+1];	//拨号用户名
	CHAR				PppoePass[PPPPOE_PASSWD_LEN+1];	//拨号密码		
	CHAR				ADD[2];	
	UINT				DdnsOpen;						//DDNS开关
	CHAR				DdnsName[DDNS_NAME_LEN+1];		//注册主机名称
	CHAR				DdnsPass[DDNS_PASS_LEN+1];		//注册主机密码	

	
	CHAR				DdnsIP[DDNS_SERVER_NAME_LEN+1];	//DDNS服务器
	CHAR				ASD[1];
	
	USHORT				DdnsPortNo;						//DDNS服务器端口
	USHORT				DdnsMapWebPort;					//本地WEB映射端口
	USHORT				DdnsMapDataPort;				//本地数据映射端口
	CHAR				ASDD[2];

	UINT				DNSHostIP;						//DNS的IP
	INT				ConnectCenter;					//是否主动连接中心
	CHAR				ConnectCenterIP[DDNS_SERVER_NAME_LEN+1];//中心IP
	CHAR				AAD[1];
	
	USHORT				ConnectCenterPort;						//中心端口

	USHORT				appFunction;					//fix me
	UCHAR				tcpSendInterval;
	UCHAR				udpSendInterval;
	USHORT				PacketMTU;

	UCHAR				CaptureMode;					//抓拍模式
	UCHAR				CapturePort[MAX_SENSOR_NUM];	//抓拍通道(0-3bit: 分别表示1 ~ 4号通道  1:开  0: 关)

	UINT				DdnsIP2;						//DDNS 2的IP,非网络字节次序
	USHORT				DdnsPortNo2;					//DDNS 2的端口

	CHAR				sysByname[DVS_NAME_LEN+1];		//域名
	UCHAR				NetReserve[1];					//保留
} PACKED DD_NET_CONFIG ;

//4，串口设置
typedef struct __DD_COM_PARAM
{
	UINT 				Baudrate;						//串口波特率				300---115200
	UCHAR				Databit;						//串口数据位长度			5=5位 6=6位 7=7位 8=8位	其他=8位
	UCHAR				Stopbit;						//停止位位数				1=1位 2=2位 其他=1位
	UCHAR				CheckType;						//校验						0=无  1=奇  2=偶  3=恒1  4=恒0 
	UCHAR				Flowctrl; 						//软流控/硬流控/无流控		备用
}PACKED DD_COM_CONFIG;


//5，图像参数
typedef struct _DD_IMAGE_PARAM
{
	UCHAR 	Bright;										// 亮度
	UCHAR 	Shades;										// 色度
	UCHAR 	Contrast;									// 对比度
	UCHAR 	Saturate;									// 饱和度
	UCHAR 	MenuBright;									// 菜单亮度
}PACKED DD_IMAGE_CONFIG;

//6，报警联动
typedef struct __DD_ALARMOUT_PARAM
{
	UCHAR 				Open;							//联动开关<0: 关  1: 开>
	UCHAR				SensorAlarm[4];					//探头报警联动	.0 ~ 3 1--4号探头	0,1位: 分别表示继电器1 2的状态  <0: 关  1: 开>
	UCHAR				MotionAlarm[4];					//视频移动联动	.0 ~ 3 1--4号探头	0,1位: 分别表示继电器1 2的状态  <0: 关  1: 开>
	UCHAR				ViLoseAlarm[4];					//视频丢失联动	.0 ~ 3 1--4号探头	0,1位: 分别表示继电器1 2的状态  <0: 关  1: 开>
	UCHAR				Reserve[8];						//备用	
}PACKED DD_ALARMOUT_CONFIG;


//7，用户设置
typedef struct _DD_USER_CONFIG
{
	UINT 		Level;									//权限
	CHAR 		Name[17];				//用户名
	CHAR 		Password[17];			//用户密码
	CHAR 		AD[2];
}PACKED DD_USER_CONFIG;


//8，报警信息条目
typedef struct __DD_ALARM_ITEM
{
	UINT 				IPAddr;							//IP
	CHAR				URL[DD_URL_ADDR_LEN + 1];		//URL地址
	CHAR				AVD[3];
	UINT				ComPortNo;						//通讯端口
	DD_NVD_DATE			AlarmTime;						//报警时间
	UCHAR				SensorAlarm;					//探头报警联动	.0 ~ 3 1--4号探头	<0: 无报警  1: 有报警>
	UCHAR				MotionAlarm;					//视频移动联动	.0 ~ 3 1--4号视频	<0: 无报警  1: 有报警>
	UCHAR				ViLoseAlarm;					//视频丢失联动	.0 ~ 3 1--4号视频	<0: 无报警  1: 有报警>
	UCHAR				Reserve[8];						//保留
	CHAR				AD[2];
}PACKED DD_ALARM_ITEM;


//9，报警信息记录
typedef struct __DD_ALARM_INFO
{
	USHORT				Count;							//报警记录个数
	CHAR				AD[2];
	DD_ALARM_ITEM		Alarms[DD_MAX_ALARMINFO_NUM];	//报警记录内容
}PACKED DD_ALARM_INFO;


//10，地址薄
typedef struct __DD_ADDRBOOK_PARAM
{
	CHAR				Name[DDNS_NAME_LEN + 1];			//名称
	CHAR				URL[DD_URL_ADDR_LEN + 1];		//URL地址
	USHORT             	ComPortNo;						//通讯端口
	UCHAR				ChannelNo;						//通道号
	UCHAR				Protocol;						//0: TCP  1:多播   2:UDP
	UCHAR				DNSOpen;						//0: 关   1:开
	UCHAR				AD;
	USHORT				StayTime;						//停留时间	5--3600秒
	UCHAR				AutoLink;						//自动连接	0:关  1:开
	UCHAR				AudioOpen;						//声音打开	0:关  1:开	
	UCHAR				TalkOpen;						//对讲打开	0:关  1:开
	CHAR				UserName[USER_NAME_LEN + 1];	//用户名
	CHAR				UserPass[USER_PASSWD_LEN + 1];	//用户密码	
	UCHAR				Reserve[8];						//保留	
	UCHAR				AD2;
}PACKED DD_ADDRBOOK_CONFIG;


//11，透明通讯
typedef struct __DD_COMM_CTRL
{
	UCHAR				COMMNo;							//串口号	0 ~ 1
	UCHAR				ADG[3];
	COM_CONFIG			COMConfig;						//串口配置
	USHORT				DataLength;						//数据长度
	UCHAR				Data[256];						//数据
	UCHAR				AD[2];
}PACKED DD_COMM_CTRL;


//12，其它控制命令结构
typedef struct __DD_DEVICE_CTRL
{
	UCHAR				CMD;							//命令
	UCHAR 				Data1;							//数据1
	UCHAR 				Data2;							//数据2
	UCHAR				AD[1];
	UINT				nReserved[8];					//保留
}PACKED DD_DEVICE_CTRL;


//13，云台信息
typedef struct _DD_PROTOCOL_INFO
{
	CHAR 		Name[YUNTAI_NAME_LEN + 1];				//云台名称
	UCHAR		Exist;									//是否可用
	UCHAR		nReserved[8];							//保留
}PACKED DD_PROTOCOL_INFO;

//14，云台设置

typedef struct _DD_YUNTAI_SET
{
	CHAR 		Name[YUNTAI_NAME_LEN + 1];				//云台名称
	UCHAR 		YTAddr;									//解码器地址
	UCHAR		AD[2];
	COM_CONFIG	ComSet;									//通讯协议<波特率...>
	
	UCHAR		nReserved[8];							//保留
}PACKED DD_YUNTAI_SET;


//15，系统所有参数(所有配置参数，网络传输用)

typedef struct _DD_NVD_CONFIG 
{
	DD_SYSTEM_PARAM 		sysParam;					//系统配置
	DD_NET_CONFIG			sysNet;						//网络设置
	DD_COM_CONFIG			sysCom[MAX_COM_NUM];		//串口设置
	DD_IMAGE_CONFIG			sysImage;					//图像参数
	UCHAR					ADD[3];
	DD_USER_CONFIG			sysUser[MAX_USER_NUM];		//用户
	DD_ALARMOUT_CONFIG		sysAlarmOut;				//报警输出
		
	DD_PROTOCOL_INFO 		YunTaiInfo[MAX_VIDEO_NUM];	//云台信息
	UCHAR					AD[2];
	DD_YUNTAI_SET 			YunTaiSet[MAX_VIDEO_NUM];	//云台协议设置		
} PACKED DD_NVD_CONFIG ;


//16，扩展信息结构
typedef struct _DD_EXTINFO_CONFIG
{
	UINT			nSize;						//结构大小

	//ntp
	NTP_CONFIG		ntpConfig;

	//upnp
	UPNP_CONFIG		upnpConfig;
	
	//MAIL
	MAIL_CONFIG		mailConfig;

	//ALARM IN/OUT NAME
	ALARM_INOUT_NAME alarmInOutName;

	//WF NETWORK
	WF_NET_CONFIG	wfConfig;

	//SEND DEST
	SEND_DEST		sendDest;
	
	CHAR			reserve[480];
}DD_EXTINFO_CONFIG;


#endif
