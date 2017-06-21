

#ifndef __GUI_H__
#define __GUI_H__

#include "MultiTask/MsgQue.h"
#include "MultiTask/Thread.h"
#include "MultiTask/Timer.h"

//#include "System/CMOS.h"  //遥控器

#include "Pages/PageDesktop.h"
#include "biz.h"
#include "Pages/PageMenueOutputDev.h"
#include "Pages/PageGuide.h"


class CDevMouse;
class CDevGraphics;
class CDevFrontboard;

class CPage;

class CPageDesktop;
#ifdef MON_PROTECT_SCREEN//add by yanjun 20061114
class CPageAdvancedSrcProtect;
#endif
class CPageNoModuleMessageBox;

//#include "System/AppEvent.h"

class CGUI : public CObject
{
public:
	PATTERN_SINGLETON_DECLARE(CGUI);
	CGUI();
	~CGUI();

//	void onAppEvent(appEventCode code, int index, appEventAction action, EVENT_HANDLER *param, const CConfigTable* data);

	VD_BOOL Start();
	VD_BOOL Stop();
	
	void HideScreen();
	
	void InitObjects();
	VD_BOOL CreatePages();
	void onFrontboardIntput(uint message, uint param0, uint param1);
	void OnMouseInput(uint message, uint param0, uint param1);
	void OnDevInput(uint message, uint param0, uint param1);
	void OnHumanInput(uint message, uint param0, uint param1, int screen = 0);
	void Logout(int iPageFlag = 0);
	void OnIdleCount(uint param);
	void OnCaret(uint param);
	void CountDown();
	CPage * GetDesktop();
	void SetDesktop(CPage *pPage);
	void RemoteEnable(VD_BOOL bFlag);
	void SetVideoStandard(uint dwStandard);
	void UpdateGraphics();

	//for resizing
	uchar	PageXRate[16];	//16种页面x坐标比率
	uchar	PageYRate[16];	//16种页面y坐标比率

	void ShowStartPage();

private:
	CTimer m_InputIdleTimer;	//自动注销定时器
	int	m_LogoutCount;
	int	m_LCDScreenClsDown;
	uchar m_OutPutChange;
	CTimer m_CaretTimer;	//插入符闪烁定时器
#ifdef MON_PROTECT_SCREEN
	int	m_ScreenSaverCount;
#endif

	// devices
	CDevMouse * m_pDevMouse;
	CDevGraphics * m_pDevGraphics;
	CDevFrontboard * m_pDevFrontboard;
//	CDevVideo		*m_pDevVideo;

	VD_BOOL m_bRemoteEnabled;
	CMutex m_Mutex;
	VD_BOOL m_bPagesCreated; //页面是否创建

	VD_RECT sNewMouseRect;

	// pages
private:
// 	CPageUpgrade	*pPageUpgrade;
// 	CPageUpgradeOK	*m_pPageUpgradeOK;
// 	CPageRemote		*pPageRemote;
// 	CPageNoModuleMessageBox *pMessageBox;
public:
	CPageDesktop	*pPageDesktop;
//	CPageDesktop	*pPageDesktop1;//csp modify 20130323

	CPageGuide		*pPageGuide;
	CPageMenueOutputDev *pPageStart;
};

#define g_GUI (*CGUI::instance())

#endif //__GUI_H__

