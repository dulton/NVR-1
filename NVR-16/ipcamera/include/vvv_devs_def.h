#ifndef __VVV_DEVS_DEF_H__
#define __VVV_DEVS_DEF_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


enum devs_protocol_type_e
{
    devs_proto_mcast_v10 = 0,
    devs_proto_bcast_v20 = 1,
    devs_proto_max 
}  ;      
    
enum devs_nodetype
{
    devs_node_device  = 0 , //设备, 可以被搜索到, 被设置等
    devs_node_control = 1 , //控制点, 可以发送搜索命令等
    devs_node_mixed   = 2 , //TODO 可以发送, 也可以搜索, 
};

enum devs_sendmode
{
    devs_sendmode_defaultnic = 0xFE,//-- 通过默认网关发送
    devs_sendmode_allnic = 0xFF,  // -- 逐个网卡发送, 请使用这个参数
};

#define MAX_NIC_NUM 8
#define DEVSEARCH_DEVIDSIZE (32)
#define VVV_SEARCH_STRLEN (20)
#define VVV_SEARCH_STRNO_LEN       (20)
#define VVV_SEARCH_CHNNO_LEN       (20)
#define VVV_SEARCH_IP_STRSIZE      (15)
#define VVV_SEARCH_MAC_STRSIZE     (17)
#define VVV_SEARCH_DEVID_STRSIZE   (32)
#define VVV_SEARCH_DEVNAME_STRSIZE (40)
#define VVV_SEARCH_SWVER_STRSIZE   (32)


#define VVV_DEVS_MAX_RESPCODE_SIZE 8
#define VVV_DEVS_MAX_RESPMSG_SIZE  128


//============ V2 
struct devs_msg_head
{
    char client_id[DEVSEARCH_DEVIDSIZE + 1];
    char device_id[DEVSEARCH_DEVIDSIZE + 1 ];
    int  cseq;
    char cmdtype[16];
    char user[32];
    char pwd[32];
};


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

/*命令反馈信息*/
typedef struct tagVVV_SEARCH_CMD_RES
{
    char* pszCmd;        /*命令*/
    char* pszResultInfo; /*反馈结果*/
} VVV_SEARCH_CMD_RES_S;

/*处理搜索接口, use by node device*/
typedef int (*pfn_devs_search_req)(char* respcode, VVV_SEARCH_DEVINFO_S* pdevinfo,void* puser);
typedef int (*pfn_devs_cmd_req)(struct devs_msg_head  *msghead, char* cmd,  char* respmsg, char* respcode, void* puser);
 

/*处理搜索响应接口*/
/*保持兼容性, 接口按照原来的样子*/
typedef int (*pfn_devs_search_resp)(
                       const void* pvHandle,        /*句柄*/
                       char* pszRNPCode,            /*码字*/
                       VVV_SEARCH_DEVINFO_S* pstruDevInfo,/*搜索结果信息*/
                       void* pvUserData);           /*用户数据*/


/*处理设置响应接口*/
typedef int (*pfn_devs_cmd_resp)(
               const void* pvHandle,/*句柄*/
               char* pszRNPCode,    /*码字*/
               VVV_SEARCH_CMD_RES_S* pstruResponseInfo,/*反馈信息*/
               void* pvUserData);  /*用户数据*/





/*接收到搜索命令后的处理接口
msg 是收到的命令
resp是回复*/
typedef int (*pfn_devs_search_req_v2)(struct devs_msg_head * msghead, char* msg,  char* resp ,char* respcode, void* puser);   

/*接收到命令后的处理接口
msg 是收到的命令
resp是回复, 要发送到control端的*/
typedef int (*pfn_devs_cmd_req_v2)(struct devs_msg_head * msghead, char* msg, char*resp ,char* respcode, void* puser);           


/*接收到搜索命令回复后的处理接口
从这里解析搜索结果
respmsg-- 搜索回复字符串*/
typedef int (*pfn_devs_search_resp_v2)(struct devs_msg_head * msghead, char* respmsg, void* puser);   

/*接收到其他命令回复后的处理接口
从这里解析搜索结果
respmsg  命令回复字符串*/
typedef int (*pfn_devs_cmd_resp_v2)(struct devs_msg_head * msghead, char* respmsg, void* puser);   

//通知消息
typedef int (*pfn_devs_cmd_notify_v2)(struct devs_msg_head * msghead, char* msg, void* puser);   

struct devs_callback
{
    void* puser;                  /*设备信息处理接口使用的用户数据*/


    
    pfn_devs_search_resp pfn_search_resp;   /*用户注册设备信息处理接口*/
    pfn_devs_cmd_resp pfn_cmd_resp; /*用户注册设备信息处理接口*/

    //node_device
    pfn_devs_search_req      pfn_search_req;
    pfn_devs_cmd_req         pfn_cmd_req;
        
    //node_control, 如果是node_device , 这两个回调设为空
    pfn_devs_search_resp_v2 pfn_search_resp_v2;   /*用户注册设备信息处理接口*/
    pfn_devs_cmd_resp_v2 pfn_cmd_resp_v2; /*用户注册设备信息处理接口*/
    pfn_devs_cmd_notify_v2 pfn_cmd_notify_v2; /*用户注册回调, 当ipc有通知时, 调用这个回调*/
    
    //node device
    pfn_devs_search_req_v2 pfn_search_req_v2;   /*用户注册设备信息处理接口*/
    pfn_devs_cmd_req_v2 pfn_cmd_req_v2; /*用户注册设备信息处理接口*/

};





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
