#ifndef APP_EVENT
#define APP_EVENT

#include "MultiTask/Guard.h"
#include "MultiTask/Timer.h"
#include "System/pool_allocator.h"
#include "Object.h"
#include "Signals.h"
//#include "Configs/ConfigWorksheet.h"
//#include "Configs/ConfigEvents.h"

#define MAX_EVENT_LATCH_NUMBER 4

typedef enum app_event_action
{
	appEventStart = 0,		// 事件开始
	appEventStop,			// 事件结束
	appEventConfig,			// 事件配置变化，不作为接收参数
	appEventLatch,			// 事件延时结束，不作为发起参数
	appEventPulse			// 脉冲事件，没有起始和结束
}appEventAction;

typedef TSignal5<appEventCode, int, appEventAction, const EVENT_HANDLER *, const CConfigTable*> SIG_EVENT;

typedef struct tagEVENT_LATCH
{
	void*	handler;			// 需要进行事件闩锁的处理者
	SIG_EVENT::SigProc proc;	// 回调函数
	int		second;				// 事件闩锁的时间
}EVENT_LATCH;

typedef struct tagEVENT_ACTIVE
{
	VD_BOOL			inSection;			// 在时间段之内
	VD_BOOL			inStopping;			// 正在停止，因为延时的存在，所以没有删除
	const EVENT_HANDLER*	parameter;			// 事件处理参数
	const CONFIG_WORKSHEET*	workSheet;			// 事件处理工作表
	EVENT_LATCH		latch[MAX_EVENT_LATCH_NUMBER]; // 事件闩锁处理
	int				latchNumber;		// 事件闩锁个数
}EVENT_ACTIVE;

typedef struct tagEVENT_INFO
{
	appEventCode	code;
	int				index;
	const EVENT_HANDLER*	parameter;			// 事件处理参数
}EVENT_INFO;

class CAppEventManager : public CObject
{
	typedef std::map<int, EVENT_ACTIVE, std::less<int>, pool_allocator<EVENT_ACTIVE> > MAP_EVENT;

private:
	CAppEventManager(void);
	void onTimer(uint arg);
	void checkTimeSectionAll();

public:
	PATTERN_SINGLETON_DECLARE(CAppEventManager);

	VD_BOOL attach(void* handler, SIG_EVENT::SigProc proc);
	VD_BOOL detach(void* handler, SIG_EVENT::SigProc proc);
	VD_BOOL checkTimeSection(const CONFIG_WORKSHEET *workSheet);
	void notify(appEventCode code, int index = 0, appEventAction action = appEventStart, const EVENT_HANDLER *param = NULL, const CONFIG_WORKSHEET *workSheet = NULL, const CConfigTable* data = NULL);
	static const char*getEventName(appEventCode code);
	void getNextEvent();
	VD_BOOL getNextEvent(void* handler, EVENT_INFO* info);
	void latchEvent(void* handler, SIG_EVENT::SigProc proc, appEventCode code, int index, int second);
	void dump();

private:
	SIG_EVENT m_sigEventHandler;
	MAP_EVENT m_mapEventActive;
	CMutex m_mutex;
	CTimer m_timer;
	SYSTEM_TIME m_oldTime;
	VD_BOOL	m_latchAllowed;		// 是否允许延时处理：配置修改或者定时时间段到，不允许延时；事件结束，允许延时
	MAP_EVENT::iterator m_pi;	// 事件遍历迭代器
};

#endif

