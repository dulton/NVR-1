#ifndef _PAGE_RESUMEDEFAULTCFG_H_
#define _PAGE_RESUMEDEFAULTCFG_H_

#include "PageFrame.h"
#include <vector>

#define CONFIG_ITEM_COUNT	33

class CPageResumeDefaultCfg:public CPageFrame
{
public:
	CPageResumeDefaultCfg(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageResumeDefaultCfg();

	VD_BOOL UpdateData( UDM mode );

protected:

private:
	int m_nDefaultItemCnr;
	CStatic* pInfoBar;
	void OnClkBtn();
	
	CButton* pButton[2];
	CCheckBox* 	pCheckBox[CONFIG_ITEM_COUNT];
	CStatic* 	pStatic[CONFIG_ITEM_COUNT];
	CTableBox* 	pTable;

};

#endif 






