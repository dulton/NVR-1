//
//  "$Id: ComboBox.cpp 123 2008-11-27 14:04:01Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

#define COMBOBOX_HIDE_INVALID 1
//////////////////////////////////////////////////////////////////
////////////        CComboBox
//这个宏的使用暂时不取消，--by wangqin 20070412
#define DRAW_COMBOBTN_NORMAL(left,top,right,bottom) \
	m_DC.Bitmap(CRect(left,top,right,bottom),bmp_combobox_normal,0,0)
#define DRAW_COMBOBTN_PUSH(left,top,right,bottom) \
	m_DC.Bitmap(CRect(left,top,right,bottom),bmp_combobox_push,0,0)
#define DRAW_COMBOBTN_SELECT(left,top,right,bottom) \
	m_DC.Bitmap(CRect(left,top,right,bottom),bmp_combobox_select,0,0)

VD_BITMAP* CComboBox::m_bmpNormal = NULL;
VD_BITMAP* CComboBox::m_bmpSelected = NULL;
VD_BITMAP* CComboBox::m_bmpPushed = NULL;
VD_BITMAP* CComboBox::m_bmpDisabled = NULL;

CComboBox::CComboBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CTRLPROC vproc,CTRLPROC vproc1, uint dwStyle/* = CB_NORMAL*/)
:CItem(pRect, pParent, IT_COMBOBOX,dwStyle|styleEditable|styleAutoFocus)
{
	//isListExist = 0;
	m_vProc = vproc;
	m_vProc1 = vproc1;
	cursel = -1;
	num = 0;
	VD_PCSTR temp = NULL;
	if(psz)
	{
		temp = GetParsedString(psz);
	}
	InitStrings(temp);
	listbox = NULL;

	m_dwStyle |= dwStyle;
	reserve = FALSE;
	m_DropDownSize.h = 0;
	m_DropDownSize.w = 0;
	if(m_bmpNormal  &&  m_bmpSelected )
	{
		m_dwStyle|= styleUseBmp;
		width = 1;
	}
	else
	{
		width = 2;
	}
	
	if((m_dwStyle & comboNoButton) == comboNoButton)
	{
		m_dwStyle &= ~styleUseBmp;
	}

	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
}

CComboBox::~CComboBox()
{
	ShowDropDown(FALSE);
	RemoveAll();
}

VD_SIZE& CComboBox::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH * 6,CTRL_HEIGHT};
	return size;
}
CComboBox* CreateComboBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CTRLPROC vproc , CTRLPROC vproc1 , uint dwStyle)
{
	return new CComboBox(pRect, pParent, psz, vproc, vproc1,dwStyle);
}

void CComboBox::SetBkColor(VD_COLORREF color, VD_BOOL redraw)
{
	m_BkColor = color;
	if(redraw)
	{
		Draw();
	}
}


