

#include "GUI/Ctrls/Page.h"

VD_BITMAP* CListBox::bmp_ltb_normal = NULL;
VD_BITMAP* CListBox::bmp_ltb_selected = NULL;
VD_BITMAP* CListBox::bmp_ltb_disabled = NULL;



///////////////////////////////////////////////////////////////////
////////////        CListBox
CListBox::CListBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CItem *p /* = NULL */, uint vstyle /* = LTS_REPORT */, CTRLPROC onEntered /* = NULL */, CTRLPROC onChecked /* = NULL */, CTRLPROC onChanged /* = NULL */)
	:CItem(pRect, pParent, IT_LISTBOX,vstyle|styleEditable|styleAutoFocus)
	,m_FlagSelectAll(FALSE)
{
	LIST_ITEM item;

	item.state = LIS_VALID;
	item.image = NULL;
	item.height = (m_dwStyle & listboxTitle) ? TEXT_HEIGHT : 0;
	item.width = (m_dwStyle & listboxIndex) ? TEXT_WIDTH * 2 : 0;
	item.format = VD_TA_CENTER;
	if(m_dwStyle & listboxIcon)
	{
		item.width += CTRL_HEIGHT1;
		item.format |= (LIF_IMAGE | LIF_IMAGE_ON_RIGHT);
	}

	num = 1;
	column = 1;
	items.push_back(item);//corner

	//init to 1 column
	item.width = m_Rect.Width() - item.width - 3;
	item.format &= ~(LIF_IMAGE | LIF_IMAGE_ON_RIGHT);
	if(psz)
	{
		item.string = GetParsedString(psz);
	}
	item.format = VD_TA_YCENTER;
	InsertColumn(0, &item);
	
	cursel = -1;
	curhl = -1;
	index_start = 0;
	col_start = 0;
	m_iTextAlign = VD_TA_YCENTER;
	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
	m_BorderColor = VD_GetSysColor(COLOR_FRAME);
	m_SelBorderColor = VD_GetSysColor(COLOR_FRAMESELECTED);
	m_TextColor = VD_GetSysColor(VD_COLOR_WINDOWTEXT);
	m_SelectColor = VD_GetSysColor(COLOR_CTRLTEXTSELECTED);
	m_NoSelectColor = VD_GetSysColor(COLOR_CTRLTEXT);
	m_TitleLine = VD_GetSysColor(COLOR_FRAME);
	m_PopupColor= VD_GetSysColor(COLOR_POPUP);
	m_CtrlSelect = VD_GetSysColor(COLOR_FRAMESELECTED);

	m_pOwner = p;
	vscrollbar = NULL;
	hscrollbar = NULL;
	//使用自动布局带来的影响，需要放到打开的时候计算，否则scrollbar会出错
	//	rows = (m_Rect.Height() - item.height) / TEXT_HEIGHT;
	m_onEntered = onEntered;
	m_onChecked = onChecked;
	m_onChanged = onChanged;
}

CListBox::~CListBox()
{
	RemoveAll();
}

VD_SIZE& CListBox::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*10, CTRL_HEIGHT*4};
	return size;
}
void CListBox::SetRows()
{
	rows = (m_Rect.Height() - items[0].height) / TEXT_HEIGHT;
}

CListBox* CreateListBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CItem * p, uint vstyle, CTRLPROC onEntered, CTRLPROC onChecked, CTRLPROC onChanged)
{
	return new CListBox(pRect,pParent,psz,p,vstyle,onEntered,onChecked,onChanged);
}

VD_BOOL CListBox::Open()
{
	SetRows();
	CItem::Open();
	ShowScrollBar(num - 1 > rows);
	if(vscrollbar){
		vscrollbar->SetRange(0, num-2, FALSE);
		vscrollbar->SetPos(index_start, FALSE);
	}
	return TRUE;
}

VD_BOOL CListBox::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	int px, py;
	signed char delta;
	int temp; 
	VD_BOOL check = FALSE;
	int new_start;
	int newhl;
	int newsel;

	newhl = curhl;
	newsel = cursel;
	new_start = index_start;
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET:
			ItemProc(m_onEntered);
			break;
		case KEY_FUNC:
			if((m_dwStyle & listboxCheck) && cursel >= 0){
				if (items[(cursel + 1) * column].state & LIS_VALID)
				{
					items[(cursel + 1) * column + 1].state = items[(cursel + 1) * column + 1].state ^ LIS_CHECKED;
					DrawSingle(cursel + 1);
					check = TRUE;
				}
				break;
			}
			return FALSE;
		case KEY_UP:
			if(newsel>0)
			{
				newsel--;
			}

			break;
		case KEY_DOWN:
			if(newsel<num - 2)
			{
				newsel++;
			}

			break;
		case KEY_PREV:
			if(new_start >= rows)
			{
				new_start -= rows - 1;
			}
			else
			{
				new_start = 0;
			}
			if(newsel>=rows)
			{
				newsel -= rows - 1;
			}
			else if(newsel > 0)
			{
				newsel = 0;
			}

			break;
		case KEY_NEXT:
			if(new_start <= num - rows * 2)
			{
				new_start +=  rows - 1;
			}
			else
			{
				new_start = MAX(0, num - rows - 1);
			}
			if(newsel<=(num-rows-2))
			{
				newsel +=  rows - 1;
			}
			else if(newsel < num - 2)
			{
				newsel = num - 2;
			}

			break;
		default:
			return FALSE;
		}
		break;

	case XM_MOUSEWHEEL:
		delta = (char)VD_HIWORD(wpa);

		if(delta < 0)
		{
			if(new_start >= rows)
			{
				new_start -= rows - 1;

			}
			else
			{
				new_start = 0;
			}
		}
		else if(delta > 0)
		{
			if(new_start <= num - rows * 2)
			{
				new_start +=  rows - 1;

			}
			else
			{
				new_start = MAX(0, num - rows - 1);

			}
		}
		break;

	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if ((m_dwStyle & listboxTopCheck) && (num > 1))// 判断是否有全选框  
		{	
			int checkIndex = (temp + 1) * column;

			// 非法项不响应消息
			if(temp != -2 && (checkIndex >= 0 ? items[checkIndex].state & LIS_VALID : true))
			{
				VD_BOOL select_all =FALSE;//标记是否全部选中
				newsel = temp;
				
				//需要设置复选框
				if((m_dwStyle & listboxCheck) && px >= m_Rect.left + 2 + items[0].width
					&& px < m_Rect.left + 2 + items[0].width + TEXT_WIDTH)
				{
					items[(newsel + 1) * column + 1].state = items[(newsel + 1) * column + 1].state ^ LIS_CHECKED;
					if(newsel == -1)//选中所有的复选框
					{
						int i;
						select_all = TRUE;
						if(items[1].state & LIS_CHECKED)
						{
							for(i =0;i<num;i++)
							{
								if (items[(i+1)*column].state & LIS_VALID)
								{
									items[(i+1)*column+1].state = items[(i+1)*column+1].state | LIS_CHECKED;
								}
							}
							m_FlagSelectAll = TRUE;
						}
						else
						{
							for(i =0; i<num; i++)
							{
								if (items[(i+1)*column].state & LIS_VALID)
								{
									items[(i+1)*column+1].state = items[(i+1)*column+1].state ^ LIS_CHECKED;
								}
							}
							m_FlagSelectAll = FALSE;
						}
						
					}
					if (select_all) 
					{
						for(int n = 0; n <num + 1; n++)
						{
							if (items[(n+1)*column].state & LIS_VALID)
							{
								DrawSingle(n + 1, -1, FALSE);
							}
						}
					}

					//处理单击全选上后,全选框的变化  zgzhit
					{	
						int n = 1;
						for (int i = 1 ; i < num ; i++)
						{
							if (items[i*column+1].state & LIS_CHECKED)
							{
								n++;
							}
						}

						if (n == num)
						{
							items[1].state = items[1].state |   LIS_CHECKED;
							DrawSingle(0);
							m_FlagSelectAll = TRUE;
						}
					}

					if (m_FlagSelectAll && newsel >= 0)   //判断全选后， 在点击相关选项时的处理   zgzhit
					{
						items[1].state = (items[1].state & 0);
						items[(newsel+1)*column+1].state = ~(items[(newsel+1)*column+1].state^ LIS_CHECKED) ;
						DrawSingle(0);
						DrawSingle(newsel+1);
						int n = 1;
						for (int i =1 ; i < num ; i++)
						{
							if (items[i*column+1].state & LIS_CHECKED)
							{
								n++;
							}

						}
						if (n == num)
						{
							items[1].state = items[1].state | LIS_CHECKED;
							DrawSingle(0);
						}
					}

					if(newsel == cursel)
					{
						DrawSingle(cursel + 1);
					}
					//不在这里立即执行ItemProc,是因为当前选中项还没有被设置
					check = TRUE;
				}

				//选中项
				if(items[(newsel + 1) * column].state & LIS_VALID)
				{
					if(msg == XM_LBUTTONDBLCLK)
					{
						ItemProc(m_onEntered);
					}
					
					if(m_pOwner)
					{
						SetCurSel(newsel, TRUE);
						m_pOwner->MsgProc(XM_LIST, cursel, (uint)this);
						//此时列表框已被关闭，立即返回
						return TRUE;
					}
				}
				break;
			}
		}
		else
		{
			
			if(temp >= 0 && items[(temp + 1) * column].state & LIS_VALID)
			{
				newsel = temp;

				//需要设置复选框
				if((m_dwStyle & listboxCheck) && px >= m_Rect.left + 2 + items[0].width
					&& px < m_Rect.left + 2 + items[0].width + TEXT_WIDTH)
				{
					items[(newsel + 1) * column + 1].state = items[(newsel + 1) * column + 1].state ^ LIS_CHECKED;
					if(newsel == cursel)
					{
						DrawSingle(cursel + 1);
					}
					//不在这里立即执行ItemProc,是因为当前选中项还没有被设置
					check = TRUE;
				}

				//选中项
				if(items[(newsel + 1) * column].state & LIS_VALID)
				{
					if(msg == XM_LBUTTONDBLCLK)
					{
						ItemProc(m_onEntered);
					}
					//printf("XM_LIST 00\n");
					if(m_pOwner)
					{
						SetCurSel(newsel, TRUE);

						//printf("XM_LIST 11\n");
						m_pOwner->MsgProc(XM_LIST, cursel, (uint)this);

						//printf("XM_LIST 22\n");
						//此时列表框已被关闭，立即返回
						return TRUE;
					}
				}
				break;
			}
		}
		return vscrollbar&&vscrollbar->MsgProc(msg, wpa, lpa);

	case XM_LBUTTONUP:
		if(vscrollbar)
		{
			return vscrollbar->MsgProc(msg, wpa, lpa);
		}
		break;
		
	case XM_MOUSEMOVE:
		if(vscrollbar && vscrollbar->GetCapture())
		{
			return vscrollbar->MsgProc(msg, wpa, lpa);
		}
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		newhl = GetAt(px, py);
		if (newhl > -2 && !(items[(newhl + 1) * column].state & LIS_VALID))
		{
			newhl = cursel;
		}
		break;

	case XM_SCROLL:
		if(vscrollbar && vscrollbar == (CScrollBar*)lpa){
			new_start = wpa;
		}
		break;

	default:
		return FALSE;
	}

	//页面首项改变
	if(new_start != index_start)
	{
		index_start = new_start;
		Draw();
		if(vscrollbar && msg != XM_SCROLL)
		{ //不要递归的调用
			vscrollbar->SetPos(index_start, TRUE);
		}
	}

	//当前选中项改变
	if(m_dwStyle & stylePopup)
	{
		newsel = newhl;
	}
	if(newsel != cursel)
	{
		SetCurSel(newsel, TRUE);
		ItemProc(m_onChanged);
	}
	else if(newhl != curhl)
	{
		temp = curhl;
		curhl = newhl;
		DrawSingle(temp + 1);
		DrawSingle(curhl + 1);
	}

	//复选框状态改变
	if(check)
	{
		ItemProc(m_onChecked);
	}

	return TRUE;
}

//此函数的序号从水平标题或垂直标题开始计数
//0 -1 false
void CListBox::DrawSingle(int index, int col /* = -1 */, VD_BOOL drawback /* = TURE */)
{
	if(!DrawCheck())
	{
		return;
	}
	if(col < 0)
	{
		for(col = 0; col <= column; col++) //col == column时画出的是一个空项
		{
			DrawSingle(index, col, drawback);
		}
		
		return;
	}

	if(index < 0)
	{
		for(index = 0; index < num; index++)
		{
			DrawSingle(index, col, drawback);
		}
		return;
	}

	int i;
	CRgn rgn;//可视区域
	CRect rect_visible; //可视矩形
	CRect rect; //项区域
	int offs;
	
	//是否需要画标题和序号， 是否在大致在可视区域内，在这里判断
	if((index == 0 && !(m_dwStyle & listboxTitle)) || (col == 0 && !(m_dwStyle & listboxIndex))
		|| (index > 0 && index < index_start + 1) || (col > 0 && col < col_start + 1))
	{
		return;
	}
	//保存旧的可视区域
	m_DC.Lock();
	m_DC.GetRgn(&rgn);

	//设置新的可视区域
	offs = 2;
	rect_visible = m_Rect;
	rect_visible.left += offs;
	rect_visible.right -= offs;
	rect_visible.top += offs;
	rect_visible.bottom -= offs;
	if(vscrollbar)
	{
		rect_visible.right -= CTRL_HEIGHT1 - offs;
	}
	if(hscrollbar)
	{
		rect_visible.bottom -= CTRL_HEIGHT1 - offs;
	}
	m_DC.IntersectRect(rect_visible);
	m_DC.DPtoLP(rect_visible);

	//计算项区域, 同时处理了空项的区域， 当index >= num 或者 col >= column时
	rect.top = offs;
	if(index > 0)
	{
		rect.top += items[0].height;
		for(i = index_start + 1; i < index; i++)
		{
			rect.top += ((i >= num) ? TEXT_HEIGHT : items[i * column].height);
			if(rect.top > rect_visible.bottom)
			{
				goto draw_single_end;
			}
		}
	}
	rect.bottom = rect.top + ((index >= num) ? TEXT_HEIGHT : items[index * column].height);

	rect.left = offs;
	if(col > 0)
	{
		rect.left += items[0].width;
		for(i = col_start + 1; i < col; i++)
		{
			if(i >= column)
			{
				break;
			}
			rect.left += items[i].width;
		}
		if(rect.left > rect_visible.right)
		{
			goto draw_single_end;
		}
	}
	rect.right = ((col >= column) ? (m_Rect.Width() - offs) : (rect.left + items[col].width));

	//画背景
	if(index >= num)
	{
		m_DC.SetBrush(m_BkColor);
	}
	else
	{
		if(items[index * column].state & LIS_SELECTED)
		{
			m_DC.SetBrush(m_CtrlSelect);
		}
		else
		{
			if(m_dwStyle & stylePopup)
			{
				m_DC.SetBrush(VD_GetSysColor(COLOR_POPUP));				
			}
			else
			{
				m_DC.SetBrush(m_BkColor);
			}
		}
	}
	if(drawback || (index < num && items[index * column].state & LIS_SELECTED))
	{
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(rect);
	}

	//画分割线
	if(!(m_dwStyle & stylePopup))
	{
		m_DC.SetPen(m_TitleLine, VD_PS_SOLID, 2);
		
		if(col == 0)
		{
			m_DC.MoveTo(rect.right - 1, rect.top);
			m_DC.LineTo(rect.right - 1, rect.bottom - 1);
		}

		if(index == 0)
		{
			m_DC.MoveTo(rect.right - 1, rect.bottom - 1);
			m_DC.LineTo(rect.left, rect.bottom - 1);
		}
	}

	//超出范围的直接返回
	if(index >= num || col >= column)
	{
		goto draw_single_end;
	}

	//画复选框,标题栏增加全选框便于选择和取消
	m_DC.SetBrush(VD_GetSysColor(COLOR_CTRL));
	m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOWTEXT), VD_PS_SOLID, 2);

	if( (m_dwStyle & listboxCheck) && (col == 1) && (((m_dwStyle & listboxTopCheck) && (num > 1)) 
		||( !(m_dwStyle&listboxTopCheck) && (index > 0))))
	{
		m_DC.SetRgnStyle(RS_NORMAL);
		m_DC.Rectangle(CRect(rect.left + 3, rect.top + 3, rect.left + 19, rect.top + 21));
		if((items[index  * column + col].state& LIS_CHECKED) || (items[1].state & LIS_CHECKED))//标题栏也画复选框。
		{
			if((index == curhl + 1) && GetFlag(IF_SELECTED) && items[index * column].state & LIS_VALID)
			{
				m_DC.SetPen(m_SelectColor, VD_PS_SOLID, 1);
			}
			else
			{
				m_DC.SetPen(m_NoSelectColor, VD_PS_SOLID, 1);
			}
			m_DC.MoveTo(rect.left + 15, rect.top + 8);
			m_DC.LineTo(rect.left + 7, rect.top + 16);
			m_DC.LineTo(rect.left + 5, rect.top + 12);
		}
		rect.left += TEXT_WIDTH;
	}

	//画图片
	if(items[index * column + col].format & LIF_IMAGE)
	{
		m_DC.SetBkMode(BM_TRANSPARENT);
		if(items[index * column + col].format & LIF_IMAGE_ON_RIGHT)
		{
			m_DC.Bitmap(CRect(rect.right - TEXT_WIDTH, rect.top, rect.right, rect.bottom), items[index * column + col].image);
			rect.right -= TEXT_WIDTH;
		}
		else
		{
			m_DC.Bitmap(CRect(rect.left, rect.top, rect.left + TEXT_WIDTH, rect.bottom), items[index * column + col].image);
			rect.left += TEXT_WIDTH;
		}
	}
	
	//画文字
	m_DC.SetBkMode(BM_TRANSPARENT);
	m_DC.SetTextAlign(items[index * column + col].format & 0x0000FFFF);
	if(!(items[index * column].state & LIS_VALID))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	else
	{
		if(index == 0)
		{
			m_DC.SetFont(m_TextColor);
		}
		//同济天跃要求被选中的item的字体颜色改为绿色，wangq modify this 20061019
		else
		{
			if(index == curhl + 1 && GetFlag(IF_SELECTED)
				&& items[index * column].state & LIS_VALID)
			{
				m_DC.SetFont(m_SelectColor);
			}
			else
			{
				m_DC.SetFont(m_NoSelectColor);
			}
		}
	}
	if((m_dwStyle & listboxAutoIndex)&& col == 0 && index > 0)
	{
		char number[8];
		sprintf(number, "%d", index);
		m_DC.VD_TextOut(CRect(rect.left + 2, rect.top, rect.right - 2, rect.bottom), number);
	}
	else
	{
		m_DC.VD_TextOut(CRect(rect.left + 2, rect.top, rect.right - 2, rect.bottom), items[index * column + col].string.c_str());
	}

draw_single_end:
	
	//恢复旧的可视区域
	m_DC.SetRgn(&rgn);
	m_DC.UnLock();

}

void CListBox::DrawFrame()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();
	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	
	int penwidth = 2;

	if(GetFlag(IF_SELECTED))
	{
		m_DC.SetPen(m_SelBorderColor,VD_PS_SOLID,penwidth);
	}
	else
	{
		m_DC.SetPen(m_BorderColor,VD_PS_SOLID,penwidth);
	}

/*-----------------------------------------------------------------------
	添加属性页面
-----------------------------------------------------------------------*/
	if (!(m_dwStyle & styleNoBorder))
	{
		m_DC.SetRgnStyle(RS_HOLLOW);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	}
/*---------------------------------------------------------------------*/
	if(GetFlag(IF_SELECTED)){
		m_DC.SetPen(m_SelBorderColor);
	}else{
		m_DC.SetPen(m_BorderColor);
	}

	m_DC.UnLock();
}

void CListBox::Draw()
{
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();
	//draw region
	if (m_dwStyle & stylePopup)
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_POPUP));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	}
	else
	{
		m_DC.SetBrush(m_BkColor);
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	}

	m_DC.UnLock();
	DrawFrame();

	//draw title
	DrawSingle(0, -1, FALSE);

	//draw items
	for(int n = 0; n < rows + 1; n++)
	{
		DrawSingle(index_start + n + 1, -1, FALSE);
	}

	if (m_dwStyle&listboxAutoIndex)
	{
		char number[8];
		sprintf(number,"%d",num-1);
		SetCorner(number,TRUE);
	}

	//不能在此打开或关闭滚动条，会引起死锁，只能重画
	if(vscrollbar){
		vscrollbar->Draw();
	}
}

