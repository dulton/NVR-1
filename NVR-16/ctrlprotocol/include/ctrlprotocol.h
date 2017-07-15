#ifndef _CTRLPROTOCOL_H_
#define _CTRLPROTOCOL_H_

#include "iflytype.h"
//#include "common_basetypes.h"
#include "camera.h"
#include "common.h"
//#include "public.h"



#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#endif

#ifdef SHENSHIYIN
#define _TOUMING_COM_//csp modify 20100311
#endif

#define IECTRL_NEW

//zlb20090916 begin
#define EMAIL_ALARM_TYPE_MD 0
#define EMAIL_ALARM_TYPE_INPUT 1
#define EMAIL_ALARM_TYPE_EXCEPTION 2
#define EMAIL_ALARM_TYPE_VL 3
#define EMAIL_ALARM_TYPE_VB 4
#define MAX_EMAIL_LENGTH 32
//zlb20090916 end

#define VERSION_OF_9508

//pw 2010/7/26
#if (defined(TL_9508) || defined(TL_9504))
	#if (defined(TIANMIN) && defined(TL_9508))
		#define MAX_ALARM_IN_NUM		8
		#define MAX_ALARM_OUT_NUM		1 	// 4	
	#elif defined(SHENSHIYIN_SP1) && defined(TL_9508)
		#define MAX_ALARM_IN_NUM		8
		#define MAX_ALARM_OUT_NUM	4
	#elif defined(SHENSHIYIN_SP1) && (defined(TL_9504_CIF_8M) || defined(TL_9504_D1_8M))
		#define MAX_ALARM_IN_NUM		4
		#define MAX_ALARM_OUT_NUM	4
	#else
		#define MAX_ALARM_IN_NUM		4
		#define MAX_ALARM_OUT_NUM		1
	#endif
#else
	#define MAX_ALARM_IN_NUM	8 // 16			//fgpa:16 6446:4
	#define MAX_ALARM_OUT_NUM	2 // 4			//fpga:4 6446:4
#endif

#define SPECIAL_DEV_INFO
#define CRUISE_POINT_NUM		128

#ifdef WIN32
#define SOCKHANDLE		SOCKET
#else
#define SOCKHANDLE		int
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#define INFINITE	0xffffffff
#endif

//extern u16 server_port;//wrchen 090907
#define CTRL_PROTOCOL_SERVERPORT		8670					//服务器端口//wrchen 090205
#define CTRL_PROTOCOL_MAXLINKNUM		256

#define CTRL_PROTOCOL_CONNECT_BLOCK		INFINITE				//连接方式:阻塞
#define CTRL_PROTOCOL_CONNECT_DEFAULT	3000					//缺省连接时间:3秒

#define CTRL_VERSION					0x0100

#define CTRL_COMMAND					0
#define CTRL_NOTIFY						1
#define CTRL_ACK						2

//命令类型
#define CTRL_CMD_BASE					10000
#define CTRL_CMD_LOGIN					CTRL_CMD_BASE+1			//远程登录命令
#define CTRL_CMD_LOGOFF					CTRL_CMD_BASE+2			//注销用户登录
#define CTRL_CMD_GETDEVICEINFO			CTRL_CMD_BASE+3			//获得设备信息
#define CTRL_CMD_GETVIDEOPROPERTY		CTRL_CMD_BASE+4			//视频属性信息
#define CTRL_CMD_GETAUDIOPROPERTY		CTRL_CMD_BASE+5			//音频属性信息
#define CTRL_CMD_GETVOIPPROPERTY		CTRL_CMD_BASE+6			//voip属性信息
#define CTRL_CMD_GETMDPROPERTY			CTRL_CMD_BASE+7			//移动侦测属性信息
#define CTRL_CMD_STOPVIDEOMONITOR		CTRL_CMD_BASE+8			//停止视频预览
#define CTRL_CMD_STOPAUDIOMONITOR		CTRL_CMD_BASE+9			//停止音频预览
#define CTRL_CMD_STOPVOIP				CTRL_CMD_BASE+10		//停止voip
#define CTRL_CMD_STOPDOWNLOAD			CTRL_CMD_BASE+11		//停止下载
#define CTRL_CMD_STOPFILEPLAY			CTRL_CMD_BASE+12		//停止文件回放
#define CTRL_CMD_STOPTIMEPLAY			CTRL_CMD_BASE+13		//停止时间回放
#define CTRL_CMD_FASTPLAY				CTRL_CMD_BASE+14		//快速播放
#define CTRL_CMD_SLOWPLAY				CTRL_CMD_BASE+15		//慢速播放
#define CTRL_CMD_SETPLAYRATE			CTRL_CMD_BASE+16		//播放速率
#define CTRL_CMD_PAUSEPLAY				CTRL_CMD_BASE+17		//暂停播放
#define CTRL_CMD_RESUMEPLAY				CTRL_CMD_BASE+18		//恢复播放
#define CTRL_CMD_SINGLEPLAY				CTRL_CMD_BASE+19		//帧进
#define CTRL_CMD_FASTBACKPLAY			CTRL_CMD_BASE+20		//快退
#define CTRL_CMD_PLAYPREV				CTRL_CMD_BASE+21		//上一段
#define CTRL_CMD_PLAYNEXT				CTRL_CMD_BASE+22		//下一段
#define CTRL_CMD_PLAYSEEK				CTRL_CMD_BASE+23		//定位
#define CTRL_CMD_PLAYMUTE				CTRL_CMD_BASE+24		//回放静音
#define CTRL_CMD_PLAYPROGRESS 			CTRL_CMD_BASE+25		//播放进度
#define CTRL_CMD_GETNETWORK				CTRL_CMD_BASE+26		//获得网络参数
#define CTRL_CMD_SETNETWORK				CTRL_CMD_BASE+27		//设置网络参数
#define CTRL_CMD_GETVGASOLLIST			CTRL_CMD_BASE+28		//获得VGA分辨率
#define CTRL_CMD_GETSYSPARAM			CTRL_CMD_BASE+29		//获得系统参数
#define CTRL_CMD_SETSYSPARAM 			CTRL_CMD_BASE+30		//设置系统参数
#define CTRL_CMD_GETRECPARAM			CTRL_CMD_BASE+31		//获得录像参数
#define CTRL_CMD_SETRECPARAM 			CTRL_CMD_BASE+32		//设置录像参数
#define	CTRL_CMD_GETSUBSTREAMPARAM		CTRL_CMD_BASE+33		//获得子码流参数
#define	CTRL_CMD_SETSUBSTREAMPARAM		CTRL_CMD_BASE+34		//设置子码流参数
#define CTRL_CMD_GETIMGPARAM			CTRL_CMD_BASE+35		//获得图像参数
#define CTRL_CMD_SETIMGPARAM 			CTRL_CMD_BASE+36		//设置图像参数
#define CTRL_CMD_GETPICADJ 				CTRL_CMD_BASE+37		//获得画面参数
#define CTRL_CMD_SETPICADJ 				CTRL_CMD_BASE+38		//设置画面参数
#define CTRL_CMD_GETALARMINPARAM 		CTRL_CMD_BASE+39		//获得报警输入参数
#define CTRL_CMD_SETALARMINPARAM 		CTRL_CMD_BASE+40		//设置报警输入参数
#define CTRL_CMD_GETALARMOUTPARAM 		CTRL_CMD_BASE+41		//获得报警输出参数
#define CTRL_CMD_SETALARMOUTPARAM 		CTRL_CMD_BASE+42		//设置报警输出参数
#define	CTRL_CMD_GETALARMNOTICYPARAM	CTRL_CMD_BASE+43		//获得报警通知参数
#define	CTRL_CMD_SETALARMNOTICYPARAM	CTRL_CMD_BASE+44		//设置报警通知参数
#define CTRL_CMD_GETRECSCHPARAM 		CTRL_CMD_BASE+45		//获得录像布防参数
#define CTRL_CMD_SETRECSCHPARAM 		CTRL_CMD_BASE+46		//设置录像布防参数
#define CTRL_CMD_GETMDPARAM 			CTRL_CMD_BASE+47		//获得移动侦测参数
#define CTRL_CMD_SETMDPARAM 			CTRL_CMD_BASE+48		//设置移动侦测参数
#define CTRL_CMD_GETVIDEOLOSTPARAM		CTRL_CMD_BASE+49		//获得视频丢失参数
#define CTRL_CMD_SETVIDEOLOSTPARAM 		CTRL_CMD_BASE+50		//设置视频丢失参数
#define CTRL_CMD_GETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+51		//获得视频遮挡参数
#define CTRL_CMD_SETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+52		//设置视频遮挡参数
#define CTRL_CMD_GETPTZPARAM			CTRL_CMD_BASE+53		//获得云台参数
#define CTRL_CMD_SETPTZPARAM 			CTRL_CMD_BASE+54		//设置云台参数
#define CTRL_CMD_SETPRESET 				CTRL_CMD_BASE+55		//设置预置点参数
#define CTRL_CMD_GETCRUISEPARAM 		CTRL_CMD_BASE+56		//获得巡航路径参数
#define CTRL_CMD_SETCRUISEPARAM 		CTRL_CMD_BASE+57		//设置巡航路径参数
#define CTRL_CMD_CTRLCRUISEPATH 		CTRL_CMD_BASE+58		//巡航路径控制
#define CTRL_CMD_CTRLPTZTRACK 			CTRL_CMD_BASE+59		//轨迹控制
#define CTRL_CMD_GETHDDINFO 			CTRL_CMD_BASE+60		//获得磁盘信息
#define CTRL_CMD_GETUSERINFO			CTRL_CMD_BASE+61		//获得用户信息
#define CTRL_CMD_SETUSERINFO 			CTRL_CMD_BASE+62		//设置用户信息
#define CTRL_CMD_SETRESTORE				CTRL_CMD_BASE+63		//恢复默认
#define CTRL_CMD_CLEARALARM				CTRL_CMD_BASE+64		//清除报警
#define CTRL_CMD_GETSYSTIME				CTRL_CMD_BASE+65		//获得系统时间
#define CTRL_CMD_SETSYSTIME				CTRL_CMD_BASE+66		//设置系统时间
#define CTRL_CMD_GETSYSINFO				CTRL_CMD_BASE+67		//获得系统信息
#define CTRL_CMD_SHUTDOWN				CTRL_CMD_BASE+68		//关闭系统
#define CTRL_CMD_REBOOT					CTRL_CMD_BASE+69		//重启系统
#define CTRL_CMD_PTZCTRL				CTRL_CMD_BASE+70		//云台控制
#define CTRL_CMD_GETMANUALREC			CTRL_CMD_BASE+71		//获得手动录像状态
#define CTRL_CMD_SETMANUALREC			CTRL_CMD_BASE+72		//设置手动录像状态
#define CTRL_CMD_LOGSEARCH				CTRL_CMD_BASE+73		//日志文件搜索
#define CTRL_CMD_RECFILESEARCH			CTRL_CMD_BASE+74		//回放文件搜索
#define CTRL_CMD_GETSPSPPS				CTRL_CMD_BASE+75		//获得spspps数据
#define CTRL_CMD_ALARMUPLOADCENTER		CTRL_CMD_BASE+76		//报警上传中心
#ifdef _TOUMING_COM_
#define CTRL_CMD_SERIALSTOP 			CTRL_CMD_BASE+80 		//停止透明串口//csp modify 20100311
#define CTRL_CMD_COM_PROTOCOL_GET 		CTRL_CMD_BASE+78 			//获得串口参数
#define CTRL_CMD_COM_PROTOCOL_SET		CTRL_CMD_BASE+79 			//设置串口参数
#endif
#define CTRL_CMD_GETSYSLANGLIST   		CTRL_CMD_BASE+83  		//获得系统语言列表				//dong	100426
#define CTRL_CMD_GETBITRATELIST 		CTRL_CMD_BASE+84 		//获得码流位率列表 
#define CTRL_CMD_GETEMAILSMTP 			CTRL_CMD_BASE+88 		//获得报警EmailSMTP信息		EMAIL_OPT
#define CTRL_CMD_SETEMAILSMTP 			CTRL_CMD_BASE+89 		//设置报警EmailSMTP信息		EMAIL_OPT

