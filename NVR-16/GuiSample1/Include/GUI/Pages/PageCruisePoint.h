#ifndef _PAGE_CRUISEPOINT_H_
#define _PAGE_CRUISEPOINT_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>
#include "GUI/Pages/PageAddCurisePoint.h"
#include "PagePtzTypes.h"

#define PG_STATICNUM 5
#define PG_BUTTONNUM 10
#define TOURPIONT_MAX	128

class CPageCruisePoint :public CPageFrame
{
public:
	CPageCruisePoint(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageCruisePoint();

	VD_BOOL UpdateData( UDM mode );

	VD_PCSTR GetDefualtTitleCenter();
	void OnClickBtn();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void SelectItem( int index );
	void SetSelItem(int nCh, int nCurPathNo, int nId, char* szName);
	void AddCruisePoint(SGuiTourPoint* pPIns);
	void ModCruisePoint(SGuiTourPoint* psPIns);
	void SetInfo(char* szInfo);
	void OnGetPara(uchar nCh, SGuiTourPoint *psPreset);
	void OnSetPara(uchar nCh, SGuiTourPoint *psPreset);
	VD_BOOL Close(UDM mode);
	void StopPreview();
	void StartPreview();
	void LoadPage();
	void Redraw();
	int GetCurPointNum();//cw_tab
protected:


private:

	int nCurPathNoIdx;
	int m_nCurPage;
	int m_nMaxPage;
	int nTPNo;
	int nSelLine;
	BOOL bPreviewStatus;
	
	int AddToTable(SGuiTourPoint* pPIns);
	CRect rtAdd, rtMod;
	uint nLeftGap, nTopGap;
	void ChangeCruisePoint();
	void DelCruisePoint();
	void TopCruisePoint();
	void BottomCruisePoint();
	void UpCruisePoint();
	void DownCruisePoint();
	int nCurPathNo;
	uchar bFirstSel;
	void SetStatus();
	void SetCruseiPoint(int nCLIdx, SGuiTourPoint* pPoint);
	void RefreshList();
	void OnComboBox();
	void ClearInfo();
	void CancelSel();
	void MouseMoveToLine(int index);
	
	int nID;
	char szName[32];
	CPage* pParent;
	SGuiTourPath* psGuiTourPath;
	SGuiTourPoint psGuiTourPoint[TOURPIONT_MAX];
	EMBIZPREVIEWMODE eMaxViewMode;
	BOOL bPrFlag;
	int m_nCurMouseLine;

	int nCh;
	int m_nTotal;
	int nChMax;
	int m_nCursel;
	int m_chSel;//cw_tab
	CStatic* pStatic[30];
	CButton* pButton[14];
	CComboBox* pComboBox;
	CEdit* pEdit;
	CTableBox* pTable0;
	CTableBox* pTable1;
	//CStatic *pIconInfo;
	CStatic* pInfoBar;

	void OnEditChange();
	
	CPageAddCruisePoint* pAddCruisePoint;
};

#endif //_PAGE_CRUISEPOINT_H_