/*void CListBox::SetCheckAll(BOOL checkFlag)
{
	m_bCheckAll = checkFlag;
}
*/
void CListBox::Select(VD_BOOL flag)
{
	ShowTip(flag);

	SetFlag(IF_SELECTED, flag);
	if(!DrawCheck()){
		return;
	}

	DrawFrame();
	if(curhl>=0)
	{
		DrawSingle(curhl + 1);
	}

	if(vscrollbar)
	{
		vscrollbar->Select(flag);
	}
}

void CListBox::Show(VD_BOOL flag)
{
	CItem::Show(flag);
	if(vscrollbar)
	{
		vscrollbar->Show(flag);
	}
}

//插入栏
int	CListBox::InsertColumn(int col, LIST_ITEM *pItem, VD_BOOL redraw /*= FALSE*/)
{
	int i;
	LIST_ITEM item;

	if(col < 0 || col > column - 1)
	{
		col = column - 1;
	}
	col++;

	item.state = LIS_VALID;
	item.image = NULL;
	item.format = VD_TA_YCENTER;

	for(i = num; i > 1; i--)
	{
		items.insert(items.begin() + column * (i - 1) + col, item);
	}
	items.insert(items.begin() + col, *pItem);
	column++;

	if(redraw)
	{
		Draw();
	}
	return col - 1;
}

//删除栏
VD_BOOL CListBox::DeleteColumn(int col, VD_BOOL redraw /*= FALSE*/)
{
	int i;

	if(col < 0 || col > column - 1)
	{
		return FALSE;
	}
	col++;

	for(i = num; i > 0; i--)
	{
		items.erase(items.begin() + column * (i - 1) + col);
	}
	column--;

	if(redraw)
	{
		Draw();
	}

	return TRUE;
}

//设置栏
VD_BOOL CListBox::SetColumn(int col, LIST_ITEM *pItem, VD_BOOL redraw /*= FALSE*/)
{
	if(col < 0 || col > column - 1)
	{
		return FALSE;
	}
	col++;

	items[col] = *pItem;

	if(redraw)
	{
		Draw();
	}

	return TRUE;
}

//得到栏
VD_BOOL CListBox::GetColumn(int col, LIST_ITEM *pItem)
{
	if(col < 0 || col > column - 1)
	{
		return FALSE;
	}
	col++;

	*pItem = items[col];

	return TRUE;
}

//删除所有栏
void CListBox::RemoveAllColumn(VD_BOOL redraw  /*= FALSE*/)
{
	if(num != 1)//要求所有项已先删除
	{
		return;
	}

	items.erase(items.begin() + 2, items.begin() + column);
	column = 2;

	if(redraw)
	{
		Draw();
	}
}
//插入项
int CListBox::InsertItem(int index, LIST_ITEM *pItem, VD_BOOL redraw /*= FALSE*/)
{
	int i;
	LIST_ITEM item;

	if(index < 0 || index > num - 1)
	{
		index = num - 1;
	}
	index++;

	item.state = LIS_VALID;
	item.image = NULL;
	item.format = VD_TA_YCENTER;

	for(i = column; i > 1; i--)
	{
		items.insert(items.begin() + index * column, item);
	}
	items.insert(items.begin() + index * column, *pItem);
	num++;

	if(index - 1 <= cursel)
	{
		cursel++;
	}
	if(index - 1 <= index_start)
	{
		index_start++;
		if(index_start > num - 2)
		{
			index_start = num - 2;
		}
	}
	if(redraw)
	{
		Draw();
	}

	//打开滚动条,设置滚动条位置,需要在index_start修改过之后
	ShowScrollBar(num - 1 > rows);
	if(vscrollbar){
		vscrollbar->SetRange(0, num - 2, redraw);
		vscrollbar->SetPos(index_start, redraw);
	}

	return index - 1;
}

//删除项
VD_BOOL CListBox::DeleteItem(int index, VD_BOOL redraw /*= FALSE*/)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}
	index++;

	items.erase(items.begin() + index * column, items.begin() + (index + 1) * column);
	num--;

	if(index - 1 == cursel)
	{
		cursel = -1;
	}
	else if(index - 1 < cursel)
	{
		cursel--;
	}
	if(index - 1 <= index_start)
	{
		index_start--;
		if(index_start < 0)
		{
			index_start = 0;
		}
	}

	if(redraw)
	{
		Draw();
	}

	//打开滚动条,设置滚动条位置,需要在index_start修改过之后
	ShowScrollBar(num - 1 > rows);
	if(vscrollbar){
		vscrollbar->SetRange(0, num - 2, redraw);
		vscrollbar->SetPos(index_start, redraw);
	}

	return TRUE;
}

//设置项
int CListBox::SetItem(int index, LIST_ITEM *pItem, VD_BOOL redraw /*= FALSE*/)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}
	index++;

	items[index * column] = *pItem;

	if(redraw)
	{
		Draw();
	}

	return TRUE;
}

