

#include "GUI/Pages/PageInputTray.h"


static char *keyMapWrong[10] = {
  "\1 ",      // 0
  "\3abc",    // 1
  "\3def",    // 2
  "\3ghi",    // 3
  "\3jkl",    // 4
  "\3mno",    // 5
  "\4pqrs",   // 6
  "\3tuv",    // 7
  "\4wxyz",   // 8
  "\0",       // 9
};

static char *keyMapCorrect[10] = {
  "\1 ",      // 0
  "\0",       // 1
  "\3abc",    // 2
  "\3def",    // 3
  "\3ghi",    // 4
  "\3jkl",    // 5
  "\3mno",    // 6
  "\4pqrs",   // 7
  "\3tuv",    // 8
  "\4wxyz",   // 9
};

extern char* numpad_signs[IPT_TOTAL][MAX_PAD_ITEMS];

CPageInputTray::CPageInputTray(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent)
	:CPage(pRect, psz, pParent, pageNoExpire | pageActiveMost),
	m_CharTimer("CEdit_Char"),
	m_DeleteTimer("CEdit_Delete")
{
	VD_RECT rect;

	bmp_input_control = VD_LoadBitmap(DATA_DIR"/temp/input_control.bmp");
	bmp_input_numeric = VD_LoadBitmap(DATA_DIR"/temp/input_numeric.bmp");
	bmp_input_lowercase = VD_LoadBitmap(DATA_DIR"/temp/input_lowercase.bmp");
	bmp_input_uppercase = VD_LoadBitmap(DATA_DIR"/temp/input_uppercase.bmp");
	bmp_input_pinyin = VD_LoadBitmap(DATA_DIR"/temp/input_pinyin.bmp");
	bmp_input_mark = VD_LoadBitmap(DATA_DIR"/temp/input_mark.bmp");




	GetClientRect(&rect);
	/*printf("rect left = %d \n",rect.left);
	printf("rect right = %d \n",rect.right);
	printf("rect top = %d \n",rect.top);
	printf("rect bottom = %d \n",rect.bottom);*/
	
	m_pButton = CreateButton(&rect, this, NULL, (CTRLPROC)&CPageInputTray::OnInputTypeShift);

	
	if (!GetKeyInputFlag(KIF_NUMERIC))//没有数字键
	{
		m_pButton->SetBitmap(bmp_input_control, bmp_input_control, bmp_input_control, bmp_input_control, FALSE);
		m_iInputType = KI_CONTROL;
	}
	else
	{
		m_pButton->SetBitmap(bmp_input_numeric, bmp_input_numeric, bmp_input_numeric, bmp_input_numeric, FALSE);
		m_iInputType = KI_NUMERIC;
	}

	//m_pButton->SetBitmap(bmp_input_control, bmp_input_control, bmp_input_control, bmp_input_control, FALSE);
	//m_iInputType = KI_CONTROL;

	m_pInputTray[m_screen] = this;

	pPageCharList = new CPageCharList(CRect(0,0,210,28), "CharList", NULL, this);

	char_index = 0;
}

CPageInputTray::~CPageInputTray()
{
	ReleaseBitmap(bmp_input_control);
	ReleaseBitmap(bmp_input_numeric);
	ReleaseBitmap(bmp_input_lowercase);
	ReleaseBitmap(bmp_input_uppercase);
	ReleaseBitmap(bmp_input_pinyin);
	ReleaseBitmap(bmp_input_mark);

	delete pPageCharList;
}

VD_BOOL CPageInputTray::UpdateData(UDM mode)
{
	switch(mode)
	{
		case UDM_OK:
		case UDM_CANCEL:
			return FALSE;
		default:
			return TRUE;
	}
}

