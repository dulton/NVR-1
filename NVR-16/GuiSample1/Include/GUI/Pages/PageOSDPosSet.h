#ifndef _PAGE_OSDPOSSET_H_
#define _PAGE_OSDPOSSET_H_

#include "GUI/Ctrls/Page.h"


#define OSD_TYPE_CHANNEL  0x0001
#define OSD_TYPE_TIME	  0x0002


class CPageOSDPosSet: public CPage
{
public:
	CPageOSDPosSet(VD_PCRECT pRect,VD_PCSTR psz =NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageOSDPosSet();

	VD_BOOL UpdateData(UDM mode);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void Draw();

	void SetOSDType(int iChannel, int osdtype, VD_BOOL bRedraw = TRUE);
	int GetChannel();
	int GetOsdType();
	std::vector<CRect> GetChnOsdRect();
	std::vector<CRect> GetTimeOsdRect();

private:
	int m_maxChn;
	int m_nChannel;
	int m_nOSDType;

	CRect m_screenRect;

	void DrawOSD();
	void AdjustRectToCif(CRect* pRect);
	void AdjustRectToScreen(CRect* pRect);
	CStatic	*pStaticTimeTitle;
	CStatic	*pStaticChannelTitle;

	CRect m_rtOSDOld[2];  // 0-channel 1-time
	CRect m_rtOSD[2];  // 0-channel 1-time

	int m_nDown;  //0 Œ¥Õœ¿≠  1channel 2time
	int m_old_x;
	int m_old_y;

	u8 IsDwell;

	//OsdPos* m_osdPos;
	

};

#endif