//得到项
VD_BOOL CListBox::GetItem(int index, LIST_ITEM *pItem)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}
	index++;

	*pItem = items[index * column];

	return TRUE;
}

VD_BOOL CListBox::SetCustomData(int index, const char *data)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}
	index++;

	items[index * column].customData = data;
	return TRUE;
}

//设置项数据
VD_BOOL CListBox::SetItemData(int index, uint data)
{
	if(index < 0 || index > num - 1)
	{
		return FALSE;
	}
	index++;

	items[index * column].data = data;

	return TRUE;
}

//得到项数据
uint CListBox::GetItemData(int index)
{
	if(index < 0 || index > num - 1)
	{
		return (uint)(-1);
	}
	index++;

	return items[index * column].data;
}

std::string CListBox::GetCustomItemData(int index)
{
	if(index < 0 || index > num - 1)
	{
		return "";
	}
	index++;
	return items[index * column].customData;
}

//删除所有项
void CListBox::RemoveAll(VD_BOOL redraw  /*= FALSE*/)
{
	if(num <= 1)
	{
		return;
	}
	
	items.erase(items.begin() + column, items.end());
	cursel = -1;
	num = 1;
	index_start = 0;
	SetCorner("", redraw);
	ShowScrollBar(FALSE);
	
	if(redraw)
	{
		Draw();
	}
}

//设置子项
VD_BOOL CListBox::SetSubItem(int index, int col, LIST_ITEM *pItem, VD_BOOL redraw /*= FALSE*/)
{
	if(index < 0 || index > num - 1 || col < 0 || col > column - 1)
	{
		return FALSE;
	}
	index++;
	col++;

	items[index * column + col] = *pItem;

	if(redraw)
	{
		DrawSingle(index, col);
	}
	return TRUE;
}

//得到子项
VD_BOOL CListBox::GetSubItem(int index, int col, LIST_ITEM *pItem)
{
	if(index < 0 || index > num - 1 || col < 0 || col > column - 1)
	{
		return FALSE;
	}
	index++;
	col++;

	*pItem = items[index * column + col];
	return TRUE;
}

void CListBox::SetTitle(VD_PCSTR psz)
{
	if(column < 2 || !psz || !items[1].string.compare(psz))
	{
		return;
	}
	
	items[1].string = psz;

	if(!DrawCheck() || !(m_dwStyle & listboxTitle))
	{	
		return;
	}

	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(items[0].width+2,0,m_Rect.Width()-2,TEXT_HEIGHT-2));
	m_DC.SetFont(VD_GetSysColor(VD_COLOR_WINDOWTEXT));
	m_DC.VD_TextOut(CRect(items[0].width+2,0,m_Rect.Width()-2,TEXT_HEIGHT),items[1].string.c_str());
}

int CListBox::AddString(VD_PCSTR string, VD_PCSTR header /* = "" */, VD_BOOL redraw /* = FALSE */)
{
	if(string == NULL || header == NULL)
	{
		return -1;
	}

	int sel;
	LIST_ITEM new_item;
	new_item.height = TEXT_HEIGHT;
	if((items[1].state & LIS_CHECKED) && (m_dwStyle & listboxTopCheck))
	{
		new_item.state = LIS_CHECKED;
	}
	else
	{
		new_item.state = LIS_VALID;
	}
	new_item.image = NULL;
	new_item.format = VD_TA_CENTER;
	new_item.data = 0;
	if(m_dwStyle & listboxAutoIndex)
	{
		char number[8];
		sprintf(number,"%d",(cursel < 0) ? 1 : cursel + 2);
		SetCorner(number , redraw);
	}
	else if(strlen(header))
	{
		SetCorner(header , redraw);
		new_item.string = header;
	}

	sel = InsertItem(-1, &new_item);
	new_item.string = GetParsedString(string);
	new_item.format = m_iTextAlign;
	SetSubItem(sel, 0, &new_item, redraw);


	return sel;
}

VD_BOOL CListBox::SetSubData(int index, int col, uint data)
{
	if(index < 0 || index > num - 1 || col < 0 || col > column - 1)
	{
		return FALSE;
	}
	index++;
	col++;

	items[index * column + col].data = data;
	return TRUE;
}

uint CListBox::GetSubData(int index, int col)
{
	index++;
	col++;
	return items[index * column + col].data;	
}

void CListBox::DeleteString(VD_BOOL redraw /* = FALSE */)
{
	if(cursel < 0)
	{
		return ;
	}
	LIST_ITEM item;
	
	DeleteItem(cursel, redraw);

	if(cursel > num - 2)
	{
		cursel = num - 2;
	}
	
	if(GetItem(cursel, &item) && item.string.size())
	{
		SetCorner(item.string.c_str() , redraw);
	}
	else
	{
		char number[8];
		sprintf(number,"%d",num - 1);
		SetCorner(number , redraw);
	}
}

void CListBox::SetString(VD_PCSTR string, VD_PCSTR header /* = "" */, VD_BOOL redraw /* = FALSE */)
{
	if(cursel < 0 || string == NULL ||  header == NULL)
	{
		return ;
	}

	items[(cursel + 1) * column].string = header;
	items[(cursel + 1) * column + 1].string = string;
}

