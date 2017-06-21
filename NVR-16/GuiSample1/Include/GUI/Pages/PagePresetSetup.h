#ifndef _PAGE_PRESET_CONFIG_H_
#define _PAGE_PRESET_CONFIG_H_

#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#ifndef MY_UCHAR_MAX//csp modify
#define MY_UCHAR_MAX	128
#endif

class CPagePtzPreset;
class CPagePresetSetup :public CPageFrame
{
public:
	CPagePresetSetup(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPagePresetSetup();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetChn(int chn);
	void OnGetPara(uchar nChn, uchar*pPchar);
	void OnSetPara(uchar nChn, uchar*pPchar);
	VD_BOOL Close(UDM mode);

protected:

private:
	int curID;
	int m_nChn;
	int m_page;
	int nChMax;
	uchar *m_ChValue[MY_UCHAR_MAX];
	BOOL bChileClose;

	SGuiPtzAdvancedPara psGuiPtzPara;

	//void OnClkConfigItem();
	void SwitchPage( int mainID,int subID);
	void ShowSubPage(int mainID,int subID, BOOL bShow);
	void OnClickSubPage();
	
	void OnClickPreset();
	void OnClickCtlBtn();
	void OnTrackMove();
	void OnCheckBox();
	void LoadPage();

	CStatic* pTitle[4];
	CStatic* pID[10];
	CStatic* pSelAll;
	CCheckBox* pEnable[10];
	CCheckBox* pAllEnable[2];
	CEdit* pName[10];
	CButton* pPreset[10];
	CScrollBar* pScrollbar;

	CButton* pBtnCtl[2];
	CTableBox* pTable;

	CPagePtzPreset* m_pPagePtzPreset;
	
};

#endif //_PAGE_PRESET_CONFIG_H_

