#ifndef _PAGE_CONFIGFRAMEWORK_H_
#define _PAGE_CONFIGFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

//NVR used
//#define CONFIGMENU_ITEMS	9
#define CONFIGMENU_ITEMS	11 //shixin modify for ShenGuang

class CPageRecordFrameWork;
class CPageBasicConfigFrameWork;
class CPageAdvanceConfigFrameWork;
class CPageLiveConfigFrameWork;
class CPagePtzConfigFrameWork;
class CPageRecordScheduleFrameWork;
class CPageAlarmCfgFrameWork;
class CPageUserManager;
class CPageNetworkConfig;
//class CPageAlarmOut;
class CPageIPCameraConfigFrameWork;//NVR used
class CPageShenGuangConfig;//shixin

class CPageConfigFrameWork :public CPageFrame
{
public:
	CPageConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageConfigFrameWork();
	
	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	VD_BOOL Close(UDM mode);//csp modify
	
	void ReflushItemName();
	VD_BOOL UpdateData(UDM mode);
	CPageUserManager* m_pUserMgr;
	
protected:
	
private:
	int curMainItemSel;
	int totalItemNum;
	BOOL bOpenPtz;
	
	void OnClkConfigItem();
	void OnTest();
	
	CStatic* pStatic[CONFIGMENU_ITEMS];
	CButton* pButton[CONFIGMENU_ITEMS];
	CStatic* pStaticPic;
	
	CPageRecordFrameWork* m_pPageRecord;
	CPageBasicConfigFrameWork* m_pPageBasic;
	CPageAdvanceConfigFrameWork* m_pPageAdvance;
	CPageLiveConfigFrameWork* m_pPageLive;
	CPagePtzConfigFrameWork* m_pPagePtzCfg;
	CPageRecordScheduleFrameWork* m_pPageRecordSche;
	CPageAlarmCfgFrameWork* m_pPageAlarmCfg;
	CPageUserManager* m_pPageUserMgr;
	CPageNetworkConfig* m_pPageNetworkCfg;
	//CPageAlarmOut* m_pPageAlarmout;
	
	CPageIPCameraConfigFrameWork* m_pPageIPCamera;//NVR used
	CPageShenGuangConfig* m_pPageShenGuangCfg; //shixin
};

#endif //_PAGE_CONFIGFRAMEWORK_H_