void CPageInputTray::Show(VD_BOOL flag, VD_BOOL redraw /* = TRUE */)
{
	//printf("CPageInputTray::Show \n");
	CItem *pOwner = GetOwner();

	if(pOwner)
	{
		uint item_style = pOwner->GetStyle();			
		if( ((item_style & edit_KI_NUMERIC) && (m_iInputType != KI_NUMERIC)) || 
		    ((item_style & edit_KI_NOCH) && (m_iInputType == KI_PINYIN)) )
		{
			OnInputTypeShift();
		}
	}
		
	if(!flag)
	{
		if(pOwner)
		{
			char_index = 0;//字符输入状态复位
			m_CharTimer.Stop();
			m_DeleteTimer.Stop(); //字符删除定时器停止
		}
	}
	else
	{
		if (!pOwner)
		{
			return;
		}
		OnInputTypeCheck();
	}

	CPage::Show(flag, redraw);
}

//处理输入法切换和字符转换
VD_BOOL CPageInputTray::MsgProc(uint msg, uint wpa, uint lpa)
{
	char **keyMap = NULL;
	int key;
	int num;
	CItem *pOwner;
	INPUT_CHAR input_char;
	int ret;
	
	pOwner = GetOwner();
	if (!pOwner)
	{
		return FALSE;
	}

	input_char.dw = 0;
	switch (msg)
	{
	case XM_KEYDOWN:
		key = wpa;
/*		if (!GetKeyInputFlag(KIF_NUMERIC) && m_iInputType != KI_CONTROL)//ATM机型控制键转数字键
		{
			switch(GetKeyInputFlag(KIF_NEWATM)) 
			{
			case 0://二代旧面板
				switch (key)
				{
				case KEY_ESC:
					key = KEY_0;
					break;
				case KEY_PREV:
					key = KEY_1;
					break;
				case KEY_SLOW:
					key = KEY_2;
					break;
				case KEY_PLAY:
					key = KEY_3;
					break;
				case KEY_FAST:
					key = KEY_4;
					break;
				case KEY_NEXT:
					key = KEY_5;
					break;
				case KEY_UP:
					key = KEY_6;
					break;
				case KEY_DOWN:
					key = KEY_7;
					break;
				case KEY_LEFT:
					key = KEY_8;
					break;
				case KEY_RIGHT:
					key = KEY_9;
					break;
				default:
					break;
				}
				break;
			case 1://二代新面板
				switch (key)
				{
				case KEY_NEXT:
					key = KEY_0;
					break;
				case KEY_UP:
					key = KEY_1;
					break;
				case KEY_LEFT:
					key = KEY_2;
					break;
				case KEY_RIGHT:
					key = KEY_3;
					break;
				case KEY_DOWN:
					key = KEY_4;
					break;
				case KEY_BACK:
					key = KEY_5;
					break;
				case KEY_PLAY:
					key = KEY_6;
					break;
				case KEY_SLOW:
					key = KEY_7;
					break;
				case KEY_FAST:
					key = KEY_8;
					break;
				case KEY_PREV:
					key = KEY_9;
					break;
				default:
					break;
				}
				break;
			case 2://惠州德赛前面板
				switch (key)
				{
				case KEY_NEXT:
					key = KEY_7;
					break;
				case KEY_UP:
					key = KEY_1;
					break;
				case KEY_LEFT:
					key = KEY_2;
					break;
				case KEY_RIGHT:
					key = KEY_3;
					break;
				case KEY_DOWN:
					key = KEY_4;
					break;
				case KEY_BACK:
					key = KEY_5;
					break;
				case KEY_PLAY:
					key = KEY_6;
					break;
				case KEY_SLOW:
					key = KEY_9;
					break;
				case KEY_FAST:
					key = KEY_0;
					break;
				case KEY_PREV:
					key = KEY_8;
					break;
				default:
					break;
				}
				break;
			case 3://广电运通,Honeywell前面板
				switch (key)
				{
				case KEY_DOWN:
					key = KEY_0;
					break;
				case KEY_BACK:
					key = KEY_1;
					break;
				case KEY_PLAY:
					key = KEY_2;
					break;
				case KEY_FAST:
					key = KEY_3;
					break;
				case KEY_PREV:
					key = KEY_4;
					break;
				case KEY_SLOW:
					key = KEY_5;
					break;
				case KEY_NEXT:
					key = KEY_6;
					break;
				case KEY_LEFT:
					key = KEY_7;
					break;
				case KEY_UP:
					key = KEY_8;
					break;
				case KEY_RIGHT:
					key = KEY_9;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		key = wpa; //add by xie
*/		switch (key)
		{
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
			num = (key - KEY_0) % 10;
			if (pOwner->GetType() != IT_EDIT)
			{
				pOwner->MsgProc(XM_KEYDOWN, key, lpa);
				break;
			}
			if (m_iInputType == KI_NUMERIC || m_iInputType == KI_CONTROL)//数字框或数字输入
			{
				input_char.ch[0] = num + '0';
				pOwner->MsgProc(XM_CHAR, input_char.dw, char_index);
			}
			else if (m_iInputType == KI_LOWERCASE || m_iInputType == KI_UPPERCASE || m_iInputType == KI_PINYIN)
			{
				if (GetKeyInputFlag(KIF_CORRECTMAP))
				{
					keyMap = keyMapCorrect;
				}
				else
				{
					keyMap = keyMapWrong;
				}
				
				//不对应任何字母,退出
				if (!keyMap[num][0])
				{
					break;
				}

				//printf("CPageInputTray::MsgProc \n");
				//输入的字符不是空格是, 打开候选词列表
				if (num != 0 && pOwner->GetType() == IT_EDIT
					&& m_iInputType == KI_PINYIN && !pPageCharList->IsOpened())
				{
					pPageCharList->SetOwner(pOwner);
					pPageCharList->Open();

					OnInputTypeCheck();

					pOwner = GetOwner();//焦点可能已转移,需要重新获取
					if (!pOwner)
					{
						return FALSE;
					}
				}

				static int timecheck=0;  //cw_panel
				static int timelast=0;
				timecheck=time(NULL);
				if ((timecheck-timelast)>=2)
					char_index=0;
				timelast=timecheck;
				
				if (char_index == 0 || last_num != num)
				{
					char_index = 1;
					input_char.ch[0] = keyMap[num][char_index];
					if (m_iInputType == KI_UPPERCASE && num != 0)//空格不需要转换
					{
						input_char.ch[0] += 'A' - 'a';
					}
				}
				else
				{
					char_index++;
					if (char_index > keyMap[num][0])
					{
						char_index = 1;
					}
					input_char.ch[0] = '\b';//退格字符
					input_char.ch[1] = keyMap[num][char_index];
					if (m_iInputType == KI_UPPERCASE)
					{
						input_char.ch[1] += 'A' - 'a';
					}
				}

				last_num = num;

				if (char_index == 1 && keyMap[num][0] == 1)//如果只对应一个字符,开始下一个字符的输入
				{
					char_index = 0;
				}
				else
				{
					m_CharTimer.Start(this, (VD_TIMERPROC)&CPageInputTray::OnTimeOver, 1000, 0, TIMER_CHAR);
				}
				//add by xie 2009-4-7
				if(input_char.ch[0] == 0x20)
				{
					//屏蔽空格字符					
					break;
				}				
				//end
				pOwner->MsgProc(XM_CHAR, input_char.dw, char_index);
			}
			else if(m_iInputType == KI_MARK)
			{
				num = (num + 9) % 10; //符号面板上的数字是从1开始的
				input_char.ch[0] = numpad_signs[IPT_MARK][num][0];
				pOwner->MsgProc(XM_CHAR, input_char.dw, 0);
			}
			break;
			
		case KEY_FUNC://case KEY_SHIFT://csp modify 20130326
			if(GetKeyInputFlag(KIF_SHIFT))//如果有shift键，处理为Fn键, 一个一个删除字符
			{
				//printf("HAS SHIFT KEY\n");
				char_index = 0;//字符输入状态复位
				m_CharTimer.Stop();
				input_char.ch[0] = '\b';
				pOwner->MsgProc(XM_CHAR, input_char.dw, char_index);
			}
			else //没有
			{
				//printf("NO SHIFT KEY\n");
				if(!m_DeleteTimer.IsStarted())
				{
					m_DeleteTimer.Start(this, (VD_TIMERPROC)&CPageInputTray::OnTimeOver, 1500, 0, TIMER_DELETE);
				}
			}
			break;
			
		case KEY_SPLIT:
		case KEY_SHIFT://case KEY_FUNC://csp modify 20130326
			OnInputTypeShift();
			break;
			
		default:
			return FALSE;
		}
		break;

	case XM_KEYUP:
		key = wpa;
		if(key == KEY_FUNC && !GetKeyInputFlag(KIF_SHIFT)) //没有弹起键时, Fn键弹起切换输入法
		{
			m_DeleteTimer.Stop();
			if(!m_DeleteTimer.IsCalled())
			{
				OnInputTypeShift();
			}
		}
		break;
		
	case XM_TIMER:
		if(wpa == TIMER_CHAR)
		{
			char_index = 0;//字符输入状态复位
			pOwner->MsgProc(XM_CHAR, input_char.dw, char_index);//发送空字符消息
		}
		else
		{
			char_index = 0;//字符输入状态复位
			input_char.ch[0] = '\f';//作为全部删除的标志字符
			pOwner->MsgProc(XM_CHAR, input_char.dw, char_index);//发送删除消息
		}
		break;
		
	case XM_CHAR: //从候选词传来的汉字字符
		input_char.dw = wpa;
		char_index = 0;//字符输入状态复位
		//printf("wpa = [%c][0x%02x]\n", wpa, wpa);
/*		//add by xie 2009-4-7
		if(wpa == 0x20)
		{
			//屏蔽空格字符			
			break;
		}		
		//end
*/		
		if (wpa) //字符有效时才转发
		{
			ret = pOwner->MsgProc(msg, wpa, lpa);//转发汉字字符
			if(!ret && input_char.ch[0] == '\b' && pOwner->GetParent() == pPageCharList) // 表示拼音退格已经到底
			{
				pOwner = pPageCharList->GetOwner();
				if(pOwner) // 将退格消息发给实际的编辑框
				{
					ret = pOwner->MsgProc(msg, wpa, lpa); // 将退格消息发给实际的编辑框
				}
			}
		}
		break;

	default:
		return CPage::MsgProc(msg, wpa, lpa);
	}
	return TRUE;
}

