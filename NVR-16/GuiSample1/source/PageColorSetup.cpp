#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/BizData.h"
#include "mod_preview.h"
#include "lib_video.h"

enum{//与下面的数组一一对应
	COLOR_BUTTON_EXIT,
	COLOR_BUTTON_DEC,
	COLOR_BUTTON_INC,
	COLOR_BUTTON_NUM,
};

char* colorShortcutBmpName[COLOR_BUTTON_NUM][2] = {    
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
video_image_para_t image_color[64];		//用来保存页面当前图像参数值
video_image_para_t pre_image_color[64];	//用来保存配置中各通道图像参数

static int GetMaxChnNum()
{
    return GetVideoMainNum();
}

CPageColorSetup::CPageColorSetup( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent)
{
	//printf("CPageColorSetup\n");
	
    nChMax = GetMaxChnNum();
	
	//csp modify 20121130
	char tmp2[20] = {0};
	GetProductNumber(tmp2);
	
    IsMaxPre = FALSE;
	isStartPage = 0;
	
    eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
	
	switch(nChMax)
	{
		case 4:
		{
			eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
		} break;
		case 8:
		{
			eMaxViewMode = EM_BIZPREVIEW_9SPLITS;
		} break;
		//csp modify 20130504
		case 10:
		{
			eMaxViewMode = EM_BIZPREVIEW_10SPLITS;
		} break;
		case 6:
		{
			eMaxViewMode = EM_BIZPREVIEW_6SPLITS;
		} break;
		case 16:
		{
			eMaxViewMode = EM_BIZPREVIEW_16SPLITS;
		} break;
		case 24:
		{
			//eMaxViewMode = EM_BIZPREVIEW_24SPLITS;
			eMaxViewMode = EM_BIZPREVIEW_25SPLITS;
		} break;
        case 32:
        {
            eMaxViewMode = EM_BIZPREVIEW_36SPLITS;
        } break;
        default:
            break;
	}
	
	m_bright = 128;
	m_chroma = 128;
	m_saturation = 128;
	m_contrast = 128;
	IsDwell = 0;
	
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
		"128",
		"&CfgPtn.Hue",
		"128",
		"&CfgPtn.Saturation",
		"128",
		"&CfgPtn.Contrast",
		"128",
	};
	
	pBmpButtonNormal[0] = VD_LoadBitmap(colorShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(colorShortcutBmpName[0][1]);
	pButton[0] = CreateButton(CRect(m_Rect.Width()-pBmpButtonNormal[0]->width-3,
									3,
									m_Rect.Width()-3,
									3+ pBmpButtonNormal[0]->height), this, NULL, (CTRLPROC)&CPageColorSetup::OnClkButton, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	
	CRect rtTmp;
	rtTmp.left = 4;
	rtTmp.right = rtTmp.left + 100;
	rtTmp.top = pBmpButtonNormal[0]->height;
	rtTmp.bottom = rtTmp.top + 20;
	
	pStatic[0] = CreateStatic(rtTmp, this, "&CfgPtn.Channel");
	pStatic[0]->SetTextAlign(VD_TA_LEFT);
	
	rtTmp.left = 4;
	rtTmp.right = m_Rect.Width()-4;
	rtTmp.top = rtTmp.bottom + 3;
	rtTmp.bottom = rtTmp.top + 20;
	pComboBox = CreateComboBox(rtTmp,this, NULL, NULL, (CTRLPROC)&CPageColorSetup::OnComboBox, 0);
	
	GetScrStrInitComboxSelDo(
		(void*)pComboBox,
		GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN, 
		EM_GSR_COMBLIST, 
		0
	);
	
	rtTmp.left = 4;
	rtTmp.right = rtTmp.left + 100;
	rtTmp.top = rtTmp.bottom + 10;
	rtTmp.bottom = rtTmp.top + 20;

	pBmpButtonNormal[1] = VD_LoadBitmap(colorShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(colorShortcutBmpName[1][1]);
	pBmpButtonNormal[2] = VD_LoadBitmap(colorShortcutBmpName[2][0]);
	pBmpButtonSelect[2] = VD_LoadBitmap(colorShortcutBmpName[2][1]);
	
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
		pButton[i*2+1] = CreateButton(rtTmp, this, NULL,  (CTRLPROC)&CPageColorSetup::OnClkButton, NULL, buttonNormalBmp);
		pButton[i*2+1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
		
		//csp modify 20121130
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			pButton[i*2+1]->Enable(FALSE);
		}
		
		rtTmp.left = rtTmp.right + 3;
		rtTmp.right = rtTmp.left + 96;
		pSlider[i] = CreateSliderCtrl(rtTmp,this,0, 255, (CTRLPROC)&CPageColorSetup::OnSliderChange, sliderNoNum);
		
		//csp modify 20121130
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			pSlider[i]->Enable(FALSE);
		}
		
 		/*
		BIZ_DT_DBG("CPageColorSetup22323\n");
		GetScrStrSetCurSelDo(
			(void*)pSlider[i],
			GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+i, 
			EM_GSR_CTRL_SLIDE, 
			pComboBox->GetCurSel()
		);
		*/
		
		rtTmp.left = rtTmp.right +3;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[2]->width;
		rtTmp.bottom = rtTmp.top+pBmpButtonNormal[2]->height;
		pButton[i*2+2] = CreateButton(rtTmp, this, NULL,	(CTRLPROC)&CPageColorSetup::OnClkButton, NULL, buttonNormalBmp);
		pButton[i*2+2]->SetBitmap(pBmpButtonNormal[2], pBmpButtonSelect[2], pBmpButtonSelect[2]);
		
		//csp modify 20121130
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			pButton[i*2+2]->Enable(FALSE);
		}
		
		rtTmp.left = 4;
		rtTmp.right = rtTmp.left + 100;
		rtTmp.top = rtTmp.bottom + 4;
		rtTmp.bottom = rtTmp.top + 20;
	}
	
	rtTmp.left = 4;
	rtTmp.right = m_Rect.Width () + -4;
	rtTmp.top = rtTmp.bottom - 12;
	rtTmp.bottom = rtTmp.top + pBmp_button_normal->height;
	
	pButton[9] = CreateButton(rtTmp, this, "&CfgPtn.Default", (CTRLPROC)&CPageColorSetup::OnClkButton, NULL, buttonNormalBmp);
	pButton[9]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	//csp modify 20121130
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		pButton[9]->Enable(FALSE);
	}
	
	rtTmp.left = 4;
	rtTmp.right = m_Rect.Width() + -4;
	rtTmp.top = rtTmp.bottom + 5;
	rtTmp.bottom = rtTmp.top + pBmp_button_normal->height;
	
	pButton[10] = CreateButton(rtTmp, this, "&CfgPtn.Save", (CTRLPROC)&CPageColorSetup::OnClkButton, NULL, buttonNormalBmp);
	pButton[10]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);	
	
	//csp modify 20121130
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		pButton[10]->Enable(FALSE);
	}
	
	//BIZ_DT_DBG("CPageColorSetup22\n");
}

