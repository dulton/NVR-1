/*add by liu*/
#ifndef _PAGE_STREAM_H_
#define _PAGE_STREAM_H_

#include "PageFrame.h"
#include "MultiTask/Timer.h"


#define MAX_STREAM_INFO 8

class CPageEditMacAddress;
class CPageStream:public CPageFrame
{
public:
	CPageStream(VD_PCRECT pRect,VD_PCSTR psz, VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageStream();
	void OnTrackMove();
	int ShowStreamInfo(uint param);

	VD_BOOL UpdateData(UDM mode);

	//void SetValue(int row, char* szText);
	//VD_BOOL UpdateData( UDM mode );
	//VD_BOOL UpdateMacAddress(char *pBuf);
	//VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	//void OnClkBtn();


private:
		
	CTableBox* pFrame;
	CTableBox* pTab1;
	CTableBox* pTab2;
	CTableBox* pTab3;
	CTableBox* pTab4;
	CStatic* pStream1[3];
	//yaogang modify 20140918
	//CStatic* pStream2[3*MAX_STREAM_INFO];
	CStatic* pStream2[5*MAX_STREAM_INFO];
	//yaogang modify 20140918
	CStatic* pStream3[2*2];
	CStatic* pStream4[4];
	CScrollBar* pScroll;
	int m_maxPage;
	int m_page;

	CTimer* m_StreamTimer;
	//CTimerManager m_StreamManager;
	//CStatic* pTitle[MAX_NETWORK_INFO];
	//CStatic* pValue[MAX_NETWORK_INFO];
	//CButton* pButton;
	//CPageEditMacAddress* m_pPageEditMacAddress;
	u8 nNVROrDecoder;//Ô¾Ìì: 1 nvr£¬2 ÂÖÑ²½âÂëÆ÷£¬3 ÇÐ»»½âÂëÆ÷
};
#endif