void CPageInputTray::Draw()
{
	CPage::Draw();
}

void CPageInputTray::OnInputTypeShift()
{
	//printf("%s\n", __func__);
	CItem *pOwner = GetOwner();

	if(!pOwner)
	{
		return;
	}
	
	KEY_INPUT_TYPE new_type = m_iInputType;
	ITEM_TYPE item_type = pOwner->GetType();
	uint item_style = pOwner->GetStyle();
	
	//非编辑框和数字框不用处理输入法
	if(item_type != IT_EDIT && (item_type != IT_NUMBERBOX || GetKeyInputFlag(KIF_NUMERIC)))
	{
		return;
	}

	switch(m_iInputType) {
	case KI_NUMERIC:
		if (item_type == IT_NUMBERBOX)
		{
			new_type = KI_CONTROL;
		}
		else
		{
			new_type = KI_UPPERCASE;
		}
		break;

	case KI_UPPERCASE:
		if (item_type == IT_NUMBERBOX)
		{
			new_type = KI_CONTROL;
		}
		else
		{
			new_type = KI_LOWERCASE;
		}
		break;

	case KI_LOWERCASE:
		if (item_type == IT_NUMBERBOX)
		{
			new_type = KI_CONTROL;
		}
		else
		{
			new_type = KI_MARK;
		}
		break;

	case KI_MARK: //只有简体中文的非密码框支持输入法
		if(!GetKeyInputFlag(KIF_NEWATM) && !GetKeyInputFlag(KIF_SHIFT))
		{
			new_type = KI_CONTROL;
		}
		else
		{
			new_type = KI_PINYIN;
		}
		
		break;
		
	case KI_PINYIN:
		if(!GetKeyInputFlag(KIF_NEWATM) && !GetKeyInputFlag(KIF_SHIFT)
			|| (item_type == IT_NUMBERBOX))
		{
			new_type = KI_CONTROL;
		}
		else
		{
			new_type = KI_NUMERIC;
		}

		break;
		
	case KI_CONTROL:
		new_type = KI_NUMERIC;
		break;
		
	default:
		break;
	}
	
	if(item_type == IT_EDIT)
	{
 		if( (item_style & edit_KI_NUMERIC) || ((item_style & edit_KI_NOCH) && (new_type == KI_PINYIN)) )
		{
			new_type = KI_NUMERIC;
		}
	}
	
	if (new_type != m_iInputType)
	{
		pOwner->MsgProc(XM_INPUTTYPE, new_type, 0);//通知输入法切换
	
		m_iInputType = new_type;
		
		OnInputTypeChanged();
		
		if (m_iInputType != KI_PINYIN && pPageCharList->IsOpened())
		{
			pPageCharList->Close(UDM_CANCEL);
			
			//选词条关闭后, 拥有者控件转移, 需要重新通知
			pOwner = GetOwner();
			if (pOwner)
			{
				pOwner->MsgProc(XM_INPUTTYPE, m_iInputType, 1);//通知输入法切换, 并打开面板
			}
		}
	}
}

