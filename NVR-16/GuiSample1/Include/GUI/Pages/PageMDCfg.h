#ifndef _PAGE_MDCFG_H_
#define _PAGE_MDCFG_H_

#include "PageFrame.h"
#include "PageAlarmDeal.h"
#include "PageAlarmCfgFrameWork.h"

#define MDCFG_SUBPAGES		2
#define MDCFG_COMMBUTTON (MDCFG_SUBPAGES+3)  //tabs + default,apply,exit

struct MDCFG_MDITEM
{
	CStatic* pChnIndex;
	CCheckBox* pChkEnable;
	CComboBox* pComboDelay;
	CButton* pBtAlarmDeal;
	CButton* pBtRgnSet;
};

typedef struct 
{
	uchar bEnable;
	uint  nDelay;
} SMDCfg;

class CPageAlarmDeal;
class CPageMotionRgnSet;
class CPageMDCfg:public CPageFrame
{
public:
	CPageMDCfg(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMDCfg();
	
	VD_BOOL UpdateData( UDM mode );
	
	void GetPara(uchar nCh, SGuiAlarmDispatch *pPara);
	void SavePara(uchar nCh, SGuiAlarmDispatch *pPara);
	void SaveDealPara2Cfg(uchar nRealChNum);
	
	void SetSch( uchar nCh, uchar idx, uint* sch);
	void GetSch( uchar nCh, uint* sch);
	void ShowSch( uchar nCh );

private:
	
	CScrollBar* 	pScrollbar;
	SMDCfg*			pTabPara;
	int             nRealRow;
	int             nPageNum;
	int             nHideRow;
	int             nCurPage;
	void 			OnTrackMove0();
	void			AdjHide();
	int 			nChMax;
	void             OnEnable();
	void             OnDelay();
	void 			LoadPara();
	
	void OnComboBoxChn();
	void OnClkCopy();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	CGridCtrl** GetCurGrid();
	uchar GetCurChn();
	
	uchar* bSchChange;
	uint** sch;
private:
	SGuiAlarmDispatch *psAlarmDispatchIns;
	uchar bModified;

	CStatic* pIconInfo;
	CStatic* pInfoBar;
	void SetInfo(char* szInfo);
	void ClearInfo();

private:
	int m_nCurID;

	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);

	void OnClickSubPage();

	CButton* pButton[MDCFG_COMMBUTTON];
	CTableBox* pTabFrame;

 	void InitPage0();	
	CTableBox* pTabPage0[2];
	CCheckBox* pChkChnAllPage0;
	CCheckBox* pChkEnableAllPage0;
	CComboBox* pComboDelayAllPage0;	
	CButton* pBtAlarmDealAllPage0;
	CButton* pBtRgnSetAllPage0;
	void OnClkAlarmDeal();
	void OnClkRgnSet();
	
	void OnClkAlarmDealAll();   //cw_md
	void OnClkRgnSetAll();

	MDCFG_MDITEM tMDPage0[4+2];

	CPageAlarmDeal* m_pPageAlarmDeal;
	CPageMotionRgnSet* m_pPageMDRgnSet;

	void InitPage1();
	CComboBox* pComboChnPage1;
	CComboBox* pComboCopy2ChnPage1;
	CButton* pBtCopyPage1;
	CTableBox* pTabSCHPage1;
	CGridCtrl* pSCHGridPage1[7];



};


#endif
