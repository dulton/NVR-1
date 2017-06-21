#ifndef _PAGE_ADD_USER_H_
#define _PAGE_ADD_USER_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define ADDUSER_SUBPAGES	2
#define ADDUSER_BTNNUM	    4

#define ADDUSER_MAXSUB   2

/*
typedef struct tagMAINFRAME_ITEM
{
	std::vector<CItem*> SubItems[ADDUSER_MAXSUB];
	int			cursubindex;
}MAINFRAME_ITEM;*/


class CPageAddUser :public CPageFrame
{
public:
	CPageAddUser(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAddUser();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );

protected:
	char** pGroupList;
	int nRealGroupNum;

private:
	int curID;
	int lastPos ;

	//void OnClkConfigItem();
	void SwitchPage( int mainID,int subID);
	void ShowSubPage(int mainID,int subID, BOOL bShow);

	void OnClickSubPage();
	void OnClickBtn();
	void OnTrackMove();

	CButton* pButton[ADDUSER_BTNNUM];
	CTableBox* pTable;
	//CButton* pItemButton[3];

	//"通用"子页面
	#define PG_ADDUSER_0_STATICNUM 6//csp modify
	#define PG_0_STATICMACNUM 5
	#define PG_0_EDITMACNUM 6
	#define PG_ADDUSER_0_EDITNUM 9//csp modify
	#define PG_0_CHECKNUM 1
	#define PG_0_COMBONUM 1
	void InitPage0();
	void OnEditChange0();
	CStatic* pStatic0[PG_ADDUSER_0_STATICNUM];
	CStatic* pStaticMac[PG_0_STATICMACNUM];
	CEdit* pEdit0[PG_ADDUSER_0_EDITNUM];
	CCheckBox* pCheckBox0[PG_0_CHECKNUM];
	CComboBox* pComboBox0[PG_0_COMBONUM];
	CTableBox* pTable0;
	
	//CTableBox* pTable01;
	
	void InitPage1();
	
	//"权限"子页0
	void InitPage10();
	#if 1//csp modify 20130519
	CCheckBox* pCheckBox10[56];
	CStatic* pStatic10[56];
	#else
	CCheckBox* pCheckBox10[20];
	CStatic* pStatic10[20];
	#endif
	CTableBox* pTable10;
	CTableBox* pTable11;
	CTableBox* pTable12;
	CScrollBar* pScrollbar10;
	
	//"权限"子页1
	void InitPage11();
	CCheckBox* pCheckBox11[23];
	CStatic* pStatic11[23];
	CTableBox* pTable13;
	CTableBox* pTable14[3];
	
	void OnCheckBoxSelectAll();
	
	//csp modify 20130519
	void OnCheckBoxSelectItem();
};

#endif //_PAGE_BASICCFGFRAMEWORK_H_

