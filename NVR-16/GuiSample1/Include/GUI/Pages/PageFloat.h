#ifndef _PAGE_FLOAT_H_
#define _PAGE_FLOAT_H_

#include "GUI/Ctrls/Page.h"

#define ROW_ITEM 4
#define CHKBOXW	 20
#define STATICW	 18
#define GAP	 5
#define GAP1 1

class CPageFloat:public CPage
{
public:
	CPageFloat(VD_PCRECT pRect,CPage * pParent = NULL,VD_PCSTR pTitle = "");
	~CPageFloat();
	
	virtual void Draw();
	virtual VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	
protected:
	BOOL m_bStartPage;
	BOOL m_bChnSelectPage;
	BOOL m_bAudioSelectPage;
	BOOL m_bPtzCtlPage;
	BOOL m_bPtzCtlPageDrawLine;
	BOOL m_bPtzTrackPageDrawLine;
	BOOL m_bPlayBackPage;
	BOOL m_bPlayBackHide;
	
	CRect m_pbRect;
	
private:
	
};

#endif