void CPageColorSetup::SetStartPageFlag(u8 flag)
{
	isStartPage = flag;
}

void CPageColorSetup::SetColorChn(int chn)
{
	m_chn = chn;
	
	pComboBox->SetCurSel(m_chn);
	pComboBox->Draw();
	
	int maxChn = GetVideoMainNum();
	
	if(m_chn == maxChn)
	{
		switch(m_chn)
		{
			case 4:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_4SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_4SPLITS, 0);
				break;
			case 8:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_9SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_9SPLITS, 0);
				break;
			//csp modify 20130504
			case 10:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_10SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_10SPLITS, 0);
				break;
			case 6:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_6SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_6SPLITS, 0);
				break;
			case 16:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_16SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_16SPLITS, 0);
				break;
			case 24:
				//SwitchPreview(EM_BIZPREVIEW_24SPLITS, 0);
				SetCurPreviewMode_CW(EM_BIZPREVIEW_25SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_25SPLITS, 0);
				break;
			case 32:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_36SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_36SPLITS, 0);
				break;
			default:
				break;
		}
	}
	else
	{
		SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
		SwitchPreview(EM_BIZPREVIEW_1SPLIT, m_chn);
	}
}

void CPageColorSetup::OnClkButton()
{
	//printf("OnClkButton\n");
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
				//printf("exit from color set\n");
				
				int maxChn = GetVideoMainNum();
				
				for(int j=0; j<maxChn; j++)
				{
					PreviewSetImageColor(j, (SBizPreviewImagePara*)&pre_image_color[j]);
				}
				
				int FreshChn = pComboBox->GetCurSel();
				
				SetSystemLockStatus(1);
				
				if(FreshChn == maxChn)//all
				{
					CPage** g_pPages = GetPage();
					CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
					
					pDeskTop->ShowChnName(3, FALSE);
					this->Close();
					pDeskTop->ShowChnName(3, TRUE);
				}
				else
				{
					this->Close();
				}
				
                //SwitchPreview(eMaxViewMode, 0);
				BizSetZoomMode(0);
				if(IsDwell)
				{
					usleep(1000 * 1000);
					ControlPatrol(1);
					IsDwell = 0;
				}
				m_pParent->Open();
				
				if(!isStartPage)
				{
					((CPage*)(this->GetParentPage()))->Show(TRUE,TRUE);
				}
				
				SetSystemLockStatus(0);
				
				isStartPage = 0;
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

			}break;
			case 2:
			{
				printf("bright ++\n");

				m_bright++;
				if(m_bright>255)
				{
					m_bright = 255;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_bright);
				pStatic[2]->SetText(str);
				pSlider[0]->SetPos(m_bright);

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

			}break;
			case 4:
			{
				printf("chroma ++\n");

				m_chroma++;
				if(m_chroma>255)
				{
					m_chroma = 255;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_chroma);
				pStatic[4]->SetText(str);
				pSlider[1]->SetPos(m_chroma);

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
				
			}break;
			case 6:
			{
				printf("saturation ++\n");

				m_saturation++;
				if(m_saturation>255)
				{
					m_saturation = 255;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_saturation);
				pStatic[6]->SetText(str);
				pSlider[2]->SetPos(m_saturation);

			}break;
			case 7:
			{
				printf("contrast --\n"); 

				m_contrast--;
				if(m_contrast<0)
				{
					m_contrast = 0;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
				pSlider[3]->SetPos(m_contrast);

			}break;
			case 8:
			{
				printf("contrast ++\n");

				m_contrast++;
				if(m_contrast>255)
				{
					m_contrast = 255;
				}

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
				pSlider[3]->SetPos(m_contrast);

			}break;
			case 9:
			{
				//printf("Default\n");
				int maxChn = GetVideoMainNum();
				int curSel = pComboBox->GetCurSel();
				char szPos[10] = {0};
				if(curSel == maxChn)
				{
					for(int j=0; j<maxChn; j++)
					{
						for(int i=0; i<4; i++)
						{
							GetScrStrSetCurSelDo(
								(void*)pSlider[i],
								GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
								EM_GSR_CTRL_SLIDE, 
								j
								);
							sprintf(szPos, "%d", pSlider[i]->GetPos());
							pStatic[2*(i+1)]->SetText(szPos);
						}
					}
				}
				else
				{
					for(int i=0; i<4; i++)
					{
						GetScrStrSetCurSelDo(
							(void*)pSlider[i],
							GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
							EM_GSR_CTRL_SLIDE, 
							curSel
						);
						sprintf(szPos, "%d", pSlider[i]->GetPos());
						pStatic[2*(i+1)]->SetText(szPos);
					}
				}
				m_bright = pSlider[0]->GetPos();
				m_chroma = pSlider[1]->GetPos();
				m_saturation = pSlider[2]->GetPos();
				m_contrast = pSlider[3]->GetPos();
				
			}break;
			case 10:
			{
                printf("Save\n");
                
				int maxChn = GetVideoMainNum();
				int chn = pComboBox->GetCurSel();

				if(chn == maxChn)//all
				{
					for(int j=0; j<maxChn; j++)
					{
						for(int i=0; i<4; i++)
						{
							SaveScrStr2Cfg(
								pSlider[i],
								GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+i, 
								EM_GSR_CTRL_SLIDE, 
								j
							);
						}
						pre_image_color[j].brightness = pSlider[0]->GetPos();
						pre_image_color[j].hue = pSlider[1]->GetPos();
						pre_image_color[j].saturation = pSlider[2]->GetPos();
						pre_image_color[j].contrast = pSlider[3]->GetPos();

						image_color[j].brightness = pSlider[0]->GetPos();
						image_color[j].hue = pSlider[1]->GetPos();
						image_color[j].saturation = pSlider[2]->GetPos();
						image_color[j].contrast = pSlider[3]->GetPos();
					}

				}
				else
				{
					for(int i=0; i<4; i++)
					{
						SaveScrStr2Cfg(
							pSlider[i],
							GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+i, 
							EM_GSR_CTRL_SLIDE, 
							chn
						);
					}
					pre_image_color[chn].brightness = pSlider[0]->GetPos();
					pre_image_color[chn].hue = pSlider[1]->GetPos();
					pre_image_color[chn].saturation = pSlider[2]->GetPos();
					pre_image_color[chn].contrast = pSlider[3]->GetPos();

					image_color[chn].brightness = pSlider[0]->GetPos();
					image_color[chn].hue = pSlider[1]->GetPos();
					image_color[chn].saturation = pSlider[2]->GetPos();
					image_color[chn].contrast = pSlider[3]->GetPos();
				}
				
				//SwitchPreview(EM_BIZPREVIEW_4SPLITS, 0);
			}break;
			default:
			break;
		}
		
		if(i >0 && i < 10)
		{
			int maxChn = GetVideoMainNum();
			int curSel = pComboBox->GetCurSel();				
			char szPos[10] = {0};
            int ret = 0;
            
			if(curSel == maxChn)
			{
				for(int j=0; j<maxChn; j++)
				{
					image_color[j].brightness = pSlider[0]->GetPos();
					image_color[j].hue = pSlider[1]->GetPos();
					image_color[j].saturation = pSlider[2]->GetPos();
					image_color[j].contrast = pSlider[3]->GetPos();
					
					ret = PreviewSetImageColor(j, (SBizPreviewImagePara*)&image_color[j]);
                    //printf("***Chn: %d, ret = %d\n", j, ret);
				}

				flag_all = 1;
			}
			else
			{
				image_color[curSel].brightness = pSlider[0]->GetPos();
				image_color[curSel].hue = pSlider[1]->GetPos();
				image_color[curSel].saturation = pSlider[2]->GetPos();
				image_color[curSel].contrast = pSlider[3]->GetPos();

				ret = PreviewSetImageColor(curSel, (SBizPreviewImagePara*)&image_color[curSel]);
                //printf("***Chn: %d, ret = %d\n", curSel, ret);

				flag_all = 0;
			}
		}
	}
}

