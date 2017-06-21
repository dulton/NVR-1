#ifndef _MOD_CONFIG_H_
#define _MOD_CONFIG_H_

typedef struct
{
	s8 nVersion[30];
	s8 nModel[20];
	s8 nproductnumber[32];//9624 产品号
	u8 nPreviewNum;
	u8 nVidMainNum;
	u8 nVidSubNum;
	u8 nAudNum;
	u8 nSnapNum;
	u8 nVoipNum;
	u8 nRecNum;
	u8 nSensorNum;
	u8 nAlarmoutNum;
	u8 nBuzzNum;
	u8 nMaxHddNum;
	u32 nMaxMainFrameSize;
	u32 nMaxSubFrameSize;
	u32 nMaxAudFrameSize;
	u32 nMaxSnapFrameSize;
	u8 nOsdOperator;
	u8 nStrOsdNum;
	u8 nImgOsdNum;
	u8 nRectOsdNum;
	u8 nRectOsdRgb[3];
	u8 nLineOsdNum;
	u16 nRefWidth;
	u16 nRefHeight;
	u8 nAudioEncType;
	u16 nAudioSampleRate;
	u16 nAudioSampleDuration;
	u16 nAudioSampleSize;
	u16 nAudioBitWidth;
	u8 nVoipEncType;
	u16 nVoipSampleRate;
	u16 nVoipSampleDuration;
	u16 nVoipSampleSize;
	u16 nVoipBitWidth;
	u8 nMdAreaRows;
	u8 nMdAreaCols;
	u8 nMdSenseMin;
	u8 nMdSenseMax;
	u16 nSchSegments;
	u16 nOutputSync;
	u8 nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
}SModConfigDvrProperty;

typedef struct
{
	s8 nDevName[32];
	u32 nDevId;
	u8 nManual;
	u8 nVideoStandard;
	u8 nAuthCheck;
	u8 nOutput;
	u8 nLangId;
	u8 nShowGuide;
	u32 nLockTime;
	u8 nDateFormat;
	u8 nTimeStandard;
	u8 nCyclingRecord;
	u8 nShowFirstMenue;//显示"菜单输出设备选择"界面
	//csp modify
	s8 sn[32];
	//QYJ
	u8 nTimeZone;
	u8 nSyscCheck;
	s8 nNtpServerAdress[32];
	u8 nSyncTimeToIPC;
	u8 P2PEnable;
	//QYJ
	u8 reserved[256-36];//QYJ
} SModConfigSystemParam;

//MainTain
typedef struct
{
	u8 nEnable;
	u8 nMainType;
	u8 nSubType;
	u8 ntime;
} SModConfigMainTainParam;

//Preview
typedef struct
{
	u8 nPreviewMode;
	u8 nModePara; 						//extend parameter for EMPREVIEWMODE参考SBizPreviewPara.nModePara
	u8 nAdioChn; 						//音频输出通道
	u8 nVolume; 						//音量
	u8 nMute; 							//是否静音
	u8 nIsPatrol; 						//默认是否开启轮巡
	u8 nInterval; 						//轮巡间隔 秒
	u8 nPatrolMode;
	u8 nStops; 							//一次轮回停几个点
	u8 pnStopModePara[256]; 				//每站预览模式参数;数组大小由上个参数nStops决定，参考SBizPreviewPara.nModePara；nStops==0xff时无效	
	u8 nVideoSrcType[128];					//视频输入源的类型
} SModConfigPreviewParam;

typedef struct
{
	u8 nHue;
	u8 nSaturation;
	u8 nContrast;
	u8 nBrightness;
} SModConfigImagePara;

//END


//osd
typedef struct
{
	u8 nShowTime;
	u16 nTimePosX;
	u16 nTimePosY;
	u8 nShowRecState;
	u8 nShowChnKbps;// 是否显示通道的实时码流带宽
}SModConfigMainOsdParam;