void CPageInputTray::OnInputTypeCheck()
{
	//printf("%s\n", __func__);
	CItem *pOwner = GetOwner();

	if(!pOwner)
	{
		return;
	}
	
	KEY_INPUT_TYPE new_type = m_iInputType;
	ITEM_TYPE item_type = pOwner->GetType();
	
	//非编辑框和数字框不用处理输入法
	if(item_type != IT_EDIT && (item_type != IT_NUMBERBOX || GetKeyInputFlag(KIF_NUMERIC)))
	{
		return;
	}
	
	switch(m_iInputType) {
	case KI_CONTROL:
		if(GetKeyInputFlag(KIF_NUMERIC))
		{
			new_type = KI_NUMERIC;
		}
		break;
		
	case KI_PINYIN:  //只有简体中文的非密码框支持输入法
		//if(1) //暂时屏蔽中文输入法
// 		if(!GetKeyInputFlag(KIF_NUMERIC) || CConfigLocation::getLatest().iLanguage != CHINESE_S || (((CEdit*)pOwner)->GetStyle() & editPassword))
// 		{
// 			//new_type = KI_NUMERIC;    //delete by xie   支持中文输入
// 		}
		break;
		
	default:
		if(item_type == IT_NUMBERBOX && new_type != KI_NUMERIC)
		{
			new_type = KI_NUMERIC;
		}
		break;
	}

	pOwner->MsgProc(XM_INPUTTYPE, new_type, 0);//通知输入法切换
	
	if (new_type != m_iInputType)
	{
		
		m_iInputType = new_type;
		
		OnInputTypeChanged();
	}
}

