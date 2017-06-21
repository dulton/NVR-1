
#ifndef __VVV_DEVS_H__
#define __VVV_DEVS_H__

#include "vvv_devs_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*初始化devs库*/
//nodetype devs_nodetype
//nodeid, 如果传入NULL. 系统自动生成, 当nodetype 为 device时, 请从外部传入
int devs_init(int nodetype, char* nodeid);

//获取版本号信息ver, 最小16Byte(含\0), 
//返回为版本号的数字标识
//ver是字符串标识
int devs_getver(char* ver);

//初始化搜索句柄, protocol_type 在 enum devs_protocol_type_e 定义, 现在请填入默认的0(devs_proto_mcast_v10)
//RET:-1, init error, RET >=0 Handle
//sendmode 对应的结构定义
//enum devs_sendmode
//{
//    devs_sendmode_defaultnic = 0xFE,-- 通过默认网关发送
//    devs_sendmode_allnic = 0xFF, -- 逐个网卡发送, 请使用这个参数
//};
int devs_init_handle(int protocol_type, int *pHandle, int sendmode);


int devs_deinit();
    
int devs_deinit_handle(int handle);

//开启设备搜索服务, 
int devs_start (int handle);

//发送设备搜索包, 这个要手工去发送
int devs_search(int handle);

//发送注册通知信息, only V2
int devs_notify_register(int handle);

//发送设备更改命令
int devs_sendcmd(int handle, VVV_SEARCH_DEVID_S *pdevinfo, char* cmd); 

int devs_regcb(int handle, struct devs_callback *pcbfuns);




#if defined(VVV_MALLOC)
    #undef VVV_MALLOC
#endif

#if defined(VVV_FREE)
    #undef VVV_FREE
#endif 

#ifdef _WIN32
    #define VVV_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
    #define VVV_FREE(x) HeapFree(GetProcessHeap(), 0, (x))    
#else
    #define VVV_MALLOC(x) malloc(x)
    #define VVV_FREE(x) free(x)
#endif


#define devs_devinfo_new_p(pdev) \
{\
    pdev = VVV_MALLOC(sizeof(VVV_SEARCH_DEVID_S));\
    pdev->pszDevID = VVV_MALLOC(64);\
    pdev->pszUserName = VVV_MALLOC(32);\
    pdev->pszPasswd = VVV_MALLOC(32);\
}
    
#define devs_devinfo_free_p(pdev) \
{\
    if (pdev) \
    {\
        if (pdev->pszDevID)  VVV_FREE(pdev->pszDevID);\
        if (pdev->pszUserName) VVV_FREE(pdev->pszUserName);\
        if (pdev->pszPasswd) VVV_FREE(pdev->pszPasswd);\
        VVV_FREE(pdev);\
    }\
}

#define devs_devinfo_setvalue(pdev, id, user, pwd) \
{\
            strcpy( pdev->pszDevID, id);\
            strcpy(pdev->pszUserName, user);\
            strcpy(pdev->pszPasswd, pwd);\
}
    
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif

