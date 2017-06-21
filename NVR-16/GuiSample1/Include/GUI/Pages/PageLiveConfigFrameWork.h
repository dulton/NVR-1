#ifndef _PAGE_LIVEFRAMEWORK_H_
#define _PAGE_LIVEFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#include "GUI/Pages/BizData.h"


#define  LIVECONFIG_SUBPAGES 5

#define  LIVECFG_COLS 4
#define  LIVECFG_ROWS 6

#define  MASK_COLS 2
#define  MASK_ROWS 8

#define  VIDEOSRC_COLS 2
#define  VIDEOSRC_ROWS 8

enum
{
    LIVECONFIG_BT_LIVE = 0,
    LIVECONFIG_BT_MAINMONITOR,	
    LIVECONFIG_BT_SPOT,
    LIVECONFIG_BT_MASK,
    LIVECONFIG_BT_VEDIO,
    LIVECONFIG_BT_DEFAULT,
    LIVECONFIG_BT_APPLY,
    LIVECONFIG_BT_EXIT,

    LIVECONFIG_BTNNUM,
};


class CPageMaskSet;
class CPageColorSetup;
class CPageLiveConfigFrameWork :public CPageFrame
{
public:
	CPageLiveConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageLiveConfigFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

	void SetCurPreviewMode(EMBIZPREVIEWMODE emMode);

private:
	int curID;
	int chnNum;
	int OutputIndex;//CVBS=0, VGA=1,2
	EMBIZPREVIEWMODE m_previewMode ;

	//void OnClkConfigItem();
	void SwitchPage( int mainID,int subID);
	void ShowSubPage(int mainID,int subID, BOOL bShow);
	void OnClickSubPage();
	void AdjustLiveRows();
	void AdjustMaskRows();
	
	void WriteLogs();

	CButton* pButton[LIVECONFIG_BTNNUM];
	CTableBox* pTable;
	CStatic* pInfoBar;
	CStatic* pIconInfo;

	CEdit* pTmpEdit;
	CCheckBox* pTmpCheck;

	void SetInfo(char* szInfo);
	void ClearInfo();
	//CButton* pItemButton[3];

	//"现场"子页面
	void InitPage0();
	void OnSetup0();
	void OnEditChange0();
	void OnTrackMove0();
	void OnCheckAll0();
	void OnCheck0();
	int m_page0;
	int m_maxPage0;
	int* bCheckValue;
	char** szChName;
	CStatic* pStatic0[LIVECFG_ROWS+6];
	CEdit* pEdit0[LIVECFG_ROWS];
	CCheckBox* pCheckBox0[LIVECFG_ROWS+3];
	CButton* pButton0[LIVECFG_ROWS+1];
	CTableBox* pTable00;
	CTableBox* pTable01;
	CScrollBar* pScroll0;
	
	//CTableBox* pTable01;

	//"主输出"子页面0
	void InitPage10();
	void OnClkButton10();
	void OnCombox10();
	CStatic* pStatic10[3];
	CComboBox* pComboBox10[6];
	CButton* pButton10[2];
	CTableBox* pTable10;
	CTableBox* pTable11;

	//"主输出"子页面1
	void InitPage11();
	CComboBox* pComboBox11;
	CTableBox* pTable12;

	//"辅助输出"子页面0
	void InitPage20();
	void OnClkButton20();
	void OnCombox20();
	CStatic* pStatic20[3];
	CComboBox* pComboBox20[6];
	CButton* pButton20[2];
	CTableBox* pTable20;
	CTableBox* pTable21;

	//"辅助输出"子页面1
	void InitPage21();
	CComboBox* pComboBox21;
	CTableBox* pTable22;

	//"遮挡"子页面
	void InitPage3();
	void OnSetup3();
	void OnTrackMove3();
	int m_page3;
	int m_maxPage3;
	CStatic* pStatic3[MASK_ROWS+MASK_COLS];
	CButton* pButton3[MASK_ROWS];
	CTableBox* pTable30;
	CScrollBar* pScroll3;

	CPageMaskSet* m_pMaskSet;
	CPageColorSetup* m_pPageColorSetup;

	//"视频源"子页面
	void InitPage4();
	void OnTrackMove4();
	int m_page4;
	int m_maxPage4;
	CStatic* pStatic4[VIDEOSRC_ROWS+VIDEOSRC_COLS];
	CComboBox* pComboBox4[VIDEOSRC_ROWS];
	CTableBox* pTable4;
	CScrollBar* pScroll4;
	unsigned char m_nVedioSrc[128];
	unsigned char m_nVedioSrcLast[128];
	void OnComb4Sel();
};

#endif //_PAGE_LIVEFRAMEWORK_H_