VD_BOOL CListBox::SetSubString(int index, int col, VD_PCSTR psz, VD_BOOL redraw /* = FALSE */)
{
	if(!psz || index < 0 || index > num - 1 || col < 0 || col > column - 1)
	{
		return FALSE;
	}
	index++;
	col++;

	items[index * column + col].string = psz;
	items[index * column + col].format = m_iTextAlign;

	if(redraw)
	{
		DrawSingle(index, col);
	}
	return TRUE;
}

void CListBox::SetTextAlign(int align/* = TA_LEFTTOP*/)
{
	m_iTextAlign = align;
}

void CListBox::SetCurSel(int sel, VD_BOOL redraw /* = FALSE */)
{
	if(sel > num - 2/* || !(items[sel].state & LIS_VALID)*/){
		return;
	}
	SetRows();
	int temp = curhl;
	curhl = sel;
	if(temp != cursel)
	{
		DrawSingle(temp + 1);
	}

	if(sel == cursel)
	{
		return;
	}

	if(sel < 0 || (sel >= index_start && sel < index_start + rows))
	{
		if(cursel >= 0)
		{
			items[(cursel + 1) * column].state &= ~LIS_SELECTED;
			DrawSingle(cursel + 1);
		}
		cursel = sel;
		if(cursel >= 0)
		{
			items[(cursel + 1) * column].state |= LIS_SELECTED;
			DrawSingle(cursel + 1);
		}
	}else
	{ //需要翻页
		if(cursel > sel)
		{
			if(index_start >= rows)
			{
				index_start -= rows - 1;
			}
			else
			{
				index_start = 0;
			}
		}
		else
		{
			if(index_start <= num - rows * 2)
			{
				index_start +=  rows - 1;
			}
			else
			{
				index_start = MAX(0, num - rows - 1);
			}
		}
		if(cursel >= 0)
		{
			items[(cursel + 1) * column].state &= ~LIS_SELECTED;
		}
		cursel = sel;
		if(cursel >= 0)
		{
			items[(cursel + 1) * column].state |= LIS_SELECTED;
		}
		if(sel < index_start || sel >= index_start + rows) //翻页不成功, 直接定位
		{
			index_start = MIN(cursel, num - rows - 1);
			index_start = MAX(0, index_start);
		}
		
		if(redraw)
		{
			Draw();
		}
		if(vscrollbar){
			vscrollbar->SetPos(index_start, redraw);
		}
	}
}

void CListBox::SetCorner(VD_PCSTR psz, VD_BOOL redraw /* = FALSE */)
{
	if(!psz)
	{
		return;
	}

	items[0].string = psz;

	if( DrawCheck() && redraw)
	{
		DrawSingle(0, 0);
	}
}

int CListBox::GetCount()
{
	return num - 1;
}

int CListBox::GetCurSel()
{
	return cursel;
}

int CListBox::GetAt(int px, int py)
{
	if ((m_dwStyle & listboxTopCheck) && (num > 1)) // 判断是否有全选框  
	{
		int sel;
		if (px >= m_Rect.left && px <= m_Rect.right - (vscrollbar?CTRL_HEIGHT1:0) && py >= m_Rect.top && py <= m_Rect.top + items[0].height)
		{
			return -1;
		}
		if(px >= m_Rect.left && px <= m_Rect.right - (vscrollbar?CTRL_HEIGHT1:0) && py >= m_Rect.top + items[0].height && py <= m_Rect.bottom)
		{
			sel = (py - m_Rect.top -items[0].height) / TEXT_HEIGHT;
			if(sel >= 0)
			{
				sel += index_start;
			}
			if(sel >= num - 1)
			{
				return -2;
			}
			return sel;
		}
		return -2;
	}
	else
	{
		int sel;
		if(px >= m_Rect.left && px <= m_Rect.right - (vscrollbar?CTRL_HEIGHT1:0) && py >= m_Rect.top + items[0].height && py <= m_Rect.bottom){
			sel = (py - m_Rect.top -items[0].height) / TEXT_HEIGHT;
			/*if(sel > rows){
			sel = rows;
			}*/
			if(sel >= 0){
				sel += index_start;
			}
			if(sel >= num - 1){
				return -1;
			}
			return sel;
		}
		return -1;
	}
}

VD_BOOL CListBox::GetCheck(int index)
{
	if(!(m_dwStyle & listboxCheck) || column < 2 || index<0 || (index > (num - 1)))
	{
		return FALSE;
	}
	return (items[(index + 1) * column + 1].state & LIS_CHECKED);
}

VD_BOOL CListBox::SetCheck(int index, VD_BOOL checked /* = TRUE */, VD_BOOL redraw /* = FALSE */)
{
	if(!(m_dwStyle & listboxCheck) || column < 2 || index<0 || index >= num - 1)
	{
		return FALSE;
	}
	if(checked && (items[(index + 1) * column + 1].state & LIS_CHECKED)
		|| !checked && !(items[(index + 1) * column + 1].state & LIS_CHECKED))
	{
		return FALSE;
	}
	else
	{
		if (checked)
		{
			items[(index + 1) * column + 1].state |= LIS_CHECKED;
		}
		else
		{
			items[(index + 1) * column + 1].state &= ~LIS_CHECKED;
		}
		if(redraw)
		{
			DrawSingle(index + 1);
		}
		return TRUE;
	}
}

