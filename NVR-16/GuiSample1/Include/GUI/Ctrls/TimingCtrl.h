//
//  "$Id: TimingCtrl.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_TIMINGSET_H__
#define __CTRL_TIMINGSET_H__

typedef enum{
	TCB_NORMAL,
	TCB_SELECTED,
	TCB_DISABLED,
}TimingCtrlBitmap;

class CTimingCtrl : public CItem          //added by zmx 04.7.9
{
protected:
	static VD_BITMAP* bmp_tcb_normal;
	static VD_BITMAP* bmp_tcb_selected;
	static VD_BITMAP* bmp_tcb_disabled;
private:
	CNumberBox	*pnb[4];
	CComboBox	*pcb[2];
	CStatic		*psp;
	TSECT tsect;
	int timefmt;
	TIMESECTION timesec;
	CTRLPROC m_onChanged;
	VD_COLORREF m_BkColor;
public:
	CTimingCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC onChanged = NULL,uint style = 0, enum time_fmt tmft = TF_24);
	virtual ~CTimingCtrl();

	virtual VD_BOOL Open();//打开
	virtual void Enable(VD_BOOL flag);						//重写基类的Enable虚函数  zhongyj add 20061230
	void Draw();
	void DrawFrame();
	void Select(VD_BOOL flag);
	void Show(VD_BOOL flag);
	void SetTime(TSECT *p);
	void SetTime(TIMESECTION *p);
	VD_BOOL GetTime(TSECT *p);
	VD_BOOL GetTime(TIMESECTION *p);
	CNumberBox* GetNumberBox(int index);
	void OnTimeChanged();
	VD_BOOL IsValid();
	int Hour12To24(int hour12, int ampm);
	int Hour24To12(int hour24);
	int Hour24ToAMPM(int hour24);
	VD_SIZE& GetDefaultSize(void);

	static void SetDefaultBitmap(TimingCtrlBitmap region, VD_PCSTR name);
};
CTimingCtrl* CreateTimingCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC onChanged = NULL,uint style = 0);


#endif //__CTRL_TIMINGSET_H__
