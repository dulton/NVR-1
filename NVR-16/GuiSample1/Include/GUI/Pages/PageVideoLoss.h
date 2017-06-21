#ifndef _PAGE_VIDEOLOSS_H_
#define _PAGE_VIDEOLOSS_H_

#include "PageFrame.h"
#include "PageAlarmDeal.h"
#include "PageAlarmCfgFrameWork.h"

#define VIDEOLOSS_ROWS 8

class CPageAlarmDeal;
class CPageVideoLoss:public CPageFrame
{
public:
	CPageVideoLoss(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageVideoLoss();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	void GetPara(uchar nCh, SGuiAlarmDispatch *pPara);
	void SavePara(uchar nCh, SGuiAlarmDispatch *pPara);
	int SaveDealPara2Cfg(uchar nRealChNum);
	
private:
	uchar nChMax;
	SGuiAlarmDispatch *psAlarmDispatchIns;
	uchar bModified;

private:
	CTableBox* pTab[2];
	CCheckBox* pChkChnAll;
	CButton* pBtDealAll;
	CButton* pButton[3]; //default apply exit
	CStatic* pInfoBar;
	CStatic* pIconInfo;
	CScrollBar* pScroll;

	int m_page;
	int m_maxPage;

	void AdjustRows();
	void OnTrackMove();
	void SetInfo(char* szInfo);
	void ClearInfo();

	void OnClkAlarmDeal();
	CButton* pBtAlarmDeal[VIDEOLOSS_ROWS];
	CStatic* pStaticChn[VIDEOLOSS_ROWS];

	void OnClkAlarmDealAll();
	void OnClkApp();

	CPageAlarmDeal* m_pPageAlarmDeal;
};

#endif

