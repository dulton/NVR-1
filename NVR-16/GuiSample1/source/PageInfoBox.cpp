//#include "GUI/Pages/PageRecordFrameWork.h"
//#include "GUI/Pages/PageSensorAlarmFrameWork.h"
#include "GUI/Pages/PageInfoBox.h"

static VD_BITMAP *pBmp_button_normal;
static VD_BITMAP *pBmp_button_push;
static VD_BITMAP *pBmp_button_select;

//csp modify//解决一个很严重的问题
//CPageInfoBox::CPageInfoBox( VD_PCRECT pRect,VD_PCSTR psz,VD_PCSTR pInfo,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ) : CPageFrame(pRect, psz, icon, pParent), curMainItemSel(0)
CPageInfoBox::CPageInfoBox( VD_PCRECT pRect,VD_PCSTR psz,VD_PCSTR pInfo,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ) : CPageFrame(pRect, psz, icon, pParent, pageDialog), curMainItemSel(0)
{
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	
	m_okFunc = NULL;
	
	static VD_BITMAP* pBmpWarning = NULL;
	pBmpWarning = VD_LoadBitmap(DATA_DIR"/temp/tip_info.bmp");
	if(pBmpWarning)
	{
		rtTmp.left = 25;
		rtTmp.right = rtTmp.left + pBmpWarning->width;
		rtTmp.top =  25;
		rtTmp.bottom = rtTmp.top + pBmpWarning->height;
		
		pStatic[0] = NULL;
		pStatic[0] = CreateStatic(rtTmp, this, pBmpWarning);
		if (NULL == pStatic[0])
		{
			printf("%s pStatic[0] == NULL\n", __func__);
			fflush(stdout);
		}
	}
	else
	{
		printf("%s pBmpWarning == NULL\n", __func__);
		fflush(stdout);
	}
	
	char* szWaring = (char *)pInfo;
	rtTmp.left = rtTmp.right + 10;
	rtTmp.right = pRect->right - 15;//rtTmp.left + 250;
	rtTmp.top =  55;
	rtTmp.bottom = rtTmp.top + 20;

	pStatic[1] = NULL;
	pStatic[1] = CreateStatic(rtTmp, this, szWaring);
	if (NULL == pStatic[1])
	{
		printf("%s pStatic[1] == NULL\n", __func__);
		fflush(stdout);
	}

	//printf("%s 1\n", __func__);
	//fflush(stdout);
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));

	//printf("%s 2\n", __func__);
	//fflush(stdout);
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));

	//printf("%s 3\n", __func__);
	//fflush(stdout);
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));

	//printf("%s 4\n", __func__);
	//fflush(stdout);
	//csp modify
	//rtTmp.left = m_Rect.Width() - 185;
	//rtTmp.right = rtTmp.left + 80;
	//rtTmp.top =  m_Rect.Height() - 40;
	//rtTmp.bottom = rtTmp.top + 22;
	
	SetMargin(0, 0, 0, 0);
	
	//csp modify
	//char* szButton[2] = 
	//{
	//	"Ok",
	//	"Cancle",
	//};
	
	/*
	int i = 0;
	for(i=0; i<ADVANCEWARNMENU_BUTTON; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageInfoBox::OnClkItem, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 10;
		rtTmp.right = rtTmp.left + 80;
	}
	*/
	
	//CRect recordRt = CRect(0, 0, 700, 400/*370*/);
	//m_pPageRecord = new CPageRecordFrameWork(NULL/*&recordRt*/, "  Record Config"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	//m_pPageRecordSchedule = new CPageRecordScheduleFrameWork(NULL/*&recordRt*/, "  Record Schedule"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	//m_pPageSensorAlarm = new CPageSensorAlarmFrameWork(NULL/*&recordRt*/, "  Sensor Alarm"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
}

CPageInfoBox::~CPageInfoBox()
{
	
}

void CPageInfoBox::SetOkCallBack(OKFUNC okFunc)
{
	m_okFunc = okFunc;
}

VD_PCSTR CPageInfoBox::GetDefualtTitleCenter()
{
	return "Basic Config";
}

void CPageInfoBox::OnClkItem()
{
	//printf("OnClkAlarmCfgItem \n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < ADVANCEWARNMENU_BUTTON; i++)
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
				if(m_okFunc != NULL)
				{
					m_okFunc();
				}

				printf("m_okFunc  \n");
			}break;
			case 1:
			{
				this->Close();
			}break;
			default:
			break;
		}
		
	}
}

VD_BOOL CPageInfoBox::MsgProc( uint msg, uint wpa, uint lpa )
{
	return TRUE;
}

