#ifndef _PAGE_IMPORT_EXPORT_FRAMEWORK_H_
#define _PAGE_IMPORT_EXPORT_FRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define PG_STATICNUM 5
#define PG_BUTTONNUM 10

class CPageImportExportFrameWork :public CPageFrame
{
public:
	CPageImportExportFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageImportExportFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	void OnClickBtn();
	void SetResult(char* szText);
	void SetName(int row, char* szText);
	void SetSize(int row, char* szText);
	void SetType(int row, char* szText);
	void SetModifyTime(int row, char* szText);
	
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void SelectDskItem( int index );
	VD_BOOL checkModel(char * path);

protected:


private:
	void SetFileCount(int i, int nFile);
	void EnableOpBtn(u8 bEnable);
	void Refresh();
	int m_nCursel;
	int m_nFile;
	int m_nCurRow;	//tab起始行在保存tab内容的数组中的序号 重新载入内容时清零
	
	CStatic* pStatic[PG_STATICNUM];
	CStatic* pName[8];
	CStatic* pSize[8];
	CStatic* pType[8];
	CStatic* pModifyTime[8];
	
	CButton* pButton[PG_BUTTONNUM];
	CComboBox* pComboBox;
	CTableBox* pTable0;
	CTableBox* pTable1;

	CStatic* pInfoBar;
	void SetInfo(char* szInfo);
	void ClearInfo();


};

#endif //_PAGE_IMPORT_EXPORT_FRAMEWORK_H_



