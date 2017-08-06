//
//  "$Id: Edit.cpp 279 2008-12-17 05:58:45Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "ez_libs/ezutil/str_opr.h"

#include "GUI/Ctrls/Page.h"

KEY_INPUT_TYPE CEdit::input_type = KI_NUMERIC; //KI_LOWERCASE
VD_BOOL CEdit::m_bMarkPadShown = FALSE;

VD_BITMAP* CEdit::bmp_etb_normal = NULL;
VD_BITMAP* CEdit::bmp_etb_selected = NULL;
VD_BITMAP* CEdit::bmp_etb_disabled = NULL;


CEdit::CEdit(VD_PCRECT pRect, CPage * pParent, int vlen/* =6 */, uint vstyle /* = 0 */, CTRLPROC onChanged /* = NULL */)
	:CItem(pRect, pParent, IT_EDIT,vstyle|styleEditable|styleAutoFocus)
{
	m_onChanged = onChanged;
	showpos = 0;
	curpos = 0;
	word_pos = 0;
	len_max = vlen;
	numpad = NULL;
	modifying = FALSE;
	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
	m_bMarkPadShown = FALSE;
}

CEdit::~CEdit()
{
}

VD_SIZE& CEdit::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH * 6,CTRL_HEIGHT};
	return size;
}

CEdit* CreateEdit(VD_PCRECT pRect, CPage * pParent, int vlen, uint vstyle , CTRLPROC onChanged )
{
	return new CEdit(pRect,pParent, vlen, vstyle, onChanged);
}
void CEdit::SetFocus(VD_BOOL flag)
{
	if(flag)
	{
		curpos = input.size();
		word_pos = items.size();
		showpos = 0;
		UpdateCaretPos();
	}
	else
	{
		curpos = 0;
		word_pos = 0;
		showpos = 0;
		UpdateCaretPos();
		modifying = FALSE;
	}

	CItem::SetFocus(flag);
}

void CEdit::Select(VD_BOOL flag)
{
	if(!flag)
	{
		ShowDropDown(FALSE);
		CItem * pSpellPad = m_pSpellPad[m_screen]; //cw_panel
		if(pSpellPad)
		{
			pSpellPad->Close();
		}
	}
	CItem::Select(flag);
}

void CEdit::Draw()
{
	int i;
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();

	m_DC.SetBrush(m_BkColor);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.SetRgnStyle(RS_HOLLOW);
	if(GetFlag(IF_SELECTED))
	{	
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
	}
	else
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
	}
	if (!(m_dwStyle & styleNoBorder))
	{
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
	}

	if(GetFlag(IF_SELECTED))
	{	
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
	}
	else
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}
	if(!GetFlag(IF_ENABLED))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	if(m_dwStyle & editPassword)
	{
		int len = input.size() - showpos;
		if(len > (m_Rect.Width() - TEXTBOX_X * 2) / 12)
		{
			len = (m_Rect.Width() - TEXTBOX_X * 2) / 12;
		}

		m_DC.SetRgnStyle(RS_FLAT);
		for(i = 0; i < len; i++)
		{
			if(i == len - 1 && modifying && curpos)//最后一个正在修改的字符高亮显示
			{
				m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOWTEXT));
			}
			m_DC.Rectangle(CRect(TEXTBOX_X + i*12,m_Rect.Height()/2-5,TEXTBOX_X+i*12+10,m_Rect.Height()/2+5), 3, 3);
		}
	}
	else
	{
		m_DC.SetBkMode(BM_TRANSPARENT);
		m_DC.SetTextAlign(VD_TA_YCENTER);
		m_DC.VD_TextOut(CRect(TEXTBOX_X,3,m_Rect.Width()-TEXTBOX_X,m_Rect.Height()+1),input.c_str() + showpos);

		//高亮显示最后一个字符
		if(modifying && curpos)
		{
			m_DC.SetFont(VD_GetSysColor(VD_COLOR_WINDOWTEXT));
			m_DC.VD_TextOut(CRect(TEXTBOX_X + caret_pos - last_char_with, 3, TEXTBOX_X + caret_pos, m_Rect.Height()+1),
				&input[curpos - 1]);
		}
	}
	
	caret_cnt = 0;
	m_DC.UnLock();
}

