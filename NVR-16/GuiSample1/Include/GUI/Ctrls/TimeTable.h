//
//  "$Id: TimeTable.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __TIME_TABLE_H__
#define __TIME_TABLE_H__

#include "Configs/ConfigWorksheet.h"

class CTimeTable :
	public CItem
{
public:
	CTimeTable(VD_PCRECT pRect, CPage * pParent, uint style = 0);
	virtual ~CTimeTable(void);
	void Draw(void);
	void SetWorksheet(TIMESECTION *pTSect);
	
	void setColor(VD_COLORREF odd, VD_COLORREF even, VD_BOOL redraw = FALSE);
	void setColor(VD_COLORREF timesect, VD_BOOL redraw = FALSE);
	VD_SIZE& GetDefaultSize(void);
private:
	CStatic *pStaticWeekday[7];
	TIMESECTION m_tSect[N_WEEKS][N_TSECT];
	int m_iWorksheet;
	int m_iyOffet;
	int m_iTableLeft;
	int m_iTableWidth;
	int m_iTableHeight;
	void DrawTable(void);

	int m_RowHight;
	VD_COLORREF	m_colorTimeSect;
	VD_COLORREF	m_colorOdd;
	VD_COLORREF	m_colorEven;
};
CTimeTable* CreateTimeTable(VD_PCRECT pRect, CPage * pParent, uint style = 0);
#endif //__TIME_TABLE_H__
