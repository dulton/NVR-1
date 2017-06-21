#ifndef __DDNSOTHER_H_
#define __DDNSOTHER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common_basetypes.h"


// #define 

// typdef 

//** typedef 
typedef struct _sOtherDdnsInitPara
{
	u16 nPort;
	
} SOtherDdnsInit;

typedef struct _sOtherDdnsRegPara
{
	char usrname[32];
	char passwd[32];
	char domain_name[32];
} SOtherDdnsReg;

typedef struct _sOtherDdnsUnRegPara
{
	
} SOtherDdnsUnReg;

typedef struct _sOtherDdnsRefreshPara
{
	
} SOtherDdnsRefresh;


// init
void ddnsother_Init( void* pInitPara );
void dvrnet_Init( void* pInitPara );
// regist
void ddnsother_Regist( void* pRegPara );
void dvrnet_Regist( void* pRegPara );
// start
void dyndns_Start( void* pStartPara );
void ddns3322_Start( void* pStartPara );
void dvrnet_Start( void* pStartPara );
void ddnsnoip_Start( void* pStartPara );
void people_Start( void* pStartPara );
void changeip_Start( void* pStartPara );
// stop
void ddnsother_Stop( void* pStopPara );
void dvrnet_Stop( void* pStartPara );
// unreg
void ddnsother_Unreg( void* pUnRegPara );

// refresh
void dyndns_Refresh( void* pRefreshPara );
void ddns3322_Refresh( void* pRefreshPara );

// deinit
void ddnsother_DeInit();
void dvrnet_DeInit();
void ddnsother_GetErr();

#ifdef __cplusplus
}
#endif


#endif // __DDNS_H_


