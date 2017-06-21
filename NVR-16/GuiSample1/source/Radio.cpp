
#include "GUI/Ctrls/Page.h"

///////////////////////////////////////////////////////////////////
////////////        CRadio
CRadio* CreateRadio(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz /* = NULL */, CTRLPROC onChanged /* = NULL */,uint style /* = 0 */)
{
	return new CRadio(pRect, pParent, psz, onChanged,style);
}
CRadio* CreateRadio(VD_PCRECT pRect, CPage * pParent, CRadio* pHead, VD_PCSTR psz,uint style)
{
	return new CRadio(pRect, pParent,pHead,psz,style);
}

VD_SIZE& CRadio::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH,CTRL_HEIGHT1};
	return size;
}
CRadio::CRadio(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz /* = NULL */, CTRLPROC onChanged /* = NULL */,uint style)
	:CItem(pRect, pParent, IT_RADIO,style|styleAutoFocus|styleEditable)
{
	if (psz)
	{
		string = GetParsedString(psz);
		//string = psz;
	}
	m_iIndex = 0;
	m_iCurSel = -1;
	m_pHead = this;
	m_Group.push_back(this);
	m_onChanged = onChanged;
}

CRadio::CRadio(VD_PCRECT pRect, CPage * pParent, CRadio* pHead, VD_PCSTR psz /* = NULL */,uint style)
	:CItem(pRect, pParent, IT_RADIO,style|styleAutoFocus|styleEditable)
{
	if (psz)
	{	
		string = GetParsedString(psz);
		//string = psz;
	}

	if (!pHead || !pHead->m_Group.size())
	{
		printf("CRadio::CRadio Invalid Head Radio!\n");
		return;
	}

	m_pHead = pHead;
	m_iIndex = m_pHead->m_Group.size();
	m_pHead->m_Group.push_back(this);
	m_onChanged = NULL;
}

CRadio::~CRadio()
{
	PRADIOGROUP::iterator  pi;
	if (m_iCurSel == m_iIndex)
	{
		m_iCurSel = 0;
	}
	if (m_Group.size())//删除的是头项
	{
		if (m_Group.size() == 1)
		{
			return;
		}
		m_Group.erase(m_Group.begin());
		for (pi = m_pHead->m_Group.begin(); pi != m_pHead->m_Group.end(); pi++)
		{
			(*pi)->m_iIndex--;
			(*pi)->m_pHead = m_Group[0];
		}
		m_Group[0]->m_Group = m_Group; //转移组信息
	}
	else //删除的是一般项
	{
		m_pHead->m_Group.erase(m_Group.begin() + m_iIndex);
		for (pi = m_pHead->m_Group.begin() + m_iIndex; pi != m_pHead->m_Group.end(); pi++)
		{
			(*pi)->m_iIndex--;
		}
	}
}

void CRadio::Select(VD_BOOL flag)
{
	int i;

	for(i = 0; i < (int)m_pHead->m_Group.size(); i++)
	{
		if (m_pHead->m_Group[i] != this)
		{
			m_pHead->m_Group[i]->CItem::Select(flag); //调用基类的函数, 否则乱死循环
		}
	}

	CItem::Select(flag);
}

void CRadio::Draw()
{
	if (!DrawCheck())
	{
		return;
	}

	//画背景
	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	//贴图
	if(m_dwStyle&styleUseBmp)
	{
		if (m_pHead->m_iCurSel == m_iIndex)
		{
			DrawCtrlRadio(ctrlStatePushed);
		}
		else
		{
			DrawCtrlRadio(ctrlStateNormal);
		}
		if(!GetFlag(IF_ENABLED))
		{
			DrawCtrlRadio(ctrlstateDisabled);
		}
	
		return;
	}

////////////////////////////////////////////////
	if (GetFlag(IF_SELECTED))
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
	}
	else
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
	}
	if(!GetFlag(IF_ENABLED))
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}

	if (m_pHead->m_iCurSel == m_iIndex)
	{
		m_DC.SetRgnStyle(RS_NORMAL);
	}
	else
	{
		m_DC.SetRgnStyle(RS_HOLLOW);
	}


	m_DC.Rectangle(CRect((m_Rect.Width() - 16) / 2, (m_Rect.Height() - 16) / 2, (m_Rect.Width() + 16) / 2, (m_Rect.Height() + 16) / 2), 5, 5);
	m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	m_DC.VD_TextOut(CRect(20, 0, m_Rect.Width(), m_Rect.Height()), string.c_str());
}

VD_BOOL CRadio::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	int newsel = m_pHead->m_iCurSel;
	int num = m_pHead->m_Group.size();

	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_UP:

			if (!num)
			{
				break;
			}
//loop1:
			if (newsel>0)
			{
				newsel--;
			}
			else
			{
				newsel = num-1;
			}
//			if (!m_pHead->m_Group[newsel]->valid)
//			{
//				goto loop1;
//			}
			break;


		case KEY_DOWN:

			if (!num)
			{
				break;
			}
//loop2:
			if (newsel<num-1)
			{
				newsel++;
			}
			else
			{
				newsel = 0;
			}
//			if (!m_pHead->m_Group[newsel]->valid)
//			{
//				goto loop2;
//			}
			break;

		default:
			return FALSE;
		}
		break;

	case XM_LBUTTONDOWN:
		newsel = m_iIndex;
		break;

	default:
		return FALSE;
	}

	if (newsel != m_pHead->m_iCurSel)
	{
		SetCurSel(newsel);
		m_pHead->ItemProc(m_pHead->m_onChanged);//调用头项的回调函数, 需要在SetCurSel之后调用
	}
	return TRUE;
}

int CRadio::SetCurSel(int sel)
{
	if (sel == m_pHead->m_iCurSel || sel >= (int)m_pHead->m_Group.size())
	{
		return -1;
	}

	int temp;

	temp = m_pHead->m_iCurSel;
	m_pHead->m_iCurSel = sel;

	//反选旧的选中项
	if (temp >= 0)
	{
		m_pHead->m_Group[temp]->Draw();
	}
	//选择新的选中项
	if (sel >= 0)
	{
		m_pHead->m_Group[sel]->Draw();
	}

	return sel;
}

int CRadio::GetCurSel()
{
	return m_pHead->m_iCurSel;
}

void CRadio::Switch()
{
	MsgProc(XM_KEYDOWN, KEY_DOWN, 0);
}

VD_BOOL CRadio::IsHead()
{
	return (m_Group.size() != 0);
}

VD_BOOL CRadio::SetBitmap(VD_BITMAP* pBmpNormal /* = NULL */, VD_BITMAP* pBmpSelect /* = NULL */, VD_BITMAP* pBmpPush /* = NULL */, VD_BITMAP* pBmpDisable /* = NULL */)
{
	if (pBmpNormal && pBmpSelect && pBmpPush)
	{
		m_pRadioImageNormal = pBmpNormal;
		m_pRadioImageSelect = pBmpSelect;
		m_pRadioImagePush = pBmpPush;
		m_pRadioImageDisable = pBmpDisable;
		m_dwStyle |= styleUseBmp;
		return TRUE;
	}
	return FALSE;
}

void CRadio::DrawCtrlRadio(CTRL_STATE state)
{
	switch(state)
	{
		case ctrlStateNormal:
			m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pRadioImageNormal);
			break;
		case ctrlStateSelect:
			m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pRadioImageSelect);
			break;
		case ctrlstateDisabled:
			m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pRadioImageDisable);
			break;
		case ctrlStatePushed:
			m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pRadioImagePush);
			break;
		
		default:
			break;
	}
}
