#ifndef _MOD_LOG_H_
#define _MOD_LOG_H_

//系统控制
#define MASTER_SYSCONTROL     					0x01      
#define SLAVE_START_DWELL      					0x01
#define SLAVE_STOP_DWELL       					0x02
#define SLAVE_MUTE            					0x03
#define SLAVE_CHANGE_VOICE    					0x04
#define SLAVE_CONTROL_PTZ      					0x05
#define SLAVE_SNAP_SUCCESS     					0x06
#define SLAVE_START_MANUAL_REC 					0x07
#define SLAVE_STOP_MANUAL_REC  					0x08
#define SLAVE_MANUAL_ALARM     					0x09
#define SLAVE_CLEAR_ALARM      					0x0a
#define SLAVE_START_MD         					0x0b
#define SLAVE_STOP_MD          					0x0c
#define SLAVE_POWER_OFF        					0x0d
#define SLAVE_REBOOT   	       					0x0e
#define SLAVE_LOGIN            					0x0f
#define SLAVE_LOGOFF           					0x10
#define SLAVE_FORMAT_SUCCESS   					0x11
#define SLAVE_FORMAT_FAIL      					0x12
#define SLAVE_UPDATE           					0x13

//系统设置
#define MASTER_SYSCONFIG 						0x02
#define SLAVE_ENABLE_PWDCHECK                   0x01
#define SLAVE_DISABLE_PWDCHECK                  0x02
#define SLAVE_CHANGE_VO                         0x03
#define SLAVE_CHANGE_MAX_NETUSER                0x04
#define SLAVE_CHANGE_VIDEOFORMAT                0x05
#define SLAVE_CHANGE_LANGUAGE                   0x06
#define SLAVE_CHANGE_ZONE                       0x07
#define SLAVE_ENABLE_AUTO_TIMESYNC              0x08
#define SLAVE_DISABLE_AUTO_TIMESYNC             0x09
#define SLAVE_CHANGE_NTP_SERVER                 0x0a
#define SLAVE_DST_CONFIG                        0x0b
#define SLAVE_CHANGE_CHN_NAME                   0x0c
#define SLAVE_CHANGE_REC_ENABLE                 0x0d
#define SLAVE_CHANGE_REC_AUDIO_ENABLE           0x0e
#define SLAVE_CHANGE_REC_CODESTREAM             0x0f
#define SLAVE_CHANGE_TIME_BEFORE_ALARM          0x10
#define SLAVE_CHANGE_TIME_AFTER_ALARM           0x11
#define SLAVE_CHANGE_REC_EXPIRETIME             0x12
#define SLAVE_CHANGE_OSD                        0x13
#define SLAVE_CHANGE_TIMER_RECSCHE              0x14
#define SLAVE_CHANGE_MD_RECSCHE                 0x15
#define SLAVE_CHANGE_SENSORALARM_RECSCHE        0x16
#define SLAVE_CHANGE_SENSOR_ENABLE              0x17
#define SLAVE_CHANGE_SENSOR_TYPE                0x18
#define SLAVE_CHANGE_SENSOR_ALARMDEAL           0x19
#define SLAVE_CHANGE_SENSOR_DETECTION_SCHE      0x1a
#define SLAVE_CHANGE_MD_ENABLE                  0x1b
#define SLAVE_CHANGE_MD_AREA_SETUP              0x1c
#define SLAVE_CHANGE_MD_ALARMDEAL               0x1d
#define SLAVE_CHANGE_MD_DETECTION_SCHE          0x1f
#define SLAVE_CHANGE_VIDEOLOSS_ALARMDEAL        0x20
#define SLAVE_CHANGE_ALARMOUT_SCHE              0x21
#define SLAVE_ENABLE_SOUND_ALARM                0x22
#define SLAVE_DISABLE_SOUND_ALARM               0x23
#define SLAVE_CHANGE_IP                         0x24
#define SLAVE_CHANGE_HTTP_PORT                  0x25
#define SLAVE_CHANGE_SERVER_PORT                0x26
#define SLAVE_CHANGE_NET_CODESTREAM             0x27
#define SLAVE_CHANGE_OUTBOX_CONFIG              0x28
#define SLAVE_CHANGE_INBOX_CONFIG               0x29
#define SLAVE_CHANGE_DNS_CONFIG                 0x2a
#define SLAVE_ADD_USER                          0x2b
#define SLAVE_DELETE_USER                       0x2c
#define SLAVE_CHANGE_USER_AUTH                  0x2d
#define SLAVE_CHANGE_PWD                        0x2e
#define SLAVE_CHANGE_SERIAL_CONFIG              0x2f
#define SLAVE_CHANGE_CRUISELINE                 0x30
#define SLAVE_FACTORY_RESET                     0x31