void CPageColorSetup::OnSliderChange()
{
	//intf("OnSliderChange\n");

	int i = 0;
	BOOL bFind = FALSE;
	CSliderCtrl* pFocusSlider = (CSliderCtrl*)GetFocusItem();
	for(i = 0; i < 4; i++)
	{
		if(pFocusSlider == pSlider[i])
		{
			bFind = TRUE;
			break;
		}
		else
		{
			//nothing
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
			case 0:
			{
				m_bright = pSlider[0]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_bright);
				pStatic[2]->SetText(str);
			}
			
			break;
			case 1:
			{
				m_chroma = pSlider[1]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_chroma);
				pStatic[4]->SetText(str);
			}

			break;
			case 2:
			{
				m_saturation = pSlider[2]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_saturation);
				pStatic[6]->SetText(str);
			}

			break;
			case 3:
			{

				m_contrast= pSlider[3]->GetPos();

				char str[4] = {0};
				sprintf(str,"%d",m_contrast);
				pStatic[8]->SetText(str);
			}

			break;
			default:
			break;
		}
		
		int maxChn = GetVideoMainNum();
		int curSel = pComboBox->GetCurSel();
		
		if(curSel == maxChn)
		{
            int ret = 0;
			for(int j=0; j<maxChn; j++)
			{
				image_color[j].brightness = pSlider[0]->GetPos();
				image_color[j].hue = pSlider[1]->GetPos();
				image_color[j].saturation = pSlider[2]->GetPos();
				image_color[j].contrast = pSlider[3]->GetPos();
				
				ret = PreviewSetImageColor(j, (SBizPreviewImagePara*)&image_color[j]);
                //printf("***Chn: %d, ret = %d\n", j, ret);
			}
			
			flag_all = 1;
		}
		else
		{
			image_color[curSel].brightness = pSlider[0]->GetPos();
			image_color[curSel].hue = pSlider[1]->GetPos();
			image_color[curSel].saturation = pSlider[2]->GetPos();
			image_color[curSel].contrast = pSlider[3]->GetPos();
			/*printf("chn%d set color:(%d %d %d %d)\n",
				curSel,
				image_color[curSel].brightness,
				image_color[curSel].contrast,
				image_color[curSel].saturation,
				image_color[curSel].hue);*/
			PreviewSetImageColor(curSel, (SBizPreviewImagePara*)&image_color[curSel]);
			flag_all = 0;
		}
	}
}

