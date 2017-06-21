#ifndef _IFLY_TIMER_H_
#define _IFLY_TIMER_H_

#include "common.h"

#define  MAX_USER_TIMER			2048

#define  TIMER_NORMAL			0
#define  TIMER_PERIOD			1

#define  TIMER_ACTIVE			0
#define  TIMER_PAUSE			1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct _tagTIMER
	{
		u32			timer_id       ;
		MSGQHANDLE	message_queque ;    
		u16			message        ;
		u16			state          ;
		u32			timer_type     ;
		int			old_interval   ;
		int			interval       ;

		u32         arg;

		struct  _tagTIMER  * next  ;
		
	}USER_TIMER ;
	
	/*struct _tagTIMER_MESSAGE
	{
		u16   sender    ;          
		u16   command   ;
		u32   timer_id  ;    
	} ;
	typedef struct _tagTIMER_MESSAGE TIMER_MESSAGE ;*/
	
	typedef ifly_msg_t TIMER_MESSAGE ;

	/*typedef struct
	{
		u16 sender;
		u16 event;
		u32 arg;
	}TIMER_MESSAGE;*/
	
	//外部接口
	int  InitTimer( void ) ;
	u32  SetTimer( MSGQHANDLE  queque ,  u16 message , int interval , u32 type ,u32 arg) ;  
	int  GetTimerInterval( u32 timer_id ) ;
	void KillTimer( u32  timer_id ) ;
	void ResetTimer( u32  timer_id ) ;
	void PauseTimer( u32  timer_id , int flag ) ;
	
	//任务入口
	void *TimerLoop( void * pParam ) ;
	
#ifdef __cplusplus
}
#endif // __cplusplus

#endif
