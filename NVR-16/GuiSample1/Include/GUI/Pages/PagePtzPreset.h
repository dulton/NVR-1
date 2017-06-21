#ifndef _PAGE_PTZ_PRESET_H_
#define _PAGE_PTZ_PRESET_H_

#include "PageFloat.h"
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "PagePtzTypes.h"

typedef struct _sPresetPara
{
	uchar 	nRate[128];
	uchar 	nId[128];
} SPresetPara;

class CPagePtzPreset:public CPageFloat
{
public:
	CPagePtzPreset(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPagePtzPreset();
	
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void OnSetPara(uchar nCh, SPresetPara* psPresetIns);
	void OnGetPara(uchar nCh, SPresetPara* psPresetIns);
	void ParentSendPara(uchar nCh, u32 nSelNumber);

private:

	void OnBtnDown();
	void ShowBtn();
	int curChn;
	int	ncurPreSit;
	int	ncurCirLine;
	int nCurChPg;
	int	PatFlag;
	int ScanFlag;
	int TourFlag;
	int nChPgNum;
	int nLastHideCh;
	int	TourGotoFlag;
	int nRealRowBtn;
	CRect m_TmpRect;
	VD_RECT	m_RectScreen;
	EMBIZPREVIEWMODE eMaxViewMode;
	BOOL bLeftBtnCtlPtz;
	BOOL IsDwell;
	
	int nChMax;
	int nCurCh; //parent send the ch to child
	int nCurSel; //parent send the select number to child
	
	SPresetPara* psPresetIns;
	STabParaUart TabPara;
	EMBIZPTZCMD  m_CurPTZCmd;
	EMBIZPTZCMD m_lastDirect;

	CStatic* pStatic[7];
	CStatic* pDrawLine;
	CSliderCtrl* pSlider;
	CComboBox* pComboBox;
	CButton* pButton[23+6+1];
	CTableBox* pTable0;
	CTableBox* pTable1;
	
	void OnClkOtherBtn();
	void OnClkChnSel();
	void OnClkDirection();
	void OnSliderChange();
	void HideWindow();

};

#endif




