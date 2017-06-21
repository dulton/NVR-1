#pragma pack(push,1)

#define IIC_ADDR	0xa0
#define MYERROR		1
#define MYSUCCESS	0
#define VSWC_PORT	5511

#define ALARM_MSG_NUM	30
#define SERIAL_MSG_NUM  20

#ifndef WIN32
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID 
{
    unsigned long        Data1; 
    unsigned short       Data2; 
    unsigned short       Data3; 
    unsigned char        Data4[8]; 
} GUID;
#endif
#endif

#pragma pack( pop )
