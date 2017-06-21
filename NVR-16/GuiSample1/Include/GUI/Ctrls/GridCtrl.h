//
//  "$Id: GridCtrl.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_MDREGION_H__
#define __CTRL_MDREGION_H__

class CGridCtrl : public CItem
{
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
	uint region[18];
	uint backup[18];
	VD_COLORREF m_ColorSelected;
	VD_COLORREF m_ColorEmpty;

	CTRLPROC	m_ondbclk; //add by cj@2000503
public:
	CGridCtrl(VD_PCRECT lpRect, CPage * pParent, int vcolumn, int vrow, CTRLPROC ondblClicked, uint style = 0);
	virtual ~CGridCtrl();

	void Draw();
	VD_BOOL GetAt(int px, int py, int* pc, int* pr);

	void DrawDynamic(VD_BOOL all = FALSE);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetMask(uint* p);
	void GetMask(uint* p);
	void SetGrid(int vcolumn, int vrow);
	void SetColor(VD_COLORREF selected, VD_COLORREF empty);
	void Select(VD_BOOL flag);
};
CGridCtrl* CreateGridCtrl(VD_PCRECT pRect, CPage * pParent, int vcolumn,int vrow,CTRLPROC ondblClicked = NULL,uint style = 0);

#endif //__CTRL_MDREGION_H__
