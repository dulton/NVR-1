//
//  "$Id: SliderCtrl.h 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __CTRL_SLIDER_H__
#define __CTRL_SLIDER_H__

enum _slider_style {
		sliderNoNum = 0x0001,//没有数字显示
		sliderNoWheel = 0x0002,//鼠标滚轮不其作用
		sliderTransparent = 0x0004,//是否对父窗口透明.但没有地方设置这么一个属性.
		//styleUseBmp=0x0008,//通用的slider地方使用图片,工程师不需要设置,程序会自己判断.
		sliderSetBmp=0x0008//设置slider图片,通常在调用setbitmap后,该标志位有效.与styleUseBmp不会同时存在.
}; 

typedef enum{
	SDB_RAIL_NORMAL,
	SDB_RAIL_SELECTED,
	SDB_TRACKER_NORMAL,
	SDB_TRACKER_SELECTED,
	SDB_TRACKER_PUSHED,
}SliderCtrlBitmap;

class CSliderCtrl : public CItem
{
	int min;
	int max;
	int newpos;
	int curpos;
	int	editpos;	//修改符位置
	VD_BITMAP * m_pBmpNormal;
	VD_BITMAP * m_pBmpElapsed;
	VD_BITMAP * m_pBmpSelect;
	VD_BITMAP * m_pBmpDisable;
	VD_BITMAP * m_pBmpBackground;
	VD_RECT rect_elapsed;
	VD_RECT rect_normal;
	VD_RECT rect_selected;
	VD_BOOL m_bWithBmp;
	VD_BOOL m_bTransparent;
	VD_BOOL m_bBmpIntergrate;

	VD_BOOL	track;			//正在滚动
	int		tracker_width;	//滚动块宽度
	int		tracker_offset;	//滚动块偏移
	int		tracker_pick;	//点击滚动块时鼠标的相对位置

	int 	m_nWheelStep; 

	int m_iSliderWidth;
	CTRLPROC m_onValueChanged;
		enum _slider_act{	//动作
	SA_POSITION = 0,
		SA_TRACK,
	};
protected:

	static VD_BITMAP* bmp_slider_normal;
	static VD_BITMAP* bmp_slider_select;
	static VD_BITMAP* bmp_tracker_normal;
	static VD_BITMAP* bmp_tracker_select;
	static VD_BITMAP* bmp_tracker_pushed;


private:
	void UpdateTracker();

public:
	CSliderCtrl(VD_PCRECT pRect, CPage * pParent,int vmin = 0,int vmax = 100, CTRLPROC vproc = NULL, int vstyle = 0, int nWheelStep = 1);
	virtual ~CSliderCtrl();
	void Draw();
	void DrawFrame();
	void DrawTracking(int old_track_offset, int new_track_offset);
	void DrawPosText();
	void Select(VD_BOOL flag);
	void SetFocus(VD_BOOL flag);
	int GetAt(int px, int py);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetPos(int newpos);
	int GetPos();
	VD_BOOL GetTrackState(int * piTrackerOffset);
	void StepIt(int step);
	void SetBitmap(VD_BITMAP*buf, VD_PCRECT pRectElapsed, VD_PCRECT pRectNormal, VD_PCRECT pRectSelected);
	void SetBitmap(VD_BITMAP* pBmpNormal, VD_BITMAP* pBmpSelect, VD_BITMAP* pBmpElapsed, VD_BITMAP* pBmpDisable, VD_BITMAP * pBmpBackground, int iTrackerWidth);
	void DrawSliderCtrl(CTRL_STATE state);

	static void SetDefaultBitmap(SliderCtrlBitmap region, VD_PCSTR name);
//	SIZE& GetDefaultSize(void);
};
CSliderCtrl* CreateSliderCtrl(VD_PCRECT pRect, CPage * pParent,int vmin = 0,int vmax = 100, CTRLPROC vproc = NULL, int vstyle = 0, int nWheelStep = 1);

#endif //__CTRL_SLIDER_H__
