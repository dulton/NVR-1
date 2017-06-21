#ifndef _PAGE_MANUALALARM_H_
#define _PAGE_MANUALALARM_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>


class CPageManualAlarm :public CPageFrame
{
public:
	CPageManualAlarm(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageManualAlarm();
	VD_BOOL UpdateData( UDM mode );

	VD_PCSTR GetDefualtTitleCenter();

protected:


private:
	int curID;

	CStatic* pStatic[8];
	CButton* pButton[3];
	CCheckBox* pCheckBox[2];
	CTableBox* pTable;

	void OnClkBtn();
	
};

#endif //_PAGE_MANUALALARM_H_