void CPageColorSetup::OnComboBox()
{
	//int curSel = pComboBox->GetCurSel();
	
	char szPos[10] = {0};
	int maxChn = GetVideoMainNum();
	int chn = pComboBox->GetCurSel();
	
	if(chn == maxChn)
	{
		SPreviewPara sPreviewPara;
		sPreviewPara.nModePara = 0;
		
		switch(chn)
		{
			case 4:
				sPreviewPara.emPreviewMode = EM_PREVIEW_4SPLITS;
				break;
			case 8:
			case 9:
				sPreviewPara.emPreviewMode = EM_PREVIEW_9SPLITS;
				break;
			//csp modify 20130504
			case 10:
				sPreviewPara.emPreviewMode = EM_PREVIEW_10SPLITS;
				break;
			case 6:
				sPreviewPara.emPreviewMode = EM_PREVIEW_6SPLITS;
				break;
			case 16:
				sPreviewPara.emPreviewMode = EM_PREVIEW_16SPLITS;
				break;
			case 24:
			case 25:
				//sPreviewPara.emPreviewMode = EM_PREVIEW_24SPLITS;
				sPreviewPara.emPreviewMode = EM_PREVIEW_25SPLITS;
				break;
			case 32:
			case 36:
				sPreviewPara.emPreviewMode = EM_PREVIEW_36SPLITS;
				break;
			default:
				break;
		}
		
		//printf("CPageColorSetup::OnComboBox - ModPreviewStart\n");
		
		//sPreviewPara.emPreviewMode = EM_PREVIEW_4SPLITS;
		SetCurPreviewMode_CW((EMBIZPREVIEWMODE)sPreviewPara.emPreviewMode);//cw_preview
		ModPreviewStart(&sPreviewPara);
		if(0 == flag_all)
		{
			#if 0
			sprintf(szPos, "%d", 128);
			pSlider[0]->SetPos(128);
			pStatic[2]->SetText(szPos);
					
			sprintf(szPos, "%d", nIs3515? 0: 128);
			pSlider[1]->SetPos(nIs3515? 0: 128);
			pStatic[4]->SetText(szPos);
					
			sprintf(szPos, "%d", 128);
			pSlider[2]->SetPos(128);
			pStatic[6]->SetText(szPos);
					
			sprintf(szPos, "%d", nIs3515? 98: 128);
			pSlider[3]->SetPos(nIs3515? 98: 128);
			pStatic[8]->SetText(szPos);
			#endif
			//int maxChn = GetVideoMainNum();
			//for(int j=0; j<maxChn; j++)
			{
				for(int i=0; i<4; i++)
				{
					GetScrStrSetCurSelDo(
						(void*)pSlider[i],
						GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
						EM_GSR_CTRL_SLIDE, 
						0
						);
					sprintf(szPos, "%d", pSlider[i]->GetPos());
					pStatic[2*(i+1)]->SetText(szPos);
				}
			}
		}
		else
		{
			sprintf(szPos, "%d", image_color[0].brightness);
			pSlider[0]->SetPos(image_color[0].brightness);
			pStatic[2]->SetText(szPos);
			
			sprintf(szPos, "%d", image_color[0].hue);
			pSlider[1]->SetPos(image_color[0].hue);
			pStatic[4]->SetText(szPos);
			
			sprintf(szPos, "%d", image_color[0].saturation);
			pSlider[2]->SetPos(image_color[0].saturation);
			pStatic[6]->SetText(szPos);
			
			sprintf(szPos, "%d", image_color[0].contrast);
			pSlider[3]->SetPos(image_color[0].contrast);
			pStatic[8]->SetText(szPos);
		}
        IsMaxPre = TRUE;
	}
	else
	{
		SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
		SwitchPreview(EM_BIZPREVIEW_1SPLIT, chn);

		sprintf(szPos, "%d", image_color[chn].brightness);
		pSlider[0]->SetPos(image_color[chn].brightness);
		pStatic[2]->SetText(szPos);
		
		sprintf(szPos, "%d", image_color[chn].hue);
		pSlider[1]->SetPos(image_color[chn].hue);
		pStatic[4]->SetText(szPos);
		
		sprintf(szPos, "%d", image_color[chn].saturation);
		pSlider[2]->SetPos(image_color[chn].saturation);
		pStatic[6]->SetText(szPos);
		
		sprintf(szPos, "%d", image_color[chn].contrast);
		pSlider[3]->SetPos(image_color[chn].contrast);
		pStatic[8]->SetText(szPos);

        IsMaxPre = FALSE;
	}
	
	m_bright = pSlider[0]->GetPos();
	m_chroma = pSlider[1]->GetPos();
	m_saturation = pSlider[2]->GetPos();
	m_contrast = pSlider[3]->GetPos();
}

