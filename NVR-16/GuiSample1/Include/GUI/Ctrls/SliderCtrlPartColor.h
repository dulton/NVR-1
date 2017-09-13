//
//  "$Id: SliderCtrl.h 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
//yaogang modify for playback 20170906

#if 0
进度条区域着色:
有录像 {	0,		174,	255,	128	},//COLOR_FRAMESELECTED
无录像 {	180,	180,	180,	150	},//COLOR_CTRLTEXT
分割线 {	102,	102,	102,	128 },//COLOR_FRAME

#endif
#include "MultiTask/Mutex.h"
//#include "GUI/Ctrls/Static.h"


#ifndef __CTRL_SLIDER_PART_COLOR_H__
#define __CTRL_SLIDER_PART_COLOR_H__

class Range
{
friend VD_BOOL isInRange(int val, const Range &r);//val 是否在this 中
friend VD_BOOL isInRangeWithoutStart(int val, const Range &r);
friend VD_BOOL isInRangeWithoutEnd(int val, const Range &r);
friend VD_BOOL isInRangeWithoutBoth(int val, const Range &r);
friend VD_BOOL operator ==(const Range &lhs, const Range &rhs);
friend VD_BOOL operator !=(const Range &lhs, const Range &rhs);

public:
	Range(int s, int e);
	~Range();
	VD_BOOL isInRange(const Range &r) const;//this 是否在r 中
	VD_BOOL selfCheck() const;
	int Width() const;

	int start;
	int end;
};


class CSliderCtrlPartColor : public CItem
{
public:
	CSliderCtrlPartColor(VD_PCRECT pRect, CPage * pParent, int vmin = 0,int vmax = 100, int SplitLineMax=10, CTRLPROC vproc = NULL);
	virtual ~CSliderCtrlPartColor();
	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);

	int SetPos(int pos);
	int GetPos();
	int GetAt(int px, int py);//是否在滑动块的区域内
	
	void SetTrackerEnable(VD_BOOL b);
	void SetSplitLineNum(int n);
	void SetSplitLineColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	void SetColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	void SetBlankColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	//把指定区域放大显示
	int SetDisplayRange(const Range &r);
	//同时改变比例尺和分割线数量
	//eg: playback module 24hr/2hr/1hr/30min
	int SetDisplayRange(const Range &r, int split_line_num);
	
	//设置着色区域(有录像的时间段)
	void SetColorRange(std::vector<Range> &vr);

	void Select(VD_BOOL flag)
	{
		SetFlag(IF_SELECTED, flag);
	}

private:	
	void DrawBackground();
	void DrawRange();
	void DrawSplitLine();
	void UpdateTracker();
	void DrawTracker();
	
private:
	CMutex m_Mutex;
	VD_BOOL m_bTracker; //滑块使能, 是否显示和拖动

	int m_iSliderWidth; //进度条长度= m_Rect.Width()
	Range m_sRealRange;//就是[min, max]   实际区间
	CTRLPROC m_onValueChanged;
	
	//游标, 颜色= m_ColorBlank
	int curpos;//[min, max]
	int newpos;//[min, max] 拖动游标使用
	VD_BOOL	track;			//正在滚动
	int		tracker_width;	//滚动块宽度
	int		tracker_offset;	//滚动块偏移[0, m_iSliderWidth]
	int		tracker_pick;	//点击滚动块时鼠标的相对位置

	//分割线(刻度)
	//void InitSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(232,232,232));
	int m_iSplitLineMax;
	int m_iSplitLineNum;//等分进度条24hr/24, 2hr/12, 1hr/12, 30min/10
	VD_COLORREF m_ColorSplitLine;
	//std::vector<CStatic*> m_vSplitLine;	

	//着色区域
	VD_COLORREF m_ColorBlank;	//空白区域颜色   无录像
	VD_COLORREF m_Color;		//着色区域	有录像
	std::vector<Range> m_vRange;

	//把指定区域放大显示
	//eg: 初始显示一天24小时, 响应用户放大显示8-10两小时区域
	Range m_sDispRange;// [min, max] 子集, 内部使用, 需要放大显示的一段区域
	//int m_iZoomFactorFactor;/还是交给上层处理, 进度条放大系数 , 外部设置. 24hr/1  2hr/12   1hr/24   30min/48
};

#endif //__CTRL_SLIDER_PART_COLOR_H__