void CComboBox::Draw()
{ 
	if(!DrawCheck()){
		return;
	}
	//draw region
	m_DC.Lock();

	//m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetBrush(m_BkColor);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.SetRgnStyle(RS_HOLLOW);
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
	if (!(m_dwStyle & styleNoBorder))
	{
		if((m_dwStyle & styleUseBmp) == styleUseBmp)
		{
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),0,0);
		}
		else
		{
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
		}
	}
	
	//csp modify
	//if(m_dwStyle & VD_TA_XCENTER)
	//{
	//	m_DC.SetTextAlign(VD_TA_XCENTER);
	//}
	
	//draw text
	m_DC.SetTextAlign(VD_TA_YCENTER);
	m_DC.SetBkMode(BM_TRANSPARENT);
	if(cursel>=0)
	{
		CRect text_rect(0, 0, m_Rect.Width(), m_Rect.Height());
		if(!(m_dwStyle & comboNoButton))
		{
			text_rect.right -= CTRL_HEIGHT1;
		}
		if(!(m_dwStyle & styleNoBorder))
		{
			text_rect.left += 2;
			text_rect.right -= 2;
			text_rect.top += 3;
			text_rect.bottom -= 0;
		}
		if(!m_pOwner)
		{
			text_rect.left += TEXTBOX_X - 2;
		}
		m_DC.VD_TextOut(&text_rect, items[cursel].string.c_str());
	}

	//draw little button
	if(!(m_dwStyle & comboNoButton))
	{
		m_DC.SetRgnStyle(RS_FLAT);


		if((m_dwStyle & styleUseBmp) == styleUseBmp)
		{
			if (m_bmpNormal && m_bmpSelected)
			{
				if(GetFlag(IF_SELECTED))
				{
					m_DC.Bitmap(CRect(m_Rect.Width()-CTRL_HEIGHT1+2,0,m_Rect.Width(),m_Rect.Height()),m_bmpSelected,0,0);
					//DRAW_COMBOBTN_SELECT(m_Rect.Width()-CTRL_HEIGHT1+2,0,m_Rect.Width(),m_Rect.Height());
				}
				else
				{
					m_DC.Bitmap(CRect(m_Rect.Width()-CTRL_HEIGHT1+2,0,m_Rect.Width(),m_Rect.Height()),m_bmpNormal,0,0);
					//DRAW_COMBOBTN_NORMAL(m_Rect.Width()-CTRL_HEIGHT1+2,0,m_Rect.Width(),m_Rect.Height());
				}
			}
		}
		else
		{

			if(GetFlag(IF_SELECTED))
			{	
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED));
			}
			else
			{
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAME));
			}
			m_DC.MoveTo(m_Rect.Width()-CTRL_HEIGHT1,2);
			m_DC.LineTo(m_Rect.Width()-CTRL_HEIGHT1,m_Rect.Height() - 3);
				if(!GetFlag(IF_ENABLED))
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
				}
				else
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
				}
				m_DC.Trapezoid(m_Rect.Width()-CTRL_HEIGHT1+6, CTRL_HEIGHT1/2, CTRL_HEIGHT1/2-6, m_Rect.Width()-6, CTRL_HEIGHT1/2 + 4, 6-CTRL_HEIGHT1/2);
			}
	}
	m_DC.UnLock();
}

void CComboBox::Select(VD_BOOL flag)
{
	if(!flag)
	{
		ShowDropDown(FALSE);
	}
	CItem::Select(flag);
}

void CComboBox::SetFocus(VD_BOOL flag)
{

	CItem::SetFocus(flag);
}

