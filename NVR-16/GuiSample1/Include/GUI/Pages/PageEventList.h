#ifndef _PAGE_EVENTLIST_H_
#define _PAGE_EVENTLIST_H_

#include "PageFrame.h"

class CPageEventList:public CPageFrame
{
public:
	CPageEventList(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageEventList();
	VD_BOOL UpdateData( UDM mode );
	
	void SetChn(int row, char* szText);
	void SetType(int row, char* szText);
	void SetBegin(int row, char* szText);
	void SetEnd(int row, char* szText);
	void SetResult(char* szText);
	
private:

	void InitPageLeft();
	void InitPageRight();

	CTableBox* pTabPageLeft[2];
	CStatic* pTitlePageLeft[4];
	CStatic* pszResult;

	CStatic* pChn[9];
	CStatic* pType[9];
	CStatic* pBegin[9];
	CStatic* pEnd[9];
	
	CButton* pBtPageCtr[4];
	void OnClickPageCtr();

	CTableBox* pTabChnSearch;
	CCheckBox* pChkChn[5];
	CStatic*	pStaticChn[4];

	CDateTimeCtrl* pBeginDatePR;
	CDateTimeCtrl* pBeginTimePR;
	CDateTimeCtrl* pEndDatePR;
	CDateTimeCtrl* pEndTimePR;
	CStatic*	pStaticBegin;
	CStatic*	pStaticEnd;

	CCheckBox* pChkType[3];
	CStatic* pStaticType[3];
	
	CButton* pBtExport;
	CButton* pBtSearch;
	CButton* pBtQuit;
	void OnBtExport();
	void OnBtSearch();
	void OnBtQuit();
};

#endif  //_PAGE_EVENTLIST_H_