typedef struct
{
	s8 nChnName[32];
	u8 nShowChnName;
	u16 nChnNamePosX;
	u16 nChnNamePosY;
	u8 nEncShowChnName;
	u16 nEncChnNamePosX;
	u16 nEncChnNamePosY;
	u8 nEncShowTime;
	u16 nEncTimePosX;
	u16 nEncTimePosY;	
}SModConfigStrOsdpara;

typedef struct
{
	u8 nShowRectOsd;
	u8 nRectOsdNum;
	u16 nRectOsdX[256];
	u16 nRectOsdY[256];
	u16 nRectOsdW[256];
	u16 nRectOsdH[256];	
}SModConfigRectOsdpara;
//end


//Encode
typedef struct 
{
	u32 nBitRate;
	u32 nFrameRate;
	u32 nGop;
	u32 nMinQP;
	u32 nMaxQP;
	u8 nResolution;
	u8 nEncodeType;
	u8 nBitRateType;
	u8 nPicLevel;
} SModConfigVideoParam;

typedef struct
{
	u8 nEncodeType;
	u32 nSampleRate;
	u32 nSampleDuration;
	u32 nSampleSize;
	u8 nBitWidth;
} SModConfigAudioParam;
//END


//Record
typedef struct
{
	u8 bRecording;
	u8 nEnable;
	u8 nStreamType;
	u8 nEncChn;
	u8 nChnEncType;
	u8 nPreTime;
	u16 nDelayTime;
	u8 nExpireDays;
	u8 nTimerRecSchType;
	u8 nMDRecSchType;
	u8 nSensorRecSchType;
} SModConfigRecordParam;
//END


//Alarm
typedef struct
{
	u8 nPtzType;
	u8 nChn; //通道号
	u8 nId; //表示预置点id/巡航线路id/轨迹id(无效)
} SModConfigAlarmPtz;
	
typedef struct
{
	u8 nFlagBuzz; //蜂鸣器
	u8 nZoomChn; //大画面报警通道号 0xff 表示无效
	u8 nFlagEmail; //邮件
	u64 nSnapChn; //存放触发抓图的通道号 最多同时64个
	u64 nAlarmOut; //触发报警 最多同时触发64个，放id;
	u64 nRecordChn; //触发录像通道;
	SModConfigAlarmPtz sAlarmPtz[256]; //最多同时触发256个联动
} SModConfigAlarmDispatch;

typedef struct
{
	u8 nEnalbe;
	u8 nSensorType;
	u16 nDelay;
	s8 name[20];
	u8 nSchType;
	SModConfigAlarmDispatch psDispatch;
} SModConfigSensorParam;

//yaogang modify 20141010
typedef struct
{
	u8 nEnalbe;
	u16 nDelay;
	SModConfigAlarmDispatch psDispatch;
} SModConfigIPCExtSensorParam;

typedef struct
{
	u8 nEnalbe;
	SModConfigAlarmDispatch psDispatch;
} SModConfigHDDParam;


typedef struct
{
	u8 nEnalbe;
	u8 nSensitivity;
    u16 nDelay;
	u8 nSchType;
	SModConfigAlarmDispatch psDispatch;
	u64 nBlockStatus[64]; //对应区域块状态
} SModConfigVMotionParam;

typedef struct
{
	u8 nEnalbe;
	u16 nDelay;
	u8 nSchType;
	SModConfigAlarmDispatch psDispatch;
} SModConfigVLostParam;

typedef struct
{
	u8 nEnalbe;
	u16 nDelay;
	u8 nSchType;
	SModConfigAlarmDispatch psDispatch;
} SModConfigVBlindParam;

typedef struct
{
	 u8 nEnalbe;
	 u8 nAlarmOutType;
	 u16 nDelay;
	 s8 name[20];
	 u8 nSchType;
} SModConfigAlarmOutParam;

