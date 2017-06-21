#ifndef _PAGE_STARTFRAMEWORK_H_
#define _PAGE_STARTFRAMEWORK_H_

#include <vector>
#include <bitset>
//#include "PageFrame.h"

#include "PageFloat.h"

#include "biz.h"

#define START_BUTTON_NUM   (12+4*2)
//yaogang modify 20151203
#define AutoCloseTimeoutms	(3*60*1000)	//3分钟


class CPageMainFrameWork;
class CPageSplitFrameWork;
class CPagePlayBackFrameWork;
class CPageStartFrameWork;
class CPageChnSelect;
class CPageAudioSelect;
class CPageColorSetup;
class CPagePtzCtl;
class CPageDesktop;

class CPagePIP;//csp modify 20130501

class CPageStartFrameWork :public CPageFloat//public CPageFrame
{
public:
	CPageStartFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageStartFrameWork();
	
	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	
	void SetCurPreview(int curPre);
	int GetCurPreview();
	
	void SetMute(BOOL bMute);
	void SetPatrol(BOOL bEnable);
	void SetDesktop(CPage * pDesktop);
	void SetPrePageDefault();
	void SetInfo(char* szInfo);
	void ClearInfo();
	
	void SetCurPreviewMode(EMBIZPREVIEWMODE previewMode);
	EMBIZPREVIEWMODE GetCurPreviewMode();
	
	void DealManualRec();
	void SetRecordingStatue(u8 nChn, u8 flag);
	void SinglePreview(BOOL bIsSigPre);
    
	void EventPreviewFreshSplit(EMBIZPREVIEWMODE mode, u8 nModPara);
	virtual VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	
	void SetPIPPreviewMode(int nModePara);//csp modify 20130501
	//yaogang modify 20151203
	void AutoCloseTimerFxn(uint param);
protected:
	
private:
	int nChMax;
	
	void OnClkStartBtn();
	void SplitEx(CPageDesktop* pDeskTop, EMBIZPREVIEWMODE mode, u8 nModPara);
	
	int m_curPre;
	u32 nCurPage;
	BOOL bIsSinglePreview;
	int OutputIndex;//CVBS=0, VGA=1,2
	
	//标记点击预览的按键
	std::bitset<8> bitvec;
	
	CButton* pButton[START_BUTTON_NUM];
	VD_RECT	m_RectScreen;
	EMBIZPREVIEWMODE previewMode;
	BOOL m_bRecord[64];
	BOOL m_bMD;
	BOOL m_bPatrol;
	BOOL m_nGoDesktop;
	
	SBizCfgPatrol psPara;
	
	CRect pbFloat;
	//CStatic *pIconInfo;
	CStatic* pInfoBar;
	CPage* m_pDesktop;
	CPageMainFrameWork* m_pPageMain;
	CPageSplitFrameWork* m_pPageSplit;
	CPagePlayBackFrameWork* m_pPagePlayBack;
	CPageChnSelect* m_pPageChnSel;
	CPageChnSelect* m_pPageRecChnSel;
	CPageChnSelect* m_pPageChnSelMul[5]; // 最多支持32路
	CPageAudioSelect* m_pPageAudioSel;
	CPageColorSetup* m_pPageColorSetup;
	CPagePtzCtl* m_pPagePtzCtl;
	
	CPagePIP* m_pPagePIP;//csp modify 20130501
	//yaogang modify 20151203
	CTimer *m_AutoCloseTimer;//一段时间无操作，关闭page
};

#endif //_PAGE_STARTFRAMEWORK_H_

