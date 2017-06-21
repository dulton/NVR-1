#ifndef _FHDEV_DISCOVER_DEFINE_H_
#define _FHDEV_DISCOVER_DEFINE_H_

#ifdef WIN32    // win32

#else           // linux

#ifndef BOOL
#   define BOOL         int
#endif

#ifndef DWORD
#   define DWORD        unsigned int
#endif

#ifndef LPDWORD
#   define LPDWORD      unsigned int*
#endif

#ifndef WORD
#   define WORD         unsigned short
#endif

#ifndef BYTE
#   define BYTE         unsigned char
#endif

#ifndef LPVOID
#   define LPVOID       void*
#endif

#ifndef DWORD64
#   define DWORD64      unsigned long long
#endif

#ifndef TRUE
#   define TRUE    1
#endif
#ifndef FALSE
#   define FALSE   0
#endif
#ifndef NULL
#   define NULL    0
#endif

#endif



typedef enum
{
    FHNPEN_DType_Client   = 0x00,     // 客户端
    FHNPEN_DType_61       = 0x01,     // 61
    FHNPEN_DType_Decoder  = 0x02,     // 解码器
    FHNPEN_DType_DVR      = 0x03,     // DVR
    FHNPEN_DType_NVR      = 0x04,     // NVR
    FHNPEN_DType_NVS      = 0x05,     // NVS
    FHNPEN_DType_DVS      = 0x06,     // DVS

    FHNPEN_DType_AllDev   = 0xff,     // 所有设备
}FHNPEN_DevType_e;


/* IP地址结构体 */
typedef struct
{
    char sIPV4[16];
    BYTE sIPV6[128];
} FHDS_IPAddr_t, *LPFHDS_IPAddr_t;


/* 公用网络信息 */
typedef struct
{
    char            sMAC[18];       // MAC
    FHDS_IPAddr_t   stIP;           // IP
    FHDS_IPAddr_t   stMaskIP;       // 子网掩码
    FHDS_IPAddr_t   stGateway;      // 网关    
    WORD            wPort;          // PORT
    BYTE            btRes[2];
} FHDS_PubNetAddr_t, *LPFHDS_PubNetAddr_t;


/* 解码器私有数据 */
typedef struct 
{
    BYTE btRes[4];
} FHDS_DECPriInfo_t, *LPFHDS_DECPriInfo_t;


/* DVR私有数据 */
typedef struct
{
    BYTE btRes[4];
} FHDS_DVRPriInfo_t, *LPFHDS_DVRPriInfo_t;


/* NVR私有数据 */
typedef struct
{
    BYTE btRes[4];
} FHDS_NVRPriInfo_t, *LPFHDS_NVRPriInfo_t;


/* NVS私有数据 */
typedef struct
{
    BYTE btRes[4];
} FHDS_NVSPriInfo_t, *LPFHDS_NVSPriInfo_t;


/* DVS私有数据 */
typedef struct
{
    BYTE btRes[4];
} FHDS_DVSPriInfo_t, *LPFHDS_DVSPriInfo_t;


/* 61私有数据 */
typedef struct
{
    BYTE btRes[4];
} FHDS_61PriInfo_t, *LPFHDS_61PriInfo_t;


/* 设备私有数据联合体 */
typedef union
{
    FHDS_DECPriInfo_t       stDECPriInfo;     // DEC私有数据
    FHDS_DVRPriInfo_t       stDVRPriInfo;     // DVR私有数据
    FHDS_NVRPriInfo_t       stNVRPriInfo;     // NVR私有数据
    FHDS_NVSPriInfo_t       stNVSPriInfo;     // NVS私有数据
    FHDS_DVSPriInfo_t       stDVSPriInfo;     // DVS私有数据
    FHDS_61PriInfo_t        st61PriInfo;      // 61私有数据
} FHDS_DevPrivateInfo_t, *LPFHDS_DevPrivateInfo_t;


/* 设备状态信息 */
typedef struct 
{
    DWORD                   dwDevType;        // 设备类型(FHNPEN_DevType_e)
    FHDS_PubNetAddr_t       stDevNetAddr;     // 网络地址
    char                    sDevVersion[48];  // 设备版本
    char                    sDeviceName[32];  // 设备名称
    char                    sSerialNumber[48];// 序列号
    DWORD                   dwTimeOut;        // 未激活次数
    BOOL                    bIsAlive;         // 在线/离线
    FHDS_DevPrivateInfo_t   stPriData;        // 设备私有信息联合体
} FHDS_DevState_t, *LPFHDS_DevState_t;


/* 设备网络地址信息 */
typedef struct 
{
    DWORD                   dwDevType;        // 设备类型(FHNPEN_DevType_e)
    FHDS_PubNetAddr_t       stDevNetAddr;     // 网络地址
} FHDS_ModifyNetAddr_t, *LPFHDS_ModifyNetAddr_t;


#endif
