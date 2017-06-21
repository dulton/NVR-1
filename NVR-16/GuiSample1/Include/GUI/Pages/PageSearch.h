#ifndef _PAGE_SEARCH_H_
#define _PAGE_SEARCH_H_

#include "PageFrame.h"

#include "PageChnSelect.h"
#include "PageFloat.h"
#include "PagePlayBackFrameWork.h"
#include "PageSnapPlayBack.h"

#define SEARCH_SUBPAGES		4
#if 1//csp modify
#define SEARCH_COMMBUTTON	(SEARCH_SUBPAGES+2)
#else
#define SEARCH_COMMBUTTON	(SEARCH_SUBPAGES+1)	//tabs + 1个右上角搜索
#endif

typedef struct
{
	int nTimer;
	int nMoving;
	int nSensoring;
	int nManual;
	int nAll;
} sSearchTypeStatus;


class CPagePlayBackFrameWork;
class CPageSearch:public CPageFrame
{
public:
	CPageSearch(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageSearch();
	void SetChn(int row, char* szText);
	void SetBegin(int row, char* szText);
	void SetEnd(int row, char* szText);
	void SetState(int row, char* szText);
	void OnDaySelected();
	VD_BOOL UpdateData( UDM mode );
	void UpdateCalendar(SYSTEM_TIME* pTime);
	void FromPanel();
	void FromMainpage();
	void SetChn2(int row, char* szText);
	void SetBegin2(int row, char* szText);
	void SetEnd2(int row, char* szText);
	
	void SetPlayPage(CPagePlayBackFrameWork* pPagePlayBack);
	void PlayFile(int tabPage, int nId);
	void SelectDskItem( int index );
	void MouseMoveToLine( int index );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	int GetCurFileNum();//cw_tab
	void QuitTab();
private:
	int nChMax;
	int m_nCursel;
	BOOL bFromMainBoard;
	CRect pbFloat;
	sSearchTypeStatus m_sSearchTypeStatus;
	
	void SetProg(int curpage, int totalpage);
	int m_nFiles;
	void OnClickChooseChn();
	void OnCalendarAllChn();
	void OnCalendarChn();
	int realIdx[32];
	VD_RECT	m_RectScreen;

	CPagePlayBackFrameWork* m_pPagePlayBack;
	
	int m_mCurID;
	int m_nCurMouseLine;

	void SearchFilesWithFile();
	void SearchFilesWithEvent();
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);

	void OnClickSubPage();

	CButton* pButton[SEARCH_COMMBUTTON];
	
	CTableBox* pTabFrame1;
	CTableBox* pTabFrame2;

	void InitPage0();
	CRecFileTimeBox* pRecTimeBox;
	CStatic* pTextPG0WndMode;
	CButton* pBtPage0Wnd1x1;
	CButton* pBtPage0Wnd2x2;
	CStatic* pTextPG0Chn;
	CTableBox* pTabPG0Chn;
	CCheckBox* pPG0ChkChn[5*8]; // 最大支持32路
	CStatic* pPG0StaticChn[4*8];
	CStatic* pTextPG0StartPlay;
	CButton* pBtPage0StartPlay;
	CButton* pBtPageCtr[4];
	void OnClickWnd1x1();
	void OnClickWnd2x2();
	void OnClickStartPlay();
	void OnPG0AllChn();
	void OnPG0Chn();
	void ShowSearchButton(VD_BOOL show);
	CStatic* pTextPG0BeginTime;
	CDateTimeCtrl* pBeginDateP0;
	CDateTimeCtrl* pBeginTimeP0;
	CStatic* pTextPG0EndTime;
	CDateTimeCtrl* pEndDateP0;
	CDateTimeCtrl* pEndTimeP0;
	void OnClickTrackPage0();
	CPageChnSelect* pPageChnSel;
	CPageChnSelect* pPageChnSel2X2;
	u8 m_nCurPlayMode;
	void OnClickPageCtr();
	void RefreshRecTime();

