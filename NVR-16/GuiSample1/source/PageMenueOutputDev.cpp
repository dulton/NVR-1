#include "GUI/Pages/PageMenueOutputDev.h"
#include "GUI/Pages/BizData.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

CPageMenueOutputDev::CPageMenueOutputDev( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect/*CRect(0, 0, 480, 300)*/, psz, NULL, pParent/*, pageDialog*/)
{
	//printf("CPageMenueOutputDev-1\n");
	
	SetMargin(0,0,0,0);
	SetRect(CRect(0, 0, 480, 300), FALSE);
	
	//printf("CPageMenueOutputDev-2\n");
	
	CRect rt;
	rt.left = 60;
	rt.top = 80;
	rt.bottom = 104;
	rt.right = rt.left+120;
	pStatic[0] = CreateStatic(rt, this, "&CfgPtn.PlsSelect");
	pStatic[0]->SetTextAlign(VD_TA_RIGHT);
	
	//printf("CPageMenueOutputDev-3\n");
	
	rt.left = rt.right + 20;
	rt.right = 420;
	pCombox = CreateComboBox(rt, this, NULL, NULL, NULL, 0);//csp modify
	pCombox->AddString("&CfgPtn.VGA_LCD");
	pCombox->AddString("&CfgPtn.CVBS_TV");
	pCombox->SetCurSel(0);
	
	//printf("CPageMenueOutputDev-4\n");
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	SBizDvrInfo bizDvrInfo;
	bizTar.emBizParaType = EM_BIZ_DVRINFO;
	ret = BizGetPara(&bizTar, &bizDvrInfo);
	if (EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync)
	{
		if (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput)
		{
			pCombox->SetCurSel(1);
		}
	}
	
	//printf("CPageMenueOutputDev-5\n");
	
	pCombox->SetBkColor(VD_RGB(67,77,87));
	pCombox->Enable(1);
	
	//printf("CPageMenueOutputDev-6\n");
	
	rt.left = 60;
	rt.top = 120;	
	rt.right = 420;
	rt.bottom = 220;
	pStatic[1] = CreateStatic(rt, this, "&CfgPtn.FirstMenuTips");
	
	//printf("CPageMenueOutputDev-7\n");
	
	rt.left = 60;
	rt.top = 226;	
	rt.right = 380;
	rt.bottom = 246;
	pStatic[2] = CreateStatic(rt, this, "&CfgPtn.AutoSkipThisMenu");
	pStatic[2]->SetTextAlign(VD_TA_RIGHT);
	
	//printf("CPageMenueOutputDev-8\n");
	
	rt.left = rt.right+10;
	rt.right = rt.left+30;
	pStatic[3] = CreateStatic(rt, this, "");
	
	//printf("CPageMenueOutputDev-9\n");
	
	rt.top = m_Rect.Height() - 40;
	rt.bottom = rt.top + 25;	
	rt.left = 200;
	rt.right = rt.left + 80;
	pButton[0] = CreateButton(rt,this,"&CfgPtn.OK", (CTRLPROC)&CPageMenueOutputDev::OnClkBtn, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	
	//printf("CPageMenueOutputDev-10\n");
	
	IsClosed = 0;
	
	//printf("file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
}

CPageMenueOutputDev::~CPageMenueOutputDev()
{
	
}

static CPageMenueOutputDev* pagethis = NULL;
static u8 g_IsRunning = 1;
static void* updatetimenow(void* p)
{
	printf("$$$$$$$$$$$$$$$$$$updatetimenow-2 id:%d\n",getpid());
	
	//printf("start updatetimenow...\n");
	
	char tmp[2] = {0};
	u8 i = 60;
	while((i) && g_IsRunning)
	{
		i--;
		sprintf(tmp,"%d",i+1);
		usleep(1000*1000);
		pagethis->pStatic[3]->SetText((VD_PCSTR)tmp);
		
		//csp modify
		if(CDC::GetModify())
		{
			GraphicsSyncVgaToCvbs();
			CDC::SetModify(FALSE);
		}
	}
	if(0 == i)
	{
		pagethis->pStatic[3]->SetText("0");
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		SBizSystemPara bizSysPara;
		int ret = BizGetPara(&bizTar, &bizSysPara);
		if(0 == ret)
		{
			bizSysPara.nShowFirstMenue = 0;
			char tmp[20] = {0};
			GetProductNumber(tmp);
			if ((0 == strcasecmp(tmp, "R9516S"))
    			|| (0 == strcasecmp(tmp, "R9508S"))
    			|| (0 == strcasecmp(tmp, "R9504S")))
    		{
    			bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_800X600;
    		}
    		else
    		{
				bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_1024X768;
			}
			BizSetPara(&bizTar, &bizSysPara);
		}
		
		pagethis->IsClosed = 1;
	}
	else
	{
		pagethis->pStatic[2]->SetText("");
		pagethis->pStatic[3]->SetText("");
	}
	
	//printf("stop updatetimenow...\n");
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

VD_BOOL CPageMenueOutputDev::UpdateData( UDM mode )
{
	if(UDM_OPENED == mode)
	{
		pagethis = this;
		pthread_t pid;
		pthread_create(&pid, NULL, updatetimenow, NULL);
	}
	return TRUE;
}

void CPageMenueOutputDev::OnClkBtn()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(0 == ret)
	{
		if(EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput)
		{
			if(0 == pCombox->GetCurSel())
			{
				char tmp[20] = {0};
				GetProductNumber(tmp);
				if ((0 == strcasecmp(tmp, "R9516S"))
	    			|| (0 == strcasecmp(tmp, "R9508S"))
	    			|| (0 == strcasecmp(tmp, "R9504S")))
	    		{
	    			bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_800X600;
	    		}
	    		else
	    		{
					bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_1024X768;
				}
			}
		}
		else
		{
			if(1 == pCombox->GetCurSel())
			{
				bizSysPara.nOutput = EM_BIZ_OUTPUT_CVBS;
			}
			else if(0 == pCombox->GetCurSel())
			{
				char tmp[20] = {0};
				GetProductNumber(tmp);
				if ((0 == strcasecmp(tmp, "R9516S"))
	    			|| (0 == strcasecmp(tmp, "R9508S"))
	    			|| (0 == strcasecmp(tmp, "R9504S")))
	    		{
	    			bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_800X600;
	    		}
	    		else
	    		{
					bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_1024X768;
				}
			}
		}
		bizSysPara.nShowFirstMenue = 0;
		BizSetPara(&bizTar, &bizSysPara);
	}
	
	//this->Close();
	IsClosed = 1;
}

VD_BOOL CPageMenueOutputDev::MsgProc(uint msg, uint wpa, uint lpa)
{
	int i = 0;

	if(msg == XM_KEYDOWN)
	{
		uchar key = wpa;
		switch(key)
		{
			case KEY_RET:
				{
					CItem * item = GetFocusItem();
					if((item != pCombox) && (item != pButton[0]))
					{
						return FALSE;
					}
				}
			case KEY_LEFT:
			case KEY_RIGHT:
			case KEY_UP:
			case KEY_DOWN:
				break;
			default:
				return FALSE;			
		}
	}	
	else if((msg != XM_LBUTTONUP) && (msg != XM_LBUTTONDOWN)&& (msg != XM_MOUSEMOVE))
	{
		return FALSE;
	}
	g_IsRunning = 0;
	return CPageFrame::MsgProc(msg, wpa, lpa);
}