//_DAYTIME_
#define CTRL_CMD_USEUTCTIME				CTRL_CMD_BASE+93 		//是否使用UTC格式时间 

#ifdef IECTRL_NEW
#define CTRL_CMD_RESETPICADJUST 		CTRL_CMD_BASE+97 				//恢复默认画面参数
#define CTRL_CMD_GETFRAMERATELIST 		CTRL_CMD_BASE+98 				//获得帧率列表
#define CTRL_CMD_GETMAX_IMGMASKNUM 		CTRL_CMD_BASE+99 			//获得支持的最大遮盖数
#define CTRL_GET_RESOLUTION_LIST 		CTRL_CMD_BASE+100 				//获得主/子码流分辨率列表
#endif

#ifdef SPECIAL_DEV_INFO
#define CTRL_CMD_GET_SPECIALDEVICEINFO 	CTRL_CMD_BASE+107 //获得特殊的系统信息，用于某些信号的兼容
#endif

#define CTRL_CMD_GETMDSENSELIST 		CTRL_CMD_BASE+129 //获取移动侦测灵敏度列表
#define CTRL_CMD_GETMDALARMDELAYLIST  	CTRL_CMD_BASE+130 //获取移动侦测报警输出延时列表
#define CTRL_CMD_GETBAUDRATELIST 		CTRL_CMD_BASE+131 //获取波特率列表
#define CTRL_CMD_GETPTZPROTOCOLLIST 	CTRL_CMD_BASE+132 //获取云台协议列表

#define CTRL_CMD_GETRECSCHPARAMBYTYPE  	CTRL_CMD_BASE+133 //按布防类型获得录像布防参数
#define CTRL_CMD_SETRECSCHPARAMBYTYPE 	CTRL_CMD_BASE+134 //按布防类型分开设置录像布防参数
#define CTRL_CMD_RECFILESEARCH2 		CTRL_CMD_BASE+139 //2.0回放文件搜索 

#define CTRL_CMD_GETDDNSLIST			CTRL_CMD_BASE+152//获取DDNS服务商列表

//csp modify 20130423
#define CTRL_CMD_SETMONITORINFO			CTRL_CMD_BASE+160//设置远程监控信息

//csp modify 20130519
#define CTRL_CMD_GETADVPRIVILEGE		CTRL_CMD_BASE+161//获取用户高级权限
//xdc
#define CTRL_CMD_GETSEACHIPCLIST		CTRL_CMD_BASE+162		//获取搜索的摄像头列表
#define CTRL_CMD_GETADDIPCLIST			CTRL_CMD_BASE+163		//获取已添加的摄像头列表
#define CTRL_CMD_SETIPC				CTRL_CMD_BASE+164		//设置对应通道摄像头
#define CTRL_CMD_ADDIPC				CTRL_CMD_BASE+165		//添加摄像头
#define CTRL_CMD_DELETEIPC			CTRL_CMD_BASE+166		//删除摄像头
#define CTRL_CMD_GETTHEOTHER		CTRL_CMD_BASE+167		//获取搜索的摄像头剩下的列表,分批获取

//yaogang modify 20141030
#define CTRL_CMD_GET_PATROL_PARA		CTRL_CMD_BASE+168		//获取轮巡参数
#define CTRL_CMD_SET_PATROL_PARA		CTRL_CMD_BASE+169		//设置轮巡参数
#define CTRL_CMD_GET_PREVIEW_PARA		CTRL_CMD_BASE+170		//获取预览参数
#define CTRL_CMD_SET_PREVIEW_PARA		CTRL_CMD_BASE+171		//设置预览参数
#define CTRL_CMD_GET_CHN_PARA		CTRL_CMD_BASE+172		//获取通道参数
#define CTRL_CMD_SET_CHN_PARA		CTRL_CMD_BASE+173		//设置通道参数
#define CTRL_CMD_CLEAN_ALARM_ICON		CTRL_CMD_BASE+174		//清除桌面报警三角标
#define CTRL_CMD_CLOSE_GUIDE		CTRL_CMD_BASE+175		//跃天解码板CMS remote CTRL
//yaogang modify 20160122
#define CTRL_CMD_GET_IPCCHN_LINKSTATUS	CTRL_CMD_BASE+176		//获取IPC通道连接状态(一个IPC有两个通道，主、子码流)
typedef struct
{
	unsigned char max_chn_num;//最大通道数
	unsigned char chn_status[20];//一个bit对应一个通道，共160通道
}PACK_NO_PADDING ifly_ipc_chn_status_t;


typedef struct  
{
	u8 chn; //通道
	u8 weekday; //星期
	u8 type;
	struct RecSchTimeField_t
	{
		u32 starttime; //起始时间
		u32 endtime; //终止时间
		u8 reserved[6]; //保留字段
	} PACK_NO_PADDING TimeFiled[24];
    u8 copy2Weekday; //复制到其他天  按位 
    u32 copy2Chnmask; //复制到其他通道。按位
	u8 reserved[16]; //保留字段
} PACK_NO_PADDING ifly_RecSchTime_t;

enum PROTOCOLTYPE {
	NETDVR_PROTOCOLTYPE_PELCO_D = 0, //protocol type : Pelco-d
	NETDVR_PROTOCOLTYPE_PELCO_P = 1, //protocol type : Pelco-p
	NETDVR_PROTOCOLTYPE_B01 = 2, //protocol type : B01
	NETDVR_PROTOCOLTYPE_SAMSUNG = 3, //protocol type : Samsung
	NETDVR_PROTOCOLTYPE_HY = 4,
	NETDVR_PROTOCOLTYPE_Panasonic = 5,
	NETDVR_PROTOCOLTYPE_Pelco_9750 = 6,
	NETDVR_PROTOCOLTYPE_PelcoD1 = 7,
	NETDVR_PROTOCOLTYPE_PelcoD_S1 = 8,
	NETDVR_PROTOCOLTYPE_PelcoP1 = 9,
	NETDVR_PROTOCOLTYPE_PelcoP5 = 10,
	NETDVR_PROTOCOLTYPE_Philips = 11,
	NETDVR_PROTOCOLTYPE_Sanli = 12,
	NETDVR_PROTOCOLTYPE_Santachi =13,
	NETDVR_PROTOCOLTYPE_Sharp = 14,
	NETDVR_PROTOCOLTYPE_Sony = 15,
	NETDVR_PROTOCOLTYPE_Yaan = 16, 
};

typedef struct  
{
	u8 maxNum;
	u8 list[19]; //云台协议列表
} ifly_PTZProtocolList_t;

typedef struct  
{
	u8 mdsenselist[12]; //移动侦测灵敏度列表
}ifly_MDSenselist_t;
 
typedef struct  
{
	u16 mdalarmdelaylist[16]; //移动侦测报警输出延时列表
}ifly_MDAlarmDelaylist_t;
 
typedef struct  
{
	int baudratelist[16]; //波特率列表
}ifly_BaudRateList_t;

#define CTRL_CMD_PPPOE_PREUP			CTRL_CMD_BASE+9999
#define CTRL_CMD_PPPOE_UP				CTRL_CMD_BASE+9998
#define CTRL_CMD_PPPOE_DOWN				CTRL_CMD_BASE+9997
#define CTRL_CMD_PPPOE_DISCONNECT		CTRL_CMD_BASE+9996

#define CTRL_CMD_DHCP_DECONFIG			CTRL_CMD_BASE+9989
#define CTRL_CMD_DHCP_BOUND				CTRL_CMD_BASE+9988
#define CTRL_CMD_DHCP_RENEW				CTRL_CMD_BASE+9987
#define CTRL_CMD_DHCP_NAK				CTRL_CMD_BASE+9986