void CPageInputTray::OnInputTypeChanged()
{
	CItem *pOwner = GetOwner();
	uint item_style = pOwner->GetStyle();
	
	switch (m_iInputType)
	{
	case KI_NUMERIC:
		m_pButton->SetBitmap(bmp_input_numeric, bmp_input_numeric, bmp_input_numeric, bmp_input_numeric, FALSE);
		break;
	case KI_UPPERCASE:
		m_pButton->SetBitmap(bmp_input_uppercase, bmp_input_uppercase, bmp_input_uppercase, bmp_input_uppercase, FALSE);
		break;
	case KI_LOWERCASE:
		m_pButton->SetBitmap(bmp_input_lowercase, bmp_input_lowercase, bmp_input_lowercase, bmp_input_lowercase, FALSE);
		break;
	case KI_MARK:
		m_pButton->SetBitmap(bmp_input_mark, bmp_input_mark, bmp_input_mark, bmp_input_mark, FALSE);
		break;
	case KI_PINYIN:
		m_pButton->SetBitmap(bmp_input_pinyin, bmp_input_pinyin, bmp_input_pinyin, bmp_input_pinyin, FALSE);
		break;
	case KI_CONTROL:
		m_pButton->SetBitmap(bmp_input_control, bmp_input_control, bmp_input_control, bmp_input_control, FALSE);
		break;
	default:
		break;
	}
	
	char_index = 0;//字符输入状态复位
	m_CharTimer.Stop();
}

