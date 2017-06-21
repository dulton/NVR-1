#ifndef _PAGE_MOTIONRGNSET_H_
#define _PAGE_MOTIONRGNSET_H_


#include "GUI/Ctrls/Page.h"

#include <vector>

class CPageMotionRgnSet: public CPage
{
public:
	CPageMotionRgnSet(VD_PCRECT pRect,VD_PCSTR psz =NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMotionRgnSet();

	VD_BOOL UpdateData(UDM mode);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void Draw();
	void SaveAllChnRgnPara();   //cw_md

	void SetGrid(int vcolumn, int vrow);
	void SetColor(VD_COLORREF selected, VD_COLORREF empty);
	void SetMask(uint64* p);
	void GetMask(uint64* p);
	void SetMotionChn(int nChn);
	void ShowCtrlPanel(BOOL bShow);
private:

	VD_BOOL GetAt(int px, int py, int* pc, int* pr);
	void DrawDynamic(VD_BOOL all = FALSE);
	int row;
	int column;
	int row_start;
	int column_start;
	int row_end;
	int column_end;
	int row_start_old;
	int column_start_old;
	int row_end_old;
	int column_end_old;
	VD_BOOL inset;
	uint64 region[64];
	uint64 backup[64];
	VD_COLORREF m_ColorSelected;
	VD_COLORREF m_ColorEmpty;

	int m_nChannel;

	void InitCtrlPanel();
//	void ShowCtrlPanel(BOOL bShow);
	std::vector<CItem*> m_pItemPanel;
	CTableBox* pTabFrame;
	CSliderCtrl* pSlider;
	CButton* pButton[5];
	CStatic* pTextInfo;
	BOOL m_bPanelIsShown;
	BOOL	IsDwell;
	BOOL is_panel_show;//cw_page
	BOOL m_Rgninit;
	void OnClkCtrlPanel();

};



#endif
