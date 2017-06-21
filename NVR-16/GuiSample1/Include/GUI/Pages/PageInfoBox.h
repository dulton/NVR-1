#ifndef _PAGE_INFOBOX_H_
#define _PAGE_INFOBOX_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define ADVANCEWARNMENU_STATIC	2
#define ADVANCEWARNMENU_BUTTON	2

typedef void (* OKFUNC)();
class CPageInfoBox:public CPageFrame
{
public:
	CPageInfoBox(VD_PCRECT pRect,VD_PCSTR pTitle,VD_PCSTR pInfo,VD_BITMAP* icon = NULL,CPage* pParent = NULL);
	~CPageInfoBox();
	
	VD_PCSTR GetDefualtTitleCenter();
	void SetOkCallBack(OKFUNC okFunc);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
protected:
	
private:
	
	int curMainItemSel;
	int totalItemNum;
	
	void OnClkItem();
	
	//void OnTest();
	OKFUNC m_okFunc;
	
	CStatic* pStatic[ADVANCEWARNMENU_STATIC];
	CButton* pButton[ADVANCEWARNMENU_BUTTON];
	
	//CPageRecordFrameWork* m_pPageRecord;
	//CPageRecordScheduleFrameWork* m_pPageRecordSchedule;
	//CPageSensorAlarmFrameWork* m_pPageSensorAlarm;
};

#endif //_PAGE_INFOBOX_H_