void CPageInputTray::OnTimeOver(uint param)
{
	VD_SendMessage(XM_TIMER, param);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////        CPageCharList
CPageCharList::CPageCharList(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent)
	:CPage(pRect, psz, pParent, pageNoExpire)
{
	int i;

	CRect m_rect;
	m_rect.SetRect(2, 2, 74, 26);
	m_pEditPinyin = CreateEdit(m_rect, this, 6, styleNoBorder, (CTRLPROC)&CPageCharList::OnInputChanged);
	for (i = 0; i < CHAR_LIST_NUM; i++)
	{
		m_rect.SetRect(76 + 24 * i, 2, 76 + 24 * (i + 1), 26);
		m_pStaticList[i] = CreateStatic(m_rect, this, (VD_PCSTR)NULL, FALSE);

		m_pStaticList[i]->SetBkColor(VD_GetSysColor(COLOR_POPUP));

	}
	m_iSelected = 0;
	m_iPageButton = PAGENONE;

	m_pEditPinyin->SetBkColor(VD_GetSysColor(COLOR_POPUP));
	m_pStaticList[0]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));

	m_nChars = CHAR_LIST_NUM;

	m_pEditPinyin->SetAutoFocus(TRUE);

	m_pSpellPad[m_screen] = this;

	//输入法初始化
	m_IMM.Open(FONT_BIN"/gbkpy.mb");
}

CPageCharList::~CPageCharList()
{
	m_IMM.Close();
}

VD_BOOL CPageCharList::UpdateData(UDM mode)
{
	CItem *pOwner;
	VD_RECT owner_rect, list_rect;

	pOwner = GetOwner();
	if(!pOwner)
	{
		return TRUE;
	}

	switch (mode) {
	case UDM_OPEN: // 打开时设置区域并清空拼音
		GetRect(&list_rect);
		pOwner->GetRect(&owner_rect);
		::OffsetRect(&list_rect, owner_rect.left - list_rect.left, owner_rect.bottom - list_rect.top);
		SetRect(&list_rect, FALSE);
		m_pEditPinyin->Empty(); // 清空拼音
		OnInputChanged(); // 更新候选字列表
		break;

	case UDM_OPENED:
		m_pEditPinyin->ShowDropDown(TRUE); //打开字母输入面板
		break;

	case UDM_OK:
		EnterChar(); // 确认字符
		break;

	default:
		break;
	}
	return TRUE;
}

void CPageCharList::EnterChar()
{
	INPUT_CHAR input_char;
	VD_PCSTR psz;
	CItem *pOwner;
	
	pOwner = GetOwner();
	if (pOwner)
	{
		psz = m_pStaticList[m_iSelected % CHAR_LIST_NUM]->GetText();
		if (psz)
		{
			input_char.dw = 0;
			strcpy(&input_char.ch[0], psz);
			pOwner->MsgProc(XM_CHAR, input_char.dw, 0);
		}
	}
}

VD_BOOL CPageCharList::WordExist(VD_PCSTR psz, int fontStyle)
{
	int nchar;
	ushort code;
	uchar raster[128];
	int cw;

	code = g_Locales.GetCharCode(psz, &nchar);

	// 非中文不需要判断
	if (nchar != 3)
	{
		return TRUE;
	}
	cw = g_Locales.GetCharRaster(code, raster, fontStyle == FS_SMALL ? FONTSIZE_SMALL : FONTSIZE_NORMAL);
	VD_SIZE fontSize;
	uchar *p = raster;

	(void)g_Locales.GetFontSize(&fontSize);
	for (int i = 0; i < fontSize.h; i++)
	{
		for (int j = 0; j < cw; j++)
		{
			if ( * (p + j / 8) & (128 >> (j % 8)))
			{
				return TRUE;
			}
		}
		p += fontSize.w / 8;
	}
	return FALSE;
}