typedef struct
{
	u8 nEnalbe; 
    u16 nDelay;
	u8 nDuration;
    u8 nInterval;
} SModConfigBuzzParam;
//END


//Schedule
typedef struct 
{
	u32 nStartTime;	//每个时间段的起始时间
	u32 nStopTime;	//结束时间
} SModConfigSchTime;

#define CONFIG_MAX_SCH_TIME_SEGMENTS 12	//每天可设置的时间段的最大数目

typedef struct 
{
	u8 nSchType; //报警布防的时间类型	
	SModConfigSchTime nSchTime[31][CONFIG_MAX_SCH_TIME_SEGMENTS];	//各段布防时间
} SModConfigSchedule;

typedef struct
{
	SModConfigSchedule psSensorSch;
} SModConfigSensorSch;

typedef struct
{
	SModConfigSchedule psVMSch;
} SModConfigVMotionSch;

typedef struct
{
	SModConfigSchedule psVLostSch;
} SModConfigVLostSch;

typedef struct
{
	SModConfigSchedule psVBlindSch;
} SModConfigVBlindSch;

typedef struct
{
	SModConfigSchedule psAlarmOutSch;
} SModConfigAlarmOutSch;

typedef struct
{
	SModConfigSchedule psRecSch;
} SModConfigRecTimerSch;

typedef struct
{
	SModConfigSchedule psRecSch;
} SModConfigRecAlarmInSch;

typedef struct
{
	SModConfigSchedule psRecSch;
} SModConfigRecVMSch;

typedef struct
{
	SModConfigSchedule psRecSch;
} SModConfigRecVLostSch;

typedef struct
{
	SModConfigSchedule psRecSch;
} SModConfigRecVBlindSch;
//END


//Ptz
typedef struct
{
	u8 nPresetId;
	u8 nSpeed;
	u32 nStayTime;
}SModConfigPresetPara;

typedef struct
{
	u64 nPresetId[4];
	u64 nTourId[4];
	SModConfigPresetPara nTourPresetId[32][128];//nTourPresetId[256][256];
}SModConfigPtzTourPresetPara;

typedef struct
{
	u8 nEnable;
	u8 nCamAddr;	
	u8 nDataBit;
	u8 nStopBit;
	u32 nBaudRate;
	u8 nCheckType;
	u8 nFlowCtrlType;
	s8 nProtocolName[20];
	SModConfigPtzTourPresetPara sTourPresetPara;
}SModConfigPtzParam;
//END


//USER
#define MOD_CONFIG_MAX_GROUP_NUM 16			//所支持的最大的用户组数目
#define MOD_CONFIG_MAX_USER_NUM 128			//所支持的最大的用户数目

typedef struct
{
	u8 nAuthor[64];
	u64 nLiveView[4];
	u64 nRemoteView[4];
	u64 nRecord[4];
	u64 nPlayback[4];
	u64 nBackUp[4];
	u64 nPtzCtrl[4];
}SModConfigUserAuthority;

typedef struct
{
	u8 GroupName[15];
	u8 GroupDescript[30];
	u8 nDoWithUser;	//对组进行修改和删除时，是否使操作应用到该组下的所有用户；[0]:否，[非0]:是
	SModConfigUserAuthority GroupAuthor;
}SModConfigUserGroupPara;

typedef struct
{
	u8 UserName[15];
	u8 Password[15];
	u8 GroupBelong[15];
	u8 UserDescript[30];
	u8 emLoginTwince;
	u8 emIsBindPcMac;
	u64 PcMacAddress;
	SModConfigUserAuthority UserAuthor;
}SModConfigUserUserPara;

typedef struct
{
	u64 nFlagGroupSetted;
	u64 nFlagUserSetted[2];
	SModConfigUserUserPara sUserPara[MOD_CONFIG_MAX_USER_NUM];
	SModConfigUserGroupPara sGroupPara[MOD_CONFIG_MAX_GROUP_NUM];
}SModConfigUserPara;
//END


