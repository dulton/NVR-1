#include "GUI/Pages/PagePlayrateSelect.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"

CPagePlayrateSelect::CPagePlayrateSelect( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/,int type )
:CPageFloat(pRect, pParent)
{
	m_type = type;
	
	CRect listRt;
	listRt.left = 0;
	listRt.top = 0;
	listRt.right = m_Rect.Width();
	listRt.bottom = m_Rect.Height();
	
	pListBox = CreateListBox(listRt,this,NULL,NULL,listboxAutoIndex,(CTRLPROC)&CPagePlayrateSelect::OnEnter,NULL,NULL);
	
	//yg modify 20140817 del >>16X
	#if 0//csp modify 20130429
	char* szFRate[10] = 
	{
		">>64X",
		">>32X",
		">>16X",
		">>8X",
		">>4X",
		">>2X",
		">>1X",
		">>1/2X",
		">>1/4X",
		">>1/8X",
	};
	
	char* szBRate[7] = 
	{
		"<<8X",
		"<<4X",
		"<<2X",
		"<<1X",
		"<<1/2X",
		"<<1/4X",
		"<<1/8X",
	};
	
	int i = 0;
	if(1 == type)
	{
		for(i=0; i<10; i++)
		{
			pListBox->AddString(szFRate[i],"",TRUE);
		}
	}
	else
	{
		for(i=0; i<7; i++)
		{
			pListBox->AddString(szBRate[i],"",TRUE);
		}
	}
	#else
	char* szFRate[7] = 
	{
		">>8X",
		">>4X",
		">>2X",
		">>1X",
		">>1/2X",
		">>1/4X",
		">>1/8X",
	};
	
	char* szBRate[7] = 
	{
		"<<8X",
		"<<4X",
		"<<2X",
		"<<1X",
		"<<1/2X",
		"<<1/4X",
		"<<1/8X",
	};
	
	int i = 0;
	for(i=0; i<7; i++)
	{
		if(1 == type)
		{
			pListBox->AddString(szFRate[i],"",TRUE);
		}
		else
		{
			pListBox->AddString(szBRate[i],"",TRUE);
		}
	}
	#endif
}

CPagePlayrateSelect::~CPagePlayrateSelect()
{
	
}

void CPagePlayrateSelect::OnClick()
{
	printf("OnClick\n");
}

void CPagePlayrateSelect::OnEnter()
{
	//printf("OnEnter\n");
	
	int curSel = pListBox->GetCurSel();
	//printf("cursel = %d\n",curSel);
	
	int playRate = 0;
	
	//yg modify 20140817 del >>16X
	#if 0//csp modify 20130429
	if(1 == m_type)
	{
		switch(curSel-3)
		{
			case -3:
				playRate = 6;
				break;
			case -2:
				playRate = 5;
				break;
			case -1:
				playRate = 4;
				break;
			case 0:
				playRate = 3;
				break;
			case 1:
				playRate = 2;
				break;
			case 2:
				playRate = 1;
				break;
			case 3:
				playRate = 0;
				break;
			case 4:
				playRate = -1;
				break;
			case 5:
				playRate = -2;
				break;
			case 6:
				playRate = -3;
				break;
			default:
				break;
		}
	}
	else
	{
		switch(curSel)
		{
			case 0:
				playRate = 3;
				break;
			case 1:
				playRate = 2;
				break;
			case 2:
				playRate = 1;
				break;
			case 3:
				playRate = 0;
				break;
			case 4:
				playRate = -1;
				break;
			case 5:
				playRate = -2;
				break;
			case 6:
				playRate = -3;
				break;
			default:
				break;
		}
	}
	#else
	switch(curSel)
	{
		case 0:
			playRate = 3;
			break;
		case 1:
			playRate = 2;
			break;
		case 2:
			playRate = 1;
			break;
		case 3:
			playRate = 0;
			break;
		case 4:
			playRate = -1;
			break;
		case 5:
			playRate = -2;
			break;
		case 6:
			playRate = -3;
			break;
		default:
			break;
	}
	#endif
	
	((CPagePlayBackFrameWork*)m_pParent)->SetPlayrate(playRate,m_type);
	
	this->Close();
}

void CPagePlayrateSelect::OnChange()
{
	printf("OnChange\n");
}