	u8 m_nFlagHaveMaxPlayChn;
	u8 m_nMaxPlayChn;

#define CHN_PER_PAGE 8

	int nCurRecTimePg;
	int nRecTimePgNum;
	int nLastPgChnNum;
	int nRecTimeChnNum;
	
	void InitPage1();
	int m_chSel;//cw_tab
	int bInTab;
	int m_curPage1;
	void SetPage1(int page);
	void SetLineOfPage1(int line, SBizRecfileInfo* info);
	void ClearLineOfPage1(int line);
	void OnEventType();
	void OnEventAllType();
	void StartPlay2();//csp modify
	CTableBox* pTabPage1[2];
	CStatic* pTitlePage1[4];
	CStatic* pChn1[9];
	CStatic* pBegin1[9];
	CStatic* pEnd1[9];
	CStatic* pType1[9];
	CStatic* pszResultPage1;
	CStatic* pTips;
	CButton* pBtPageCtrPage1[4];
	VD_BITMAP* pBmpBtPage1[4][2];
	void OnClickPageCtrPage1();
	CCheckBox* pChkPage1[5];
	CStatic* pTextChkPage1[3];


	void InitPage2();
	void SetPage2(int page);
	void ClearPage2();
	void SetResult2(int page);
	int m_maxPage2;
	int m_page2;
	SBizSearchPara sSearchPara;
	SBizSearchResult sSearchResult;
	SBizSearchResult sSearchResult0;
	CTableBox* pTabPage2[2];
	CStatic* pTitlePage2[5];
	CStatic* pszResultPage2;
	
	CStatic* pChn2[9];
	CStatic* pBegin2[9];
	CStatic* pEnd2[9];
	CStatic* pState2[9];
	CButton* pBtPageCtrPage2[4];
	VD_BITMAP* pBmpBtPage2[4][2];
	void OnClickPageCtrPage2();
	CCheckBox* pChkPage2[9];
	CStatic* pszChooseAll;
	CButton* pBtDealFile[2];
	void OnDelFilePage2();
	void OnLockFilePage2();


	void InitPage3();
	CStatic* pTextPG3BeginTime;
	CDateTimeCtrl* pBeginDateP3;
	CDateTimeCtrl* pBeginTimeP3;
	CStatic* pTextPG3EndTime;
	CDateTimeCtrl* pEndDateP3;
	CDateTimeCtrl* pEndTimeP3;
	CStatic* pTextPG3Chn;
	CTableBox* pTabPG3Chn;
	CCheckBox* pPG3ChkChn[5*8]; // 最大支持32路
	CStatic* pPG3StaticChn[4*8];
	CButton* pBtPage3Search;
	CPageSnapPlayBack *m_pPageSnapPB;
	void OnClickSearchSnap();
	void OnPG3AllChn();
	void OnPG3Chn();
	/*
	CTableBox* pTabPicFrmPage3;
	CButton* pBtPageCtrPage3[4];
	VD_BITMAP* pBmpBtPage3[4][2];
	void OnClickPageCtrPage3();
	CButton* pBtDealPic[4];
	void OnClickDealPicPage3();
	void SetPage3(int page);
	void ClearPage3();
	void SetResult3(int page);
	void SearchSnap();
	*/
	int m_maxPage3;
	int m_page3;
	


	void InitCalendar();
	void SwitchCalendar(int type); //0-普通calendar 1-搜索图片
	CCalendar* pCalendar;
	CTableBox* pTabChnSearch;
	CCheckBox* pChkChn[5*8]; // 最大支持32路
	CStatic*	pStaticChn[4*8];

	CDateTimeCtrl* pBeginDatePR;
	CDateTimeCtrl* pBeginTimePR;
	CDateTimeCtrl* pEndDatePR;
	CDateTimeCtrl* pEndTimePR;
	CStatic*	pStaticBegin;
	CStatic*	pStaticEnd;
};

#endif