void CComboBox::ShowDropDown(VD_BOOL show)
{
	VD_RECT rect;
	VD_RECT full_rect;
	int index;
	int n = 0;
	int ww = 0;
	int len;
	
	m_pDevGraphics->GetRect(&full_rect);
	if(show && !listbox){
		
#ifdef COMBOBOX_HIDE_INVALID
		for(index = 0; index < num; index++){//有效项统计
			if(items[index].valid)
			{
				if ((len = g_Locales.GetTextExtent(items[index].string.c_str())) > ww)
				{
					ww = len;
				}
				n++;
			}
		}
#else
		n = num;
#endif
		n = MAX(1, n);//没有项，则显示一个空项
		SetRectEmpty(&rect);
		if(m_DropDownSize.h)//使用设置后的下拉列表高度
		{
			n = (m_DropDownSize.h - 4) / TEXT_HEIGHT;
		}

		rect.left = m_Rect.left;
		if(m_DropDownSize.w)//使用设置后的下拉列表宽度
		{
			rect.right = m_Rect.left + m_DropDownSize.w;
		}
		else if(ww + TEXTBOX_X * 2 > m_Rect.Width())
		{
			rect.right = rect.left + ww + TEXTBOX_X * 2;
		}
		else
		{
			rect.right = m_Rect.right;
		}

		if(m_Rect.bottom + TEXT_HEIGHT * n + 4> full_rect.bottom && full_rect.bottom - m_Rect.bottom < m_Rect.top - full_rect.top)
		{
			rect.top = MAX(m_Rect.top - TEXT_HEIGHT * n - 4, full_rect.top);
			rect.bottom = m_Rect.top;
			rect.top = rect.bottom - ((rect.bottom - rect.top - 4) / TEXT_HEIGHT * TEXT_HEIGHT + 4);//使下拉列表合适列表项
		}
		else
		{
			rect.top = m_Rect.bottom;
			rect.bottom = MIN(m_Rect.bottom + TEXT_HEIGHT*n + 4, full_rect.bottom);
			rect.bottom = rect.top + ((rect.bottom - rect.top - 4) / TEXT_HEIGHT * TEXT_HEIGHT + 4);//使下拉列表合适列表项
		}
		SetFlag(IF_CAPTURED, TRUE);       //表示弹出下拉列表框后，鼠标若不在当前项上时，也不能激活其他项
		m_pParent->ScreenToClient(&rect);
		listbox = CreateListBox(&rect,m_pParent, "", this, stylePopup, NULL, NULL);//mofified constructors --by wangqin 	
		if(listbox){
			listbox->Editable(FALSE);
			for(index = 0; index < num; index++){
#ifdef COMBOBOX_HIDE_INVALID
				if(items[index].valid)
				{
					listbox->AddString(items[index].string.c_str());
				}
#else
				listbox->AddString(items[index].string.c_str());
				listbox->SetValid(index, items[index].valid);
#endif
			}
			listbox->SetCurSel(IndexConvert(cursel, TRUE));
			listbox->Open();
			listbox->Select(TRUE);
		}
	}else if(!show && listbox){
		//printf("list close\n");
		listbox->Close();
		//printf("list close end\n");
		delete listbox;
		listbox = NULL;
		
		SetFlag(IF_CAPTURED, FALSE);

	}
}

void CComboBox::SetDropDownSize(VD_PSIZE pSize)
{
	m_DropDownSize = *pSize;
}

