#ifndef _PAGE_CRUISELINE_H_
#define _PAGE_CRUISELINE_H_

#include "PageFrame.h"
#include "PageCruisePoint.h"
#include "PagePtzTypes.h"
//#include "PageChild.h"
#include <vector>
#include <bitset>
#include <iostream>

using std::bitset;

#define PG_STATICNUM 5
#define PG_BUTTONNUM 10

#define TOURPATH_MAX	32
#define TOUR_LIMIT		9

//SBizPtzAdvancedPara

class CPageCruisePoint;
class CPageCruiseLine :public CPageFrame
{
public:
	CPageCruiseLine(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL, int nChn = 0);
	~CPageCruiseLine();
	
	VD_BOOL UpdateData( UDM mode );

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void OnClickBtn();
	void SelectItem( int index );
	void GetCruisePoint(uchar nCh, SGuiTourPath* psPIns);
	void SetCruisePoint(uchar nCh, SGuiTourPath* psPIns);
	void GetTourLine(SGuiTourPath* psPIns);
	void SetChn(int chn);
	int bGetCrLineNum(int index);
	void OnClose();
	void LoadPage();
	VD_BOOL Close(UDM mode);
	int GetCurCLineNum();//cw_tab
protected:

private:
	uchar bFirstSel;
	void SetStatus();
	void OnGetPathId(uchar nCh, uchar* nId);
	void DelPath(uchar nPathNo);
	void DelItem(uchar nIdx);
	void CancelSel();
	void MouseMoveToLine(int index);
	
	void AddPath(SGuiTourPath* pPIns);
	SGuiTourPath* psGuiTourPath[128];
	SGuiPtzAdvancedPara psGuiPtzPara;
	SGuiTourPath pPIns[TOURPATH_MAX];

	int m_nTotal;
	int m_nCurPage;
	int m_nlastPage;
	int m_nMaxPage;
	int nChMax;
	int m_nChn;
	int m_nCursel;
	int m_nCurMouseLine;
	bitset<TOURPATH_MAX> bitTourAlive;
	bitset<TOURPATH_MAX> bitRecTourStartStatus;
	
	CPage * pParent;
	
	CStatic* pStatic[21];
	CButton* pButton[10];
	CTableBox* pTable0;
	CTableBox* pTable1;

	CPageCruisePoint* m_pPageCruisePoint;


};

#endif //_PAGE_CRUISELINE_H_