//回放
#define MASTER_PLAYBACK 						0x03
#define SLAVE_PB_STOP 							0x01
#define SLAVE_PB_PAUSE 							0x02
#define SLAVE_PB_RESUME 						0x03
#define SLAVE_PB_BACKWARD 						0x04
#define SLAVE_PB_FORWARD 						0x05

//备份
#define MASTER_BACKUP 							0x04
#define SLAVE_START_BACKUP 						0x01
#define SLAVE_END_BACKUP 						0x02

//检索
#define MASTER_SEARCH 							0x05
#define SLAVE_SEARCH_DATA_BYTIME 				0x01
#define SLAVE_SEARCH_RECTIME 					0x02

//查看信息
#define MASTER_CHECKINFO 						0x06

//错误
#define MASTER_ERROR 							0x07
#define SLAVE_ERROR_LOGINPWD 					0x02
#define SLAVE_ERROR_HDD_ABNORMAL				0x03  //硬盘异常
#define SLAVE_ERROR_ENC_ABNORMAL				0x04  //编码异常
#define SLAVE_ERROR_FILE_ABNORMAL 				0x05  //录像文件异常

typedef struct
{
	u32 nMaxLogs;
	void* pHddMgr;
}SLogInitPara;

typedef struct
{
	u8  nMode;									//查询方式
	u16 nMasterMask;							//查询类型
	time_t nStart;								//起始时间
	time_t nEnd;								//结束时间
}SLogSearchPara;

typedef struct
{
	u16 nType;									//返回类型
	u8 magic1;//报警源的序号或通道
	u8 magic2;
	time_t nTime;								//返回起始时间
	char aIp[16];								//IP地址
	char aUsername[32];							//用户名
	u32 logId;									//cw_log
}SLogInfo;


typedef struct
{
	u32 nRealNum;								//返回的日志数
	u32 nMaxLogs;
	SLogInfo* psLogList;	    				//返回日志查询的结果
}SLogResult;

typedef struct
{
	int init_flag;
	
	FileHeadInfo index_head;	//索引文件头信息
	unsigned int max_log_num;
	
	unsigned int using_disk_idx;
	
	char logFile[32];
	char disk_sn[64];			//硬盘序列号、唯一
	
	pthread_mutex_t lock;		//使对文件的访问串行化，支持多线程
}DiskLog;


typedef void (*LOGEXPORTCB)(SLogInfo* pLog, u8* strLog);

#ifdef __cplusplus
extern "C" 
{
#endif

s32 ModLogInit(SLogInitPara* psInitPara);

s32 ModLogDeinit();

s16 ModMakeType(s8 mainType, s8 slaveType);

s8 ModGetMaster(s16 type);

s8 ModGetSlave(s16 type);

s32 ModWriteLog(SLogInfo* psLogInfo);

s32 ModSearchLog(SLogSearchPara* psLogSearch, SLogResult* psLogResult);

//导出日志文件格式由回调函数决定
s32 ModRegistLogExportCB(LOGEXPORTCB logExportCb);

//如果psLogResult为空，则导出所有日志
//否则，导出psLogResult列表日志
s32 ModExportLog(SLogResult* psLogResult, char* pPath);
s32 ModLogSetFormatFlag(u8 flag);


#ifdef __cplusplus
}
#endif

#endif  //_MOD_LOG_H_

