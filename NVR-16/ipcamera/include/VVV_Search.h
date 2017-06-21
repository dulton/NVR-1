
#ifndef __VVV_SEARCH_H__
#define __VVV_SEARCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define VVV_SEARCH_STRNO_LEN       (20)
#define VVV_SEARCH_CHNNO_LEN       (20)
#define VVV_SEARCH_IP_STRSIZE      (15)
#define VVV_SEARCH_MAC_STRSIZE     (17)
#define VVV_SEARCH_DEVID_STRSIZE   (32)
#define VVV_SEARCH_DEVNAME_STRSIZE (40)
#define VVV_SEARCH_SWVER_STRSIZE   (32)


/*设备标识信息，用于修改设备信息用*/
typedef struct tagVVV_SEARCH_DEVID
{
    char* pszDevID;      /*设备标识，设备唯一标识*/
    char* pszUserName;   /*用户名*/
    char* pszPasswd;     /*密码*/
} VVV_SEARCH_DEVID_S;

/*码流信息*/
typedef struct tagVVV_SEARCH_STREAMINFO
{
    char szStream[VVV_SEARCH_STRNO_LEN + 1];/*码流号("chnno"+1或"chnno"+2)*/
    unsigned int bIsSub;                   /*标明是主码流还是从码流，0代表主码流,1代表从码流*/
}VVV_SEARCH_STREAMINFO_S;

/*通道信息*/
typedef struct tagVVV_SEARCH_CHNINFO
{
    char szChn[VVV_SEARCH_CHNNO_LEN + 1]; /*通道号(1－16)*/
    unsigned int u32StreamCnt;          /*某个通道存在u32StreamCnt个码流*/
    VVV_SEARCH_STREAMINFO_S* pstruStreamInfo;/*该通道对应的码流信息*/ 
}VVV_SEARCH_CHNINFO_S;

/*设备信息*/
typedef struct tagVVV_SEARCH_DEVINFO
{
    char aszIP[VVV_SEARCH_IP_STRSIZE + 1];          /*IP地址*/
    char aszMASK[VVV_SEARCH_IP_STRSIZE + 1];        /*子网掩码*/
    char aszMAC[VVV_SEARCH_MAC_STRSIZE + 1];        /*MAC地址*/
    char aszGTW[VVV_SEARCH_IP_STRSIZE + 1];         /*网关地址*/
    char aszDevID[VVV_SEARCH_DEVID_STRSIZE + 1];    /*设备ID，随机生成的32个字符*/
    char aszDevMDL[VVV_SEARCH_DEVNAME_STRSIZE + 1]; /*设备型号*/
    char aszSwVersion[VVV_SEARCH_SWVER_STRSIZE + 1];/*软件版本*/
    char aszDevName[VVV_SEARCH_DEVNAME_STRSIZE + 1];/*设备名*/
    char aszHttpPort[VVV_SEARCH_IP_STRSIZE + 1];    /*HTTP 监听端口*/
    char aszMediaPort[VVV_SEARCH_IP_STRSIZE + 1];   /*媒体监听端口*/
    unsigned int  u32ChnCnt;                       /*某设备存在u32ChnCnt个通道*/
    VVV_SEARCH_CHNINFO_S* pstruChnInfo;             /*该设备中的通道信息*/
} VVV_SEARCH_DEVINFO_S;

/*处理搜索响应接口*/
typedef int (*PTR_VSCP_DEVS_Search_RPNProcFN)(
                       const void* pvHandle,        /*句柄*/
                       char* pszRNPCode,            /*码字*/
                       VVV_SEARCH_DEVINFO_S* pstruDevInfo,/*搜索结果信息*/
                       void* pvUserData);           /*用户数据*/

/*命令反馈信息*/
typedef struct tagVVV_SEARCH_CMD_RES
{
    char* pszCmd;        /*命令*/
    char* pszResultInfo; /*反馈结果*/
} VVV_SEARCH_CMD_RES_S;

/*处理设置响应接口*/
typedef int (*PTR_VSCP_DEVS_Cmd_RPNProcFN)(
               const void* pvHandle,/*句柄*/
               char* pszRNPCode,    /*码字*/
               VVV_SEARCH_CMD_RES_S* pstruResponseInfo,/*反馈信息*/
               void* pvUserData);  /*用户数据*/

/*初始化设备搜索*/
int VVV_SEARCH_Init();

/*去初始化设备搜索*/
int VVV_SEARCH_DeInit();

/*启动设备搜索*/
int VVV_SEARCH_Start();

/*发送设备信息修改命令*/
int VVV_SEARCH_Modify(const VVV_SEARCH_DEVID_S* pstruDEV,
                     const char* pszCmd);/*设备修改命令,比如"netconf set -devname eth0 -ipaddr 192.168.0.134 -netmask 255.255.255.0 -gateway 192.168.0.1 -hwaddr 00:00:35:12:01:22"*/

/*注册设备搜索结果回调*/
int VVV_SEARCH_RegistSearchResultProc(PTR_VSCP_DEVS_Search_RPNProcFN pfunSearchRProc,void* pvUserData);

/*注册修改设备信息命令的回调*/
int VVV_SEARCH_RegistModifyResultProc(PTR_VSCP_DEVS_Cmd_RPNProcFN pfunCmdRProc,void* pvUserData);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __VVV_SEARCH_H__ */
    
    

