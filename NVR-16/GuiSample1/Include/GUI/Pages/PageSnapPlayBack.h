#ifndef _PAGE_SNAP_PLAY_BACK_H_
#define _PAGE_SNAP_PLAY_BACK_H_

#include "PageFloat.h"
class CPageSearch;
class CPageDataBackup;
class CPageBackupInfo;


typedef enum
{
	EM_PLAYBACK,
	EM_BACKUP,
	EM_SNAP_MAX,
}EM_PAGE_SNAP_MODE;

class CPageSnapPlayBack:public CPageFloat
{
public:
	CPageSnapPlayBack(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageSnapPlayBack();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void OnClickDisPlay();
	void SetSearchPage(CPageSearch* pPage);
	void SetBackupPage(CPageDataBackup* pPage);
	void SetDesktop(CPage* pDesktop);
	void SetSearchPara(SBizSearchPara *psSearchPara);
	void SetWorkmode(EM_PAGE_SNAP_MODE mode);
	
private:
	CPageSearch* m_pPageSearch;
	CPageDataBackup* m_pPageBackup;
	CPageBackupInfo* m_pPageBackupInfo;
	
	CPage* m_pDesktop;
	int nlist_rows;//包括第一行显示列标题
	CTableBox* pTablist;
	CStatic* pTitleList[3];
	
	CStatic** ppChn;
	CCheckBox **ppChkbox;
	CStatic** ppTime;
	CButton** ppBtDisplay;
	CButton* pBtPageJump;
	CButton* pBtBackup;
	CButton* pBtExit;
	CButton* pBtPageCtrPage[4];
	VD_BITMAP* pBmpBtPage[4][2];
	CStatic* pszResultPage;
	CEdit* pEditPageJump;

	SBizSearchPara sSearchPara;
	SBizSearchResult sSearchSnapResult;
	int m_curPage;//搜索结果列表显示当前页0~n-1
	int cur_line;
	EM_PAGE_SNAP_MODE work_mode;
	SBizBackTgtAtt sBackTgtAtt;
	SBizSearchCondition sSearchCondition;
	SBizRecSnapInfo *sFileList;
	
	void OnClickBt();
	void SearchSnap();
	void SetPageList(int nPage);
	void SetLineOfPage(int line, SBizRecSnapInfo *pSnapInfo);
	void ClearLineOfPage(int line);
	void ClearPage();
	void MouseMoveToLine( int line);
	void ShowModeItem(EM_PAGE_SNAP_MODE mode, BOOL bShow);
	/*
	CStatic* pChn[50];
	CStatic* pTime[50];
	CButton* pBtDisplay[50];
	*/
};


#endif