void CEdit::DrawCaret()
{
	if(!DrawCheck() || !GetFlag(IF_FOCUSED)){
		return;
	}
	m_DC.Lock();
	caret_cnt++;

	m_DC.SetPen(m_BkColor ^ VD_GetSysColor(COLOR_FRAMESELECTED), VD_PS_SOLID, 2);

	m_DC.SetRop(ROP_XOR);
	m_DC.MoveTo(TEXTBOX_X+caret_pos-1, 6);
	m_DC.LineTo(TEXTBOX_X+caret_pos-1, m_Rect.Height()-6);
	m_DC.SetRop(ROP_COPY);
	m_DC.UnLock();
}

void CEdit::ShowDropDown(VD_BOOL show)
{
	VD_RECT rect;            //输入板区域 
	VD_RECT full_rect;      //全屏区域
	VD_SIZE array_size;    //输入板区域大小行列数
	INPUT_PAD_TYPE pad_type;  //输入板类型
	
	if(show && !numpad)
	{
		switch(input_type) 
		{
		case KI_CONTROL:        //原始输入
		case KI_NUMERIC:        //数字输入
			pad_type = IPT_NUMBER;
			break;
		case KI_UPPERCASE:      //小写字母输入
			pad_type = IPT_UPPERCASE;
			break;
		case KI_LOWERCASE:      //大写字母输入
		case KI_PINYIN:
			pad_type = IPT_LOWERCASE;
			break;
		case KI_MARK:
			pad_type = IPT_MARK;
			break;

		default:
			return;
		}


		//pad_type = IPT_LOWERCASE;
		//下拉显示的区域
		CNumberPad::GetPadSize(pad_type, &array_size);     //直接取各种输入板大小
		m_pDevGraphics->GetRect(&full_rect);
		rect.left = m_Rect.left;
		rect.right = m_Rect.left + array_size.w ;
		if(m_Rect.bottom + array_size.h> full_rect.bottom)
		{
			rect.top = m_Rect.top - array_size.h;
			rect.bottom = m_Rect.top;
		}
		else
		{
			rect.top = m_Rect.bottom;
			rect.bottom = m_Rect.bottom + array_size.h;
		}
		m_pParent->ScreenToClient(&rect);

		//创建并打开下拉写字板
		SetFlag(IF_CAPTURED, TRUE);
		numpad = new CNumberPad(&rect, m_pParent, this, pad_type);
		//numpad = new CNumberPad(&rect, m_pParent, this, IPT_LOWERCASE);
		
		if(numpad)
		{
			numpad->Editable(FALSE);
			numpad->Open();
		}
	}
	else if(!show && numpad)
	{
		SetFlag(IF_CAPTURED, FALSE);
		numpad->Close();
		delete numpad;
		numpad = NULL;
	}
}

