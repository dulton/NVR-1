#include "GUI/Pages/PageGuide.h"
#include "GUI/Pages/PageMacAddress.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageInfoFrameWork.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

char szInfo0[MAX_GUIDE_INFO][64] = {
	"&CfgPtn.SystemDetects",
	"&CfgPtn.FunCanNotBeUsed",
	"&CfgPtn.CanNotOpenTheVideo",
	"&CfgPtn.CanNotPlayBack",
	"&CfgPtn.CanNotLog",
	"",//"&CfgPtn.CanNotSensorAlarm",
	"",
	"",
	"",
	"",
	"",
	"",
};

char szInfo1[MAX_GUIDE_INFO][64] = {
	"&CfgPtn.SystemDetects",
	"&CfgPtn.FollowForMatDisk",
	"&CfgPtn.ForMatDiskStep",
	"&CfgPtn.SelectForMatDisk",
	"&CfgPtn.PleaseWaiting",
	"",//"&CfgPtn.CanNotSensorAlarm",
	"",
	"",
	"",
	"",
	"",
	"",
};

CPageGuide::CPageGuide( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	printf("CPageGuide:CPageGuide start\n");
	
	SetMargin(0,0,0,0);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pFrame = CreateTableBox(CRect(17, 60, m_Rect.Width()-17, m_Rect.Height()-45), this, 1, 1);
	pFrame->SetFrameColor(VD_RGB(56,108,148));
	
	CRect tableRT(22, 65, m_Rect.Width()-22, m_Rect.Height()-50);
	pTab = CreateTableBox(tableRT, this, 1, MAX_GUIDE_INFO);
	
	int i=0;
	for(i=0; i<MAX_GUIDE_INFO; i++)
	{
		CRect rt;
		pTab->GetTableRect(0, i, &rt);
		pTitle[i] = CreateStatic(CRect(rt.left+tableRT.left+3, rt.top+tableRT.top, rt.right+tableRT.left, rt.bottom+tableRT.top+2), this, "");
	}
	
	char* btntext[3] = {
		"&CfgPtn.Prev",
		"&CfgPtn.Complete",
		"&CfgPtn.Exit",
	};
    
    int top = 380;
	pButton[0] = CreateButton(CRect(37, top, 109, top+24),this, btntext[0], 
                            (CTRLPROC)&CPageGuide::OnClkBtn, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
    
    pButton[1] = CreateButton(CRect(121, top, 193, top+24),this, btntext[1], 
                            (CTRLPROC)&CPageGuide::OnClkBtn, NULL, buttonNormalBmp);
	pButton[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
    
    pButton[2] = CreateButton(CRect(567, top, 625, top+24),this, btntext[2], 
                            (CTRLPROC)&CPageGuide::OnClkBtn, NULL, buttonNormalBmp);
	pButton[2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	//pButton->Show(FALSE, FALSE);
	
    m_count0 = 0;
    m_count1 = 0;
}

CPageGuide::~CPageGuide()
{
	
}

void CPageGuide::OnClkBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 3; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
			case 0:
			{
			    CPage** page = GetPage();
			    if(page && page[EM_PAGE_DISKMGR])
			    {
			        //this->Close();
			        page[EM_PAGE_DISKMGR]->UpdateData(UDM_GUIDE);
			        if (page[EM_PAGE_DISKMGR]->Open())
			        {
			            this->Close();
			        }
			        //page[EM_PAGE_DISKMGR]->UpdateData(UDM_EMPTY);
			    }
			} break;
			case 1:
			case 2:
			{
				this->Close();
			} break;			
			default:
				break;			
		}		
	}	
}

void CPageGuide::SetValue(int row, const char* szText)
{
	if( row < 0 || row >= MAX_GUIDE_INFO)
	{
		return;
	}
	
	pTitle[row]->SetText(szText);	
}

