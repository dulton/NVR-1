
#include "GUI/Pages/PagePlayBackVoColorSetup.h"
#include "GUI/Pages/BizData.h"
#include "mod_preview.h"
#include "lib_video.h"

enum{//与下面的数组一一对应
	COLOR_BUTTON_EXIT,
	COLOR_BUTTON_DEC,
	COLOR_BUTTON_INC,
	COLOR_BUTTON_NUM,
};


static char* colorShortcutBmpName[COLOR_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/exit.bmp",	DATA_DIR"/temp/exit_f.bmp"},
	{DATA_DIR"/temp/btn_dec.bmp",	DATA_DIR"/temp/btn_dec_f.bmp"},
	{DATA_DIR"/temp/btn_add.bmp",	DATA_DIR"/temp/btn_add_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[COLOR_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[COLOR_BUTTON_NUM];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

//9624
//video_image_para_t image_color[64];		//用来保存页面当前图像参数值
//video_image_para_t pre_image_color[64];	//用来保存配置中各通道图像参数

static int GetMaxChnNum()
{
    return GetVideoMainNum();
}

CPagePlayBackVoColorSetup::CPagePlayBackVoColorSetup( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent)
{
	//printf("CPageVoColorSetup\n");
	
    nChMax = GetMaxChnNum();
	
	m_bright = 100;
	m_chroma = 100;
	m_saturation = 100;
	m_contrast = 100;
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	char* staticText[8] = 
	{
		"&CfgPtn.Brightness",
		"100",
		"&CfgPtn.Hue",
		"100",
		"&CfgPtn.Saturation",
		"100",
		"&CfgPtn.Contrast",
		"100",
	};
	
	pBmpButtonNormal[0] = VD_LoadBitmap(colorShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(colorShortcutBmpName[0][1]);
	pButton[0] = CreateButton(CRect(m_Rect.Width()-pBmpButtonNormal[0]->width-3,
									3,
									m_Rect.Width()-3,
									3+pBmpButtonNormal[0]->height), this, NULL, (CTRLPROC)&CPagePlayBackVoColorSetup::OnClkButton, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	
	CRect rtTmp;
	rtTmp.left = 4;
	rtTmp.right = rtTmp.left + 100;
	rtTmp.top = pBmpButtonNormal[0]->height;//rtTmp.bottom + 10;//csp modify
	rtTmp.bottom = rtTmp.top + 20;
	
	pBmpButtonNormal[1] = VD_LoadBitmap(colorShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(colorShortcutBmpName[1][1]);
	pBmpButtonNormal[2] = VD_LoadBitmap(colorShortcutBmpName[2][0]);
	pBmpButtonSelect[2] = VD_LoadBitmap(colorShortcutBmpName[2][1]);
	
	//csp modify
	//pDrawLine = CreateStatic(CRect(1,rtTmp.top - 8,m_Rect.Width(),rtTmp.top - 6), this, "");
	//pDrawLine->SetBkColor(VD_RGB(120,118,118));
	
	int i = 0;
	for(i=0; i<4; i++)
	{
		pStatic[i*2+1] = CreateStatic(rtTmp, this, staticText[i*2]);
		pStatic[i*2+1]->SetTextAlign(VD_TA_LEFT);

		rtTmp.left = m_Rect.Width()-38;
		rtTmp.right = rtTmp.left + 36;

		pStatic[i*2+2] = CreateStatic(rtTmp, this, staticText[i*2+1]);
		pStatic[i*2+2]->SetTextAlign(VD_TA_LEFT);

		rtTmp.left = 4;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[1]->width;
		rtTmp.top = rtTmp.bottom+3;
		rtTmp.bottom = rtTmp.top+pBmpButtonNormal[1]->height;
		pButton[i*2+1] = CreateButton(rtTmp, this, NULL,  (CTRLPROC)&CPagePlayBackVoColorSetup::OnClkButton, NULL, buttonNormalBmp);
		pButton[i*2+1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);

		rtTmp.left = rtTmp.right +3;
		rtTmp.right = rtTmp.left +96;
		pSlider[i] = CreateSliderCtrl(rtTmp,this,0, 100, (CTRLPROC)&CPagePlayBackVoColorSetup::OnSliderChange, sliderNoNum);
		
		rtTmp.left = rtTmp.right +3;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[2]->width;
		rtTmp.bottom = rtTmp.top+pBmpButtonNormal[2]->height;
		pButton[i*2+2] = CreateButton(rtTmp, this, NULL,	(CTRLPROC)&CPagePlayBackVoColorSetup::OnClkButton, NULL, buttonNormalBmp);
		pButton[i*2+2]->SetBitmap(pBmpButtonNormal[2], pBmpButtonSelect[2], pBmpButtonSelect[2]);

		rtTmp.left = 4;
		rtTmp.right = rtTmp.left + 100;
		rtTmp.top = rtTmp.bottom+4;
		rtTmp.bottom = rtTmp.top + 20;
	}
	
	rtTmp.left = 4;
	rtTmp.right = m_Rect.Width() + -4;
	rtTmp.top = rtTmp.bottom - 12;
	rtTmp.bottom = rtTmp.top + pBmp_button_normal->height;
	
	pButton[9] = CreateButton(rtTmp, this, "&CfgPtn.Default", (CTRLPROC)&CPagePlayBackVoColorSetup::OnClkButton, NULL, buttonNormalBmp);
	pButton[9]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	BIZ_DT_DBG("CPagePlayBackVoColorSetup22\n");
}

void CPagePlayBackVoColorSetup::SetColorChn(int chn)
{
	
}

void CPagePlayBackVoColorSetup::OnClkButton()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 11; i++)
	{
		if(pFocusButton == pButton[i])
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
				printf("exit from color set\n");
				this->Close();
				//m_pParent->Open();
			}break;
			case 1:
			{
				printf("bright --\n");

				m_bright--;
				if(m_bright<0)
				{
					m_bright = 0;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_bright);
				pStatic[2]->SetText(str);
				pSlider[0]->SetPos(m_bright);
				
				SaveScrStr2Cfg(
					pSlider[0],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+0, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 2:
			{
				printf("bright ++\n");

				m_bright++;
				if(m_bright>100)
				{
					m_bright = 100;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_bright);
				pStatic[2]->SetText(str);
				pSlider[0]->SetPos(m_bright);
				
				SaveScrStr2Cfg(
					pSlider[0],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+0, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 3:
			{
				printf("chroma --\n");

				m_chroma--;
				if(m_chroma<0)
				{
					m_chroma = 0;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_chroma);
				pStatic[4]->SetText(str);
				pSlider[1]->SetPos(m_chroma);
				
				SaveScrStr2Cfg(
					pSlider[1],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+1, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 4:
			{
				printf("chroma ++\n");

				m_chroma++;
				if(m_chroma>100)
				{
					m_chroma = 100;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_chroma);
				pStatic[4]->SetText(str);
				pSlider[1]->SetPos(m_chroma);
				
				SaveScrStr2Cfg(
					pSlider[1],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+1, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 5:
			{
				printf("saturation --\n");

				m_saturation--;
				if(m_saturation<0)
				{
					m_saturation = 0;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_saturation);
				pStatic[6]->SetText(str);
				pSlider[2]->SetPos(m_saturation);
				
				SaveScrStr2Cfg(
					pSlider[2],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+2, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 6:
			{
				printf("saturation ++\n");

				m_saturation++;
				if(m_saturation>100)
				{
					m_saturation = 100;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_saturation);
				pStatic[6]->SetText(str);
				pSlider[2]->SetPos(m_saturation);

				SaveScrStr2Cfg(
					pSlider[2],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+2, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 7:
			{
				printf("contrast --\n"); 

				m_contrast--;
				if(m_contrast < 15)
				{
					m_contrast = 15;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
				pSlider[3]->SetPos(m_contrast);
				
				SaveScrStr2Cfg(
					pSlider[3],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+3, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			}break;
			case 8:
			{
				printf("contrast ++\n");

				m_contrast++;
				if(m_contrast>100)
				{
					m_contrast = 100;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
				pSlider[3]->SetPos(m_contrast);
			}

			SaveScrStr2Cfg(
					pSlider[3],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+3, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
			break;
			case 9:
			{
				printf("Default\n");

				char szPos[32] = {0};

				for(int i=0; i<4; i++)
				{
					GetScrStrSetCurSelDo(
						(void*)pSlider[i],
						GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
						EM_GSR_CTRL_SLIDE, 
						0
						);
					sprintf(szPos, "%d", pSlider[i]->GetPos());
					pStatic[2*(i+1)]->SetText(szPos);
				}

				for(int i=0; i<4; i++)
				{
					SaveScrStr2Cfg(
					pSlider[i],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+i, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
					sprintf(szPos, "%d", pSlider[i]->GetPos());
					pStatic[2*(i+1)]->SetText(szPos);
				}

				m_bright = pSlider[0]->GetPos();
				m_chroma = pSlider[1]->GetPos();
				m_saturation = pSlider[2]->GetPos();
				m_contrast = pSlider[3]->GetPos();
				
				vo_image_color.voLuma = pSlider[0]->GetPos();
				vo_image_color.voHue = pSlider[1]->GetPos();
				vo_image_color.voSaturation = pSlider[2]->GetPos();
				vo_image_color.voContrast = pSlider[3]->GetPos();
				
				//PreviewSetImageColor(j, (SBizPreviewImagePara*)&image_color[j]);
				PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
				}break;
				case 10:
				{
					int maxChn = GetVideoMainNum();
					printf("Save\n");
				}break;
			default:
			break;
		}
		
		if(i >0 && i < 9)
		{
			vo_image_color.voLuma = pSlider[0]->GetPos();
			vo_image_color.voHue = pSlider[1]->GetPos();
			vo_image_color.voSaturation = pSlider[2]->GetPos();
			vo_image_color.voContrast = pSlider[3]->GetPos();
			
			PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
		}
	}
}

void CPagePlayBackVoColorSetup::OnSliderChange()
{
	//intf("OnSliderChange\n");

	int i = 0;
	BOOL bFind = FALSE;
	CSliderCtrl* pFocusSlider = (CSliderCtrl*)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusSlider == pSlider[i])
		{
			bFind = TRUE;
			break;
		}
		else
		{
			//nothing
		}
	}

	if (bFind)
	{
		switch(i)
		{
			case 0:
			{
				m_bright = pSlider[0]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_bright);
				pStatic[2]->SetText(str);
				
				SaveScrStr2Cfg(
				pSlider[0],
				GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+0, 
				EM_GSR_CTRL_SLIDE, 
				0
				);
			}
			
			break;
			case 1:
			{
				m_chroma = pSlider[1]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_chroma);
				pStatic[4]->SetText(str);

				SaveScrStr2Cfg(
				pSlider[1],
				GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+1, 
				EM_GSR_CTRL_SLIDE, 
				0
				);
			}

			break;
			case 2:
			{
				m_saturation = pSlider[2]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_saturation);
				pStatic[6]->SetText(str);
				
				SaveScrStr2Cfg(
				pSlider[2],
				GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+2, 
				EM_GSR_CTRL_SLIDE, 
				0
				);
			}
			
			break;
			case 3:
			{
				m_contrast= pSlider[3]->GetPos();
				if(m_contrast < 15)
				{
					m_contrast = 15;
					pSlider[3]->SetPos(m_contrast);
				}

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
				
				SaveScrStr2Cfg(
				pSlider[3],
				GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+3, 
				EM_GSR_CTRL_SLIDE, 
				0
				);
			}
			break;
			default:
			break;
		}
		
		vo_image_color.voLuma = pSlider[0]->GetPos();
		vo_image_color.voHue = pSlider[1]->GetPos();
		vo_image_color.voSaturation = pSlider[2]->GetPos();
		vo_image_color.voContrast = pSlider[3]->GetPos();
		
		PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
	}
}

void CPagePlayBackVoColorSetup::OnComboBox()
{

}


CPagePlayBackVoColorSetup::~CPagePlayBackVoColorSetup()
{

}

void CPagePlayBackVoColorSetup::SetPlayBackColor()
{
	char szPos[32] = {0};
	for(int i=0; i<4; i++)
	{
		GetScrStrSetCurSelDo(
			(void*)pSlider[i],
			GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+i, 
			EM_GSR_CTRL_SLIDE, 
			0
			);
		
		sprintf(szPos, "%d", pSlider[i]->GetPos());
		pStatic[2*(i+1)]->SetText(szPos);
	}
	
	vo_image_color.voLuma = pSlider[0]->GetPos();
	vo_image_color.voHue = pSlider[1]->GetPos();
	vo_image_color.voSaturation = pSlider[2]->GetPos();
	vo_image_color.voContrast = pSlider[3]->GetPos();
	
	PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
}

void CPagePlayBackVoColorSetup::SetDefaultVoImageParam()
{
	char szPos[32] = {0};

	for(int i=0; i<4; i++)
	{
		GetScrStrSetCurSelDo(
			(void*)pSlider[i],
			GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
			EM_GSR_CTRL_SLIDE, 
			0
			);
		sprintf(szPos, "%d", pSlider[i]->GetPos());
		pStatic[2*(i+1)]->SetText(szPos);
	}
	
	vo_image_para_t image_color;
	
	image_color.voLuma = pSlider[0]->GetPos();
	image_color.voHue = pSlider[1]->GetPos();
	image_color.voSaturation = pSlider[2]->GetPos();
	image_color.voContrast = pSlider[3]->GetPos();
	
	//PreviewSetImageColor(j, (SBizPreviewImagePara*)&image_color[j]);
	PlayBackSetImageColor((SBizVoImagePara*)&image_color);
}

VD_BOOL CPagePlayBackVoColorSetup::UpdateData( UDM mode )
{
	char szPos[32] = {0};
	
	switch(mode)
	{
		case UDM_CANCEL:
		{
			//右键退出设置页面操作			
			this->Close();
			//m_pParent->Open();
		}
		break;
		case UDM_OPEN:
		{
			for(int i=0; i<4; i++)
			{
				GetScrStrSetCurSelDo(
					(void*)pSlider[i],
					GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS+i, 
					EM_GSR_CTRL_SLIDE, 
					0
					);
				
				sprintf(szPos, "%d", pSlider[i]->GetPos());
				pStatic[2*(i+1)]->SetText(szPos);
			}
			
			vo_image_color.voLuma = pSlider[0]->GetPos();
			vo_image_color.voHue = pSlider[1]->GetPos();
			vo_image_color.voSaturation = pSlider[2]->GetPos();
			vo_image_color.voContrast = pSlider[3]->GetPos();
			
			PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
			
			m_bright = pSlider[0]->GetPos();
			m_chroma = pSlider[1]->GetPos();
			m_saturation = pSlider[2]->GetPos();
			m_contrast = pSlider[3]->GetPos();
		}
		break;
	}
	return TRUE;
}

VD_BOOL CPagePlayBackVoColorSetup::MsgProc( uint msg, uint wpa, uint lpa )
{
    switch(msg)
	{
		case XM_KEYDOWN://cw_panel
		{
			switch(wpa)
			{
				case KEY_RET:
				case KEY_UP:
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_LEFT:
				case KEY_ESC:
					break;
				default:
					return false;
			}
		}
	    case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:
			vo_image_color.voLuma = pSlider[0]->GetPos();
			vo_image_color.voHue = pSlider[1]->GetPos();
			vo_image_color.voSaturation = pSlider[2]->GetPos();
			vo_image_color.voContrast = pSlider[3]->GetPos();
			
			PlayBackSetImageColor((SBizVoImagePara*)&vo_image_color);
        break;
        default:
            break;
    }
	return CPage::MsgProc(msg, wpa, lpa);
}

