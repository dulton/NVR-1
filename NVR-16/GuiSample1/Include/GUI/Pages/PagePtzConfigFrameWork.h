#ifndef _PAGE_PTZCFGFRAMEWORK_H_
#define _PAGE_PTZCFGFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>
#include "PagePtzPreset.h"
#include "PagePtzTrack.h"
#include "PageCruiseLine.h"
#include "PagePtzTypes.h"

#define PTZCFG_SUBPAGES	2
#define PTZCFG_BTNNUM	5
#define EDITLEN	64

class CPageCruiseLine;
class CPagePtzTrack;
class CPagePtzPreset;
class CPagePresetSetup;
class CPagePtzConfigFrameWork :public CPageFrame
{
public:
	CPagePtzConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPagePtzConfigFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
		
	void SaveCruiseLine(uchar nCh, SGuiTourPath* pPIns);
	void SavePresetSet(uchar nCh, uchar *pCChar, uchar npPInt);
	void SetOpenFlag(BOOL bFlag);
	VD_BOOL Close(UDM mode);
	void ChildClose();
protected:


private:
	void OnCombo0();
	void OnCheckBox0();
	void OnEdit0();
	BOOL Digital(const char *s);
	void MessgeBox(int index);
	BOOL bChildClose;

	void LoadPara();
#define TAB_NUM	2
	CScrollBar* 	pScrollbar[TAB_NUM];
	STabParaUart*	pTabPara;
	int             nRealRow[TAB_NUM];
	int             nPageNum[TAB_NUM];
	int             nHideRow[TAB_NUM];
	int             nCurPage[TAB_NUM];
	void 			OnTrackMove0();
	void 			OnTrackMove1();
	void			AdjHide();
	
	void GetCruiseLine(uchar nCh);
	SGuiPtzAdvancedPara *psGuiPtzPara;

	int nChMax;
	int curID;
	char *pcPtzAddr[EDITLEN];
	BOOL m_bOpenFlag; //父页面0，子页面1

	//void OnClkConfigItem();
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);
	void OnClickSubPage();

	CButton* pButton[PTZCFG_BTNNUM];
	CTableBox* pTable;
	CStatic* pInfoBar;
	CStatic* pIconInfo;
	//CButton* pItemButton[3];

	void SetInfo(char* szInfo);
	void ClearInfo();

	//"现场"子页面
	void InitPage0();
	CStatic* pStatic0[10+2];
	CCheckBox* pCheckBox0[6+2];
	CComboBox* pCombo0[2*6+2];
	CTableBox* pTable00;
	CTableBox* pTable01;
	CEdit* pEdit0[4+2];
	
	//CTableBox* pTable01;

	//"主输出"子页面
	void InitPage1();
	void OnPresetSetup();
	void OnCruiseLineSetup();
	void OnTrackSetup();
	CStatic* pStatic1[8+4];
	CButton* pButton1[12+4*3];
	CTableBox* pTable10;


	CPageCruiseLine* m_pPageCruiseLine;
	CPagePtzTrack* m_pPagePtzTrack;
	CPagePtzPreset* m_pPagePtzPreset;
	CPagePresetSetup* m_pPagePresetSetup;

};

#endif //_PAGE_PTZCFGFRAMEWORK_H_




