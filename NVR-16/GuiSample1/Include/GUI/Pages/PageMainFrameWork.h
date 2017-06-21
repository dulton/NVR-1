#ifndef _PAGE_MAINFRAMEWORK_H_
#define _PAGE_MAINFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define MAINMENU_ITEMS	10

class CPageConfigFrameWork;
class CPageSearch;
class CPageManualAlarm;
class CPageDataBackup;
class CPageInfoFrameWork;
class CPageDiskMgr;
class CPageUpdate;
class CPageTest;
class CPageMainFrameWork :public CPageFrame
{
public:
	CPageMainFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMainFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	void ReflushItemName();

protected:
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:

	int curMainItemSel;

	int totalItemNum;

	void OnClkMainItem();

	CStatic* pStatic[MAINMENU_ITEMS];
	CButton* pButton[MAINMENU_ITEMS];
	CStatic* pStaticPic;

	CPageConfigFrameWork* m_pPageConfig;
	CPageSearch* m_pPageSearch;
	CPageManualAlarm* m_pPageManualAlarm;
	CPageDataBackup* m_pPageDataBackup;
	CPageInfoFrameWork* m_pPageInfo;
	CPageDiskMgr* m_pPageDskMgr;
	CPageUpdate* m_pPageUpdate;

	//CPageTest* m_pPageTest;
	//VD_BOOL UpdateData(UDM mode);
};

#endif //_PAGE_MAINFRAMEWORK_H_