VD_BOOL CEdit::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int key;
	int cl;
	int n;	//字符字节计数
	INPUT_CHAR input_char;	//字符消息中的字符
	CItem * pSpellPad = m_pSpellPad[m_screen]; //拼写面板

	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_UP:
			if(word_pos > 0)
			{
				word_pos--;
				curpos -= items[word_pos].len;
				UpdateCaretPos(TRUE);
			}
			break;
		case KEY_DOWN:
			if(word_pos < (int)items.size())
			{
				curpos += items[word_pos].len;
				word_pos++;
				UpdateCaretPos(TRUE);
			}
			break;


		case KEY_RET:
			if(!numpad)
			{  
				if(input_type != KI_PINYIN || (m_pParent && m_pParent == pSpellPad))
				{
					if(input_type == KI_MARK && m_bMarkPadShown)
					{
						m_bMarkPadShown = FALSE;
					}
					ShowDropDown(TRUE);
				}
				else if(pSpellPad && !pSpellPad->IsActive())
				{
					pSpellPad->SetOwner(this);
					pSpellPad->Open();
				}
			}
			else if(numpad)
			{
				if(!numpad->MsgProc(msg, wpa, lpa))
				{
					ShowDropDown(FALSE);
				}
				return TRUE;
			}
			break;
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
		case KEY_SPLIT:
		case KEY_FUNC:
			if(GetKeyInputFlag(KIF_SHIFT))//如果有shift键，处理为Fn键，然后退出
			{
				break;
			}//no break here;

		default:
			return FALSE;
		}
		break;

	case XM_CHAR:
		input_char.dw = wpa;
		//输入字符个数满后, 不能更改
		if(!modifying && (int)input.size() == len_max && input_char.ch[0] == '\b' && input_char.ch[1])
		{
			return TRUE;
		}

		modifying = lpa;
		for(n = 0; n < 4 && input_char.ch[n]; n++)
		{
			if(input_char.ch[n] == '\f') 
			{
				if(word_pos > 0)
				{
					word_pos = 0;
					input.clear();
					curpos = 0;
					items.clear();
				}
				else
				{
					//trace("CEdit::MsgProc string delete all error\n");
					return TRUE;
				}
			}
			else if(input_char.ch[n] == '\b')
			{
				if(word_pos > 0)
				{
					word_pos--;
					input.erase(input.begin() + curpos - items[word_pos].len, input.begin() + curpos);
					curpos -= items[word_pos].len;
					items.erase(items.begin() + word_pos);
				}
				else
				{
					//trace("CEdit::MsgProc string delete one error\n");
					return FALSE; // 表示没有对退格字符处理
				}
			}
			else
			{
				EDIT_ITEM edit_item;

				g_Locales.GetCharCode(&input_char.ch[n], &cl);
				edit_item.len = cl;
				if((int)(input.size() + edit_item.len) > len_max)//输入字符已满
				{
					modifying = FALSE;
					return TRUE;
				}
				if(n + edit_item.len > 4)
				{
					modifying = FALSE;
					printf("CEdit::MsgProc string insert error\n");
					return TRUE;
				}
				input.insert(input.begin() + curpos, input_char.ch[n]);
				for(cl = 1; cl < edit_item.len; cl++)
				{
					input.insert(input.begin() + curpos + cl, input_char.ch[n + cl]);
				}
				n += edit_item.len - 1; //字节计数增加
				curpos += edit_item.len;
				items.insert(items.begin() + word_pos, edit_item);
				word_pos++;
			}
		}
		UpdateCaretPos();
		Draw();
		if(input_char.ch[0]) //如果有新的字符, 才调用回调函数
		{
			ItemProc(m_onChanged);
		}
		break;
		
	case XM_MOUSEMOVE:
		//printf("numpad->MsgProc(msg, wpa, lpa)\n");
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
			//printf("@@@@11111\n");
			if(input_type != KI_PINYIN || (m_pParent && m_pParent == pSpellPad))
			{
				if(input_type == KI_MARK && m_bMarkPadShown)
				{
					m_bMarkPadShown = FALSE;
				}
				ShowDropDown(TRUE);
			}
			else if(pSpellPad && !pSpellPad->IsActive())
			{
				pSpellPad->SetOwner(this);
				pSpellPad->Open();
			}
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

	case XM_CARET:
		DrawCaret();
		break;

	case XM_INPUTTYPE:
		//printf("%s: input type: %d\n", __func__, (KEY_INPUT_TYPE)wpa);
		if(input_type != (KEY_INPUT_TYPE)wpa || lpa) // lpa表示是否强制打开面板
		{
			input_type = (KEY_INPUT_TYPE)wpa;
			if(modifying)
			{
				modifying = FALSE;
				Draw();
			}
			if(numpad || (pSpellPad && pSpellPad->IsActive()) || lpa)
			{
				if(numpad)
				{
					ShowDropDown(FALSE); // 关闭字符面板
				}

				if(m_bMarkPadShown && input_type != KI_MARK) //从符号面板切换到其他面板时, 判断历史状态
				{
					m_bMarkPadShown = FALSE;
				}
				else if(input_type == KI_PINYIN)
				{
					if(pSpellPad && !pSpellPad->IsActive()) // 打开拼写面板
					{
						pSpellPad->SetOwner(this);
						pSpellPad->Open();
					}
				}
				else if(m_pParent != pSpellPad)
				{
					ShowDropDown(TRUE); // 打开字符面板
				}
			}
			else if(input_type == KI_MARK) //输入符号时, 始终弹出面板
			{
				m_bMarkPadShown = TRUE;
				
				ShowDropDown(TRUE);
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

VD_BOOL CEdit::IsPadOpen()
{
	return numpad?TRUE:FALSE;
}

void CEdit::Empty()
{
	input.erase();
	items.clear();
	curpos = 0;
	word_pos = 0;
	UpdateCaretPos();
	Draw();
	modifying = FALSE;//字符输入状态复位
}

int CEdit::GetText(char *buf, int vlen)
{
	if(vlen <= 0)
	{
		return 0;
	}
	
	int maxlen = MIN(vlen, (int)input.size());
	memcpy(buf, input.c_str(), maxlen);
	
	if(maxlen < vlen)
	{
		buf[maxlen] = '\0';
	}
	
	if(m_dwStyle & editPassword)
	{
		trim_blank(buf, 0);
	}

	return maxlen;
}

void CEdit::SetText(const char *psz)
{
	int i, len;
	int cl;

	if(psz)
	{
		input = psz;
	}
	else
	{
		input.erase();
	}
	len = input.size();
	if(len > len_max)
	{
		input.erase(len_max, len - len_max); //删除多余的字符
	}
	curpos = 0;
	
	//计算词的当前指针
	EDIT_ITEM edit_item;
	items.clear();
	word_pos = 0;
	for(i = 0; i < len; i += cl)
	{
		g_Locales.GetCharCode(&input[i], &cl);
		if(cl == 0)
		{
			input.erase(i, len - i); //删除无法解析的字节
			break;
		}
		edit_item.len = cl;
		items.push_back(edit_item);
	}

	UpdateCaretPos();
	Draw();
}

void CEdit::UpdateCaretPos(VD_BOOL redraw /* = FALSE */)
{
	int right_width;		//插入符右边的字符串宽度
	int box_width = m_Rect.Width() - TEXTBOX_X * 2;		//编辑框的宽度
	int curpos_minus1;
	int i, j;
	int w;
	VD_BOOL flag = FALSE;

	if(caret_cnt % 2)
	{
		DrawCaret();
	}
	
	curpos_minus1 = curpos - (curpos ? items[word_pos - 1].len: 0);
	right_width = GetTextExtent(curpos_minus1);
	
	//插入符位置在显示窗口之外之前,移动显示位置
	if(curpos_minus1 < showpos
		|| GetTextExtent(showpos, curpos - showpos) > box_width)
	{
		flag = TRUE;
		if(curpos_minus1 >= showpos && right_width >= box_width) //插入符在显示窗口右侧, 且右侧字符串充足
		{
			showpos = curpos_minus1;
		}
		else
		{
			if(curpos_minus1 < showpos) //插入符在显示窗口左侧
			{
				i = curpos;
				j = word_pos - 1;
			}
			else //插入符在显示窗口右侧
			{
				i = input.size();
				j = items.size() - 1;
			}
			
			for(w = 0; j >= 0; j--)
			{
				i -= items[j].len;
				w += GetTextExtent(i, items[j].len);
				if(w > box_width)
				{
					i += items[j].len;
					break;
				}
			}
			showpos =  i;
		}
	}
	
	caret_pos = GetTextExtent(showpos, curpos - showpos);
	if(curpos)
	{
		last_char_with = GetTextExtent(curpos - 1, 1);
	}
	
	if(flag && redraw)//显示窗口变化导致重画
	{
		Draw();
	}
	DrawCaret();
}

void CEdit::SetBkColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_BkColor = color;
	if(redraw)
	{
		Draw();
	}
}

//得到字符串宽度
int CEdit::GetTextExtent(int pos, int len /* = 1024 */)
{
	int width;
	
	if(m_dwStyle & editPassword)//密码框特殊处理
	{
		if(pos + len > (int)input.size())
		{
			width = (input.size() - pos) * 12;
		}
		else
		{
			width = len * 12;
		}
		/*if(modifying && pos < curpos && pos + len <= curpos) //显示选择的字符
		{
			width = width - 12 + g_Locales.GetTextExtent(input.c_str() + curpos - 1, 1);
		}*/
		return width;
	}
	else
	{
		return g_Locales.GetTextExtent(input.c_str() + pos, len);
	}
}

uint CEdit::GetStyle()
{
	return m_dwStyle;
}

void CEdit::SetDefaultBitmap(EditBitmap region, VD_PCSTR name)
{
	switch(region){
		case ETB_NORMAL:
			bmp_etb_normal = VD_LoadBitmap(name);
			break;
		case ETB_SELECTED:
			bmp_etb_selected = VD_LoadBitmap(name);
			break;
		case ETB_DISABLED:
			bmp_etb_disabled =VD_LoadBitmap(name);
			break;
		default:
			break;
			
	}
}

//add by nike.xie 20090907  //对域名编辑框做特殊处理
CDDNSEdit::CDDNSEdit(VD_PCRECT pRect, CPage * pParent, int vlen/*=6*/, uint vstyle/* = 0*/, CTRLPROC onChanged/* = NULL*/)
	:CEdit(pRect, pParent, vlen, vstyle, onChanged)
{

}

CDDNSEdit::~CDDNSEdit()
{

}

VD_BOOL CDDNSEdit::MsgProc(uint msg, uint wpa, uint lpa)
{
	switch(msg)
	{
		case XM_CHAR:
			if((wpa =='/') || (wpa ==':') || (wpa =='?') || (wpa =='@') || (wpa =='#') || (wpa =='%'))
				return FALSE;
		default:
			break;
	}
	
	return CEdit::MsgProc(msg, wpa, lpa);
}
//end