VD_BOOL CPageGuide::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
        m_count0 = 1;
        m_count1 = 1;
    	
        //检测产品是否支持传感器,0表示不支持
        u8 SenSport = 0;
        SenSport = GetSenSorNum();
        #if 0
        if(!SenSport)
        {
            char tmp[64] = "&CfgPtn.CanNotSensorAlarm";
			
            for(int i = 0; i < MAX_GUIDE_INFO; i++)
            {
                if(szInfo1[i][0] == '\0')
                {
                    strncpy(szInfo0[i], tmp, strlen(tmp)+1);
                    strncpy(szInfo1[i], tmp, strlen(tmp)+1);
                    break;
                }
            }
        }
        #endif
        
    	//检测硬盘
    	u8 bDiskIsExist = 0;
    	bDiskIsExist = GetHardDiskNum();
        if(!bDiskIsExist)
	    {
            for(int i = 0; i < MAX_GUIDE_INFO; i++)
            {
                switch(i)
                {
                    case 0:
                    {
                        char tmp[256] = {0};
                        sprintf(tmp, "%s %s.", GetParsedString(szInfo0[i]), GetParsedString("&CfgPtn.NoDisk"));
                        SetValue(i, tmp);
                    } break;
                    case 1:
                    {
                        SetValue(i, GetParsedString(szInfo0[i]));
                    } break;
                    default:
                    {
                        char tmp[256] = {0};
                        
                        if(szInfo0[i][0] != '\0')
                        {
                            sprintf(tmp, "%d.%s", m_count0++, GetParsedString(szInfo0[i]));
                            SetValue(i, tmp);
                        }
						//csp modify 20130320
                        //else if((szInfo1[i][0] == '\0') && !SenSport)
                        else if((szInfo0[i][0] == '\0') && !SenSport)
                        {
                            sprintf(tmp, "%d.%s", m_count0++, GetParsedString("&CfgPtn.CanNotSensorAlarm"));
                            SetValue(i, tmp);
                            
                            return TRUE; //此处直接用return是有问题的
                        }
                    } break;
                }
            }
        }
        else
        {
            for(int i = 0; i < MAX_GUIDE_INFO; i++)
            {
            	//printf("CPageGuide::UpdateData - step%d\n",i);
				//fflush(stdout);
				
                switch(i)
                {
                    case 0:
                    {
                        char tmp[256] = {0};
                        sprintf(tmp, "%s %d %s.", GetParsedString(szInfo1[i]), bDiskIsExist, GetParsedString("&CfgPtn.Disks"));
                        SetValue(i, tmp);
                    } break;
                    case 1:
                    {
                        SetValue(i, GetParsedString(szInfo1[i]));
                    } break;
                    default:
                    {
                        if(szInfo1[i][0] != '\0')
                        {
                        	//printf("i=%d,m_count0=%d,GetParsedString=%s,StringLen=%d\n",i,m_count0,GetParsedString(szInfo1[i]),strlen(GetParsedString(szInfo1[i])));
							//fflush(stdout);
							
							char tmp[256] = {0};
                            sprintf(tmp, "%d.%s", m_count0++, GetParsedString(szInfo1[i]));
                            SetValue(i, tmp);
                        }
                        else if((szInfo1[i][0] == '\0') && !SenSport)
                        {
                            SetValue(i, GetParsedString(szInfo0[1]));
							
                            char tmp[256] = {0};
                            sprintf(tmp, "%d.%s", m_count1++, GetParsedString("&CfgPtn.CanNotSensorAlarm"));
                            SetValue(i+1, tmp);
                            
                            return TRUE; //此处直接用return是有问题的
                        }
                    } break;
                }
            }
        }
	}
	
	return TRUE;
}

VD_BOOL CPageGuide::MsgProc(uint msg, uint wpa, uint lpa)
{
	switch(msg) 
	{
	case XM_RBUTTONDOWN:
    case XM_RBUTTONDBLCLK:	
        {            
            return TRUE;
        }
		break;
	default:
		break;
	}
    
	return CPage::MsgProc(msg, wpa, lpa);
}

