#ifndef _PAGE_SYSLOG_H_
#define _PAGE_SYSLOG_H_

#include "PageFrame.h"
#include <vector>
#include "GUI/Pages/BizData.h"

//#include "biz_types.h"

class CPageSysLog:public CPageFrame
{
public:
	CPageSysLog(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageSysLog();

	void SetType(int row, char* szText);
	void SetUsername(int row, char* szText);
	void SetUsernameWithChn(int row, u8 chn);
	void SetTime(int row, char* szText);
	void SetIP(int row, char* szText);
	void SetResult(char* szText);

	VD_BOOL UpdateData( UDM mode );

	
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	VD_BOOL SelectItem( int index );

private:

	void InitPageLeft();
	void InitPageRight();

	
	CTableBox* pTabPageLeft[2];
	CStatic* pTitlePageLeft[4];
	CStatic* pszResult;

	CStatic* pType[9];
	CStatic* pUsername[9];
	CStatic* pTime[9];
	CStatic* pIP[9];
	
	CButton* pBtPageCtr[4];
	void OnClickPageCtr();

	CTableBox* pTabPageRight[2];
	CTableBox* pTabChnSearch;
	CCheckBox* pChk[7];
	CStatic*	pStaticChk[7];

	CDateTimeCtrl* pBeginDatePR;
	CDateTimeCtrl* pBeginTimePR;
	CDateTimeCtrl* pEndDatePR;
	CDateTimeCtrl* pEndTimePR;
	CStatic*	pStaticBegin;
	CStatic*	pStaticEnd;

	CScrollBar* pScroll;
	CTableBox* pTable;

	CStatic* pInfoBar;
	CButton* pBtExport;
	CButton* pBtSearch;
	CButton* pBtQuit;
	void OnBtExport();
	void OnBtSearch();
	void OnBtQuit();

	void SetInfo(char* szInfo);
	void ClearInfo();

	SBizLogResult* pLogSearchResult;
	VD_BOOL GetLogType(unsigned short type, char* tmp);
	void SetOneRow(int page, int row, SBizLogResult* src);
	int curPage;
	int m_nCursel;
	char nFlagSearched;
	void GetLogsFileName(char* name);

	int m_nCurMouseLine;
	void MouseMoveToLine( int index );
};

#endif  //_PAGE_SYSLOG_H_


