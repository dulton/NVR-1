//
//  "$Id: NumberBox.cpp 157 2008-12-01 12:20:07Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

VD_BITMAP* CNumberBox::m_bmpNormal = NULL;
VD_BITMAP* CNumberBox::m_bmpSelected = NULL;
VD_BITMAP* CNumberBox::m_bmpPushed = NULL;
VD_BITMAP* CNumberBox::m_bmpDisabled = NULL;

/************************************************************************/
/* 20070315 修改了构造函数，将风格参数移到最后一个默认参数，便于Item统一处理，
其间的各个页面和各个控件使用numberbox的地方都已经做了修改。请确认处理
HoneyWell版本的需要做另外调整。
															--by wangqin */
/************************************************************************/
///////////////////////////////////////////////////////////////////
////////////        CNumberBox

CNumberBox::CNumberBox(VD_PCRECT pRect, CPage * pParent,int vmin,int vmax,VD_PCSTR vfmt,
						CTRLPROC vproc,CTRLPROC vproc1,CTRLPROC vproc2,uint vstyle)
						:CItem(pRect, pParent, IT_NUMBERBOX,vstyle|styleAutoFocus|styleEditable)
{
	min = vmin;
	max = vmax;
	editpos = 0;
	value = vmin;
	fmt = vfmt;
	m_vProc = vproc;
	m_vProc1 = vproc1;
	m_vProc2 = vproc2;
	numpad = NULL;
	caret_cnt = 0;

	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
	
}

CNumberBox::~CNumberBox()
{
}

CNumberBox* CreateNumberBox(VD_PCRECT pRect, CPage * pParent,int min /* = 0 */,int max /* = 65535 */,VD_PCSTR vfmt /* = "%d" */,
				CTRLPROC vproc /* = NULL */, CTRLPROC vproc1 /* = NULL */, CTRLPROC vproc2 /* = NULL */,uint vstyle /* = 0 */)
{
	return new CNumberBox(pRect,pParent,min,max,vfmt,vproc,vproc1,vproc2,vstyle);
}


void CNumberBox::SetBkColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_BkColor = color;
	if(redraw)
	{
		Draw();
	}
}


VD_SIZE& CNumberBox::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*6,CTRL_HEIGHT};
	return size;
}

void CNumberBox::Draw()
{
	if(!DrawCheck()){
		return;
	}
	char string[16];

	m_DC.Lock();
	//选取颜色
	if(GetFlag(IF_SELECTED))
	{	
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
	}
	else
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}
	if(!GetFlag(IF_ENABLED))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	//m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetBrush(m_BkColor);
	

	//格式化字符串
	sprintf(string,fmt,value);
	
	m_DC.SetTextAlign(VD_TA_YCENTER);
	m_DC.SetBkMode(BM_TRANSPARENT);
	string_width = g_Locales.GetTextExtent(string);

	if(m_dwStyle & styleNoBorder){//嵌入式数字框
		//画背景

		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,MAX(m_Rect.Width(),string_width),m_Rect.Height()));

		//画字符串
		m_DC.VD_TextOut(CRect(0,0,string_width,m_Rect.Height()),string);
	}
	else
	{//一般数字框
		//画背景
		VD_BITMAP* bkbmp = (GetFlag(IF_SELECTED) || GetFlag(IF_FOCUSED))?m_bmpSelected:m_bmpNormal;
		if (bkbmp) 
		{
			m_DC.SetBkMode(BM_NORMAL);
			m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), bkbmp);
			if(GetFlag(IF_FOCUSED))
			{
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRL));
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(4,4,m_Rect.Width()-4,m_Rect.Height()-4),3,3);
			}
			m_DC.SetRgnStyle(RS_HOLLOW);
		}
		else
		{
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
			m_DC.SetRgnStyle(RS_HOLLOW);
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
			//m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),RS_RAISED,3,3);
			//m_DC.Rectangle(CRect(x+1,y+1,w-2,h-2),RS_SUNKEN,3,3);
		}

		//画字符串
		m_DC.SetBkMode(BM_TRANSPARENT);
		m_DC.VD_TextOut(CRect(TEXTBOX_X,2,m_Rect.Width()-4,m_Rect.Height()-2),string);
	}

	m_DC.UnLock();

	DrawCaret(TRUE);
}

