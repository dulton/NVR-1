#ifndef _PAGE_MASKSET_H_
#define _PAGE_MASKSET_H_

#include "GUI/Ctrls/Page.h"

#define MASK_MAX	4

typedef struct
{
	int bModify;
	int realNum;
	CRect osdRect[MASK_MAX];
}OsdRect;

class CPageMaskSet: public CPage
{
public:
	CPageMaskSet(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMaskSet();

	VD_BOOL UpdateData(UDM mode);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void Draw();

	void SetMaskChannel(int iChannel);
	void SetMaskRect(int iChannel);
	void SetMaskNum(int num);
	OsdRect* GetMaskRect(int iChannel);

private:
	int m_nChannel;
	BOOL m_bDown;

	int m_start_x;
	int m_start_y;

	CStatic * m_pMasklist[MASK_MAX];
	int m_nCurMask;

	OsdRect* osdRects;
	int maxChn;

	void DrawMask(int index, VD_PCRECT pRect);

	void AdjustMaskList(int maskId);
	void AdjustMaskNum();

	u8 m_MaskMax;
	u8 IsDwell;
};

#endif