VD_BOOL CComboBox::MsgProc(uint msg, uint wpa, uint lpa)
{
	//printf("CComboBox::MsgProc \n");
	int px, py;
	int key;
	signed char delta;

	newsel = cursel < 0 ? -1 : cursel;
	if (newsel < 0)
	{
		return FALSE;
	}
	
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_UP:
		{
			if (listbox)
			{
				if(!num)
				{
					break;
				}
loop1:			if(newsel>0)
					newsel--;
				else
					newsel = num-1;
				if(!items[newsel].valid)
					goto loop1;
			}
			else
				return FALSE;
		}
			break;
		case KEY_DOWN:
		{
			if (listbox)
			{
				if(!num)
				{
					break;
				}
loop2:			if(newsel<num-1)
					newsel++;
				else
					newsel = 0;
				if(!items[newsel].valid)
					goto loop2;
			}
			else
				return FALSE;
		}
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
		case KEY_11:
		case KEY_12:
		case KEY_13:
		case KEY_14:
		case KEY_15:
		case KEY_16:
			if(!listbox)
			{
				int ch;
				int i;

				//自动匹配到数字相同的项
				for(i = 0; i < num; i++)
				{
					sscanf(items[i].string.c_str(), "%d", &ch);
					if((key - KEY_0) == ch)
					{
						newsel = i;
						break;
					}
				}
			}
			break;
		case KEY_RET:
			if(!listbox)
			{  
				ShowDropDown(TRUE);
			}
			else if(listbox)
			{
				if(!reserve)// && !listbox->MsgProc(msg, wpa, lpa))
				{
					ShowDropDown(FALSE);
				}
				reserve = FALSE;//使标志失效
				return TRUE;
			}
			break;
		case KEY_ESC:
			if(listbox && !reserve)
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

	case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(m_Rect.PtInRect(px, py) && !listbox)
		{  //弹出下拉列表框或反动作
			ShowDropDown(TRUE);
			isListExist = 0;
		}
		else if(listbox)
		{
			isListExist = 1;
			//printf("ShowDropDown(FALSE) 000\n");
			//printf("combox 000 \n");
			if(!reserve && !listbox->MsgProc(msg, wpa, lpa))
			{
				//printf("ShowDropDown(FALSE) \n");
				ShowDropDown(FALSE);
			}
			reserve = FALSE;//使标志失效

			//printf("combox 111 \n");
			return TRUE;
		}
		break;

	case XM_LBUTTONUP:
		if(listbox){
			listbox->MsgProc(msg, wpa, lpa);
			return TRUE;
		}else{
			return FALSE;
		}
		break;

	case XM_MOUSEMOVE:
		if(listbox){
			listbox->MsgProc(msg, wpa, lpa);
			return TRUE;
		}
		break;

	case XM_MOUSEWHEEL:
	{
		if(!listbox)
		{
			if(!num)
			{
				break;
			}
			delta = (char)VD_HIWORD(wpa);
			if(delta < 0)
			{
loop3:			if(newsel > 0)
					newsel--;
				else
					newsel = num-1;
				if(!items[newsel].valid)
				{
					goto loop3;
				}
				break;
			}
			else if(delta > 0)
			{
loop4:			if(newsel < num-1)
					newsel++;
				else
					newsel = 0;
				if(!items[newsel].valid)
				{
					goto loop4;
				}
				break;
			}
		}
		else
		{
			if(listbox)
			{
				listbox->MsgProc(msg, wpa, lpa);
				return TRUE;
			}	
		}
	 }
		break;

	case XM_RBUTTONDOWN:
		 if(listbox) {   //cw_test
			isListExist = 1;  
		 }
	case XM_RBUTTONDBLCLK:
		if(listbox){
			ShowDropDown(FALSE);
		}else{
			return FALSE;
		}
		break;

	case XM_LIST:
		if(listbox && listbox == (CListBox*)lpa){
			//printf("combox 222 \n");
			newsel = IndexConvert(wpa, FALSE);
			ShowDropDown(FALSE);
		}
		break;
    //add by Lirl on Nov/23/2011,防止点击scrollBar点击第二次时自动关闭
    case 0x2000a:
        {
            msg = XM_LBUTTONDOWN;
            return this->MsgProc(msg, wpa, lpa);
        }
        break;
    //end
	default:
		return FALSE;
	}
	//if value changed
	if(newsel!=cursel){
		//if(!listbox)
		{
			ItemProc(m_vProc);
		}
		cursel = newsel;
		//if(!listbox)
		{
			ItemProc(m_vProc1);
		}
		Draw();
		if(listbox){
			listbox->SetCurSel(IndexConvert(cursel, TRUE), TRUE);
		}
	}
	return TRUE;
}

void CComboBox::SetCurSel(int sel)
{
	if(cursel == sel || sel<-1 || sel>num-1)
	{
		return;
	}
	if ((sel >= 0 && !items[sel].valid))
	{
		cursel = -1; //所有项都被屏蔽掉了
		for (int i = 0; i < num; i++)
		{
			if(items[i].valid)
			{
				cursel = i;
				break;
			}
		}
	}
	else
	{
		cursel = sel;
	}
	Draw();
}

int CComboBox::GetCurSel()
{
	return cursel;
}

void CComboBox::InitStrings(VD_PCSTR psz)
{
	if(psz == NULL){
		return;
	}
	VD_PCSTR string = psz;
	COMBO_ITEM item;

	item.valid = TRUE;
	num = 0;
	items.clear();
	while(*psz && num<COMBO_MAX)
	{
		if(*psz++ == '|')
		{
			item.string.assign(string, psz - string - 1);
			items.push_back(item);
			num++;
			string = psz;
		}
	}

	item.string.assign(string, psz - string);
	items.push_back(item);
	num++;
	cursel = num - 1;
}

