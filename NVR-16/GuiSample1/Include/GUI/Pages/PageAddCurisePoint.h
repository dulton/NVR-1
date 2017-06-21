#ifndef _PAGE_ADDCRUISEPOINT_H_
#define _PAGE_ADDCRUISEPOINT_H_

#include "PageFloat.h"
#include "PagePtzTypes.h"

class CPageAddCruisePoint:public CPageFloat
{
public:
	CPageAddCruisePoint(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAddCruisePoint();

	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void onSetCh(int nChn);
	
	void SetCurCruisePoint(char* szPreset, char* szRate, char* szTime);
private:
	uchar bModify;
	int nCurCh;
	int nRecPreset;
	BOOL bPresetIsNone;
	CPage*	m_pParent;
	SGuiPtzAdvancedPara psGuiPtzPara;
	int nPresetCount[TOURPIONT_MAX];
	void OnClickButtion();
	void OnClickPresetCombox();
	
	CStatic* 	pStatic[3];
	CComboBox* 	pCombox[3];
	CEdit* 		pEdit;
	CButton* 	pButton[2];
};

#endif // _PAGE_ADDCRUISEPOINT_H_

