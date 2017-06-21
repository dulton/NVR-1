#ifndef _PAGE_ADVANCECONFIGFRAMEWORK_H_
#define _PAGE_ADVANCECONFIGFRAMEWORK_H_

#include "PageFrame.h"
#include "GUI/Pages/PageMainTain.h"
#include <vector>

#define ADVANCECFGMENU_ITEMS	5


//class CPageAdvanceWarning;
class CPageImportExportFrameWork;
class CPageResumeDefaultCfg;
class CPageMainTain;

class CPageAdvanceConfigFrameWork :public CPageFrame
{
public:
	CPageAdvanceConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAdvanceConfigFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	void ReflushItemName();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );//cw_lock

protected:


private:

	int curMainItemSel;
	int totalItemNum;

	void OnClkAdvanceCfgItem();

	//void OnTest();

	CStatic* pStatic[ADVANCECFGMENU_ITEMS];
	CButton* pButton[ADVANCECFGMENU_ITEMS];
	CStatic* pStaticPic;
	CStatic* pInfoBar;
	
	//CPageAdvanceWarning* m_pPageAdvanceWarn;
	CPageImportExportFrameWork* m_pPageImportExport;
	CPageResumeDefaultCfg* m_pPageResumeDefaultCfg;
	CPageMainTain*	m_pPageMainTain;

};

#endif //_PAGE_ADVANCECONFIGFRAMEWORK_H_



