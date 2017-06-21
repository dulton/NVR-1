#ifndef _PAGE_PTZ_TRACK_H_
#define _PAGE_PTZ_TRACK_H_

#include "PageFloat.h"
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "PagePtzTypes.h"


class CPagePtzTrack:public CPageFloat
{
public:
	CPagePtzTrack(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPagePtzTrack();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	VD_BOOL UpdateData( UDM mode );
	void ParentSendCh(uchar nCh);

private:
	void OnBtnDown();
	void ShowBtn();
	int nCurChPg;
	int nChPgNum;
	int nLastHideCh;
	int nRealRowBtn;
	EMBIZPREVIEWMODE eMaxViewMode;
	BOOL bLeftBtnCtlPtz;
	BOOL IsDwell;

	int nChMax;
	int curChn;
	int RecFlag;
	int PatFlag;
	CRect m_TmpRect;
	VD_RECT	m_RectScreen;
	EMBIZPTZCMD m_CurPTZCmd;
	EMBIZPTZCMD m_lastDirect;

	CStatic* pStatic[7];
	CStatic* pDrawLine;
	CSliderCtrl* pSlider;
	STabParaUart	TabPara;
	//CComboBox* pComboBox[2];
	CButton* pButton[24+6+1];
	CTableBox* pTable0;
	CTableBox* pTable1;
	CTableBox* pTable2;
	
	void OnClkOtherBtn();
	void OnClkChnSel();
	void OnClkDirection();
	void OnSliderChange();
	void HideWindow();

};

#endif



