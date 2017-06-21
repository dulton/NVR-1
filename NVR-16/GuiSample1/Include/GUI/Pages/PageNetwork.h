#ifndef _PAGE_NETWORK_H_
#define _PAGE_NETWORK_H_

#include "PageFrame.h"

#define MAX_NETWORK_INFO 11

class CPageEditMacAddress;
class CPageNetwork:public CPageFrame
{
public:
	CPageNetwork(VD_PCRECT pRect,VD_PCSTR psz, VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageNetwork();

	void SetValue(int row, char* szText);
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL UpdateMacAddress(char *pBuf);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void OnClkBtn();


private:
	CTableBox* pFrame;
	CTableBox* pTab;
	CStatic* pTitle[MAX_NETWORK_INFO];
	CStatic* pValue[MAX_NETWORK_INFO];
	CButton* pButton;
	CPageEditMacAddress* m_pPageEditMacAddress;

};
#endif

