#ifndef _PAGE_RECORDSCHEDULEFRAMEWORK_H_
#define _PAGE_RECORDSCHEDULEFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define RECSCHEDULE_SUBPAGES	3
#define RECSCHEDULE_BTNNUM	6

class CPageRecordScheduleFrameWork :public CPageFrame
{
public:
	CPageRecordScheduleFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageRecordScheduleFrameWork();

	VD_BOOL UpdateData( UDM mode );
	VD_PCSTR GetDefualtTitleCenter();
	void SetSch( uchar nCh, uchar idx, uint* sch);
	void GetSch( uchar nCh, uint* sch);
	void ShowSch( uchar nCh );

protected:
	void WriteLogs(int PageIndex);

private:

	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	CGridCtrl** GetCurGrid();
	uchar GetCurChn();
	CComboBox** GetCurCombox();
	void GridSave();
	uchar** bSchChange;
	uint*** sch;
	//uchar bSchChange[3][4];
	//uint sch[3][4][7];
	int curID;

	//int totalItemNum;

	//void OnClkConfigItem();
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);

	void OnClickSubPage();
	void OnDBClkGrid(CGridCtrl** pGridSCH, uchar nCh);
	
	CButton* pButton[RECSCHEDULE_BTNNUM];
	CTableBox* pTable;
	CStatic* pInfoBar;
	CStatic* pIconInfo;

	void SetInfo(char* szInfo);
	void ClearInfo();
	//CButton* pItemButton[3];

	//"定时录像"子页面
	//#define PG_SCH_0_STATICNUM 16//csp modify
	void InitPage0();
	void OnDBClkGrid0();
	void OnClkEditPen0();
	void OnClkEraser0();
	void OnClkCopy0();
	void OnComboBox0();
	CStatic* pStatic0[16];
	CComboBox* pComboBox0[2];
	CButton* pButton0[3];
	CGridCtrl* pGridSCH0[7];
	CTableBox* pTable0;

	//"移动报警录像"子页面
	void InitPage1();
	void OnDBClkGrid1();
	void OnClkEditPen1();
	void OnClkEraser1();
	void OnClkCopy1();
	void OnComboBox1();
	CStatic* pStatic1[16];
	CComboBox* pComboBox1[2];
	CButton* pButton1[3];
	CGridCtrl* pGridSCH1[7];
	CTableBox* pTable1;

	//"传感器报警录像"子页面
	void InitPage2();
	void OnDBClkGrid2();
	void OnClkEditPen2();
	void OnClkEraser2();
	void OnClkCopy2();
	void OnComboBox2();
	CStatic* pStatic2[16];
	CComboBox* pComboBox2[2];
	CButton* pButton2[3];
	CGridCtrl* pGridSCH2[7];
	CTableBox* pTable2;


};

#endif //_PAGE_RECORDSCHEDULEFRAMEWORK_H_


