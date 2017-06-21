#ifndef __DDNSTL_H_
#define __DDNSTL_H_

#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif


// #define 
#define DDNS_USER_LEN_MAX	32


// typdef 

// support prot
typedef enum
{
	DDNS_LOGIN = 0,
	DDNS_REGIST,
	DDNS_CANCEL,
	DDNS_LOGIN_TRY,
	DDNS_HEART
} CMD_TYPE;

//** typedef 
typedef struct _sTlDdnsInitPara
{
	u16 nPort;
	
} STlDdnsInit;

typedef struct _sTlDdnsRegPara
{
	char usrname[DDNS_USER_LEN_MAX];
	char passwd[DDNS_USER_LEN_MAX];
	char domain_name[DDNS_USER_LEN_MAX];
} STlDdnsReg;

typedef struct _sTlDdnsUnRegPara
{
	
} STlDdnsUnReg;

typedef struct _sTlDdnsRefreshPara
{
	
} STlDdnsRefresh;


// init
void ddnstl_Init( void* pInitPara );

// regist
void ddnstl_Regist( void* pRegPara );
void jmdvr_Regist( void* pRegPara );
void jsjdvr_Regist( void* pRegPara );

// unreg
void ddnstl_Unreg( void* pUnRegPara );

// start
void ddnstl_Start( void* pStartPara );
void jmdvr_Start( void* pRegPara );
void jsjdvr_Start( void* pRegPara );

// start try
void ddnstl_StartTry( void* pStartPara );

// stop
void ddnstl_Stop( void* pStopPara );
void jmdvr_Stop( void* pRegPara );
// refresh
void ddnstl_Refresh( void* pRefreshPara );

// deinit
void ddnstl_DeInit();

// get err
void ddnstl_GetErr();

int check_ip_tl(int* Time_check);

void konlan_Regist( void* pRegPara );
void konlan_Start( void* pStartPara );
void konlan_Stop( void* pRegPara );

#ifdef __cplusplus
}
#endif


#endif // __DDNSTL_H_

