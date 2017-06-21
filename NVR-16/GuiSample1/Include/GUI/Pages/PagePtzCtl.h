#ifndef _PAGE_PTZ_CTL_H_
#define _PAGE_PTZ_CTL_H_

#include <bitset>
#include "PageFloat.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "PagePtzTypes.h"

using std::bitset;

#define TOURPATH_MAX	32

//用来保存各个通道云台控制状态位
//轨迹,扫描,巡航
//最多支持64通道数
typedef struct 
{
	bitset<64> bitTrack;
	bitset<64> bitScan;
	bitset<64> bitTour;
	uchar  nPos[64];
} SPtzCtlInfo;

typedef enum
{
	EM_PTZ_BTM_GOTOPRESET = 20,
	EM_PTZ_BTM_GOTOTOUR,
	EM_PTZ_BTM_TRACK,
	EM_PTZ_BTM_SCAN,
	EM_PTZ_BTM_TOUR,
} EMPTZBTMCTL;

class CPagePtzCtl:public CPageFloat
{
public:
	CPagePtzCtl(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPagePtzCtl();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	BOOL ChnPtzIsAble(u32 nChn);
	void MessgeBox();
	BOOL ReflushChnName();
	void BtmStatusUpdate(u32 nChn);
	void SetCtlBtmStatusCtl(EMPTZBTMCTL emBtmCtl);
	BOOL BtmEnable(EMPTZBTMCTL emBtmCtl);
	BOOL AllBtmEnable();
	void SetCruiseStage(int flag);
	int GetCruiseState();
private:
	void OnBtnDown();
	void ShowBtn();
	int curChn;
	int	ncurPreSit;
	int	ncurCirLine;
	int nCurChPg;
	BOOL bPatFlag;
	BOOL bPatIsRunning;
	BOOL bScanFlag;
	BOOL bScanIsRunning;
	BOOL bTourFlag;
	BOOL bTourIsRunning;
	BOOL IsDwell;
	int nFixCruiseNu;
	int nChPgNum;
	int nLastHideCh;
	int nRealRowBtn;
	CRect m_TmpRect;
	VD_RECT	m_RectScreen;
	EMBIZPREVIEWMODE eMaxViewMode;
	BOOL bLeftBtnCtlPtz;
	SPtzCtlInfo *psPtzCtlInfo;
	
	int nChMax;
	int nPath;
	int nPresetSet;
	int nLastClkBtn;
	EMBIZPTZCMD m_CurPTZCmd;
	EMBIZPTZCMD m_lastDirect;
	
	
	CStatic* pStatic[8];
	CSliderCtrl* pSlider;
	CComboBox* pComboBox[2];
	CButton* pButton[27+12+1]; // 最多一行16个按钮 1-5 other(22) 6-16 all down 
	SGuiPtzAdvancedPara *psGuiPtzPara;
	sGuiPtzAdvanPara *psGuiPtzCount;
	STabParaUart	TabPara;
	int bincurisepage;
	void OnClkOtherBtn();
	void OnClkChnSel();
	void OnClkDirection();
	void OnSliderChange();
	void OnComboSel();
	void HideWindow();

	void SetPresetList();

};

#endif