int CComboBox::AddString(VD_PCSTR string, VD_BOOL redraw /* = FALSE */,int bindingNum/* = -1*/)
{
	if(num >= COMBO_MAX || string == NULL){
		return -1;
	}

	COMBO_ITEM item;
	item.string =GetParsedString(string) ;
	item.valid = TRUE;
	items.push_back(item);

	//cursel = num;
	num++;

	if(redraw){
		Draw();
	}
	if(listbox){
		listbox->AddString(string, "", redraw);
	}
	if (bindingNum != -1) //设置与项的绑定数据WANGQIN 2007-05-22
	{
		SetItemData(cursel,bindingNum);
	}
	return cursel;
}

char * CComboBox::GetString(int sel)
{
	if (sel >= GetCount() || sel < 0)  //zlb20111121 当无当前选项时
	{
		return "";
	}

	return (char *)((*(items.begin() + (sel == -1 ? cursel : sel))).string.c_str());
}

void CComboBox::DeleteString(VD_BOOL redraw /* = FALSE */)
{
	if(cursel<0)
		return ;

	items.erase(items.begin() + cursel);

	num--;

	if(cursel>num-1)
		cursel = num-1;

	if(redraw){
		Draw();
	}
	if(listbox){
		listbox->DeleteString(redraw);
	}
}

void CComboBox::SetString(VD_PCSTR string, VD_BOOL redraw /* = FALSE */)
{
	if(cursel < 0 || string == NULL){
		return ;
	}
	items[cursel].string = string;
	if(redraw){
		Draw();
	}
	if(listbox){
		listbox->SetString(string, "", redraw);
	}
}

void CComboBox::RemoveAll()
{
	COMBO_ITEM_VECTOR::iterator pi;
	if(num){
		items.clear();
		cursel = -1;
		num = 0;
		Draw();
	}
}

void CComboBox::SetMask(uint mask)
{
	int i;
	int n = MIN(32, num);
	for(i = 0; i < n; i++)
	{
		items[i].valid = (mask & BITMSK(i));
	}
	if (cursel >= 0 && !items[cursel].valid)
	{
		cursel = -1; //所有项都被屏蔽掉了
		for (i = 0; i < n; i++)
		{
			if(items[i].valid)
			{
				cursel = i;
				break;
			}
		}
	}
	Draw();
}

//序号转换，bValid为真表示从总序号转换成有效序号，否则表示从有效序号转换成总序号
int CComboBox::IndexConvert(int index, VD_BOOL bValid)
{
#ifdef COMBOBOX_HIDE_INVALID
	int ret;
	int i;
	if(bValid)
	{
		if(index < 0 || index >= num || !items[index].valid)
		{
			return -1;
		}
		for(i = 0, ret = 0; i < index; i++)
		{
			if(items[i].valid)
			{
				ret++;
			}
		}
		return ret;
	}
	else
	{
		if(index < 0)
		{
			return -1;
		}
		for(i = 0, ret = -1; i < num; i++)
		{
			if(items[i].valid)
			{
				ret++;
			}
			if(ret == index)
			{
				return i;
			}
		}
		return -1;
	}
#else
	return index;
#endif
}

int CComboBox::GetCount()
{
	return num;
}

int CComboBox::GetListBoxStatus()
{
    if (NULL == listbox)
    {
        return 0;
    }

    return 1;
}


//设置项数据
VD_BOOL CComboBox::SetItemData(int index, uint data)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}

	items[index].data = data;

	return TRUE;
}

//得到项数据
uint CComboBox::GetItemData(int index)
{
	if(index < 0 || index > num - 1)
	{
		return (uint)(-1);
	}

	return items[index].data;
}

void CComboBox::SetDefaultBitmap(ComboBoxBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case CBB_NORMAL:
		m_bmpNormal = VD_LoadBitmap(name);
		break;
	case CBB_SELECTED:
		m_bmpSelected = VD_LoadBitmap(name);
		break;
	case CBB_PUSHED:
		m_bmpPushed = VD_LoadBitmap(name);
		break;
	case CBB_DISABLED:
		m_bmpDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}