CPageColorSetup::~CPageColorSetup()
{
	
}

VD_BOOL CPageColorSetup::UpdateData( UDM mode )
{
	char szPos[32] = {0};
	
	switch(mode)
	{
		case UDM_CANCEL:
		{
			//printf("CPageColorSetup UDM_CANCEL - hehe1\n");
			//fflush(stdout);
			
			//右键退出设置页面操作
			int maxChn = GetVideoMainNum();
			//重设图像参数
			for(int j=0; j<maxChn; j++)
			{
				PreviewSetImageColor(j, (SBizPreviewImagePara*)&pre_image_color[j]);
			}
			
			//printf("CPageColorSetup UDM_CANCEL - hehe2\n");
			//fflush(stdout);
			
			int FreshChn = pComboBox->GetCurSel();
			
			SetSystemLockStatus(1);
			
			//printf("CPageColorSetup UDM_CANCEL - hehe3\n");
			//fflush(stdout);
			
			if(FreshChn == maxChn)//all
			{
				CPage** g_pPages = GetPage();
				CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
				
				pDeskTop->ShowChnName(3, FALSE);
				this->Close();
				pDeskTop->ShowChnName(3, TRUE);
			}
			else
			{
				this->Close();
			}
			
			//printf("CPageColorSetup UDM_CANCEL - hehe4\n");
			//fflush(stdout);
			
			BizSetZoomMode(0);
			
			if(IsDwell)
			{
				usleep(500 * 1000);
				ControlPatrol(1);
				IsDwell = 0;
			}
			
			//printf("CPageColorSetup UDM_CANCEL - hehe5,m_pParent=0x%08x\n",(unsigned int)m_pParent);
			//fflush(stdout);
			
			m_pParent->Open();
			
			//printf("CPageColorSetup UDM_CANCEL - hehe5.1\n");
			//fflush(stdout);
			
			if(!isStartPage)
			{
				((CPage*)(this->GetParentPage()))->Show(TRUE,TRUE);
			}
			
			SetSystemLockStatus(0);
			
			isStartPage = 0;
			
			//printf("CPageColorSetup UDM_CANCEL - hehe6\n");
			//fflush(stdout);
		}
		break;
		case UDM_OPEN:
		{			
			//BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS err\n");
			flag_all = 0;
			int j = 0;

			if(!isStartPage)
			{
				((CPage*)((this->GetParentPage())->GetParentPage()))->Show(FALSE);
				
				((CPage*)(((this->GetParentPage())->GetParentPage())->GetParentPage()))->Show(FALSE);
				
				((CPage*)(this->GetParentPage()))->Show(FALSE);

				m_DC.Lock();
			
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.SetBrush(CDevGraphics::instance(m_screen)->GetKeyColor());
				m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
				m_DC.UnLock();

				CPage::Draw();
			}
			
			pComboBox->SetCurSel(m_chn);

            IsMaxPre = FALSE;
			
			int maxChn = GetVideoMainNum();
			int curSel = pComboBox->GetCurSel();

			BizSetZoomMode(1);
			
			if(GetDwellStartFlag())
			{
				IsDwell = 1;
				ControlPatrol(0);
			}
			
			for(j=0; j<maxChn; j++)
			{
				for(int i=0; i<4; i++)
				{
					GetScrStrSetCurSelDo(
						(void*)pSlider[i],
						GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+i, 
						EM_GSR_CTRL_SLIDE, 
						j
						);
				
					sprintf(szPos, "%d", pSlider[i]->GetPos());
					pStatic[2*(i+1)]->SetText(szPos);
				}
				pre_image_color[j].brightness = pSlider[0]->GetPos();
				pre_image_color[j].hue = pSlider[1]->GetPos();
				pre_image_color[j].saturation = pSlider[2]->GetPos();
				pre_image_color[j].contrast = pSlider[3]->GetPos();

				image_color[j].brightness = pSlider[0]->GetPos();
				image_color[j].hue = pSlider[1]->GetPos();
				image_color[j].saturation = pSlider[2]->GetPos();
				image_color[j].contrast = pSlider[3]->GetPos();
				
			}
			
			if(curSel == maxChn)
			{
			#if 0
				sprintf(szPos, "%d", 128);
				pSlider[0]->SetPos(128);
				pStatic[2]->SetText(szPos);
						
				sprintf(szPos, "%d", nIs3515? 0: 128);
				pSlider[1]->SetPos(nIs3515? 0: 128);
				pStatic[4]->SetText(szPos);
						
				sprintf(szPos, "%d", 128);
				pSlider[2]->SetPos(128);
				pStatic[6]->SetText(szPos);
						
				sprintf(szPos, "%d", nIs3515? 98: 128);
				pSlider[3]->SetPos(nIs3515? 98: 128);
				pStatic[8]->SetText(szPos);
				#endif

				//int maxChn = GetVideoMainNum();
				//for(int j=0; j<maxChn; j++)
				{
					for(int i=0; i<4; i++)
					{
						GetScrStrSetCurSelDo(
							(void*)pSlider[i],
							GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS+GSR_DEFAULT_OFFSET+i, 
							EM_GSR_CTRL_SLIDE, 
							0
							);
						sprintf(szPos, "%d", pSlider[i]->GetPos());
						pStatic[2*(i+1)]->SetText(szPos);
					}
				}
			}
			else
			{
				sprintf(szPos, "%d", pre_image_color[curSel].brightness);
				pSlider[0]->SetPos(pre_image_color[curSel].brightness);
				pStatic[2]->SetText(szPos);
				
				sprintf(szPos, "%d", pre_image_color[curSel].hue);
				pSlider[1]->SetPos(pre_image_color[curSel].hue);
				pStatic[4]->SetText(szPos);
				
				sprintf(szPos, "%d", pre_image_color[curSel].saturation);
				pSlider[2]->SetPos(pre_image_color[curSel].saturation);
				pStatic[6]->SetText(szPos);
				
				sprintf(szPos, "%d", pre_image_color[curSel].contrast);
				pSlider[3]->SetPos(pre_image_color[curSel].contrast);
				pStatic[8]->SetText(szPos);
			}
			
			m_bright = pSlider[0]->GetPos();
			m_chroma = pSlider[1]->GetPos();
			m_saturation = pSlider[2]->GetPos();
			m_contrast = pSlider[3]->GetPos();
			//BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS err\n");
		}
		break;
        case UDM_CLOSED:
        {
            if(FALSE == IsMaxPre)
			{
                SetCurPreviewMode_CW(eMaxViewMode);//cw_preview
        		SwitchPreview(eMaxViewMode, 0);
                IsMaxPre = TRUE;
            }
			//printf("CPageColorSetup::UpdateData UDM_CLOSED\n");
        }
        break;
	}
	
	return TRUE;
}

VD_BOOL CPageColorSetup::MsgProc( uint msg, uint wpa, uint lpa )
{
    switch(msg)
	{
		case XM_KEYDOWN://cw_panel
		{
			if((wpa == KEY_UP)
			|| (wpa == KEY_DOWN)
			|| (wpa == KEY_LEFT)
			|| (wpa == KEY_RIGHT)
			|| (wpa == KEY_RET)
			|| (wpa == KEY_ESC))
			break;
			else
			return -1;
		}
	    case XM_RBUTTONDOWN:
     	case XM_RBUTTONDBLCLK:
			//this->Close();
			//{
				//this->Close();
				//SwitchPreview(eMaxViewMode, 0);
			//}
        break;
        default:
            break;
    }
	return CPage::MsgProc(msg, wpa, lpa);
}