//事件通知类型
#define CTRL_NOTIFY_BASE				20000
#define	CTRL_NOTIFY_CONNLOST			CTRL_NOTIFY_BASE+0		//断链消息
#define	CTRL_NOTIFY_HEARTBEAT_REQ		CTRL_NOTIFY_BASE+1		//心跳请求
#define	CTRL_NOTIFY_HEARTBEAT_RESP		CTRL_NOTIFY_BASE+2		//心跳回复
#define CTRL_NOTIFY_PLAYEND				CTRL_NOTIFY_BASE+3		//放像结束
#define CTRL_NOTIFY_PLAYPROGRESS		CTRL_NOTIFY_BASE+4		//放像进度通知
#define CTRL_NOTIFY_HASAUDIO			CTRL_NOTIFY_BASE+5		//回放文件是否有音频
#define CTRL_NOTIFY_UPDATEPROGRESS		CTRL_NOTIFY_BASE+6		//升级进度
#define CTRL_NOTIFY_ALARMINFO			CTRL_NOTIFY_BASE+7		//异步报警信息

//应答类型
#define CTRL_SUCCESS					0						//成功
#define CTRL_FAILED_BASE				30000					//错误码偏移量
#define CTRL_FAILED_USER				CTRL_FAILED_BASE+1		//不存在的用户名
#define CTRL_FAILED_PASSWORD			CTRL_FAILED_BASE+2		//密码错误
#define CTRL_FAILED_COMMAND				CTRL_FAILED_BASE+3		//未认可的命令
#define CTRL_FAILED_PARAM				CTRL_FAILED_BASE+4		//无效参数
#define CTRL_FAILED_OUTOFMEMORY			CTRL_FAILED_BASE+5		//内存不足
#define CTRL_FAILED_RESOURCE			CTRL_FAILED_BASE+6		//资源不足
#define CTRL_FAILED_FILENOTEXIST		CTRL_FAILED_BASE+7		//文件不存在
#define CTRL_FAILED_DATABASE			CTRL_FAILED_BASE+8		//数据库错误
#define CTRL_FAILED_RELOGIN				CTRL_FAILED_BASE+9		//重复登录
#define CTRL_FAILED_BAUDLIMIT			CTRL_FAILED_BASE+10		//每一路通道最多支持5路实时监控			
#define CTRL_FAILED_CREATESOCKET		CTRL_FAILED_BASE+11		//创建套结字失败
#define CTRL_FAILED_CONNECT				CTRL_FAILED_BASE+12		//网络连接失败
#define CTRL_FAILED_BIND				CTRL_FAILED_BASE+13		//绑定失败
#define CTRL_FAILED_LISTEN				CTRL_FAILED_BASE+14		//侦听失败
#define CTRL_FAILED_NETSND				CTRL_FAILED_BASE+15		//网络发送出错
#define CTRL_FAILED_NETRCV				CTRL_FAILED_BASE+16		//网络接收出错
#define CTRL_FAILED_TIMEOUT				CTRL_FAILED_BASE+17		//网络连接超时
#define CTRL_FAILED_CHNERROR			CTRL_FAILED_BASE+18		//超出通道限制
#define CTRL_FAILED_DEVICEBUSY			CTRL_FAILED_BASE+19		//设备正在忙
#define CTRL_FAILED_WRITEFLASH			CTRL_FAILED_BASE+20		//烧写flash出错
#define CTRL_FAILED_VERIFY				CTRL_FAILED_BASE+21		//校验错
#define CTRL_FAILED_CONFLICT			CTRL_FAILED_BASE+22		//系统资源冲突
#define CTRL_FAILED_BUSY				CTRL_FAILED_BASE+23		//系统正在忙
#define CTRL_FAILED_LINKLIMIT			CTRL_FAILED_BASE+24		//已达到连接上限
#define CTRL_FAILED_USER_SAME			CTRL_FAILED_BASE+25		//用户名相同07-08-02
#define CTRL_FAILED_MACADDR				CTRL_FAILED_BASE+26		//远程访问的pc mac地址错误
#define CTRL_FAILED_NOINIT				CTRL_FAILED_BASE+27		//模块未初始化
#define CTRL_FAILED_USER_MAX			CTRL_FAILED_BASE+28		//用户数最多//wrchen 080529
#define CTRL_FAILED_UNKNOWN				CTRL_FAILED_BASE+9999	//未知错误
//连接
#define CTRL_CONNECTION_NULL			0x0
#define CTRL_CONNECTION_TCPCLIENT		0x1
#define CTRL_CONNECTION_TCPSERVER		0x2

//组播搜索设备
#define CTRL_DEVICESEARCH_ACKCLIENT		0X1
#define CTRL_DEVICESEARCH_ACKSERVER		0x2
#define SEARCHPORT						6666
#define ACKSEARCHPORT					SEARCHPORT+1
#define MULTICASTGROUP					"224.0.1.2"

//码流传输
#define VIDEO_CHANNELS			16
#define AUDIO_CHANNELS			16
#define EACH_STREAM_TCP_LINKS	5

#define MAX_REMOTE_PLAYER_NUM	4//8//csp modify 20121018

#define VOIP_NUM				1
#define DOWNLOAD_NUM			1
#define	UPDATE_NUM				1
#define MEDIA_LINKMAX_SVR		(VIDEO_CHANNELS+AUDIO_CHANNELS)*(EACH_STREAM_TCP_LINKS)+MAX_REMOTE_PLAYER_NUM+VOIP_NUM
#define	STREAM_LINK_MAXNUM		MEDIA_LINKMAX_SVR+DOWNLOAD_NUM+UPDATE_NUM

//2010/01/06/
//   ***************************************************************
//说明
//恢复默认画面参数
//客户端发送CTRL_CMD_RESETPICADJUST+ifly_reset_picadjust_t;

//获得帧率列表
//客户端发送CTRL_CMD_GETFRAMERATELIST+chn（1字节）+type（1字节），设备端返回ifly_framerate_list_t

//获得支持的最大遮盖数
//客户端发送CTRL_CMD_GETMAX_IMGMASKNUM，设备端返回数目（1字节，取值 <=4）

//获得主/子码流分辨率列表
//客户端CTRL_GET_RESOLUTION_LIST +chn（1字节）+type（1字节），设备返回ifly_videoresolu_list_t

//恢复默认画面参数　CTRL_CMD_RESETPICADJUST
typedef struct  
{
	u8 chn;  //通道 0开始
	u8 adjtype; //按位表示类型，第1-4位分别表示亮度，对比度，色调，饱和度，置为1的表示恢复默认值， 0xf表示全恢复
}ifly_reset_picadjust_t;

//帧率列表 CTRL_CMD_GETFRAMERATELIST
typedef struct
{
	u8 chn; //通道
	u8 type; // 0 主码流 1 子码流
	u8 framerate[10]; //帧率列表 未使用的填0
}ifly_framerate_list_t;

//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
#define VIDEORESOLU_BEGIN 1
#define VIDEORESOLU_QCIF 1
#define VIDEORESOLU_CIF 2
#define VIDEORESOLU_HD1 3
#define VIDEORESOLU_D1 4 // 20111014 by lanston, org:3
#define VIDEORESOLU_END VIDEORESOLU_D1

typedef struct  
{
	u8 chn; //通道
	u8 type; // 0 主码流 1 子码流
	u8 videoresolu[8]; //分辨率列表 未使用的填0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
} PACK_NO_PADDING ifly_videoresolu_list_t;


//  ********************************************************************
//#pragma pack( push, 1 )

typedef struct
{
	u8 bUseUTC;
	u8 reserved[16]; //预留
}PACK_NO_PADDING ifly_utctime_t;//是否使用UTC格式时间 


//录像布防

typedef struct
{
	u8	data[6];
}PACK_NO_PADDING CPGuid;

typedef struct
{
	u32         ip;
	u16			port;
	CPGuid		guid;
	SOCKHANDLE	sockfd;
	u8			conntype;
	u8          newmsgcome;
	u8          nolnkcount;
	u8 reserve;
	//yaogang modify for server heart beat check
	pthread_mutex_t lock;
	time_t last_msg_time;//上次通信时间，命令或心跳回应都可以
}PACK_NO_PADDING ifly_cp_t,*CPHandle;



typedef struct
{
	u32		length;						//消息长度		
	u16		type;						//消息类型		
	u16		event;						//消息名		
	u16		number;						//流水号		
	u16		version;					//版本号		
}PACK_NO_PADDING ifly_cp_header_t;

//协议头结构
typedef struct 
{
	u8	safeguid[16];					// PROTOGUID
	u8  protocolver;					//协议版本
	u8	byConnType;						//连接类型，0x1：控制信令；0x2：码流传输；0x3：广播搜索0x4  轮巡同步(只对已经使能轮巡的设备有效)
	u8	reserved[2];					//保留字段
}PACK_NO_PADDING ifly_ProtocolHead_t;


//设备信息
typedef struct 
{
	u32	deviceIP; 						//设备IP  
	u16	devicePort;						//设备端口 
	char device_name[32];				//设备名称
	char device_mode[32];				//设备型号
	u8	maxChnNum;						//最大通道数
	u8	maxAduioNum;					//最大音频数
	u8	maxSubstreamNum;				//最大子码流数
	u8	maxPlaybackNum;					//最大回放数
	u8	maxAlarmInNum;					//最大报警输入数
	u8	maxAlarmOutNum;					//最大报警输出数
	u8	maxHddNum;						//最大硬盘数
	u8 	nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	u8	reserved[15];					//预留
}PACK_NO_PADDING ifly_DeviceInfo_t;


//视频属性
typedef struct  
{
	u8	videoEncType;					//视频编码类型
	u16	max_videowidth;					//
	u16	max_videoheight;					
	u8	reserved[3];					//预留
}PACK_NO_PADDING ifly_Video_Property_t;

