#ifndef _PAGE_PLAYBACKFRAMEWORK_H_
#define _PAGE_PLAYBACKFRAMEWORK_H_

#include "PageFloat.h"
//#include "PageChild.h"
#include <vector>

#include "biz.h"


#define PB_BUTTON_NUM   17
#define PB_STATIC_NUM   4

#define MAX_SUB_PER_MAIN	5

/*

typedef struct tagMAINFRAME_ITEM{
	VD_BITMAP*		bmp1;
	VD_BITMAP*		bmp2;
	std::string	string;
	std::string	tip;
	CButton*	pBtMainItem;
	CButton*	pBtSubItem[MAX_SUB_PER_MAIN];
	std::vector<CItem*> SubItems[MAX_SUB_PER_MAIN];
	int			cursubindex;
}MAINFRAME_ITEM;*/

class CPageAudioSelect;
class CPageChnSelect;
class CPageColorSetup;
class CPagePlayrateSelect;
class CPageSearch;
class CPagePlayBackVoColorSetup;
class CPagePlayBackFrameWork :public CPageFloat
{
public:
	CPagePlayBackFrameWork(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPagePlayBackFrameWork();

	VD_BOOL UpdateData( UDM mode );

	VD_PCSTR GetDefualtTitleCenter();
	void StopPb();
	void SetMute(BOOL bMute);
	void SetPlayrate(int rate, int type); //type 1: forward  0:backward
	void SetPbProg(int nProg);
	void SetPbCurTime(char* curTime);
	void SetPbTotalTime(char* totalTime);
	void SetPbTotalTime(int totalTime);
	void SetPbStartTime(int startTime);

	void SetPbInfo(SBizSearchPara sp);
	void SetPreviewMode(EMBIZPREVIEWMODE previewMode);
	void SetDesktop(CPage* pDesktop);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);	//消息处理
	void SetPlayChnNum(int nChnNum);
	void SetSearchPage(CPageSearch* pPage);
	void GetCurPlayMute(u8* mute);
	void SetCurPlayMute(u8 mute);
	void SetIsZoomed(u8 zoom);
		
	void SetZoomStatue(u8 flag);
	void SetExitStatue(u8 flag);
	u8 GetZoomStatue();
	int GetPbNum();
	int GetPbPauseStatue();

private:
	
	int nPlayChnNum;
	int totalItemNum;
	
	int isStop;
	int isPause;
	char m_nPlayText[10];
	
	int playRate;
	int playRate_back;
	int m_nForward;
	CPage* m_pDesktop;
	
	u8 m_nMute;
	u8 exitstatue;
	
	EMBIZPREVIEWMODE previewMode;
	
	//PbMgrHandle m_hPbMgr;
	SBizSearchPara sBizSearchParam;
	
	long long m_startTime;
	int m_total;
	u8 m_AllowOperateMenue; //如果回放的所有通道还没有全部打开，则需要等待，否则此时点击菜单按钮会造成死机
	
	void OnClkPbCtl();
	
	CButton* pButton[PB_BUTTON_NUM];
	CSliderCtrl* m_pSlider;
	void OnSlider();
	
	CCheckBox* m_pChnName;
	CStatic* m_pStatic[PB_STATIC_NUM];
	
	CPageChnSelect* m_pPageChnSel;
	CPageAudioSelect* m_pPageAudioSel;
	CPageColorSetup* m_pPageColorSetup;
	CPagePlayBackVoColorSetup* m_pPagePlayBackVoColorSetup;
	CPagePlayrateSelect* m_pPagePlayrate0;
	CPagePlayrateSelect* m_pPagePlayrate1;
	CPageSearch* m_pPageSearch;
	
	int nScreenWidth;
	int nScreenHeight;
	u8  IsPbZoom;
	
	u8 m_nIsZoomed;
	
	u8 GetMouseInWhichChn(int x, int y, int chnNum);
	
	u8 m_nIsFinished;//csp modify 20121118
	
public:
	u8 GetPBFinished();//csp modify 20121118
	u8 SetPBFinished(u8 state);//csp modify 20121118
	
	//CMutex m_Mutex;//csp modify 20121118
};

#endif //_PAGE_PLAYBACKFRAMEWORK_H_