/*
 配置接口存取网络参数结构
*/
typedef struct
{
	char	szSmtpServer[64];
	
	// 发件人邮箱
	char	szSendMailBox[64];
	char	szSMailPasswd[64];
	
	u8		nSSLFlag;
	
	// 收件人邮箱
	char	szReceiveMailBox[64];
	char	szReceiveMailBox2[64];
	char	szReceiveMailBox3[64];
	
	// 附加图片
	char	szSnapPic[64];
	
	// 抓图间隔
	u32		nSnapIntvl;
} SModConfigNetAdvancePara;

typedef struct
{
	// 基本参数
	u8     	Version[8];			    // 8字节的版本信息
    char   	HostName[16];           // 主机名
    u32  	HostIP;                 // IP 地址
    u32  	Submask;                // 子网掩码
    u32  	GateWayIP;              // 网关 IP
    u32  	DNSIP;                  // DNS IP
    u32  	DNSIPAlt;                 // DNS IP
    u8 		MAC[18];				// MAC Address
	
    // 外部接口
    u32   	AlarmServerIP;          // 报警中心IP
    u16 	AlarmServerPort;        // 报警中心端口
    u32   	SMTPServerIP;           // SMTP server IP
    u16 	SMTPServerPort;         // SMTP server port
	
    u32   	LogServerIP;            // Log server IP
    u16 	LogServerPort;          // Log server port
	
    // 本机服务端口
    u16 	HttpPort;               	// HTTP服务端口号
    u16 	MobilePort;              	// 手机监控端口号
    
    u16 	TCPMaxConn;             	// TCP 最大连接数
    u16 	TCPPort;                	// TCP 侦听端口
	
	u8		nSubStreamMax;				// 子码流数上限
	u8		nSendBufNumPerChn;			// 每通道发送buff数
	u32		nFrameSizeMax;				// 帧buff大小上限
	
	// 网路子码流
	u8		nVideoMediaType;			// 视频编码类型
	u8		nAudioMediaType;			// 预览音频编码类型
	u8		nVoipMediaType;				// 对讲音频编码类型
	u8		nAudioSampleMode;			// 音频采样模式
	
    u16 	UDPPort;                	// UDP 侦听端口
	
    u8		McastFlag;
    u16 	McastPort;              	// 组播端口
    u32 	McastIP;                	// 组播IP
    
	u8		PPPOEFlag;					// pppoe 使能开关
	u8		PPPOEUser[64];
	u8		PPPOEPasswd[64];
	
	u8		DhcpFlag;					// Dhcp 使能开关
	
	u8		DDNSFlag;					// DDNS 控制位
	u8		DDNSDomain[64];
	u8		DDNSUser[64];
	u8		DDNSPasswd[64];
	u16		UpdateIntvl;				// DDNS	IP更新间隔
	
    // 其他
    u8    	MonMode;                	// 监视协议 TCP|UDP|MCAST
    u8    	PlayMode;               	// 回放协议 TCP|UDP|MCAST
    u8    	AlmSvrStat;             	// 报警中心状态 <开/关>
	
	// 高级配置
    SModConfigNetAdvancePara	sAdancePara;
	
	//csp modify 20130321
	u8		UPNPFlag;
} SModConfigNetParam;

//NVR used
//EM_CONFIG_PARA_IPCAMERA
typedef struct
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
	//char reserved[63];
	//yaogang modify for yuetian private
	char main_audio_sw;//主码流音频开关
	char sub_audio_sw;//次码流音频开关
	char reserved[61];
} SModConfigIPCameraParam;