//音频属性
typedef struct  
{
	u8	audioEnctype;					//预览音频编码类型
	u8	audioBitPerSample;				//预览音频位数
	u16 audioSamplePerSec;				//预览音频采样率
	u16 audioFrameSize;					//预览音频每帧大小
	u16	audioFrameDurTime;				//预览音频每帧间隔
	u8	reserved[4];					//预留
}PACK_NO_PADDING ifly_Audio_Property_t;

//VOIP属性
typedef struct  
{
	u8	VOIPBitPerSample;				//语音对讲位数
	u16 VOIPSamplePerSec;				//语音对讲采样率
	u16 VOIPFrameSize;					//语音对讲每帧大小
	u8	reserved[3];					//预留
}PACK_NO_PADDING ifly_VOIP_Property_t;

//移动侦测属性
typedef struct  
{
	u8	MDCol;							//移动侦测列数
	u8	MDRow;							//移动侦测行数
	u8	reserved[4];					//预留
}PACK_NO_PADDING ifly_MD_Property_t;

#ifdef _TOUMING_COM_
//透明串口发送数据头
typedef struct  
{
	u8	channel; //串口通道，使用485串口时有效，从1开始；232串口作为透明通道时该值设置为0
	u32 	dwDataSize; //发送的数据长度
	u8 	reserved[16]; //预留
}PACK_NO_PADDING ifly_serial_hdr;

typedef struct
{
	u8 com_protocol; //协议编号 0:N5,1:D4,2:S8,3:S7,4:F2,5:G2E,6:G2N,7:CR,8:TY,9:YH
	u32 com_baudrate; //波特率
	u8 com_checkbit; //校验位 0:NONE,1:奇校验，2:偶校验
	u8 com_databit; //数据位 8，7，6
	u8 com_stopbit; //停止位 1，2
	u8 serialport;// 1-232, 2-485
	u8 reserved[23]; //保留
}ifly_com_param_t;
#endif

//码流传输请求结构
typedef struct 
{
	u8 command;							//0：预览 1：文件回放 2：时间回放 3：文件下载 4：升级 5 VOIP
	union
	{
		struct
		{
			u8		chn;				//预览通道
			u8		type;				//0:主码流视频 1：主码流音频 2：子码流视频
		}PACK_NO_PADDING Monitor_t;						//预览 command = 0
		
		struct
		{
			char	filename[64];		//回放的文件
			u32		offset;				//文件偏移
		}PACK_NO_PADDING FilePlayBack_t; 				//文件回放 command = 1
		
		struct
		{
			u8		channel;			//通道数
			u16		type;				//类型
			u32		start_time;			//开始时间
			u32		end_time;			//终止时间
		}PACK_NO_PADDING TimePlayBack_t ;				//时间回放 command = 2
		
		struct
		{
			char	filename[64];		//下载的文件
			u32		offset;				//文件偏移
			u32		size;				//文件大小
		}PACK_NO_PADDING FileDownLoad_t;				//文件下载 command = 3
		
		struct
		{
			u32		size;				//文件长度
			u32		verify;				//校验和
			u16		version;			//文件版本号
			u16		updateType;			//0:主板升级 1：面板升级, 2: 开机LOGO, 3: APP LOGO
		}PACK_NO_PADDING Update_t;						//远程升级  command = 4；
		
		//VOIP command
		u8 voipreserved;				//VOIP预留
	#ifdef _TOUMING_COM_
		//透明通道
		u8 SerialPort; //透明通道 1-232串口  2-485串口 
	#endif
		
		//远程格式化硬盘
		u8 formatHddIndex; //要格式化的硬盘号 0开始
		struct
		{
			u16 chnMask; //通道数 按位
			u16 type; //类型
			u32 start_time; //开始时间
			u32 end_time; //终止时间
		}PACK_NO_PADDING MultiTimePlayBack_t ; //多路时间回放 command = 11
	};
	u8	reserved[7];					//预留	
}PACK_NO_PADDING ifly_TCP_Stream_Req;					 


//码流传输主机端回应结构
typedef struct
{
	u32	errcode;						//连接成功返回0， 否则返回其他错误码
	u32	ackid;							//对于文件回放，时间回放。 用于后续客户端控制回放操作和区分进度, 预览控制
}PACK_NO_PADDING ifly_TCP_Stream_Ack;

typedef struct  
{
	u16 errcode;
	u8 pos;
}PACK_NO_PADDING ifly_TCP_Pos;

//预览，回放和 VOIP帧结构
typedef struct
{
	u8	m_byMediaType;					//类型
	u8	m_byFrameRate;					//帧率
	u8	m_bKeyFrame;					//是否关键帧
	int m_nVideoWidth;					//宽
	int m_nVideoHeight;					//高
	u32 m_dwDataSize;					//数据大小
	u32 m_dwFrameID;					//帧ID
	u32 m_dwTimeStamp;					//时间戳
}PACK_NO_PADDING ifly_MediaFRAMEHDR_t;

//登录
typedef struct
{
	char	username[12];				//用户名
	char	loginpass[12];				//密码
	char	macAddr[18];				//MAC地址		
	u32		ipAddr;						//IP地址
}PACK_NO_PADDING ifly_loginpara_t;

