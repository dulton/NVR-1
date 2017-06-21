#ifndef _CAMERA_H_
#define _CAMERA_H_

//#include "iflytype.h"
#include "common_basetypes.h"

#define _hikvision_
#define _YT_CONTROL_OPT_

//摄像头的类型
#define CAM_TYPE_PELCO_D				0
#define CAM_TYPE_PELCO_P				1
#define CAM_TYPE_B01					2
#define CAM_TYPE_SAMSUNG				3

//pw 2010/7/23
#define CAM_TYPE_YAAN					4
#define CAM_TYPE_HY0					5

#define CAM_TYPE_jabsco_d				6
#define CAM_TYPE_PELCOD_2				7

#define CAM_TYPE_WV_CS850				8
#define CAM_TYPE_WV_CS650				9


//摄像头控制方式
#define CAM_CTRLWAY_GENERAL				0//正向
#define CAM_CTRLWAY_REVERSE				1//反向

//摄像头的控制命令
#define CAM_COMMAND_STOPALL				0//停止

#define CAM_COMMAND_MOVEUP				1//向上移动
#define CAM_COMMAND_MOVEDOWN			2//向下移动
#define CAM_COMMAND_MOVELEFT			3//向左移动
#define CAM_COMMAND_MOVERIGHT			4//向右移动

#define CAM_COMMAND_ZOOMTELE			5//拉近摄像头
#define CAM_COMMAND_ZOOMWIDE			6//拉远摄像头

#define CAM_COMMAND_FOCUSFAR			7//将焦距调远
#define CAM_COMMAND_FOCUSNEAR			8//将焦距调近

#define CAM_COMMAND_IRISOPEN			9//光圈+
#define CAM_COMMAND_IRISCLOSE			10//光圈-

#define CAM_COMMAND_SETPRESET			11//设置摄象头预置点
#define CAM_COMMAND_SHOTPRESET			12//访问摄象头预置点
#define CAM_COMMAND_CLRPRESET			13//删除摄象头预置点

#define CAM_COMMAND_STARTPATTERN		14//纪录轨迹开始
#define CAM_COMMAND_STOPPATTERN			15//纪录轨迹结束
#define CAM_COMMAND_RUNPATTERN			16//运行轨迹

#define CAM_COMMAND_STARTCRUISE			17//启动自动巡航
#define CAM_COMMAND_STOPCRUISE			18//停止自动巡航
#define CAM_COMMAND_INSERTCRUISEPOS		19//添加巡航点
#define CAM_COMMAND_DELETECRUISEPOS		20//删除巡航点
#define CAM_COMMAND_CLEARCRUISE			21//清除巡航路径
#define CAM_COMMAND_SHOWCRUISE			22//显示巡航路径

#define CAM_COMMAND_LIGHTOPEN			23//灯光控制使用
#define CAM_COMMAND_LIGHTCLOSE			24//灯光控制停用			

#define CAM_COMMAND_WIPEROPEN			25//雨刷控制使用
#define CAM_COMMAND_WIPERCLOSE			26//雨刷控制停用	

#define CAM_COMMAND_SETAUX				27//打开辅助功能
#define CAM_COMMAND_CLRAUX				28//清除辅助功能

#define CAM_COMMAND_ON					29//打开摄像头
#define CAM_COMMAND_0FF					30//关闭摄像头
#define CAM_COMMAND_REMOTERESET			31//远程复位
#define CAM_COMMAND_MOVEHOME			32//回归
#define CAM_COMMAND_FLIP				33//翻转180度

#define CAM_COMMAND_AUTOFOCUS			34//自动聚焦
#define CAM_COMMAND_MANUALFOCUS			35//手动聚焦
#define CAM_COMMAND_AUTOIRIS			36//自动调节光圈
#define CAM_COMMAND_MANUALIRIS			37//手动调节光圈
#define CAM_COMMAND_DIGITALZOOMOPEN		38//数字变倍开
#define CAM_COMMAND_DIGITALZOOMCLOSE	39//数字变倍关
#define CAM_COMMAND_WHITEBALANCE		40//白平衡模式
#define CAM_COMMAND_RAYREDEEMOPEN		41//背光模式开
#define CAM_COMMAND_RAYREDEEMCLOSE		42//背光模式关