typedef enum
{
    EM_CONFIG_PARALIST_BASE = 0,
    EM_CONFIG_PARALIST_LANGSUP, // 支持的语言列表
    EM_CONFIG_PARALIST_LANGSRC, // 支持的语言资源文件列表 ;仅支持字符串列表
    EM_CONFIG_PARALIST_PREVIEWMODE, // 支持的预览模式
    EM_CONFIG_PARALIST_BITRATE, // 支持的位率列表
    EM_CONFIG_PARALIST_BITTYPE, // 位率类型列表
    EM_CONFIG_PARALIST_PICLEVEL, // 图像质量列表
    EM_CONFIG_PARALIST_VMAINRESOLH_NUM, // 主码流通用支持的分辨率列表1的通道数目//cw_9508S
    EM_CONFIG_PARALIST_VMAINRESOL, // 主码流通用支持的分辨率列表
    EM_CONFIG_PARALIST_VMAINRESOL2, // 主码流通用支持的分辨率列表2
    EM_CONFIG_PARALIST_VSUBRESOL, // 子码流通用支持的分辨率列表
    EM_CONFIG_PARALIST_VMOBRESOL, // 手机码流通用支持的分辨率列表
    EM_CONFIG_PARALIST_FPSPAL, // P制常用下支持的帧率列表
    EM_CONFIG_PARALIST_FPSNTSC, // N制常用下支持的帧率列表
    EM_CONFIG_PARALIST_FPSPALD1, // P制D1下支持的帧率列表
    EM_CONFIG_PARALIST_FPSNTSCD1, // N制D1下支持的帧率列表
    EM_CONFIG_PARALIST_FPSPALCIF, // P制Cif下支持的帧率列表
    EM_CONFIG_PARALIST_FPSNTSCCIF, // N制Cif下支持的帧率列表
    EM_CONFIG_PARALIST_STREAMTYPE, // 录像码流类型列表
    EM_CONFIG_PARALIST_VIDEOSTANDARD, // 视频制式列表
    EM_CONFIG_PARALIST_OUTPUT, // 视频输出列表
    
    EM_CONFIG_PARALIST_SWITCHPICTURE, //轮巡画面选择 
    EM_CONFIG_PARALIST_LOCKTIME, // 锁定超时间选项列表
    EM_CONFIG_PARALIST_DATEFORMAT, // 日期格式列表
    EM_CONFIG_PARALIST_TIMESTANDARD, // 时间制式列表
    EM_CONFIG_PARALIST_SITEMAINOUT, // 现场主输出列表
    EM_CONFIG_PARALIST_INTERVAL, // 时间间隔列表
    EM_CONFIG_PARALIST_SENSORTYPE, // 传感器类型列表
    EM_CONFIG_PARALIST_ALARMOUTTYPE, // 报警输出类型列表
    EM_CONFIG_PARALIST_DELAY, // 延时时间选项列表
    EM_CONFIG_PARALIST_PRETIME, // 预录时间选项列表
    EM_CONFIG_PARALIST_RECDELAY, // 录像延时时间选项列表
    EM_CONFIG_PARALIST_RECEXPIRE, // 录像过期天数选项列表
    EM_CONFIG_PARALIST_SCHEDULETYP, // 布防类型选项列表
    EM_CONFIG_PARALIST_PTZLINKTYPE, // 云台联动类型选项列表
    EM_CONFIG_PARALIST_BAUDRATE, // 波特率选项列表
    EM_CONFIG_PARALIST_DATABIT, // 数据位类型选项列表
    EM_CONFIG_PARALIST_STOPBIT, // 停止位类型选项列表
    EM_CONFIG_PARALIST_CHECKTYPE, // 校验类型类型选项列表
    EM_CONFIG_PARALIST_FLOWCTRLTYPE, // 流控方式类型选项列表
    EM_CONFIG_PARALIST_DDNSDOMAIN, // DDNSDomain
    EM_CONFIG_PARALIST_DDNSIPUPDATEITVL, // DDNS IP update    
    EM_CONFIG_PARALIST_VIDEOSRCTYPE, // 视频源类型选项列表
    //
    EM_CONFIG_PARALIST_PTZPROTOCOL, // 云台协议选项列表
    //yaogang modify 20141210 ShenGuang
    EM_CONFIG_PARALIST_RECORD_SNAP_RES,//抓图分辨率
    EM_CONFIG_PARALIST_RECORD_SNAP_QUALITY,//抓图质量
    EM_CONFIG_PARALIST_RECORD_SNAP_INTERVAL,//抓图时间间隔
    EM_CONFIG_PARALIST_RepairName,//报修系统名称
    EM_CONFIG_PARALIST_RepairType,//报修系统类型
    EM_CONFIG_PARALIST_Maintain,//维护维修
    EM_CONFIG_PARALIST_Test,//测试
    EM_CONFIG_PARALIST_AlarmInput,//报警图片配置: 输入端子
    
    EM_CONFIG_PARALIST_NUM,
}EMCONFIGPARALISTTYPE;

