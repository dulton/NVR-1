#ifndef __SG_PLATFORM_H_
#define __SG_PLATFORM_H_
//yaogang modify 20141209 深广平台

#include "common_basetypes.h"
#include "hddcmd.h"
#include "diskmanage.h"
#include "Cmdcallback.h"
#include "netcomm.h"

#ifdef __cplusplus
extern "C" {
#endif

//消息类型
typedef enum
{
	EM_TYPE_MIN = 0,
	EM_PIC_ALARM_LINK_UPLOAD = 1,	// 1 报警联动图像
	EM_PIC_MAINTAIN_UPLOAD,	// 2 保养上传图像
	EM_PIC_DAILY_UPLOAD,	// 3 日常上传图像
	EM_PIC_TEST_UPLOAD,	// 4 测试上传图像
	EM_PIC_SERVER_GET,		// 5 主动调阅图像 1
	EM_SYS_Other = 11,		// 11 系统其他故障
	EM_Perimeter_Fault,		// 12 周界报警故障
	EM_Networking_Fault,		// 13 联网报警故障
	EM_LocalAlarm_Fault,		// 14 本地报警故障
	EM_VideoMonitor_Fault,		// 15 视频监控故障
	EM_BuildingIntercom_Fault,	// 16 楼宇对讲故障
	EM_AccessControl_Fault,	// 17 出入控制故障
	EM_ElectronicPatrol_Fault,	// 18 电子巡更故障
	EM_DVR_STARTUP,		// 19 DVR系统启动
	EM_DVR_EXIT_NORMOAL,	// 20 DVR系统退出
	EM_DVR_EXIT_ABNORMAL,	// 21 DVR异常退出
	EM_DVR_PARAM_SET,		// 22 DVR参数设置
	EM_DVR_PARAM_SAVE,	// 23 DVR参数保存
	EM_DVR_VLOST,			// 24 DVR视频丢失
	EM_DVR_VMOTION,		// 25 DVR移动侦测
	EM_DVR_ALARM_EXT,		// 26 DVR外部触发
	EM_DVR_ALARM_RESUME,	// 27 系统报警解除
	EM_DVR_EXIT_ILLEGALITY,	// 28 DVR非法退出
	EM_REPAIR_CHECK,		// 29 系统维修签到
	EM_MAINTAIN_CHECK,		// 30 系统维保签到
	EM_DVR_PLAYBACK,		// 31 DVR本地回放
	EM_REMOTE_PLAYBACK,	// 32 DVR远程回放
	EM_PIC_ACCEPT_UPLOAD,	// 33 验收上传图像
	EM_DVR_MOTION,			// 34 偏位报警 1
	EM_PIC_ALARM_UPLOAD_UNREC = 36,	// 36 报警上传图像（未录像或移动侦测）
	EM_PIC_MAINTAIN_UPLOAD_UNREC,// 37 保养上传图像（未录像或移动侦测）
	EM_PIC_DAILY_UPLOAD_UNREC,	// 38 日常上传图像（当日未录像）
	EM_PIC_TEST_UPLOAD_UNREC,		// 39 测试上传图像（未录像或移动侦测）
	EM_PIC_ACCEPT_UPLOAD_UNREC,	// 40 验收上传图像（未录像或移动侦测）
	EM_DVR_HDD_ERR,	// 41 DVR磁盘错误
	EM_HEART_TIMEOUT,	// 42 系统心跳超时
	EM_HEART_RESUME,	// 43 系统心跳恢复
	EM_VEDIO_OTHER,	// 44视频其他事件	
	EM_HEART,			// 心跳，非SG平台需要类型，用于本系统
	EM_TYPE_MAX,
}EM_MSG_TYPE;

typedef enum
{
	EM_SG_ALARM_SENSOR, 		//传感器报警
	EM_SG_ALARM_VMOTION, 		//移动侦测
	EM_SG_ALARM_VLOSS, 		//视频丢失
	EM_SG_ALARM_IPCCOVER,		//IPC遮挡
	EM_SG_ALARM_IPCEXT,		//IPC外部传感器
	EM_SG_ALARM_DISK_LOST,	//硬盘丢失
	EM_SG_ALARM_DISK_WRERR,		//硬盘读写错误
	EM_SG_ALARM_DISK_NONE,	//开机无硬盘
	EM_SG_ALARM_485EXT,		//485报警
} EM_ALM_TYPE;
/*
typedef struct
{
	EM_ALM_TYPE type;
	int status;
} ALM_INFO_TYPE;
//详情
typedef union
{
	ALM_INFO_TYPE alm_info;//报警通道
	char note[64];		//对应于SG上报信息中的Note
} DetailsType;
*/
typedef struct
{
	EM_MSG_TYPE	type;
	u32 chn;		//上传图像通道
	//ALM_INFO_TYPE alm_info;//报警通道
	char note[64];		//对应于SG上报信息中的Note
	//DetailsType details;
} SSG_MSG_TYPE;

s32 net_sg_init(PSNetCommCfg pCfg);
void upload_sg(SSG_MSG_TYPE *pmsg);
char *base64_encode_v1(const char* data, int data_len);
char *base64_decode_v1(const char *data, int data_len) ;
//时间必须作为参数，可以指定，因为要上传之前的预录图片
s32 upload_sg_proc(SSG_MSG_TYPE *pmsg, time_t time, void *SnapData, unsigned int DataSize, int GUID);



#ifdef __cplusplus
}
#endif

#endif // __NETCOMM_H_