void CNumberBox::DrawCaret(VD_BOOL bForced)
{
	if(!DrawCheck() || !GetFlag(IF_FOCUSED)){
		return;
	}
	if(!bForced)
	{
		caret_cnt++;
	}

	m_DC.Lock();

	if(caret_cnt%2)
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), VD_PS_SOLID, 2);
	}
	else
	{
		m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOW), VD_PS_SOLID, 2);
	}

	if(m_dwStyle & styleNoBorder)//嵌入式数字框
	{
		m_DC.MoveTo(string_width, 4);
		m_DC.LineTo(string_width, m_Rect.Height()-4);
	}
	else//一般数字框
	{
		m_DC.MoveTo(TEXTBOX_X+string_width+1, 6);
		m_DC.LineTo(TEXTBOX_X+string_width+1, m_Rect.Height()-6);
	}
	m_DC.UnLock();
}

void CNumberBox::SetFocus(VD_BOOL flag)
{
	if(!flag)
	{
		vvalue = value;
		if(vvalue < min)
		{
			vvalue = min;
		}
		else if(vvalue > max)
		{
			vvalue = max;
		}
		editpos = 3;
		//if value changed
		if(vvalue != value)
		{
			ValueChanged();
		}
		SetValue(vvalue);
		ShowDropDown(FALSE);
	}
	if(m_pOwner)
		m_pOwner->SetFocus(flag);
	CItem::SetFocus(flag);
}

void CNumberBox::SetValue(int svalue)
{
	if(value == svalue || svalue < min || svalue > max)
	{
		return;
	}
	value = svalue;
	while(svalue)
	{
		svalue /= 10;
		editpos++;
	}
	if (editpos)
	{
		editpos -= 1;
	}
	Draw();
}

void CNumberBox::SetRange(int vmin, int vmax)
{
	min = vmin;
	max = vmax;
	if(value < min){
		value = min;
		Draw();
	}
	if(value >max){
		value = max;
		Draw();
	}
	editpos = 3;
}

int CNumberBox::GetValue()
{
	return value;
}

void CNumberBox::ShowDropDown(VD_BOOL show)
{
	VD_RECT rect;
	VD_RECT full_rect;

	m_pDevGraphics->GetRect(&full_rect);
	// 当下拉numpad的边界超出右边界时，修改起始点位置
	if ( m_Rect.left + TEXT_WIDTH * 3 + 6 > full_rect.right)
	{
		rect.left = m_Rect.left-60;
		rect.right = m_Rect.left + TEXT_WIDTH * 3 + 6-60;
	}
	else
	{
		rect.left = m_Rect.left;
		rect.right = m_Rect.left + TEXT_WIDTH * 3 + 6;
	}
	if(m_Rect.bottom + TEXT_HEIGHT * 4 + 8 > full_rect.bottom)
	{
		rect.top = m_Rect.top - TEXT_HEIGHT * 4 - 6;
		rect.bottom = m_Rect.top;
	}
	else
	{
		rect.top = m_Rect.bottom;
		rect.bottom = m_Rect.bottom + TEXT_HEIGHT * 4 + 6;
	}
	m_pParent->ScreenToClient(&rect);
	if(show && !numpad){
		SetFlag(IF_CAPTURED, TRUE);
		numpad = new CNumberPad(&rect, m_pParent, this);
		if(numpad){
			numpad->Editable(FALSE);
			numpad->Open();
		}
		//old_value = value;
	}else if(!show && numpad){
		SetFlag(IF_CAPTURED, FALSE);
		numpad->Close();
		delete numpad;
		numpad = NULL;
		//value = old_value;
	}
}