typedef enum
{
	EM_CONFIG_PARA_SYSTEM = 0,
	EM_CONFIG_PARA_PREVIEW = 100,
	EM_CONFIG_PARA_IMAGE = 200,
	EM_CONFIG_PARA_MAIN_OSD = 300,
	EM_CONFIG_PARA_STR_OSD = 330,
	EM_CONFIG_PARA_RECT_OSD = 360,	
	EM_CONFIG_PARA_SENSOR = 400,
	//yaogang modify 20141010
	EM_CONFIG_PARA_IPCCOVER,
	EM_CONFIG_PARA_IPCEXTSENSOR,
	EM_CONFIG_PARA_485EXTSENSOR,
	EM_CONFIG_PARA_HDD,
	
	EM_CONFIG_PARA_VMOTION = 500,
	EM_CONFIG_PARA_VLOST = 600,
	EM_CONFIG_PARA_VBLIND = 700,
	EM_CONFIG_PARA_ALARMOUT = 800,
	EM_CONFIG_PARA_BUZZ = 900,
	EM_CONFIG_PARA_SENSOR_SCH = 1000,
	EM_CONFIG_PARA_IPCEXTSENSOR_SCH,
	EM_CONFIG_PARA_IPCCOVER_SCH,
	EM_CONFIG_PARA_VMOTION_SCH = 1100,
	EM_CONFIG_PARA_VLOST_SCH = 1200,
	EM_CONFIG_PARA_VBLIND_SCH = 1300,
	EM_CONFIG_PARA_ALARMOUT_SCH = 1400,
	EM_CONFIG_PARA_BUZZ_SCH = 1500,	
	EM_CONFIG_PARA_VO_IMAGE = 1600,
	EM_CONFIG_PARA_PATROL = 1700,
	EM_CONFIG_PARA_MAINTAIN = 1800,
	//NVR used
	//IPCamera参数控制
	EM_CONFIG_PARA_IPCAMERA = 1900,//
	//
	EM_CONFIG_PARA_ENC_MAIN = 4000,
	EM_CONFIG_PARA_ENC_SUB = 4100,
	EM_CONFIG_PARA_ENC_MOB,
	//
	EM_CONFIG_PARA_RECORD = 6000,
	EM_CONFIG_PARA_TIMER_REC_SCH = 6100,
	EM_CONFIG_PARA_SENSOR_REC_SCH = 6200,
	EM_CONFIG_PARA_VMOTION_REC_SCH = 6300,
	EM_CONFIG_PARA_VLOST_REC_SCH = 6400,
	EM_CONFIG_PARA_VBLIND_REC_SCH = 6500,	
	//
	EM_CONFIG_PARA_USER = 8000,
	EM_CONFIG_PARA_PTZ = 8100,
	EM_CONFIG_PARA_NETWORK = 8200,	
	//
	EM_CONFIG_PARA_DVR_PROPERTY = 9000,
	//yaogang modify 20141209
	//深广平台参数
	EM_CONFIG_PARA_SG,
	EM_CONFIG_PARA_SG_DAILY,
	EM_CONFIG_PARA_SG_ALARM,
	EM_CONFIG_PARA_SNAP_CHN,
	
	//yaogang modify for bad disk
	EM_CONFIG_BAD_DISK,
	
	EM_CONFIG_PARA_ALL = 10000,
	//
}EMCONFIGPARATYPE;