VD_BOOL CListBox::GetValid(int index)
{
	if(index<0 || index >= num - 1)
	{
		return FALSE;
	}
	return (items[(index + 1) * column].state & LIS_VALID);
}

VD_BOOL CListBox::SetValid(int index, VD_BOOL valid /* = TRUE */, VD_BOOL redraw /* = FALSE */)
{
	if(index<0 || index >= num - 1){
		return FALSE;
	}
	if(valid && (items[(index + 1) * column].state & LIS_VALID)
		|| !valid && !(items[(index + 1) * column].state & LIS_VALID))
	{
		return FALSE;
	}
	else
	{
		if (valid)
		{
			items[(index + 1) * column].state |= LIS_VALID;
		}
		else
		{
			items[(index + 1) * column].state &= ~LIS_VALID;
		}
		if(redraw)
		{
			DrawSingle(index + 1);
		}
		return TRUE;
	}
}

void CListBox::ShowScrollBar(VD_BOOL show)
{
	//自己都没有打开时，不用打开或关闭滚动条，打开后立即会调用一次
	if(!GetFlag(IF_OPENED)||!GetFlag(IF_SHOWN))//Modified 2007-05-31 by WANGQIN 如果整个listbox被设置了Show(FALSE)则vscrollbar也不能被显示。
	{
		return;
	}
	if(show && !vscrollbar){
		CRect rect(m_Rect.right - CTRL_HEIGHT1, m_Rect.top + 1, m_Rect.right - 1, m_Rect.bottom - 1);
		m_pParent->ScreenToClient(rect);
		/*begin:added by wangqin for scrollbar styles changed 20070316*/
		uint styles=0;
		if(m_dwStyle & stylePopup)
			styles=scrollbarY|stylePopup;
		else
			styles = scrollbarY;
		vscrollbar = CreateScrollBar(rect,m_pParent, styles, 0,num-2,rows);
		/*end wangqin modified at 20070316*/
		if(vscrollbar){
			vscrollbar->SetOwner(this);
			vscrollbar->Editable(FALSE);
			vscrollbar->Open();
			vscrollbar->SetPos(index_start, FALSE);
			vscrollbar->SetColor(SLB_BK,m_BkColor,FALSE);
			vscrollbar->SetColor(SLB_BORDER,m_BorderColor,FALSE);
			vscrollbar->SetColor(SLB_SELBORDER,m_SelBorderColor,FALSE);
			vscrollbar->SetColor(SLB_NOSEL_ITEM_TEXT,m_NoSelectColor,FALSE);
		}
	}else if(!show && vscrollbar){
		vscrollbar->Close();
		delete vscrollbar;
		vscrollbar = NULL;
	}
}


void CListBox::SetRect(VD_PCRECT pRect, VD_BOOL bReDraw /* = TRUE */)
{
	SetRows();
	//rows = (m_Rect.Height() - items[0].height)/TEXT_HEIGHT;
	rows = MAX(rows, 1);
	CItem::SetRect(pRect, bReDraw);
	ShowScrollBar(FALSE);
	ShowScrollBar(num - 1 > rows);
}

void CListBox::SetColor(int nRegion, VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	switch(nRegion) {
	case LTC_TEXT:
		m_TextColor = color;
		break;
	case LTC_BK:
		m_BkColor = color;
		break;
	case LTC_BORDER:
		m_BorderColor = color;
		break;
	case LTC_SELBORDER:
		m_SelBorderColor = color;
		break;
	case LTC_SEL:
		m_SelectColor = color;
		break;
	case LTC_NOSEL:
		m_NoSelectColor = color;
		break;
	case LTC_TITLELINE:
		m_TitleLine = color;
		break;
	default:
		break;
	}
	if(redraw)
	{
		Draw();
	}
}

//设置全部选中的复选框有效,必须在listboxCheck属性有效的情况下才有效。
void CListBox::SetSelectAll(VD_BOOL bSet )
{
	if(m_dwStyle & listboxCheck)
	{
		if(bSet)
			m_dwStyle|=listboxTopCheck;
		else
			m_dwStyle&=~listboxTopCheck;
		//m_bSetCheckAll = bSet;
	}
}
//added wangq 20070111 返回当前是否被全选的状态
VD_BOOL CListBox::GetIfSelectAll()
{
	if(m_dwStyle & listboxTopCheck)
	{
		return m_FlagSelectAll;
	}
	else
	{
		return FALSE;
	}
}

VD_BOOL CListBox::Close()
{
	//恢复页面初始值设置。
	if(m_FlagSelectAll)
	{
		m_FlagSelectAll = FALSE;
	}
	for(int n = 0; n < num; n++)
	{
		items[n  * column +1].state = items[n  * column].state & 0;
	}
	ShowScrollBar(FALSE);
	return CItem::Close();
}

void CListBox::SetDefaultBitmap(ListBoxBitmap region, VD_PCSTR name)
{
	switch(region) {
	case LTB_NORMAL:
		bmp_ltb_normal = VD_LoadBitmap(name);
		break;
	case LTB_SELECTED:
		bmp_ltb_selected = VD_LoadBitmap(name);
		break;
	case LTB_DISABLED:
		bmp_ltb_disabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}

