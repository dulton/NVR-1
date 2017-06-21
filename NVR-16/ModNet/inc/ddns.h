#ifndef __DDNS_H_
#define __DDNS_H_

#include "ddns_tl.h"
#include "ddns_other.h"

#ifdef __cplusplus
extern "C" {
#endif


// #define 
//#define DDNS_USER_LEN_MAX	32

// typdef 

// DDNS操作回调
typedef void (*PDDNSInitCB)( void* para );
typedef void (*PDDNSRegistCB)( void* para );
typedef void (*PDDNSUnRegCB)( void* para );
typedef void (*PDDNSStartCB)( void* para );
typedef void (*PDDNSStopCB)( void* para );
typedef void (*PDDNSRefreshCB)( void* para );
typedef void (*PDDNSDeInitCB)();
typedef void (*PDDNSGetError)( s32* nErr );

// 支持协议
typedef enum 
{
#if 1//csp modify
	EM_DDNS_PROT_TL = 1,
	EM_DDNS_PROT_3322,
	EM_DDNS_PROT_DYNDNS,
	EM_DDNS_PROT_DVRNET,
	EM_DDNS_PROT_NOIP,
	EM_DDNS_PROT_JMDVR,
	EM_DDNS_PROT_CHANGEIP,
	EM_DDNS_PROT_JSJDVR,
	
	EM_DDNS_PROT_KONLAN,
	
	EM_DDNS_PROT_CCTVSVR,
	EM_DDNS_PROT_CCTVDVR,
	
	EM_DDNS_PROT_MYQSEE,
	EM_DDNS_PROT_DYNDDNS,
	EM_DDNS_PROT_PEOPLE,
	EM_DDNS_PROT_MAX
#else
	EM_DDNS_PROT_TL = 1,
	EM_DDNS_PROT_3322,
	EM_DDNS_PROT_DYNDNS,
	EM_DDNS_PROT_DVRNET,
	EM_DDNS_PROT_NOIP,
	EM_DDNS_PROT_JMDVR,
	EM_DDNS_PROT_CHANGEIP,
	EM_DDNS_PROT_JSJDVR,
	EM_DDNS_PROT_CCTVSVR,
	EM_DDNS_PROT_CCTVDVR,
	EM_DDNS_PROT_KONLAN,	
	EM_DDNS_PROT_MYQSEE,
	EM_DDNS_PROT_DYNDDNS,
	EM_DDNS_PROT_PEOPLE,
	EM_DDNS_PROT_MAX
#endif
} EM_DDNS_PROT;

// 
// 协议参数结构
typedef struct _sDDNSProtCfg
{
	u8				nIntvl;
	
	PDDNSInitCB 	pInit;
	PDDNSRegistCB 	pReg;
	PDDNSUnRegCB 	pUnreg;
	PDDNSStartCB	pStart, pStartTry;
	PDDNSStopCB		pStop;
	PDDNSRefreshCB 	pRefresh;
	PDDNSDeInitCB 	pDeInit;
	PDDNSGetError	pGetErr;
	
	union
	{
		// TL
		struct {
			;
		} sTL;
		
		// 3322
		struct {
			;	
		} s3322;
		
		// DynDns
		struct {
			;
		} sDynDns;
	};
	
	
} SDDNSProtCfg;

// Init Para
typedef struct _sDDNSInitPara
{
	u16 nPort;
	union
	{
		STlDdnsInit sTlIns;
	};
} SDDNSInitPara;

typedef struct _sDDNSBaseInfo
{
	char 			szUser[DDNS_USER_LEN_MAX];
	char 			szPasswd[DDNS_USER_LEN_MAX];
	char 			szDomain[DDNS_USER_LEN_MAX];
} SDDNSBaseInfo;

// Regist para
typedef struct _sDDNSRegPara
{
	SDDNSBaseInfo sBase;
	
} SDDNSRegPara;

// UnRegist para
typedef struct _sDDNSUnRegPara
{	
	SDDNSBaseInfo sBase;
} SDDNSUnRegPara;


// Start para
typedef struct _sDDNSStartPara
{
	u8	bTry; // 是否测试登陆 0 否， 1 是
	SDDNSBaseInfo sBase;
} SDDNSStartPara;


// Stop para
typedef struct _sDDNSStopPara
{	
	SDDNSBaseInfo sBase;
} SDDNSStopPara;

// 
// extern ..

// DDNS init
s32 DDNS_Init( u8 nFlag, u32 nUpIntvl );

// Install prot
s32 DDNS_Install( EM_DDNS_PROT eProt, SDDNSProtCfg* pProtCfg );

// Start prot
s32 DDNS_Start( EM_DDNS_PROT eProt );

// Uninstall
s32 DDNS_UnInstall( EM_DDNS_PROT eProt );

// init
s32 DDNS_SvcInit( EM_DDNS_PROT eProt, SDDNSInitPara* pInitPara );

// regist
s32 DDNS_SvcRegist( EM_DDNS_PROT eProt, SDDNSRegPara* pRegPara );

// unreg
s32 DDNS_SvcUnreg( EM_DDNS_PROT eProt, SDDNSUnRegPara* pUnRegPara );

// start
s32 DDNS_SvcStart( EM_DDNS_PROT eProt, SDDNSStartPara* pUnRegPara );

// stop
s32 DDNS_SvcStop( EM_DDNS_PROT eProt, SDDNSStopPara* pUnRegPara );

// refresh
s32 DDNS_SvcRefresh( EM_DDNS_PROT eProt, SDDNSRegPara* pRefreshPara );

// deinit
s32 DDNS_SvcDeInit(  EM_DDNS_PROT eProt );


#ifdef __cplusplus
}
#endif


#endif // __DDNS_H_


