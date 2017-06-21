//
//  "$Id: NumberBox.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_NUMBERBOX_H__
#define __CTRL_NUMBERBOX_H__

typedef enum{
	NBB_NORMAL,
	NBB_SELECTED,
	NBB_PUSHED,
	NBB_DISABLED,
}NumberBoxBitmap;

class CNumberBox : public CItem
{
	int min;
	int max;
	int value;
	int old_value;	//数字面板弹出时的值
	int vvalue;		//临时值
	int	editpos;	//修改符位置
	VD_PCSTR fmt;
	CNumberPad* numpad;	//数字输入框
	int string_width; //字符串宽度
	int caret_cnt;
	CTRLPROC m_vProc;
	CTRLPROC m_vProc1;
	CTRLPROC m_vProc2;
	static VD_BITMAP* m_bmpNormal;
	static VD_BITMAP* m_bmpSelected;
	static VD_BITMAP* m_bmpPushed;
	static VD_BITMAP* m_bmpDisabled;

	VD_COLORREF 	m_BkColor;

	//add by nike.xie	
	uint old_time;   //上次输入数字的时间
	uint cur_time;   //当前输入数字的时间
	//end
	
public:

	CNumberBox(VD_PCRECT pRect, CPage * pParent,int min = 0,int max = 65535,VD_PCSTR vfmt = "%d",
						CTRLPROC vproc = NULL, CTRLPROC vproc1 = NULL, CTRLPROC vproc2 = NULL,uint vstyle = 0);
	virtual ~CNumberBox();
	
	void Draw();
	void SetBkColor(VD_COLORREF color, VD_BOOL redraw = FALSE );
	void DrawCaret(VD_BOOL bForced = FALSE);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetFocus(VD_BOOL flag);
	void ShowDropDown(VD_BOOL show);
	void EditValue(uchar key);
	void SetValue(int svalue);
	void SetRange(int vmin, int vmax);
	int GetValue();
	VD_SIZE& GetDefaultSize(void);
	void SetFormat(VD_PCSTR pcFmt);
	static void SetDefaultBitmap(NumberBoxBitmap region, VD_PCSTR name);

private:
	void ValueChanged();
};
CNumberBox* CreateNumberBox(VD_PCRECT pRect, CPage * pParent,int min = 0,int max = 65535,VD_PCSTR vfmt = "%d",
								  CTRLPROC vproc = NULL, CTRLPROC vproc1 = NULL, CTRLPROC vproc2 = NULL,uint vstyle = 0);

#endif //__CTRL_NUMBERBOX_H__