//网络参数
typedef struct
{
	char	mac_address[18];			//mac地址
	u32		ip_address;					//ip地址
	u16		server_port;				//设备端口
	u32		net_mask;					//掩码
	u32		net_gateway;				//网关
	u32		dns;						//dns
	u8		flag_multicast;				//组播启用标记
	u32		multicast_address;			//组播地址
	u16		http_port;					//http端口
	u8		flag_pppoe;					//pppoe启用标记
	char	pppoe_user_name[64];		//pppoe用户名
	char	pppoe_passwd[64];			//pppoe密码
	u8      flag_dhcp;					//dhcp启用标志
	u8		ddnsserver;					//ddns服务商
	u8		flag_ddns;					//ddns启用标志
	char	ddns_domain[64];			//ddns域名
	char	ddns_user[64];				//ddns用户名
	char	ddns_passwd[64];			//ddns密码
	u32		centerhost_address;			//中心服务器地址
	u16		centerhost_port;			//中心服务器端口
	u16		mobile_port;				//手机监控端口
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_NetWork_t;

//分辨率最多支持16种不支持的请置0
typedef struct
{
	u16		width;						//分辨率宽
	u16		height;						//分辨率高
	u8		flashrate;					//分辨率刷新率
}PACK_NO_PADDING ifly_VGA_Pro;

typedef struct 
{
	ifly_VGA_Pro vgapro[16];
}PACK_NO_PADDING ifly_VGA_Solution;

typedef struct 
{
	char DDNSlist[20][20];
}PACK_NO_PADDING ifly_DDNSList_t;

//csp modify 20130423
typedef struct
{
	u32	id;
	u8	chn;
	u8	wnd_num;
	u8	wnd_index;
	u8	reserved[9];
}PACK_NO_PADDING ifly_wndinfo_t;

//csp modify 20130519
typedef struct
{
	char	username[12];				//用户名
	char	reserved1[4];
	u64		nRemoteView[1];				//远程预览权限
	u64		reserved2[29];
}PACK_NO_PADDING ifly_AdvPrivilege_t;

//系统参数
typedef struct
{
	u16		device_id;					//设备id
	char	device_name[32];			//设备名称
	u8		flag_overwrite;				//硬盘满时覆盖标记
	u16		lock_time;					//键盘锁定时间			
	u16		switch_time;				//切换时间
	u8		flag_statusdisp;			//状态显示标记
	u8		video_format;				//视频制式					
	u8		vga_solution;				//VGA分辨率 ifly_VGA_Solution[16];
	u8		transparency;				//菜单透明度
	u8		language;					//系统语言
	u8		disable_pw;					//密码禁用
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_SysParam_t;

//录像参数
typedef struct									
{
	u8		channelno;					//通道号
	u8		code_type;					//码流类型
	u8		bit_type;					//位率类型
	u32		bit_max;					//位率上限
	u16		intraRate;					//关键帧间隔
	u8		qi;							//关键帧量化因子
	u8		minQ;						//最小量化因子
	u8		maxQ;						//最大量化因子
	u8		quality;					//图像质量
	u8		frame_rate;					//视频帧率
	u16		pre_record;					//预录时间
	u16		post_record;				//录像延时
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
 #ifdef IECTRL_NEW
	u8 		supportdeinter;    //是否支持deinter设置 1是 0否 (待用)
	u8  		deinterval;     //deinter强度 0-4 禁用，弱，中，强，超强
 	u8  		supportResolu;    //是否支持设置录像分辨率
 	u8  		resolutionpos;    //分辨率选项值
 	u8		reserved[12];				//保留字段
 #endif

}PACK_NO_PADDING ifly_RecordParam_t;

//子码流参数
typedef struct									
{
	u8		channelno;					//通道号
	u8		sub_flag;					//区分多种子码流列问 0 cif 1 qcif
	u8		sub_bit_type;				//子码流位率类型
	u16		sub_intraRate;				//关键帧间隔
	u8		sub_qi;						//关键帧量化因子
	u8		sub_minQ;					//最小量化因子
	u8		sub_maxQ;					//最大量化因子
	u8		sub_quality;				//子码流图像质量
	u8 		sub_framerate;				//子码流的帧率
	int 	sub_bitrate;				//子码流的位率
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_SubStreamParam_t;

//图像参数
typedef struct
{
	u8		channel_no;					//通道号
	char	channelname[32];			//通道名，必须是UTF-8格式
	u8		flag_name;					//名称位置显示
	u16		chnpos_x;					//名称x坐标
	u16		chnpos_y;					//名称y坐标
	u8		flag_time;					//时间位置显示
	u16		timepos_x;					//时间x坐标
	u16		timepos_y;					//时间y坐标
	u8		flag_mask;					//遮盖
	struct  Mask_t
	{
		u16	 	x;
		u16		y;
		u16		width;
		u16		height;
	}PACK_NO_PADDING MaskInfo[4];
	u8		flag_safechn;				//安全通道标记
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_ImgParam_t;

//画面参数
typedef	struct  
{
	u8		channel_no;					//通道号
	u8		flag;						//调节标志:0-3
	u8		val;						//调节值
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
}PACK_NO_PADDING ifly_PicAdjust_t;


typedef struct _AlarmPtzDeal
{
	u8		flag_preset;			//预置点
	u8		preset;
	u8		flag_cruise;			//巡航点
	u8		cruise;
	u8		flag_track;				//轨迹
} PACK_NO_PADDING AlarmPtzDeal_t;

//报警输入参数
typedef struct									
{
	u8		inid;						//报警输入量 
	u8		typein;						//报警输入类型
	u8		flag_deal;					//是否触发报警输入处理
	u32		triRecChn;					//触发录像通道，每一位一通道
	u32		triAlarmoutid;				//触发报警输出，按位
	u8		flag_buzz;					//触发蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16		delay;						//报警输出延时
	AlarmPtzDeal_t AlarmInPtz [32];
	u32		copy2AlarmInmask;			//复制到其他报警输入。按位
	u8		flag_enable;				//报警启用标志
	u8		reserved[15];				//保留字段
}PACK_NO_PADDING ifly_AlarmInParam_t;

//报警输出参数
typedef struct									
{
	u8		outid;						//报警输出量 
	u8		typeout;					//报警输出类型
	u32		copy2AlarmOutmask;			//复制到其他报警输出。按位
	u16		alarmoutdelay;				//报警输出延时
	u8		flag_buzz;					//触发蜂鸣器
	u16		buzzdelay;					//蜂鸣器延时
	u8		reserved[11];
}PACK_NO_PADDING ifly_AlarmOutParam_t;

//报警通知参数: email报警手机报警
typedef struct									
{

	char	alarm_email[32];			//报警email地址
	char	alarm_mobile[32];			//报警手机地址
	u8		reserved[32];				//保留字段
}PACK_NO_PADDING ifly_AlarmNoticeParam_t;

//报警EMail SMTP信息							EMAIL_OPT
typedef struct  
{
 char	alarm_email1[32];	//接收邮箱1
 char	alarm_email2[32];	//接收邮箱2
 char	alarm_email3[32];	//接收邮箱3
 char 	SMTP_svr[32];  		//emali的SMTP服务器
 char 	username[32];  		//邮箱用户名
 char 	userpw[32];   		//邮箱密码
 u16  smtp_port;			//SMTP端口pw 2010/11/18
 u8  reserved[13];  		//预留
} PACK_NO_PADDING ifly_AlarmEmail_SMTP_t;

//录像布防
typedef struct
{
	u8		chn;						//通道
	u8		weekday;					//星期
	struct RecTimeField_t
	{
		u32 starttime;					//起始时间
		u32 endtime;					//终止时间
		u8	flag_sch;					//定时录像
		u8 	flag_md;					//移动侦测录像
		u8  flag_alarm;					//报警录像
		u8	reserved[4];				//保留字段
	}PACK_NO_PADDING TimeFiled[4];
	u8		copy2Weekday;				//复制到其他天  按位 
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_RecordSCH_t;

#if 1

//移动侦测
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		sense;						//灵敏度
	u8		block[44*36];				//区域
	AlarmPtzDeal_t AlarmInPtz[32];		//云台联动
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		flag_enable;				//报警启用标志
	u8		reserved[11];				//保留字段
} PACK_NO_PADDING ifly_MDParam_t;

//视频丢失
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	AlarmPtzDeal_t AlarmInPtz [32];		//云台联动
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[12];				//保留字段
} PACK_NO_PADDING ifly_VideoLostParam_t;

#else

//移动侦测
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		sense;						//灵敏度
	u8		block[44*36];				//区域
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_MDParam_t;

//视频丢失
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_VideoLostParam_t;

#endif

//视频遮挡
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		reserved[16];				//保留字段
}PACK_NO_PADDING ifly_VideoBlockParam_t;

//云台参数
typedef struct
{
	u8		chn;						//通道
	u16		address;					//解码器地址
	u8		baud_ratesel;				//波特率 选项值
	u8		data_bitsel;				//数据位 选项值
	u8		stop_bitsel;				//停止位 选项值
	u8		crccheck;					//校验
	u8		flow_control;				//流控
	u8		protocol;					//协议类型
	u32		copy2Chnmask;				//复制到其他通道 按位	
	u8		comstyle;					//串口类型(232 or 485)
	u8		enableptz;					//通道云台(串口)使能
}PACK_NO_PADDING ifly_PTZParam_t;

//预置点控制
typedef struct
{
	u8		chn;						//通道
	u8		presetpoint;				//预置点号
	u8		option;						//操作 0添加 1删除 2 到预置点
}PACK_NO_PADDING ifly_PtzPresetCtr_t;

//巡航路径参数
typedef struct
{
	u8		chn;						//通道
	u8		cruise_path;				//巡航路径
	struct Cruise_point_t
	{
		u8  preset;						//预置点
		u8  weeltime;					//停留时间
		u8  rate;						//速度
		u8  flag_add;					//0删除  1添加
	}PACK_NO_PADDING Cruise_point[CRUISE_POINT_NUM];
}PACK_NO_PADDING ifly_PtzCruisePathParam_t;

//巡航路径控制
typedef struct
{
	u8		chn;						//通道
	u8		cruisepath;					//巡航路径号
	u8		flagoption;					//操作 0停止 1开始 
}PACK_NO_PADDING ifly_CruisePathCtr_t;

//轨迹控制
typedef struct
{
	u8		chn;						//通道
	u8		flagoption;					//操作 0记录 1停止记录  2开始轨迹 3停止轨迹 
}PACK_NO_PADDING ifly_TrackCtr_t;

//硬盘管理
typedef struct 
{
	u8		hdd_index;					//硬盘序号
	u8		hdd_exist;  				//1 exist; 0 not exist
	u32		capability;					//MB
	u32		freesize;					//MB
	u8		reserved[2];				//预留
}PACK_NO_PADDING  ifly_hddInfo_t;

//用户信息
typedef struct
{
	char	name[12];					//用户名
	char	passwd[12];					//密码
	char	macaddr[18];				//物理地址
	u8		rcamer;						//远程云台权限
	u8		rrec;						//远程录像权限
	u8		rplay;						//远程回放权限
	u8		rsetpara;					//远程设置参数
	u8		rlog;						//远程日志权限
	u8		rtool;						//远程工具权限
	u8		rpreview;					//远程预览权限
	u8		ralarm;						//远程报警权限
	u8		rvoip;						//语音对讲权限
	u8		lcamer;						//本地云台权限
	u8		lrec;						//本地录像权限
	u8		lplay;						//本地回放权限
	u8		lsetpara;					//本地设置参数
	u8		llog;						//本地日志权限
	u8		ltool;						//本地工具权限
}PACK_NO_PADDING ifly_user_t;

typedef struct
{
	ifly_user_t	userNum[8];		//最多8 个用户
}PACK_NO_PADDING ifly_userNumber_t;

typedef struct
{
	ifly_user_t	userInfo;		//用户信息
	u8		flagOption;					//0添加用户 1编辑用户 2删除用户
}PACK_NO_PADDING ifly_userMgr_t;

//系统时间
typedef struct
{
	u32		systemtime;					//系统时间
	u8		format;						//时间格式 选项值
	u8		flag_time;					//预览时间位置显示
	u16		timepos_x;					//预览时间x坐标
	u16		timepos_y;					//预览时间y坐标
}PACK_NO_PADDING ifly_sysTime_t;

//系统信息
typedef struct
{
	char	devicename[32];				//设备名
	char	devicemodel[32];			//设备型号
	char	deviceser[32];				//设备序列号
	char	version[64];				//软件版本
}PACK_NO_PADDING ifly_sysinfo_t;

//云台控制
//控制命令:
//0-停止 1上 2下 3 左 4右 5自转 
//6变倍加 7变倍减 8焦距加 9焦距减 10光圈加 
//11光圈减 12灯光开 13灯光关 14雨刷开 15雨刷关 
//16 快速 17常速 18慢速
typedef struct
{
	u8  chn;							//通道
	u8  cmd;							
	u8	reserved;
}PACK_NO_PADDING ifly_PtzCtrl_t;

//手动录像
typedef struct
{
	u32  chnRecState;					//通道手动录像状态 按位	
}PACK_NO_PADDING ifly_ManualRecord_t;

//日志搜索
typedef struct
{
	u8		query_mode;					//查询方式		
	u8		main_type;					//主类型		
	u8		slave_type;					//次类型		
	u16		max_return;					//最大返回数		
	u16		startID;					//返回从第一条记录开始		
	u32		start_time;					//开始时间		
	u32		end_time;					//结束时间			
	u8      reserved[16];				//预留		
}PACK_NO_PADDING ifly_search_log_t;

typedef struct
{
	u32		startTime;
	u8		main_type;					//主类型		
	u8		slave_type;					//次类型	
	u8 		chn;							//通道号或序号
	char	loginfo[32];				//具体信息		
}PACK_NO_PADDING ifly_logInfo_t;

typedef struct
{
	u16		sum;						//总记录数		
	u16		startID;					//开始的记录,基址为1.当无返回数据时,startID的值为0		
	u16		endID;						//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录		
}PACK_NO_PADDING ifly_search_desc_t;


//xdc 搜索摄像头
typedef struct
{
	u32 	protocol_type;//0 all,1 onvif, 2 i13 
	u16		max_return;					//最大返回数	
}PACK_NO_PADDING ifly_search_ipc_t;

//摄像头的信息
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
	char reserved[47];
	unsigned int net_mask;
	unsigned int net_gateway;
	unsigned int dns1;
	unsigned int dns2;
}PACK_NO_PADDING ifly_ipc_info_t;

//yaogang modify 20141030
//轮巡参数
typedef struct
{
	unsigned char nIsPatrol; //当前轮巡是否启用(数字:0=否;1=是)
	unsigned char nInterval; //当前轮巡切换时间(数字:单位秒s)
	unsigned char nPatrolMode; //当前轮巡时所用的预览模式(数字:参考Mode 1 4 9 16)	
	unsigned char nPatrolChnNum;//轮巡通道数16 32 48 64，64路解码器专用
	unsigned char nInterval_num;//轮巡切换时间可供选择的数量
	unsigned char nPatrolMode_num;//轮巡预览模式可供选择的数量
	unsigned char value[1];//存放轮巡时间和模式的值。eg: 5 10 20 30 60 1 4 9 16
} ifly_patrol_para_t;
//预览参数
typedef struct
{
	unsigned char nPreviewMode; //当前所用的预览模式(数字:参考Mode 1 4 9 16)	
	unsigned char ModePara;//预览起始通道0-15
} ifly_preview_para_t;

//回放文件搜索
#if 1
typedef struct
{
	u16		channel_mask;				//通道 按位组合 特指通道1-16
	u16		type_mask;					//类型 按位组合
	u32		start_time;					//开始时间
	u32		end_time;					//终止时间
	u16		startID;					//返回的第一条记录,从1开始
	u16		max_return;					//每次返回的最大记录数
	u16		chn17to32mask;				//通道17-32 按位组合
	u8		reserved[5];				//预留
	char	bankcardno[21];				//卡号
}ifly_recsearch_param_t;
#else
typedef struct
{
	u16		channel_mask;				//通道 按位组合
	u16		type_mask;					//类型 按位组合
	u32		start_time;					//开始时间
	u32		end_time;					//终止时间
	u16		startID;					//返回的第一条记录,从1开始
	u16		max_return;					//每次返回的最大记录数
	u8		reserved[4];				//预留
}PACK_NO_PADDING ifly_recsearch_param_t;
#endif

////远程回放返回的信息结构
typedef struct
{
	u8		channel_no;
	u8		type;
	u32		start_time;
	u32		end_time;
	u8		image_format;				//3:cif;4:4cif
	u8		stream_flag;				//0:视频流;1:复合流
	u32		size;
	u32		offset;
	char	filename[64];
}PACK_NO_PADDING ifly_recfileinfo_t;

//放像进度通知
typedef struct
{
	u32	 id;							//回放id
	u32  currPos;						//当前进度
	u32  totallen;						//总时间
}PACK_NO_PADDING ifly_progress_t;

//回放文件是否有音频
typedef struct  
{
	u8 bHasAudio;					 	// 1有音频，0无音频
}PACK_NO_PADDING ifly_play_hasaudio_t;

//异步报警信息
typedef struct
{
	u32	 time;							//报警发生时间
	u32  alarmInfo;						//报警值
	u32  reserved;						//预留
}PACK_NO_PADDING ifly_alarminfo_t;

#ifdef ALARM_UPLOAD
typedef struct
{
	CPHandle g_cph;
	u8 g_state;
}PACK_NO_PADDING ifly_AlarmUploadCenter_t;

#if 1
typedef struct
{
 u8 type;       //0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问
 u8 state;       //  1报警 2恢复
 u8 id;        //通道,硬盘号,报警输入号,取决于type 
 u16 reserved1;      //预留
 u32  reserved2;      //预留
}PACK_NO_PADDING ifly_alarmstate_t;
#else
typedef struct 
{
	u8 type;
	u32 state;
	u32  reserved;						//预留
}PACK_NO_PADDING ifly_alarmstate_t;
#endif
#endif


typedef struct
{
	u8 m_byMediaType;
	u8 m_byFrameRate;
	u8 m_bKeyFrame;
	int m_wVideoWidth;
	int m_wVideoHeight;
	u32 m_dwDataSize;
	u32 m_dwFrameID;
	u32 m_dwTimeStamp;
}PACK_NO_PADDING IFLY_MediaFRAMEHDR_t;




typedef struct
{
	char	ifrname[16];
	u32		ip;
	u32		subnet;
	u32		broadcast;
	u32		gateway;
	u32		dns;
}PACK_NO_PADDING ifly_cp_dhcp_t;

/************************/
//07-09-03
typedef struct  
{
 	u16		width;
 	u16		height;
}PACK_NO_PADDING ifly_pic_resolution_t;							//主机发送给客户端的图像分辨率

typedef struct
{
 	u8		channel_no;
}PACK_NO_PADDING ifly_need_resolution_t;						//本通道需要主机发送图像分辨率
/*************************/

typedef struct 
{
	char	usrname[12];
}PACK_NO_PADDING ifly_usrname_header;

typedef struct
{
	u16		startID;
	u16		max_return;
}PACK_NO_PADDING ifly_remusr_send_t;

//pw 2010/7/9
#if 0
#ifndef MAX_CHN_NUM
#define MAX_CHN_NUM		8//16//fpga:16 6446:4
#endif
#endif

//远程手动录像结构
typedef struct
{
	//u8		chan_rec[MAX_CHN_NUM];
	u8		chan_rec[16];
	u8		allrec;
	u8		allstop;
}PACK_NO_PADDING ifly_hand_rec_t;

//远程日志查询
typedef struct
{
	u8		query_mode;							//查询方式
	u8		main_type;							//主类型
	u8		slave_type;							//次类型
	u16		max_return;							//最大返回数
	u16		startID;							//返回从第一条记录开始
	u32		start_time;							//开始时间
	u32		end_time;							//结束时间	
	u8 langID;									//查询结果显示语言
}PACK_NO_PADDING ifly_research_log_t;

typedef struct
{
	u32		startTime;
	char	main[64];							//主类型
	char	slave[64];							//次类型
	char	loginfo[32];						//具体信息07-10-09
}PACK_NO_PADDING ifly_log_back_t;

#if 0//csp modify 20130519
typedef struct
{
	char	username[12];
	char	loginpass[12];
}PACK_NO_PADDING ifly_logininfo_t;
#endif

typedef struct
{
	char	usrname[12];
	char	remote_privilege[16];
}PACK_NO_PADDING ifly_remusr_limit;

typedef struct
{
	int		link_online;
}PACK_NO_PADDING ifly_link_online;

//远程用户返回信息结构
typedef struct
{
	char	name[12];
	char	password[12];
	char	local_privilege[16];
	char	remote_privilege[16];
	char	mac_address[18];
}PACK_NO_PADDING ifly_userinfo_t;

//远程用户编辑参数结构
typedef struct
{
	char	name[12];							//用户名
	char	passwd[12];							//密码
	char	confirm[12];						//校正密码
	char	macaddr[18];						//物理地址
	u32		index;								//用户权限
	u32		rcamer;								//远程云台权限
	u32		rrec;								//远程录像权限
	u32		rplay;								//远程回放权限
	u32		rsetpara;							//远程设置参数
	u32		rlog;								//远程日志权限
	u32		rtool;								//远程工具权限
	u32		rpreview;							//远程预览权限
	u32		ralarm;								//远程报警权限
	u32		rvoip;								//语音对讲权限
	u32		lcamer;								//本地云台权限
	u32		lrec;								//本地录像权限
	u32		lplay;								//本地回放权限
	u32		lsetpara;							//本地设置参数
	u32		llog;								//本地日志权限
	u32		ltool;								//本地工具权限
}PACK_NO_PADDING ifly_useredit_t;

//远程用户删除参数结构
typedef struct
{
	u32		user_index;							//当前删除用户记录
	char	name[12];							//当前欲删除用户名
}PACK_NO_PADDING ifly_userdel_t;

//远程用户密码修改参数
typedef struct 
{
	char	name[12];
	char	oldpass[12];
	char	newpass[12];
	char	confirm[12];
}PACK_NO_PADDING ifly_editpass_t;

//远程返回硬盘信息结构
typedef	struct 
{
	s64		capability;//Byte
	s64		freesize;//Byte
}PACK_NO_PADDING ifly_remhdd_t;

typedef struct
{
	//u8		hdd_exist[MAX_HDD_NUM];
	u8		hdd_exist[8];
}PACK_NO_PADDING ifly_hdd_desc_t;

//远程系统信息返回 
typedef struct
{
	char	devicename[32];
	char	devicemodel[32];
	char	deviceser[32];
	char	version[64];
}PACK_NO_PADDING ifly_recsysinfo_t;
	
															//dong	100424
enum SYS_LANGUAGE{		
 SYS_LANGUAGE_CHS = 0,	  //简体中文
 SYS_LANGUAGE_ENU = 1,    //美式英文
 SYS_LANGUAGE_CHT = 2, 	//繁体諫文
 SYS_LANGUAGE_GER = 3,    //德语
 SYS_LANGUAGE_FRE = 4,    //法语
 SYS_LANGUAGE_SPA = 5,    //西班牙语
 SYS_LANGUAGE_ITA = 6,    //意大利
 SYS_LANGUAGE_POR = 7,    //葡萄牙语
 SYS_LANGUAGE_RUS = 8, 	//俄语
 SYS_LANGUAGE_TUR = 9, 	//土耳其语
 SYS_LANGUAGE_THA = 10,	//泰国语
 SYS_LANGUAGE_JAP = 11,	//日语
 SYS_LANGUAGE_HAN = 12,	//韩语
 SYS_LANGUAGE_POL = 13,	//波兰语
 SYS_LANGUAGE_HEB = 14,   //希伯来语Hebrew
 SYS_LANGUAGE_HUN = 15,	//匈牙利语Hungarian
 SYS_LANGUAGE_ROM = 16,	//罗马语Roma
 SYS_LANGUAGE_IRA = 17, //伊朗语
 SYS_LANGUAGE_CZE = 18, //捷克语
 SYS_LANGUAGE_VIE = 19, //越南语
 SYS_LANGUAGE_LIT = 20, //立陶宛
 SYS_LANGUAGE_SLO = 21, //斯洛伐克
 SYS_LANGUAGE_ARA = 22, //阿拉伯语
 SYS_LANGUAGE_INVALID = -1  //仅作填充无效值用
};

typedef struct  
{
 u8 max_langnum;  //最多支持语言数 <=32
 u8 langlist[32]; // language list, 最多32项,每项值为SYS_LANGUAGE(转u8)
 u8 reserved[16]; //预留
}PACK_NO_PADDING ifly_SysLangList_t;
															//dong	100424

//远程系统参数结构 added by kmli
typedef struct
{
	u16		device_id;
	char	device_name[32];
	u8		flag_overlap;
	u32		time;
	u8		lock_time;
	u8		switch_time;
	u8		flag_statusdisp;
	u8		video_format;
	u8		vga_solution;
	u8		transparency;//07-10-16
	u8		language;//07-10-16
}PACK_NO_PADDING ifly_syspara_send_t;

//zlb20090916 begin
typedef struct {
	u8 chn;
	u8 b_email;
	u8 alarm_type;
}PACK_NO_PADDING emailalarm_send_t;

typedef struct
{
	u8 preview_mode;
	u16 buzz_delay;
	u8 reserved1[21];
	u8 reserved2[28];
	char alarm_email[MAX_EMAIL_LENGTH];
	u16 md_emailalarm;
	u16 in_emailalarm;
	u16 vl_emailalarm;
	u16 vb_emailalarm;
	u8 reserved[68 - MAX_EMAIL_LENGTH];
}PACK_NO_PADDING ifly_reserved_send_t;
//zlb20090916 end


//远程云台设置
typedef struct
{
	u8		channel_no;
	u16		address;
	u8		baud_ratesel;
	u8		data_bitsel;
	u8		stop_bitsel;
	u8		crccheck;
	u8		flow_control;
	u8		protocol;
}PACK_NO_PADDING ifly_yuntai_send_t;

typedef struct
{
	CPGuid	guid;								//本地mac地址
	u32		dwIp;								//本地ip
	u16		wPort;								//本地接收数据的端口
	u8		byChn;								//通道号,基址为0
	u8		byFlag;								//0:audio;1:video
}PACK_NO_PADDING ifly_nat_t;

//远程监控
typedef struct
{
	CPGuid	guid;								//本地mac地址
	u32		dwIp;								//本地ip
	u16		wPort;								//本地接收数据的端口
	u8		byChn;								//通道号,基址为0
	u8 		byFlagSubcode;						//是否开启子码流
}PACK_NO_PADDING ifly_monitor_param_t;

//按文件远程回放
typedef struct
{
	u32		dwIp;								//接收媒体数据的ip
	u16		wVideoPort;							//接收视频数据的端口
	u16		wAudioPort;							//接收音频数据的端口
	char	filename[64];						//回放的文件
	u32		offset;								//文件偏移
}PACK_NO_PADDING ifly_remote_playback_t;

typedef struct
{
	u32		id;
}PACK_NO_PADDING ifly_play_handle_t;

typedef struct
{
	u32		curpos;
	u32		totaltime;
}PACK_NO_PADDING ifly_play_progress_t;

typedef struct
{
	u32		dwIp;								//接收文件数据的ip
	u16		wPort;								//接收文件数据的端口
	char	filename[64];						//下载的文件
	u32		offset;								//文件偏移
	u32		size;								//文件大小
}PACK_NO_PADDING ifly_remote_download_t;

typedef struct
{
	u32		id;
}PACK_NO_PADDING ifly_download_handle_t;

/*********************/
typedef struct
{
	u32		dwIp;
	u16		wPort;
}PACK_NO_PADDING ifly_remote_layout_head;

typedef struct
{
	char	filename[64];
	u32		size;
	u16		reserved;
}PACK_NO_PADDING ifly_remote_layout_t;

typedef struct  
{
	u8		channel_no;							//通道号
	u8		flag_mask;							//是否显示
	u8		mask_index;							//遮盖区域索引		
	u32		x;									//遮盖区域左上x坐标
	u32		y;									//遮盖区域左上y坐标
	u32   	width;								//遮盖区域宽度
	u32   	height;								//遮盖区域高度
}PACK_NO_PADDING ifly_maskset_t;

typedef	struct  
{
	u8		channel_no;							//通道号
	u8		flag;								//调节标志:0-3
	u8		val;								//调节值
}PACK_NO_PADDING ifly_picset_t;
/*********************/

typedef struct
{
	char	filename[64];						//文件名
	u32		size;								//文件长度
	u32		verify;								//校验和
	u16		version;							//文件版本号
	u16		reserved;
}PACK_NO_PADDING ifly_remote_upload_t;

typedef struct
{
	u32		dwIp;
	u16		wPort;
}PACK_NO_PADDING ifly_voip_t;



typedef struct
{
	u16		sum;								//总记录数
	u16		startID;							//开始的记录,基址为1.当无返回数据时,startID的值为0
	u16		endID;								//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录
}PACK_NO_PADDING ifly_recfile_desc_t;

typedef struct
{
	char	mac_address[18];
	u32		ip_address;
	u16		server_port;
	u32		net_mask;
	u32		net_gateway;
	u32		dns;
	u32		multicast_address;
	u32		admin_host;
	u16		host_port;
	u16		http_port;
	//u16		port2;			//pw:26/05/2010
	u8		flag_pppoe;
	char	user_name[64];
	char	passwd[64];
	char	pwdcheck[64];
	u8    	flag_dhcp;//cyl commet  高4位表示是否启用多播..低四位表示DHCP
	u8		flag_ddns;
	char	ddns_domain[64];
	char	ddns_user[64];
	char	ddns_passwd[64];
}PACK_NO_PADDING ifly_network_t;

//远程图像设置输入结构
typedef struct
{
	u8		channel_no;							//通道号
	char	channelname[32];					//通道名
	//与位置相关的
	u8		flag_name;							//名称位置显示
	//时间相关的
	u8		flag_time;							//时间位置显示
	u8		flag_mask;							//遮盖
	u8		overlay;							//遮挡报警
	u8		lost;								//视频丢失
	u8		motion;								//移动侦测
	//处理方式
	//char	record_channel[8];					//触发的录像通道
	char	record_channel[16];					//触发的录像通道
	u8		flag_buzz;							//声音报警
	u8		flag_send;							//上传中心
	//char	alarm_out[2];						//触发报警输出
	char	alarm_out[16];						//触发报警输出
}PACK_NO_PADDING ifly_imagset_t;

//OSD位置设置 07-07-27
typedef struct 
{
	u8		channel_no;							//通道名
	u8		flag_name;							//名称位置是否显示
	u16		x;									//名称位置左上角x坐标
	u16		y;									//名称位置左上角y坐标
}PACK_NO_PADDING ifly_channelset_t;

typedef struct 
{
	u8		channel_no;							//通道名
	u8		flag_time;							//时间位置是否显示
	u16		x;									//时间位置左上角x坐标
	u16		y;									//时间位置左上角y坐标
}PACK_NO_PADDING ifly_timeset_t;

typedef struct									//遮挡、视频丢失、移动侦测布防参数结构
{
	u8		channel_no;							//通道号
	u8		week;								//日期
	u8		flag_alltime;						//是否全天布防						
	u32		starttime[4];						//时间段1~4的起始时间
	u32		endtime[4];							//时间段1~4的终止时间
}PACK_NO_PADDING ifly_valid_t;

typedef struct									//远程移动侦测报警处理参数结构
{
	u8		channel_no;							//通道号
	//u8		recchan[MAX_CHN_NUM];						//触发的录像通道
	u8		recchan[16];						//触发的录像通道
	u8		flag_buzz;							//是否声音报警
	u8		flag_send;							//是否上传中心
	u8		alarmout;							//是否触发报警输出
	u8		alarm_out[4];						//具体某路的报警输出
}PACK_NO_PADDING ifly_motion_t; 

typedef struct									//远程异常处理页面参数结构
{
	u8		exceptiontype;						//异常类型
	u8		flag_display;						//是否屏上显示
	u8		flag_buzz;							//是否声音报警
	u8		flag_send;							//是否上传中心
	u8		alarmout;							//是否触发报警输出
	u8		alarm_out[4];						//具体某路的报警输出
}PACK_NO_PADDING ifly_exceptioninfo_t;

//位率列表											//dong	100424
typedef struct  
{
 u8 chn;    //通道
 u8 videotype;   //0 主码流, 1 子码流
 u32 bitratelist[16]; //位率列表, 单位kb, 未使用的填0
 u8 reserved[16];  //预留
}PACK_NO_PADDING ifly_bitRateList_t;


typedef struct									//远程录像配置页面参数结构
{
	u8		channelno;							//通道号
	char	channelname[32];					//通道名
	u8		code_type;							//视频类型
	u8		video_resolution;					//分辨率
	u8		bit_type;							//位率类型
	u32		bit_max;							//位率上限
	u8		quality;							//图像质量
	u8		frame_rate;							//视频帧率
	u16		pre_record;							//预录时间
	u16		post_record;						//录像延时
	u8		flag_record;						//是否启动录像
	//wrchen 090210
	u8 		yfactor;							//判断当前视频制式05-13

	u8 		sub_size;									//子码流的大小 qcif cif
	u8 		sub_framerate;								//子码流的帧率
	int 		sub_bitrate;								//子码流的位率
}PACK_NO_PADDING ifly_recordinfo_t;

typedef struct									//远程录像设置页面参数结构
{
	u8		channelno;							//通道号
	u8		week;								//日期
	u8		flag_alltime;						//是否全天布防
	u8		alltype;							//全天布防时，录像类型
	u32		start_time[4];						//时间段1~4的起始时间
	u32		end_time[4];						//时间段1~4的终止时间
	u8		type[4];							//时间段1~4的录像类型
}PACK_NO_PADDING ifly_recordtime_t;

typedef struct									//远程报警设置页面
{
	u8		inid;								//报警输入量 
	u8		typein;								//报警输入类型
	u8		flag_deal;							//是否触发报警输入处理
	u8		copyin;
	u8		in_copy;							//是否选择报警量的复制
	u8		outid;								//选择报警输出
	u8		typeout;							//报警输出类型 
	u8		out_copy;							//报警输出复制
	u8		copyout;
	u16		delay;								//报警输出延时	
}PACK_NO_PADDING ifly_alarmset_t;

struct _alarmset_in_
{
	u8		typein;								//报警输入类型
	u8		flag_deal;							//是否触发报警输入处理
}PACK_NO_PADDING;

struct _alarmset_out_
{
	u8		typeout;							//报警输出类型 
	u16		delay;								//报警输出延时	
}PACK_NO_PADDING;

typedef struct													//远程报警设置页面
{
	//struct _alarmset_in_ alarmset_in[MAX_ALARM_IN_NUM];			//报警输入参数
	struct _alarmset_in_ alarmset_in[16];			//报警输入参数
	u8 copyin;													//报警输入复制目标号
	u8 in_copy;													//是否选择报警量的复制	
	//struct _alarmset_out_ alarmset_out[MAX_ALARM_OUT_NUM];		//报警输出参数
	struct _alarmset_out_ alarmset_out[4];		//报警输出参数
	u8 copyout;													//输出报警设置复制目标号
	u8 out_copy;												//是否选择报警输出量的复制	
} PACK_NO_PADDING alarmset_t;

typedef struct									//远程ptz联动
{
	u8		inid;								//报警输入量
	u8		channo;								//通道号
	u8		flag_preset;						//预置点
	u8		preset;
	u8		flag_cruise;						//巡航点
	u8		cruise;
	u8		flag_track;							//轨迹
}PACK_NO_PADDING ifly_alarmin_ptz;

typedef struct									//报警布防
{
	u8		id;									//选择报警量
	u8		week;								//日期	
	u8		flag_alltime;						//全天布防
	u32		starttime[4];						//时间段1~4的起始时间
	u32		endtime[4];							//时间段1~4的终止时间
}PACK_NO_PADDING ifly_alarm_settime;

#define MD_ROW			18
#define MD_COL			22
#define MD_MIN_SENSE	0						//最不灵敏
#define MD_MAX_SENSE	5						//最灵敏

typedef struct
{
	u8	channel_no;
	u8	flag;									//是否开启移动侦测
	u8	sense;									//灵敏度 MD_MIN_SENSE-MD_MAX_SENSE
	u8	block[MD_ROW*MD_COL];					//非0选中 0不选中
}PACK_NO_PADDING ifly_md_param_t;

typedef struct
{
	u8	channel_no;
	u8	byCmdId;								//摄象头控制命令类型
	u8	abyParam[MAX_PARAM_NUM];				//参数
	u8	byCtrlWay;								//摄象头正反向控制方式 CAM_CTRLWAY_GENERAL:正向;CAM_CTRLWAY_REVERSE:反向
}PACK_NO_PADDING ifly_camera_ctrl_t;


//#pragma pack( pop )

typedef enum									//播放速率枚举
{
	PLAYRATE_1 = 1,								//正常播放
		PLAYRATE_2 = 2,								//2x快放
		PLAYRATE_4 = 4,								//4x快放
		PLAYRATE_MAX = 8,							//8x快放
		PLAYRATE_1_2 = -2,							//1/2x慢放
		PLAYRATE_1_4 = -4,							//1/4x慢放
		PLAYRATE_MIN = -8,							//1/8x慢放
		PLAYRATE_SINGLE = 0,						//帧进
		PLAYRATE_BACK_1 = -1						//1x退放
}re_em_play_rate;


typedef struct
{
	SOCKHANDLE	sockfd;
	u8	bStart;
	u32 linkid;
	ifly_TCP_Stream_Req req;
	union
	{
		struct  
		{
			u32 frameid;
		}PACK_NO_PADDING Monitor_t;
		
		struct
		{
			CPHandle cph;
			u8   byFlag;//0:按文件播放;1:按时间回放
			u8   byUse;
			u8   byStop;
			u8   byPause;
			u8   byMute;
			u8   byStatus;
			u8   bySkip;
			re_em_play_rate rate;
			u32  curPos;
			u32  totalTime;
			u32  refTime;
			u32  seekPos;
		}PACK_NO_PADDING PlayBack_t;
		
		struct
		{
			u8   byUse;
			u8   byStop;
		}PACK_NO_PADDING DonwLoad_t;
		
		struct  
		{
			CPHandle cph;
		}PACK_NO_PADDING Update_t;
	};
	
}PACK_NO_PADDING ifly_stearmsnd_t;


#ifdef SPECIAL_DEV_INFO
typedef struct
{
 u8 mainbordmode; //主板型号 预留
 u8 characterset; //字符集: 0-ascii，1-GB2312, 2-UTF8, 3-UTF16 
 u8 reserved[62]; //预留
}PACK_NO_PADDING ifly_specialinfo_t;
#endif

typedef u16 (*pMESSAGE_CB)(CPHandle cph,u16 event,u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext);
void SetMsgCallBack(pMESSAGE_CB pCB,void *pContext);

//自动搜索设备
//sever端：设置设备信息
void SetDeviceInfo(ifly_DeviceInfo_t deviceinfo);

//client端：注册搜索回调函数
typedef void (*pFindDeivce_CB)(ifly_DeviceInfo_t deviceinfo, void* pContext);
void SetFindDeviceCB(pFindDeivce_CB pCB, void *pContext);
//client端：开始搜索
int SearchDevice();

//sever端：注册码流连接回调函数
//当有码流传输的连接时，在回调里进行处理
//pAddStreamTCPLink： 成功时返回0，否则-1
typedef int (*pAddStreamTCPLink)(SOCKHANDLE hSock, ifly_TCP_Stream_Req req, void* pContext);
void SetAddStreamLinkCB(pAddStreamTCPLink pCB, void *pContext);

//发送tcp连接头，用于区别是信令还是码流传输
int SendTcpConnHead(SOCKHANDLE hSock, u8 type);
void SetCliAckPort(u16 wPort);

u16  CPLibInit(u16 wPort);
u16  CPLibCleanup();

/************************
**07-08-21 verifying connecting req from custorm
*************************/
CPHandle CurConnect_out(u32 i);
//连接服务器
CPHandle CPConnect(u32 dwServerIp, u16 wServerPort, u32 dwTimeOut, u16 *pwErrorCode);
//清处连接
u16  CleanCPHandle(CPHandle cph);
//发送事件通知
u16  CPPost(CPHandle cph, u16 event, const void *content, int length);
//发送命令
u16  CPSend(CPHandle cph, u16 event, const void *content, int length, void* ackbuf, int ackbuflen, int *realacklen, u32 dwTimeOut);

//得到TCP连接的本地ip地址与端口
BOOL GetCPHandleLocalAddr(CPHandle cph, struct sockaddr_in *pAddr);

//发送NAT探测包
BOOL SendSTUNPacket(u32 dwLocalIpAddr, u16 wLocalPort, u32 dwRemoteIpAddr, u16 wRemotePort, u8 *pbyBuf, u32 dwBufLen);

u16  GetTransactionNum();



/***************************************************************************************************************/
//发送交互命令给主机，并返回控制处理的最终结果
//交互过程为：
//	1.client端将要发送的参数按特定的格式打包，然后调用本函数
//	2.server端接收到请求后，进行相应的处理
//	3.server根据处理结果返回对应的信息给client，并关闭socket 连接！

//输入参数：
//	dwHostIp：主机的IP地址
//	dwTimeOut：交互的超时时间
//	pBuf：发送的数据头及真正的数据信息，例如网络监控中，pBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwBufLen：pBuf的长度
//返回值：
//	0：调用成功
//	其它：调用失败
/***************************************************************************************************************/
u16  SendToHost(u32 dwHostIp, u32 dwTimeOut, void *pBuf, u32 dwBufLen);

/***************************************************************************************************************/
//发送交互命令给主机，并需要从主机获得额外的信息
//交互过程为：
//	1.client端将要发送的参数按特定的格式打包，然后调用本函数
//	2.server端接收到请求后，进行相应的处理
//	3.server根据处理结果返回对应的信息给client，并关闭socket 连接！

//输入参数：
//	dwHostIp：主机的IP地址
//	dwTimeOut：交互的超时时间
//	pinBuf：发送的数据头及真正的数据信息，例如网络监控中，pinBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwinBufLen：pinBuf的长度
//	dwoutBufLen：poutBuf最大长度，防止内存溢出
//输出参数：
//	poutBuf：从主机接收的数据头及真正的数据信息，例如远程回放查询中，poutBuf = ifly_cp_header_t + 查询的结果
//	dwoutBufLen：实际接收到的buf的长度
//返回值：
//	0：调用成功
//	其它：调用失败
//注意：
//	主机应根据输入的dwoutBufLen来控制返回给client的最大数据，否则可能导致数据溢出
/***************************************************************************************************************/
u16  SendToHost2(u32 dwHostIp, u32 dwTimeOut, void *pinBuf, u32 dwinBufLen, void *poutBuf, u32 *dwoutBufLen);

int check_link_stat();

//pw 2010/9/10
typedef int SOCKET;
int loopsend(SOCKET s, char *buf, unsigned int sndsize);
int looprecv(SOCKET s, char *buf, unsigned int rcvsize);

void SetUpnpId(char* upnpid);

//csp modify 20130406
void SetUpnpPW(char* upnppw);

#ifdef __cplusplus
}
#endif

#endif

