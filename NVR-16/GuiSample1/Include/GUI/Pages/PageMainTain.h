#ifndef _PAGE_MAINTAIN_H_
#define _PAGE_MAINTAIN_H_

#include "PageFrame.h"
#include "PageChnSelect.h"
#include <vector>

class CPageMainTain:public CPageFrame
{
public:
	CPageMainTain(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMainTain();

	VD_BOOL UpdateData( UDM mode );
	void OnCheckSel();
	void OnComTypeSelect();
	void OnClkBtn();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);

private:
	u8 CurTimeSel;

	
private:
	CStatic* pStatic0[3];
	CStatic** pStatic1;

	CComboBox* pCombox[4];//0=类型选项  1，2，3=类型子选项
	CCheckBox** pCheckbox;
	
	CTableBox* pTabPage0;
	CTableBox* pTabPage1;

	CButton* pButton[2];	
};

#endif //_PAGE_EDIT_PWD_H_





