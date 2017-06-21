//
//  "$Id: DateTimeCtrl.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_DATETIME_H__
#define __CTRL_DATETIME_H__

#define DTC_ITEMS 23

typedef enum{
	DTB_NORMAL,
	DTB_SELECTED,
	DTB_PUSHED,
	DTB_DISABLED,
}DateTimeCtrlBitmap;

/// 常量固定，不能随便修改，使用时传给CreateDateTimeCtrl构造函数的
/// 风格属性，可以是固定一个DTS_DATETIME，DTS_DATE，DTS_TIME也可以是
/// DTS_YEAR, DTS_MONTH, DTS_DAY, DTS_HOUR, DTS_MINUTE, DTS_SECOND按位或的结果
/// 置1表示该位对应数字显示，否则表示不显示
enum DTS_TYPE
{
	DTS_DATETIME = 0x3f,	///< 共6位表示 
	DTS_DATE = 0x38,		///< 6位中的高三位
	DTS_TIME = 0x07,		///< 6位中的低三位
	DTS_YEAR = 0x20,		///< 6位中的最高位表示Year
	DTS_MONTH = 0x10,		
	DTS_DAY	= 0x08,
	DTS_HOUR = 0x04,
	DTS_MINUTE = 0x02,
	DTS_SECOND = 0x01,
}; 

//时间表示类型
/*enum COLOR_TYPE
{
	COLOR_BK,
	COLOR_TEXT,
};*/

class CDateTimeCtrl : public CItem
{
	CNumberBox *pnb[6];
	CComboBox *pcb;
	VD_BOOL single;
	int datefmt;
	int datesprtr;
	int timefmt;
 	VD_RECT m_sClientRect;
	int m_iDtcNumWidth;
	int m_iDtcLinePos;
	CTRLPROC m_vProc;
	int width;
	uint m_dtStyle;
	VD_COLORREF  m_BkColor;
	VD_BOOL  m_CustomColor;
	static VD_BITMAP* m_bmpNormal;
	static VD_BITMAP* m_bmpSelected;
	static VD_BITMAP* m_bmpPushed;
	static VD_BITMAP* m_bmpDisabled;
public:
	SYSTEM_TIME time;
	
	CDateTimeCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC vproc = NULL, uint dwStyle = DTS_DATETIME);
	virtual ~CDateTimeCtrl();
	virtual VD_BOOL Open();//打开

	virtual void Show(VD_BOOL flag, VD_BOOL redraw = TRUE);	

	void Draw();
	void DrawFrame();
	void SetBkColor(VD_COLORREF color, VD_BOOL redraw = FALSE );
	void SetTimeFormat(int vdatefmt, int vsprtrfmt, int vtimefmt);
	void SetDateTime(SYSTEM_TIME *p);
	void GetDateTime(SYSTEM_TIME *p);
	void OnDateChanged();
	void OnHourChanged();
	void OnTimeChanging();
	void OnTimeChanging1();
	void Select(VD_BOOL flag);
	void Enable(VD_BOOL flag);
	void Show(VD_BOOL flag);
	void SetRect(VD_PCRECT pRect, VD_BOOL bReDraw = TRUE);
	void DrawSplit();
	VD_SIZE& GetDefaultSize(void);
	static int GetDaysOfMonth(int year,int month);
	static int Hour12To24(int hour12, int ampm);
	static int Hour24To12(int hour24);
	static int Hour24ToAMPM(int hour24);
	static void SetDefaultBitmap(DateTimeCtrlBitmap region, VD_PCSTR name);
};

CDateTimeCtrl* CreateDateTimeCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC vproc = NULL, uint dwStyle = DTS_DATETIME);

#endif //__CTRL_DATETIME_H__