#define CAM_COMMAND_ZOOMSPEED			43//设置变焦速度
#define CAM_COMMAND_FOCUSSPEED			44//设置聚焦速度

#define CAM_COMMAND_STARTLINESCAN		45//开始线扫
#define CAM_COMMAND_STOPLINESCAN		46//停止线扫
#define CAM_COMMAND_SETLINESCAN			47//启动线扫

#define CAM_COMMAND_AUTOSCAN			48//自动扫描
//错误类型
#define CAM_SUCCESS						0//成功
#define CAM_ERROR_PARAM					1//参数错误
#define CAM_ERROR_TYPE					2//不支持的摄像头类型
#define CAM_ERROR_PROTOCOL_TYPE			3//不支持的摄像头协议类型
#define CAM_ERROR_PROTOCOL_PARAM		4//不支持的摄像头协议参数
#define CAM_ERROR_NOACK					5//摄像头没有回应消息
#define CAM_ERROR_NORESPONE				6//摄像头没有响应
#define CAM_ERROR_UNKNOWN				7//未知错误

//白平衡模式
#define WHITE_BALANCE_MANUAL			0//白平衡手动模式
#define WHITE_BALANCE_AUTO				1//白平衡自动模式
#define WHITE_BALANCE_INSIDE			2//白平衡室内模式
#define WHITE_BALANCE_OUTSIDE			3//白平衡室外模式
#define WHITE_BALANCE_LOCK				4//白平衡锁定模式
#define WHITE_BALANCE_ATW				5//白平衡ATW模式

//命令参数范围
#define MAX_PAN_SPEED					15//pan speed 0-MAX_PAN_SPEED
#define MAX_TILT_SPEED					15//tilt speed 0-MAX_TILT_SPEED
#define MAX_ZOOM_SPEED					15//zoom speed 0-MAX_ZOOM_SPEED
#define MAX_FOCUS_SPEED					15//focus speed 0-MAX_FOCUS_SPEED
#define MAX_PRESET_NUM					128//preset limit 0-MAX_PRESET_NUM
#define MAX_CRUISE_PATH_NUM				16//每个云台最多有MAX_CRUISE_PATH_NUM条巡航路径
#define MAX_CRUISE_POS_NUM				16//每条巡航路径最多有MAX_CRUISE_POS_NUM个巡航点
#define MAX_CRUISE_SPEED				9//最大巡航速度
#define MAX_DWELL_TIME					255//巡航点最大驻留时间

#define MAX_PROTOCOL_LEN				256//协议的最大长度
#define MAX_PARAM_NUM					72//最大参数个数

#define STOP_TYPE_POS					0
#define PAN_SPEED_POS					0
#define TILT_SPEED_POS					1
#define ZOOM_SPEED_POS					1
#define FOCUS_SPEED_POS					1
#define AUX_INDEX_POS					1
#define LIGHT_POS						1
#define WIPER_POS						1
#define WHITE_BALANCE_POS				1

#define CRUISE_PATH_POS					0

#define CRUISE_INDEX_POS				0
#define PRESET_INDEX_POS				1
#define DWELL_TIME_POS					2
#define CRUISE_SPEED_POS				3

#define LINE_SCAN_START_POS				0
#define LINE_SCAN_END_POS				1
#define PATTERN_POS						0
#define SENDER_ADDR_POS					(MAX_PARAM_NUM-1)

//Note:巡航路径号、预置点、摄像头地址都从1开始

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u8	m_byType;				//摄像头的类型
	u32 m_dwId;					//摄像头的地址
	u8	m_byCmdId;				//命令类型
    u8	m_abyParam[MAX_PARAM_NUM];//参数
	u8	m_byCtrlWay;			//摄象头正反向控制方式
}TCamInfo;
typedef struct
{
	u32 m_dwLen;					//协议的实际长度
	u8	m_abyData[MAX_PROTOCOL_LEN];//协议内容
	u32 m_dwInterval;
}TCamData;

u16 GetCamProtocol(TCamInfo *ptCamInfo,TCamData *ptCamData);

#ifdef __cplusplus
}
#endif

#endif