VD_BOOL CPageCharList::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	int char_temp, page_temp;
	int px, py;
	VD_BOOL ret;
	VD_RECT rect;

	char_temp = m_iSelected;
	page_temp = m_iPageButton;
	switch (msg){
	case XM_KEYDOWN:
		key = wpa;
		switch (key){
		case KEY_LEFT:
			if (char_temp > 0)
			{
				char_temp--;
			}
			break;
			
		case KEY_RIGHT:
			if (char_temp < m_nChars - 1)
			{
				char_temp++;
			}
			break;
		case KEY_RET: //cw_panel
			{
				EnterChar(); // 输入字符
				m_pEditPinyin->Empty(); // 清空拼音
				OnInputChanged(); // 更新候选字列表
			}
			break;
pageup_proc:
		case KEY_PREV:
			if (char_temp >= CHAR_LIST_NUM)
			{
				char_temp = ((char_temp / CHAR_LIST_NUM) - 1) * CHAR_LIST_NUM;
			}
			break;

pagedown_proc:
		case KEY_NEXT:
			if (char_temp <= (m_nChars - 1) / CHAR_LIST_NUM * CHAR_LIST_NUM - 1)
			{
				char_temp = ((char_temp / CHAR_LIST_NUM) + 1) * CHAR_LIST_NUM;
			}
			break;
			
		case KEY_0://对0键和10键不予处理,否则会输入空格
		case KEY_10:
			return FALSE;

		default:
			return CPage::MsgProc(msg, wpa, lpa);
		}
		break;
		
	case XM_MOUSEMOVE:
	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		page_temp = GetPageAt(px, py); // 先判断是否翻页, 因为翻页会用到选字的参数char_temp
		if(page_temp != m_iPageButton)
		{
			m_iPageButton = page_temp;
			UpdatePageButton();
		}
		if(m_iPageButton != PAGENONE)
		{
			if(msg == XM_LBUTTONDOWN) // 鼠标翻页
			{
				if(m_iPageButton == PAGEUP)
				{
					goto pageup_proc;
				}
				else
				{
					goto pagedown_proc;
				}
			}
			break;
		}
		char_temp = GetCharAt(px, py); // 先判断是否选字
		if(char_temp >= 0 && char_temp < m_nChars)
		{
			m_iSelected = char_temp;
			if(msg == XM_LBUTTONDOWN) // 鼠标选字
			{
				EnterChar(); // 输入字符
				m_pEditPinyin->Empty(); // 清空拼音
				OnInputChanged(); // 更新候选字列表
			}
			break;
		}

		ret = CPage::MsgProc(msg, wpa, lpa);
		if(!ret && msg == XM_LBUTTONDOWN) // 鼠标点击区域之外时, 关闭选词条
		{
			GetRect(&rect);
			if(!::PtInRect(&rect, px, py))
			{
				ret = Close(UDM_CANCEL);
			}
		}
		return ret;

	default:
		SetFocusItem(m_pEditPinyin); //保证输入焦点一直是m_pEditPinyin
		return CPage::MsgProc(msg, wpa, lpa);
	}

	if (char_temp != m_iSelected)
	{

		m_pStaticList[m_iSelected % CHAR_LIST_NUM]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), TRUE);
		m_pStaticList[char_temp % CHAR_LIST_NUM]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), TRUE);
		if (char_temp / CHAR_LIST_NUM != m_iSelected / CHAR_LIST_NUM)
		{
			m_iSelected = char_temp;
			GenCandidateList();
		}
		m_iSelected = char_temp;
	}
	return TRUE;
}

int CPageCharList::GetCharAt(int x, int y)
{
	int i;

	for(i = 0; i < CHAR_LIST_NUM; i++)
	{
		VD_RECT rect;
		m_pStaticList[i]->GetRect(&rect);
		if(::PtInRect(&rect, x, y))
		{
			return m_iSelected / CHAR_LIST_NUM * CHAR_LIST_NUM + i;
		}
	}

	return -1;
}

int CPageCharList::GetPageAt(int x, int y)
{
	if(x >= m_Rect.right - CTRL_HEIGHT1 / 2 - 6 && x <= m_Rect.right
		&& y >= m_Rect.top && y <= (m_Rect.top + m_Rect.bottom) / 2)
	{
		return PAGEUP;
	}
	else if(x >= m_Rect.right - CTRL_HEIGHT1 / 2 - 6 && x <= m_Rect.right
		&& y >= (m_Rect.top + m_Rect.bottom) / 2  && y <= m_Rect.bottom)
	{
		return PAGEDOWN;
	}
	else
	{
		return PAGENONE;
	}
}

