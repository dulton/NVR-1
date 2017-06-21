
#include "GUI/Pages/PageMessageBox.h"

static UDM m_IsOk;

CPageMessageBox::CPageMessageBox(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent, int bModal)
:CPageFrame(pRect/*CRect(0, 0, 480, 300)*/, psz, NULL, pParent, pageDialog)
{
	SetMargin(0,0,0,0);
	
#if 1
	SetRect(CRect(0, 0, 480, 300), FALSE);
	
	m_bModal = bModal;
	m_IsOk = UDM_EMPTY;
	
	CRect rt;
	
	if (icon)
	{
		rt.left = 20;
		rt.top = 60;
		rt.right = rt.left  + icon->width;
		rt.bottom = rt.top + icon->height;
		pStaticIcon = CreateStatic(rt, this, icon);
		
		rt.left = rt.right;
		rt.top = 80;
	}
	else
	{
		rt.left = 20;
		rt.top = 80;
	}
	rt.left += 15;
	rt.right = m_Rect.Width() - 20;
	rt.bottom = m_Rect.Height() - 40;
	
	pStaticText = CreateStatic(rt,this, "");
	
	rt.left = m_Rect.Width() - 200;
	rt.right = rt.left + 80;
	rt.top = m_Rect.Height() - 40;
	rt.bottom = rt.top + 25;
	pOK = CreateButton(rt,this,"&CfgPtn.OK", ID_OK);
	
	rt.left = rt.right+20;
	rt.right = rt.left + 80;
	pCancel = CreateButton(rt,this, "&CfgPtn.Cancel", ID_CANCEL);
	
	rt.left = 200;
	rt.right = rt.left + 80;
	pOK1 = CreateButton(rt,this,"&CfgPtn.OK", ID_OK);
	
#else
	
	SetRect(CRect(0, 0, /*320, 240*/320,240), FALSE);
	
	m_bModal = bModal;
	
	CRect rt;
	
	if (icon)
	{
		rt.left = 20;
		rt.top = 50;
		rt.right = rt.left  + icon->width;
		rt.bottom = rt.top + icon->height;
		pStaticIcon = CreateStatic(rt, this, icon);
		
		rt.left = rt.right;
		rt.top = 60;
	}
	else
	{
		rt.left = 20;
		rt.top = 60;
	}
	rt.left += 15;
	rt.right = m_Rect.Width() - 20;
	rt.bottom = m_Rect.Height() - 40;
	
	pStaticText = CreateStatic(rt,this,"");
	
	rt.left = m_Rect.Width()/2 - 80;
	rt.right = rt.left + 80;
	rt.top = m_Rect.Height() - 40;
	rt.bottom = rt.top + 25;
	pOK = CreateButton(rt,this,"&CfgPtn.OK", ID_OK);
	
	rt.left = rt.right+20;
	rt.right = rt.left + 80;
	pCancel = CreateButton(rt,this, "&CfgPtn.Cancel", ID_CANCEL);
	
	rt.left = 130;
	rt.right = rt.left + 80;
	pOK1 = CreateButton(rt,this,"&CfgPtn.OK", ID_OK);
#endif
	
	pOK->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pCancel->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pOK1->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
}

CPageMessageBox::~CPageMessageBox()
{
	
}

VD_BOOL CPageMessageBox::UpdateData(UDM mode)
{
	VD_RECT rect;
	if(mode == UDM_OPEN)
	{
		if (type & MB_OKCANCEL)
		{
			pOK1->Show(FALSE);
			pOK->Show(TRUE);
			pCancel->Show(TRUE);
		}
		else if (type & MB_OK)
		{
			pOK1->Show(TRUE);
			pOK->Show(FALSE);
			pCancel->Show(FALSE);
		}
		else if(type & MB_OK)
		{
			pOK1->Show(FALSE);
			pOK->Show(TRUE);
			pCancel->Show(FALSE);
		}
		else if(type & MB_NULL)
		{
			pOK->Show(FALSE);
			pCancel->Show(FALSE);
			pOK1->Show(FALSE);
		}
	}
	return TRUE;
}

UDM	GetRunStatue()
{
	return m_IsOk;
}

UDM CPageMessageBox::Run(VD_PCSTR psz, int vtype)
{
	pStaticText->SetText(psz);
	type = vtype;
	if ((type & MB_OKCANCEL))
	{
		if ((type & MB_DEFBUTTON2))
		{
			SetFocusItem(pCancel);
		} 
		else
		{
			SetFocusItem(pOK);
		}
	}
	else
	{
		SetFocusItem(pOK1);
	}
	m_ExitState = UDM_EMPTY;//cw_panel
	return CPage::PopupRun();
}

void CPageMessageBox::SetType(int vtype)
{
	type = vtype;
}

UDM MessageBox(VD_PCSTR pText, VD_PCSTR pCaption, int nType ,int bModal)
{
	VD_BITMAP* pBmp = NULL;
	
	if ((nType & MB_ICONERROR))
	{
		pBmp = VD_LoadBitmap(DATA_DIR"/temp/error.bmp");
	}
	else if ((nType & MB_ICONQUESTION))
	{
		pBmp = VD_LoadBitmap(DATA_DIR"/temp/question.bmp");
	}
	else if ((nType & MB_ICONWARNING))
	{
		pBmp = VD_LoadBitmap(DATA_DIR"/temp/warning.bmp");
	}
	else if ((nType & MB_ICONINFORMATION))
	{
		pBmp = VD_LoadBitmap(DATA_DIR"/temp/tip_info.bmp");
	}
	
	CPageMessageBox box(NULL, pCaption, pBmp, NULL, bModal);
	
	box.SetType(nType);
	
	UDM ret = box.Run(pText, nType);
	
	m_IsOk = ret;
	
	return ret;
}



VD_BOOL CPageMessageBox::MsgProc( uint msg, uint wpa, uint lpa )
{
    u8 lock_flag = 0;//cw_lock
    
    GetSystemLockStatus(&lock_flag);
	
	//printf("CPageMessageBox::MsgProc lock_flag=%d\n",lock_flag);
	
    if(lock_flag)
    {
    	return FALSE;
    }
	
	if(m_bModal)
	{
		while(1)
		{
			sleep(1);
		}
	}
	else
	{
		switch(msg)
		{
            case XM_RBUTTONDOWN:
            case XM_RBUTTONDBLCLK:
            {
                return TRUE;
            } break;
	    case XM_KEYUP:
	    case XM_KEYDOWN://cw_backup
	    {
		if (wpa==KEY_RET)
		{
		    CItem* temp = GetFocusItem();
			if ((temp!=pOK)&&(temp!=pOK1)&&(temp!=pCancel))
			{
				return FALSE;
			}
		}
	    }break;
            default:
                break;
     	}
	}
    
    return CPage::MsgProc(msg, wpa, lpa );
}