s32 ModConfigInit(void);
s32 ModDefaultConfigRelease(void);
s32 ModDefaultConfigResume(void);
s32 ModConfigGetParam(EMCONFIGPARATYPE emType, void* para, s32 nId);//nId是通道号等， -1 表示无效
s32 ModConfigSetParam(EMCONFIGPARATYPE emType, void* para, s32 nId);
s32 ModConfigGetDefaultParam(EMCONFIGPARATYPE emType, void* para, s32 nId);
s32 ModConfigDefault(void);
s32 ModConfigBackup(s8* pFilePath);//需要包含文件名，如/tmp/usb/config_backup.ini，传入/tmp/usb无效
s32 ModConfigResume(EMCONFIGPARATYPE emType, s32 nId, s8* pFilePath);//同上//导入配置文件要用到

s32 ModConfigResumeDefault(EMCONFIGPARATYPE emType, s32 nId);
s32 ModConfigGetConfigFilePath(s8* pFilePath, u32 nMaxPathLen);
s32 ModConfigSyncFileToFlash(u8 nMode);//nMode说明 (0 或者 非0)
									   //[0:系统关机或重启前调用, 非0:平时需要同步文件到flash时使用]

//获得字符串取值列表
s32 ModConfigGetParaStr(EMCONFIGPARALISTTYPE emParaType, PARAOUT s8* strList[], PARAOUT u8* nRealListLen, u8 nMaxListLen, u8 nMaxStrLen);
//根据数字取值获得字符串列表中的index
s32 ModConfigGetParaListIndex(EMCONFIGPARALISTTYPE emParaType, s32 nValue, PARAOUT u8* pnIndex);
//根据index获得实际参数取值
s32 ModConfigGetParaListValue(EMCONFIGPARALISTTYPE emParaType, u8 nIndex, PARAOUT s32* pnValue);


//yaogang modify 20141209
//深广平台参数
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	u8 normal_exit;// 1: 系统正常关机0: 异常退出
} SModConfigSGParam;

//报警图片配置，每个报警通道一个结构
//m路本地，n路IPC外部报警(m 16--n 32)
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SModAlarmPicCFG;

//日常图片配置
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SModSGTime;

typedef struct
{
	SModSGTime Time1;		//定时上传时间1 
	SModSGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//每个IPC通道各占一位，0xffff ffff表示所有通道
	u32 IpcChn2;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SModDailyPicCFG;

#if 0
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SModSGTime;
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SModAlarmPicCFG;
typedef struct
{
	SModSGTime Time1;		//定时上传时间1 
	SModSGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//16个IPC通道各占一位，0x1000 0000表示所有通道
	u32 IpcChn2;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SModDailyPicCFG;
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	SModAlarmPicCFG SAlarmPicParam;//报警图片配置参数
	SModDailyPicCFG SDailyPicParam;//日常图片配置参数
} SModConfigSGParam;
#endif

//yaogang modify 20150105
typedef struct 
{
	u8 Enable;	//启用
	u8 Res;		//分辨率
	u8 Quality;	//图片质量
	u8 Interval;	//时间间隔
} SModConfigSnapPara;
typedef struct 
{
	SModConfigSnapPara TimeSnap;		//定时抓图
	SModConfigSnapPara EventSnap;	//事件抓图
} SModConfigSnapChnPara;

//yaogang modify for bad disk
typedef struct 
{
	u32 time;
	char disk_sn[64];//硬盘序列号、唯一
} SModConfigBadDisk;



#endif //_MOD_CONFIG_H_

