//
//  "$Id: Calendar.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_CALENDAR_H__
#define __CTRL_CALENDAR_H__

class CCalendar : public CItem
{
	static std::string m_strMonth;	//月份字符串
	static std::string m_strWeeks[7];	//星期字符串
	static	int	m_weekstart;
	CComboBox	*m_pMonth;	//月份组合框
	CNumberBox	*m_pYear;	//年数字框
	CButton		*m_pPrev;	//上一月
	CButton		*m_pNext;	//下一月
	CButton		*m_pDays[7][6];//
	CGridCtrl	*m_pGrid;
	uint		m_dwMask;	//位标志
	int			m_nMaxDays;	//当月天数
	int			m_nStartWeekDay; //当月第一天星期
	int			m_iCursel;	//当前选中的天,从0开始
	CTRLPROC	m_onMonthChanged;
	CTRLPROC	m_onDaySelected;

public:
	CCalendar(VD_PCRECT pRect, CPage * pParent, CTRLPROC onMonthChanged = NULL, CTRLPROC onDaySelected = NULL,uint style = 0);
	virtual ~CCalendar();
	
	virtual void Show(VD_BOOL flag, VD_BOOL redraw = TRUE);	//显示/隐藏控件

	VD_SIZE& GetDefaultSize(void);
	void DrawSingle(int index);
	int GetAt(int px, int py);
	void DrawFrame();
	void Draw();
	void Select(VD_BOOL flag);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void OnPrevMonth();
	void OnNextMonth();
	void OnMonthChanged();
	void OnDaySelected();
	void SetDate(SYSTEM_TIME *p);
	VD_BOOL  GetDate(SYSTEM_TIME *p);
	void SetMask(uint mask);
	int GetDaysOfMonth(int year,int month);
	void UpdateDays();
	static VD_BOOL SetStrings(VD_PCSTR month, VD_PCSTR week);
	static void SetWeekStart(int weekstart);
};
CCalendar* CreateCalendar(VD_PCRECT pRect, CPage * pParent, CTRLPROC onMonthChanged = NULL, CTRLPROC onDaySelected = NULL,uint style = 0);
#endif //__CTRL_CALENDAR_H__