void CPageCharList::Draw()
{
	if (!DrawCheck())
	{
		return;
	}

	m_DC.Lock();

	m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED), VD_PS_SOLID, 2);
	m_DC.SetBrush(VD_GetSysColor(COLOR_POPUP));

	m_DC.SetRgnStyle(RS_NORMAL);
	m_DC.Rectangle(CRect(0, 0, m_Rect.Width(), m_Rect.Height()), 3, 3);

	m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED) ,VD_PS_SOLID, 1);

	m_DC.MoveTo(74, 0);
	m_DC.LineTo(74, m_Rect.Height());
	m_DC.UnLock();

	CPage::Draw();
}

void CPageCharList::OnInputChanged()
{
	char input[8];

	m_pStaticList[m_iSelected % CHAR_LIST_NUM]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), TRUE);
	m_iSelected = 0;
	m_pStaticList[m_iSelected % CHAR_LIST_NUM]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), TRUE);

	m_pEditPinyin->GetText(input, 8);
	printf("%s input: %s\n", __func__, input);
	m_nChars = m_IMM.Filter(input);
	//printf("%s m_nChars: %d\n", __func__, m_nChars);
	GenCandidateList();
}

void CPageCharList::GenCandidateList()
{
	int realCnt = 0;
	int page_start;
	ushort code;
	char buf[8];

	memset(buf, 0 , 8);

	page_start = m_iSelected / CHAR_LIST_NUM * CHAR_LIST_NUM;
	for (int i = page_start; i < page_start + CHAR_LIST_NUM; i++)
	{
		if (i < m_nChars)
		{
			memcpy(&code, m_IMM.GetChar(i), 2);
			code = ((code >> 8) | (code << 8));

			//UCS2 -> UTF8
			buf[0] = (0xe0 | code >> 12);
			buf[1] = (0x80 | ((code >> 6) & 0x3f));
			buf[2] = (0x80 | (code & 0x3f));

			if (WordExist(buf))
			{
				m_pStaticList[realCnt++]->SetText(buf);
				//printf("buf: %s, 0x%2x 0x%2x 0x%2x\n", buf, buf[0], buf[1], buf[2]);
			}
			else
			{
				printf("word is not exist!\n");
			}
		}
		else
		{
			m_pStaticList[i % CHAR_LIST_NUM]->SetText(NULL);
		}
	}

	UpdatePageButton();
}

void CPageCharList::UpdatePageButton()
{
	VD_COLORREF show_color;
	VD_COLORREF hide_color;

	//画翻页提示标志

	hide_color = VD_GetSysColor(COLOR_POPUP);


	m_DC.Lock();
	show_color = VD_GetSysColor((m_iPageButton == PAGEUP) ? COLOR_CTRLTEXTSELECTED : COLOR_CTRLTEXT);
	m_DC.SetBrush((!m_nChars || m_iSelected < CHAR_LIST_NUM)
		? hide_color : show_color);
	m_DC.Trapezoid(m_Rect.Width() - CTRL_HEIGHT1 / 2,
		m_Rect.Height() / 2 - 6,
		6 - CTRL_HEIGHT1 / 2,
		m_Rect.Width() - CTRL_HEIGHT1 / 2,
		m_Rect.Height() / 2 - 2,
		CTRL_HEIGHT1 / 2 - 6);

	show_color = VD_GetSysColor((m_iPageButton == PAGEDOWN) ? COLOR_CTRLTEXTSELECTED : COLOR_CTRLTEXT);
	m_DC.SetBrush((!m_nChars || m_iSelected > (m_nChars - 1) / CHAR_LIST_NUM * CHAR_LIST_NUM - 1)
		? hide_color : show_color);
	m_DC.Trapezoid(m_Rect.Width() - CTRL_HEIGHT1 + 6,
		m_Rect.Height() / 2 + 2,
		CTRL_HEIGHT1 / 2 - 6,
		m_Rect.Width() - 6,
		m_Rect.Height() / 2 + 6,
		6 - CTRL_HEIGHT1 / 2);
	m_DC.UnLock();
}