VD_BOOL CNumberBox::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int key;
	signed char delta;
	int len,temp;
	INPUT_CHAR input_char;	//字符消息中的字符

	len  = 1;
	temp =  max;
	while(temp /= 10)
		len++;
	vvalue = value;
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_DOWN:
			vvalue--;
			if(vvalue<min)
				vvalue = max;
			ItemProc(m_vProc2);
			break;

		case KEY_UP:
			vvalue++;
			if(vvalue>max)
				vvalue = min;
			ItemProc(m_vProc2);
			break;

		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
		case KEY_10:
			int num;
			num = (key-KEY_0)%10;			

			//add by nike.xie  输入时间间隔大于5秒，清空数字输入
			cur_time = SystemGetMSCount();
			if((cur_time - old_time) > 5000)
			{
				editpos = 0;  //重新录入数字
			}
			old_time = cur_time;
			//end
			
			if(editpos == 0)
				vvalue = num;
			else if (vvalue < max)
				vvalue = vvalue*10+num;
			else
			{
				editpos = 0;
				vvalue = num;
			}
			if(editpos != 0 || num !=0)//第一个零不用移动editpos
			{
				editpos = (++editpos > len) ? len : editpos;
			}
			if(editpos == len){
				if(vvalue>max)
					vvalue = max;
				else if (vvalue < min)
					vvalue = min;
			}
			ItemProc(m_vProc2);
			break;

		
		case KEY_RET:
		case KEY_ESC:
			if(numpad)
			{
				ShowDropDown(FALSE);
			}
			else
			{
				return FALSE;
			}
			break;

		default:
			return FALSE;
		}
		break;

	case XM_MOUSEWHEEL:
		delta = (char)VD_HIWORD(wpa);
		if(delta < 0){
			vvalue++;
			if(vvalue>max)
				vvalue = min;
		}else if(delta > 0){
			vvalue--;
			if(vvalue<min)
				vvalue = max;
		}
		editpos = 3;
		ItemProc(m_vProc2);
		break;
		
	case XM_MOUSEMOVE:
		if(numpad){
			numpad->MsgProc(msg, wpa, lpa);
			return TRUE;//调用numpad->MsgProc后一定要返回
		}
		break;

	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(PtInRect(m_Rect, px, py) && !numpad)
		{  //弹出下拉列表框或反动作
			ShowDropDown(TRUE);
		}
		else if(numpad)
		{
			if(!numpad->MsgProc(msg, wpa, lpa))
			{
				ShowDropDown(FALSE);
			}
			return TRUE;//调用numpad->MsgProc后一定要返回
		}
		break;

	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:
		if(numpad){
			ShowDropDown(FALSE);
		}else{
			return FALSE;
		}
		break;

	case XM_CHAR:
		input_char.dw = wpa;
		if(input_char.ch[0] == '\b') // 退格
		{
			if(editpos)
			{
				editpos--;
			}
			vvalue /= 10;
		}
		else // 清空
		{
			vvalue = 0;
			editpos = 0;
		}
		ItemProc(m_vProc2);
		break;

	case XM_CARET:
		DrawCaret();
		return TRUE;

	default:
		return FALSE;
	}

	//if value changed
	if(vvalue != value)
	{
		ValueChanged();
		Draw();
	}
	return TRUE;
}

void CNumberBox::ValueChanged()
{
	//if(!numpad)
	{
		if(value >= min && value <= max)//输入的数字有效时, 才调用回调函数
		{
			if(value >= min && value <= max)//输入的数字有效时, 才调用回调函数
			{
				ItemProc(m_vProc);
			}
		}
	}
	value = vvalue;
	//if(!numpad)
	{
		if(value >= min && value <= max)//输入的数字有效时, 才调用回调函数
		{
			if(value >= min && value <= max)//输入的数字有效时, 才调用回调函数
			{
				ItemProc(m_vProc1);
			}
		}
	}
	
}

void CNumberBox::SetDefaultBitmap(NumberBoxBitmap region, VD_PCSTR name)
{
	switch(region) {
		case NBB_NORMAL:
			m_bmpNormal = VD_LoadBitmap(name);
			break;
		case NBB_SELECTED:
			m_bmpSelected = VD_LoadBitmap(name);
			break;
		case NBB_PUSHED:
			m_bmpPushed = VD_LoadBitmap(name);
			break;
		case NBB_DISABLED:
			m_bmpDisabled = VD_LoadBitmap(name);
			break;
		default:
			break;
	}
}

void CNumberBox::SetFormat(VD_PCSTR pcFmt)
{
	if (NULL != pcFmt)
	{
		fmt = pcFmt;
	}	
}

