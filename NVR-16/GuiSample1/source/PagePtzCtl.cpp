#include "GUI/Pages/PagePtzCtl.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/BizData.h"
#include "Biz.h"


#define ROW_CHN_MAX	16
#define ROW_BTN_MAX	(ROW_CHN_MAX+1) // 一排最多的按钮个数

enum{//与下面的数组一一对应
	PTZ_BUTTON_BK,
	PTZ_BUTTON_UP,
	PTZ_BUTTON_DOWN,
	PTZ_BUTTON_LEFT,
	PTZ_BUTTON_RIGHT,
	PTZ_BUTTON_LEFTUP,
	PTZ_BUTTON_RIGHTUP,
	PTZ_BUTTON_LEFTDOWN,
	PTZ_BUTTON_RIGHTDOWN,
	PTZ_BUTTON_STOP,
	PTZ_BUTTON_DEC,
	PTZ_BUTTON_INC,
	PTZ_BUTTON_GOTO,
	PTZ_BUTTON_HIDE,
	PTZ_BUTTON_EXIT,
	PTZ_BUTTON_TRIANGLE,
	PTZ_BUTTON_NUM,
};


char* ptzShortcutBmpName[PTZ_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/ptz_ctrl_bk.bmp",	DATA_DIR"/temp/ptz_ctrl_bk.bmp"},
	{DATA_DIR"/temp/ptz_up.bmp",	DATA_DIR"/temp/ptz_up_f.bmp"},
	{DATA_DIR"/temp/ptz_down.bmp",	DATA_DIR"/temp/ptz_down_f.bmp"},
	{DATA_DIR"/temp/ptz_left.bmp",	DATA_DIR"/temp/ptz_left_f.bmp"},
	{DATA_DIR"/temp/ptz_right.bmp",	DATA_DIR"/temp/ptz_right_f.bmp"},
	{DATA_DIR"/temp/ptz_up_left.bmp",	DATA_DIR"/temp/ptz_up_left_f.bmp"},
	{DATA_DIR"/temp/ptz_up_right.bmp",	DATA_DIR"/temp/ptz_up_right_f.bmp"},
	{DATA_DIR"/temp/ptz_down_left.bmp",	DATA_DIR"/temp/ptz_down_left_f.bmp"},
	{DATA_DIR"/temp/ptz_down_right.bmp",	DATA_DIR"/temp/ptz_down_right_f.bmp"},
	{DATA_DIR"/temp/ptz_stop.bmp",	DATA_DIR"/temp/ptz_stop_f.bmp"},
	{DATA_DIR"/temp/btn_dec.bmp",	DATA_DIR"/temp/btn_dec_f.bmp"},
	{DATA_DIR"/temp/btn_add.bmp",	DATA_DIR"/temp/btn_add_f.bmp"},
	{DATA_DIR"/temp/goto.bmp",	DATA_DIR"/temp/goto_f.bmp"},
	{DATA_DIR"/temp/player_hide.bmp",	DATA_DIR"/temp/player_hide_f.bmp"},
	{DATA_DIR"/temp/exit_pb.bmp",	DATA_DIR"/temp/exit_pb_f.bmp"},
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
		
};

static VD_BITMAP* pBmpButtonNormal[PTZ_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[PTZ_BUTTON_NUM];

static VD_BITMAP* pBmp_button_normal;
static VD_BITMAP* pBmp_button_push;
static VD_BITMAP* pBmp_button_select;

static VD_BITMAP* pBmpPtzChn_normal;
static VD_BITMAP* pBmpPtzChn_push;
static VD_BITMAP* pBmpPtzChn_select;
int bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);

static int GetMaxChnNum() 
{ 
    return GetVideoMainNum(); 
}

int bizData_GetUartPara(uchar nChn, STabParaUart* pTabPara);

char* btnTextOK[3] = {
	"&CfgPtn.StartTrack",
	"&CfgPtn.StartAitpScan",
	"&CfgPtn.StartCruise",

};

char *btnTextStop[3] = {
    "&CfgPtn.StopTrack",
	"&CfgPtn.StopAitpScan",
	"&CfgPtn.StopCruise",
};

CPagePtzCtl::CPagePtzCtl( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent,psz)
{	
	nChMax = GetMaxChnNum();
    m_CurPTZCmd = EM_BIZPTZ_CMD_NULL;
    m_lastDirect = EM_BIZPTZ_CMD_NULL;
    curChn = -1;
	IsDwell = 0;
    bincurisepage = 0;
    //初始化按键初始状态
    psPtzCtlInfo = (SPtzCtlInfo *)malloc(sizeof(SPtzCtlInfo));
    psPtzCtlInfo->bitScan.set();  //1,置为可用状态
    psPtzCtlInfo->bitTour.set();
    psPtzCtlInfo->bitTrack.set();
    memset(psPtzCtlInfo->nPos, 0, sizeof(psPtzCtlInfo->nPos));
    
	m_bPtzCtlPage = TRUE;
	if(nChMax==4)
	{
        m_bPtzCtlPageDrawLine = TRUE;
	}
	else
	{
        m_bPtzCtlPageDrawLine = FALSE;
	}

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
	
	//m_lastDirect = EM_BIZPTZ_CMD_NULL;

	curChn = 0;    
    nPath = 0;
    nPresetSet = 0;
    bPatFlag = bScanFlag = bTourFlag = FALSE;
    bPatIsRunning = bScanIsRunning = bTourIsRunning = FALSE;

    nChMax = GetVideoMainNum();
    psGuiPtzPara = (SGuiPtzAdvancedPara *)malloc(sizeof(SGuiPtzAdvancedPara)*(nChMax+1));
	if(!psGuiPtzPara)
	{
		PUBPRT("Here");  //zlb20111117
		 exit(1);
	}

    psGuiPtzCount = (sGuiPtzAdvanPara *)malloc(sizeof(sGuiPtzAdvanPara)*(nChMax+1));
    if (!psGuiPtzCount)
    {
    	PUBPRT("Here");  //zlb20111117
        exit(1);
    }

	BIZ_DT_DBG("CPagePtzCtl1\n");

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmpPtzChn_normal = VD_LoadBitmap(DATA_DIR"/temp/ptz_chn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmpPtzChn_push = VD_LoadBitmap(DATA_DIR"/temp/ptz_chn_no.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmpPtzChn_select = VD_LoadBitmap(DATA_DIR"/temp/ptz_chn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
#if 1

	int nStart;
	int nBtnNum;
	int nWidth 		= pRect->right - pRect->left;
	int nBtnWidth 	= pBmpPtzChn_normal->width;
	if(nChMax < ROW_BTN_MAX)
	{	
		nCurChPg = 0;
		nChPgNum = 1;
		nLastHideCh = nChMax;
	
		nStart = nWidth-(nChMax)*nBtnWidth; //(nChMax+1)
		if(nStart<0)
		{
			nStart=0;
		}
		else
		{
			nStart=nStart>>1;
		}
		nBtnNum = nChMax+1;
		nRealRowBtn = nChMax;
	}
	else
	{
		nCurChPg = 0;
		nChPgNum = (nChMax+ROW_CHN_MAX-1) / ROW_CHN_MAX;
		nLastHideCh = ROW_CHN_MAX - (nChMax % ROW_CHN_MAX);
		
		nStart  = 1;
		nBtnNum = ROW_BTN_MAX;
		nRealRowBtn = ROW_CHN_MAX;
	}
	
	CRect rtTmp;
	rtTmp.left = nStart;
	rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	rtTmp.top = 1;
	rtTmp.bottom = rtTmp.top + pBmpPtzChn_normal->height;
	
	int i = 0;
	for(i=0; i<nBtnNum; i++)
	{
		int nBtnIdx = 0;
		if(i > 4)
		{
			nBtnIdx = i+22;
		}
		else
		{
			nBtnIdx = i;
		}
		
		if(i == nBtnNum-1)
		{
			if(i >= 16)
			{
				rtTmp.left = nWidth-pBmpPtzChn_normal->width-1;
				rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
			}
			pButton[nBtnIdx] = CreateButton(rtTmp, this, "&CfgPtn.All" , (CTRLPROC)&CPagePtzCtl::OnClkChnSel, NULL, buttonNormalBmp);
            
            //屏蔽掉ALL按键
            pButton[nBtnIdx]->Show(FALSE, FALSE);
		}
		else
		{
			char chnID[4] = {0};
			sprintf(chnID,"%d",i+1);
			pButton[nBtnIdx] = CreateButton(rtTmp, this, chnID , (CTRLPROC)&CPagePtzCtl::OnClkChnSel, NULL, buttonNormalBmp);
		}
		
		pButton[nBtnIdx]->SetBitmap(pBmpPtzChn_normal, pBmpPtzChn_select, pBmpPtzChn_select, pBmpPtzChn_push);

		rtTmp.left = rtTmp.right-1;
		rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	}

	//printf("ptz ctrl 0\n");
	
	if(nChMax>=ROW_BTN_MAX)
	{
		pBmpButtonNormal[PTZ_BUTTON_NUM-1] = VD_LoadBitmap(ptzShortcutBmpName[PTZ_BUTTON_NUM-1][0]);
		pBmpButtonSelect[PTZ_BUTTON_NUM-1] = VD_LoadBitmap(ptzShortcutBmpName[PTZ_BUTTON_NUM-1][1]);
		
		pButton[i+22] = CreateButton(CRect(
						nWidth-pBmpPtzChn_normal->width-6-pBmpButtonNormal[PTZ_BUTTON_NUM-1]->width, 
						rtTmp.top , 
						nWidth-pBmpPtzChn_normal->width-6, 
						rtTmp.top+pBmpButtonNormal[PTZ_BUTTON_NUM-1]->height), 
						this, NULL, (CTRLPROC)&CPagePtzCtl::OnBtnDown, NULL, buttonNormalBmp);

		pButton[i+22]->SetBitmap(pBmpButtonNormal[PTZ_BUTTON_NUM-1], pBmpButtonSelect[PTZ_BUTTON_NUM-1], pBmpButtonSelect[PTZ_BUTTON_NUM-1]);
	}
	
	//printf("ptz ctrl 1\n");
#else
	CRect rtTmp;
	rtTmp.left = 226;
	rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	rtTmp.top = 1;
	rtTmp.bottom = rtTmp.top + pBmpPtzChn_normal->height;
	
	int i = 0;
	for(i=0; i<5; i++)
	{
		if(i == 4)
		{
			pButton[i] = CreateButton(rtTmp, this, "&CfgPtn.All" , (CTRLPROC)&CPagePtzCtl::OnClkChnSel, NULL, buttonNormalBmp);
		}
		else
		{
			char chnID[4] = {0};
			sprintf(chnID,"%d",i+1);
			pButton[i] = CreateButton(rtTmp, this, chnID , (CTRLPROC)&CPagePtzCtl::OnClkChnSel, NULL, buttonNormalBmp);
		}
		
		pButton[i]->SetBitmap(pBmpPtzChn_normal, pBmpPtzChn_select, pBmpPtzChn_select, pBmpPtzChn_push);
			
		rtTmp.left = rtTmp.right + 1;
		rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	}
#endif
	//utton[curChn]->Enable(FALSE);

	pBmpButtonNormal[0] = VD_LoadBitmap(ptzShortcutBmpName[0][0]);
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	int baseX = 8;
	int baseY = rtTmp.bottom + 5;
	
	rtTmp.left = baseX;
	rtTmp.right = baseX + pBmpButtonNormal[0]->width;
	rtTmp.top = baseY;
	rtTmp.bottom = baseY + pBmpButtonNormal[0]->height;
	pStatic[0] =  CreateStatic(rtTmp, this, pBmpButtonNormal[0],0);

	//up
	pBmpButtonNormal[1] = VD_LoadBitmap(ptzShortcutBmpName[1][0]);
	ReplaceBitmapBits(pBmpButtonNormal[1], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[1] = VD_LoadBitmap(ptzShortcutBmpName[1][1]);
	ReplaceBitmapBits(pBmpButtonSelect[1], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+43; //53;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[1]->width;
	rtTmp.top = baseY+12;//65;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[1]->height;
	pButton[5] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[5]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);

	//down
	pBmpButtonNormal[2] = VD_LoadBitmap(ptzShortcutBmpName[2][0]);
	ReplaceBitmapBits(pBmpButtonNormal[2], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[2] = VD_LoadBitmap(ptzShortcutBmpName[2][1]);
	ReplaceBitmapBits(pBmpButtonSelect[2], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+43; //53;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[2]->width;
	rtTmp.top = baseY+74;//127;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[2]->height;
	pButton[6] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[6]->SetBitmap(pBmpButtonNormal[2], pBmpButtonSelect[2], pBmpButtonSelect[2]);

	//left
	pBmpButtonNormal[3] = VD_LoadBitmap(ptzShortcutBmpName[3][0]);
	ReplaceBitmapBits(pBmpButtonNormal[3], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[3] = VD_LoadBitmap(ptzShortcutBmpName[3][1]);
	ReplaceBitmapBits(pBmpButtonSelect[3], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+12; //22;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[3]->width;
	rtTmp.top = baseY+42; //95;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[3]->height;
	pButton[7] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[7]->SetBitmap(pBmpButtonNormal[3], pBmpButtonSelect[3], pBmpButtonSelect[3]);

	//right
	pBmpButtonNormal[4] = VD_LoadBitmap(ptzShortcutBmpName[4][0]);
	ReplaceBitmapBits(pBmpButtonNormal[4], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[4] = VD_LoadBitmap(ptzShortcutBmpName[4][1]);
	ReplaceBitmapBits(pBmpButtonSelect[4], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+73; //83;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[4]->width;
	rtTmp.top = baseY+42; //95;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[4]->height;
	pButton[8] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[8]->SetBitmap(pBmpButtonNormal[4], pBmpButtonSelect[4], pBmpButtonSelect[4]);

	//left up
	pBmpButtonNormal[5] = VD_LoadBitmap(ptzShortcutBmpName[5][0]);
	ReplaceBitmapBits(pBmpButtonNormal[5], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[5] = VD_LoadBitmap(ptzShortcutBmpName[5][1]);
	ReplaceBitmapBits(pBmpButtonSelect[5], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+23; //33;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[5]->width;
	rtTmp.top = baseY+21; //74;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[5]->height;
	pButton[9] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[9]->SetBitmap(pBmpButtonNormal[5], pBmpButtonSelect[5], pBmpButtonSelect[5]);

	//right up
	pBmpButtonNormal[6] = VD_LoadBitmap(ptzShortcutBmpName[6][0]);
	ReplaceBitmapBits(pBmpButtonNormal[6], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[6] = VD_LoadBitmap(ptzShortcutBmpName[6][1]);
	ReplaceBitmapBits(pBmpButtonSelect[6], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+63;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[6]->width;
	rtTmp.top = baseY+21;//74;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[6]->height;
	pButton[10] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[10]->SetBitmap(pBmpButtonNormal[6], pBmpButtonSelect[6], pBmpButtonSelect[6]);

	//left down
	pBmpButtonNormal[7] = VD_LoadBitmap(ptzShortcutBmpName[7][0]);
	ReplaceBitmapBits(pBmpButtonNormal[7], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[7] = VD_LoadBitmap(ptzShortcutBmpName[7][1]);
	ReplaceBitmapBits(pBmpButtonSelect[7], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+22;//32;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[7]->width;
	rtTmp.top = baseY+64;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[7]->height;
	pButton[11] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[11]->SetBitmap(pBmpButtonNormal[7], pBmpButtonSelect[7], pBmpButtonSelect[7]);
			
	//right down	
	pBmpButtonNormal[8] = VD_LoadBitmap(ptzShortcutBmpName[8][0]);
	ReplaceBitmapBits(pBmpButtonNormal[8], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[8] = VD_LoadBitmap(ptzShortcutBmpName[8][1]);
	ReplaceBitmapBits(pBmpButtonSelect[8], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+63;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[8]->width;
	rtTmp.top = baseY+64;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[8]->height;
	pButton[12] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[12]->SetBitmap(pBmpButtonNormal[8], pBmpButtonSelect[8], pBmpButtonSelect[8]);

	//stop
	pBmpButtonNormal[9] = VD_LoadBitmap(ptzShortcutBmpName[9][0]);
	ReplaceBitmapBits(pBmpButtonNormal[9], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[9] = VD_LoadBitmap(ptzShortcutBmpName[9][1]);
	ReplaceBitmapBits(pBmpButtonSelect[9], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+42;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[9]->width;
	rtTmp.top = baseY+42;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[9]->height;
	pButton[13] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[13]->SetBitmap(pBmpButtonNormal[9], pBmpButtonSelect[9], pBmpButtonSelect[9]);
		

	pBmpButtonNormal[10] = VD_LoadBitmap(ptzShortcutBmpName[10][0]);
	ReplaceBitmapBits(pBmpButtonNormal[10], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[10] = VD_LoadBitmap(ptzShortcutBmpName[10][1]);
	ReplaceBitmapBits(pBmpButtonSelect[10], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonNormal[11] = VD_LoadBitmap(ptzShortcutBmpName[11][0]);
	ReplaceBitmapBits(pBmpButtonNormal[11], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[11] = VD_LoadBitmap(ptzShortcutBmpName[11][1]);
	ReplaceBitmapBits(pBmpButtonSelect[11], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	char* staticText[7] = {
		"&CfgPtn.Zoom",
		"&CfgPtn.Focus",
		"&CfgPtn.Iris",
		"&CfgPtn.Preset",
		"&CfgPtn.Cruise",
		"&CfgPtn.Speed",
		"5",
	};
	
	rtTmp.left = 113;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[10]->width;
	rtTmp.top = 54;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
	int j = 0;
	for(i = 14; i < 17; i++)
	{
		pButton[i+j] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzCtl::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+j]->SetBitmap(pBmpButtonNormal[10], pBmpButtonSelect[10], pBmpButtonSelect[10]);

		rtTmp.left = rtTmp.right;
		rtTmp.right = rtTmp.left + 73;
		pStatic[i-13] = CreateStatic(rtTmp, this, staticText[i-14]);
		pStatic[i-13]->SetTextAlign(VD_TA_CENTER);

		rtTmp.left = rtTmp.right;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[11]->width;

		pButton[i+j+1] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzCtl::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+j+1]->SetBitmap(pBmpButtonNormal[11], pBmpButtonSelect[11], pBmpButtonSelect[11]);

		rtTmp.left = 113;//73;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[10]->width;
		rtTmp.top = rtTmp.top + pBmpButtonNormal[10]->height +8;//117;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
		j++;
	}

	//printf("ptz ctrl 2\n");
	
	rtTmp.left = 270;//73;
	rtTmp.right = rtTmp.left + 85;
	rtTmp.top = 53+pBmpButtonNormal[10]->height + 8;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
	
	pBmpButtonNormal[12] = VD_LoadBitmap(ptzShortcutBmpName[12][0]);
	ReplaceBitmapBits(pBmpButtonNormal[12], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[12] = VD_LoadBitmap(ptzShortcutBmpName[12][1]);
	ReplaceBitmapBits(pBmpButtonSelect[12], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	for(i=0; i<2 ;i++)
	{
		pStatic[i+4] = CreateStatic(rtTmp, this, staticText[i+3]);
		pStatic[i+4]->SetTextAlign(VD_TA_LEFT);

		if(i == 1)
		{
			pComboBox[i] = CreateComboBox(CRect(rtTmp.right-20,rtTmp.top+1,rtTmp.right+80,rtTmp.bottom-1),
				this, NULL, NULL, (CTRLPROC)&CPagePtzCtl::OnComboSel, 0);
		}
		else
		{
			pComboBox[i] = CreateComboBox(CRect(rtTmp.right-20,rtTmp.top+1,rtTmp.right+80,rtTmp.bottom-1),
				this, NULL, NULL, NULL, 0);
		}

		switch (i)
		{
			case 0:
			{
                /*
				pComboBox[i]->AddString("preset001");
				pComboBox[i]->AddString("preset002");
				pComboBox[i]->AddString("preset003");
				pComboBox[i]->AddString("preset004");
				pComboBox[i]->AddString("preset005");
				*/
				/*
				char s[10] = {0};
				for (int j = 0; j<nPath; j++)
				{
                    sprintf(s,"%d",j+1);
		            pComboBox[i]->AddString(s);
                }
				pComboBox[i]->SetCurSel(0);
				*/
			}break;
			case 1:
			{
                /*
				pComboBox[i]->AddString("line001");
				pComboBox[i]->AddString("line002");
				pComboBox[i]->AddString("line003");
				pComboBox[i]->AddString("line004");
				pComboBox[i]->AddString("line005");
				*/
				/*
				char s[10] = {0};
				for (int j = 0; j<nPresetSet; j++)
				{
                    sprintf(s,"%d",j+1);
		            pComboBox[i]->AddString(s);
                }
				pComboBox[i]->SetCurSel(0);
				*/
			}break;
			default:
				break;
		}
		
		pButton[i+20] = CreateButton(CRect(rtTmp.right+90,rtTmp.top,rtTmp.right+90+pBmpButtonNormal[12]->width,rtTmp.bottom ), this, NULL, (CTRLPROC)&CPagePtzCtl::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+20]->SetBitmap(pBmpButtonNormal[12], pBmpButtonSelect[12], pBmpButtonSelect[12]);
		
		rtTmp.left = 270;//73;
		rtTmp.right = rtTmp.left + 85;
		rtTmp.top = rtTmp.top +pBmpButtonNormal[10]->height + 8;//117;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
	}

	//printf("ptz ctrl 3\n");
	
	char* btnText[3] = {
		"&CfgPtn.StartTrack",
		"&CfgPtn.StartAitpScan",
		"&CfgPtn.StartCruise",
	};
	
	rtTmp.left = 490;//73;
	rtTmp.right = rtTmp.left + 136;
	rtTmp.top = 54;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height - 2; //减去2可以减少闪的效果
	
	for(i=0; i<3; i++)
	{
		pButton[i+22] = CreateButton(rtTmp, this, btnText[i], (CTRLPROC)&CPagePtzCtl::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+22]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);

		rtTmp.top = rtTmp.top+pBmp_button_normal->height+8;
		rtTmp.bottom = rtTmp.top+pBmp_button_normal->height;		
	}

	rtTmp.left = 30;
	rtTmp.right = rtTmp.left + 50;
	rtTmp.top = m_Rect.Height()- 27;
	rtTmp.bottom = rtTmp.top + 20;
	pStatic[6] = CreateStatic(rtTmp, this, staticText[5]);
	pStatic[6]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left = rtTmp.right;
	rtTmp.right = rtTmp.left + 15;
	pStatic[7] = CreateStatic(rtTmp, this, staticText[6]);
	pStatic[7]->SetTextAlign(VD_TA_CENTER);
	
	rtTmp.left = 113;//73;
	rtTmp.right = rtTmp.left + 128;
	rtTmp.top = m_Rect.Height()- 27;
	rtTmp.bottom = rtTmp.top + 20;

	pSlider = CreateSliderCtrl(rtTmp,this,1, 10, (CTRLPROC)&CPagePtzCtl::OnSliderChange, sliderNoNum);
    pSlider->SetPos(5); //设置焦点为5

	for(i=0; i<2; i++)
	{
		pBmpButtonNormal[i+13] = VD_LoadBitmap(ptzShortcutBmpName[i+13][0]);
		ReplaceBitmapBits(pBmpButtonNormal[i+13], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
		pBmpButtonSelect[i+13] = VD_LoadBitmap(ptzShortcutBmpName[i+13][1]);
		ReplaceBitmapBits(pBmpButtonSelect[i+13], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

		rtTmp.left = 560 + i*pBmpButtonNormal[i+13]->width + 10 ;//73;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[i+13]->width;
		rtTmp.top = m_Rect.Height()- 28;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[i+13]->height;

		pButton[i+25] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzCtl::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+25]->SetBitmap(pBmpButtonNormal[i+13], pBmpButtonSelect[i+13], pBmpButtonSelect[i+13]);
	}
	
	BIZ_DT_DBG("CPagePtzCtl2\n");
}


void CPagePtzCtl::ShowBtn()
{
	int nChIdx;
#if 0	
	for(int i=0; i<nRealRowBtn; i++)
	{
		char szCh[4]={0};
		sprintf(szCh, "%d", i+nCurChPg*ROW_CHN_MAX+1);
		nChIdx = i;
		if(i>4) nChIdx += 22;
		
		pButton[nChIdx]->SetText(szCh);
		pButton[nChIdx]->Show(TRUE,TRUE);
	}
	
	if(nCurChPg == nChPgNum-1 && nChPgNum > 1)
	{		
		for(int i=nLastHideCh; i<ROW_CHN_MAX; i++)
		{
			nChIdx = i+ROW_CHN_MAX-nLastHideCh;
			if(nChIdx>4) nChIdx += 22;
			
			pButton[nChIdx]->Show(FALSE,TRUE);
		}
	}
#else
	for(int i=0; i < nRealRowBtn; i++)
	{
		char szCh[4]={0};
		sprintf(szCh, "%d", i+nCurChPg*ROW_CHN_MAX+1);
		nChIdx = i;
		if(i > 4) nChIdx += 22;
		
		pButton[nChIdx]->Show(FALSE,TRUE);
		pButton[nChIdx]->SetText(szCh);		
	}
	
	if(nCurChPg == nChPgNum-1 && nChPgNum > 1)
	{		
		for(int i=0; i < nLastHideCh; i++)
		{
			nChIdx = i;
			if(nChIdx > 4)
            {
                nChIdx += 22;
			}
			
			pButton[nChIdx]->Show(TRUE,TRUE);
		}
	}
	else
	{
		for(int i=0; i<nRealRowBtn; i++)
		{
			nChIdx = i;
			if(nChIdx>4) 
            {
                nChIdx += 22;
			}
			
			pButton[nChIdx]->Show(TRUE,TRUE);
		}
	}
#endif
}
	
void CPagePtzCtl::OnBtnDown()
{
	nCurChPg += 1;
	if(nCurChPg >= nChPgNum)
	{
		nCurChPg = 0;
	}

    /*
	int nBtnIdx = curChn%nRealRowBtn;
	if(curChn != nCurChPg*nRealRowBtn+nBtnIdx )
	{
		nBtnIdx = (nBtnIdx>4)?(nBtnIdx+22):nBtnIdx;
		pButton[nBtnIdx]->Enable(TRUE);
	}
	else
	{
		nBtnIdx = (nBtnIdx>4)?(nBtnIdx+22):nBtnIdx;
		pButton[nBtnIdx]->Enable(FALSE);
	}
	*/
	pButton[nLastClkBtn]->Enable(TRUE);
    
	ShowBtn();
	
	return;
}

void CPagePtzCtl::OnComboSel()
{
	/*int sel = pComboBox[1]->GetCurSel();
	char* line = pComboBox[1]->GetString(sel);
	char* pathNo = line + 4;
	int no = 0;
	sscanf(pathNo,"%d",&no);
	*/

	//SetPresetList();	
}


void CPagePtzCtl::OnClkOtherBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 14; i < 27; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		if((curChn<0) && (i < 25))
		{
            MessgeBox();
            return ;
		}

		switch(i)
		{
			case 20:
			{
                if (!ChnPtzIsAble(curChn))
                {
                    printf("***Chn is Error Or Ptz is not Open***\n");
                    return;
                }

                //全部按键可能状态时才可以执行
                if (TRUE == AllBtmEnable())
                {
    				//printf("preset position  goto\n"); //跳至预置点N
    				//测试选项是否为空
    				ncurPreSit = this->pComboBox[0]->GetCount()?this->pComboBox[0]->GetCurSel():-1;
                    if (-1 == ncurPreSit)
                    {
                        printf("Preset is NULL\n");
                        return;
                    }
                    
                    ncurPreSit = atoi(this->pComboBox[0]->GetString(ncurPreSit));
                    if (-1 != ncurPreSit)
                    {
                        //printf("preset position  goto chn:[%d]-[%d]\n", (u8)curChn, ncurPreSit);
    				    BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_PRESET_GOTO, ncurPreSit);
                    }
                }				
			}break;
			case 21:
			{
                if (!ChnPtzIsAble(curChn))
                {
                    printf("***Chn is Error Or Ptz is not Open***\n");
                    return;
                }
                //跳至巡航线N
                //开始巡航按键可用时
				if (TRUE == BtmEnable(EM_PTZ_BTM_TOUR))
                {
    				ncurCirLine = this->pComboBox[1]->GetCount()?this->pComboBox[1]->GetCurSel():-1;
                    if (-1 == ncurCirLine)
                    {
                        printf("CruiseLine is NULL\n");
                        return;
                    }
                    
                    nFixCruiseNu = atoi(this->pComboBox[1]->GetString(ncurCirLine)) - 1;                    
                    if (-1 != nFixCruiseNu)
                    {
                        psPtzCtlInfo->nPos[curChn] = nFixCruiseNu;
                        printf("circle line  goto chn:[%d]-[%d]\n", (u8)curChn, nFixCruiseNu); 
    				    SetCtlBtmStatusCtl(EM_PTZ_BTM_GOTOTOUR);
                    }
				}	
			}break;
			case 22:
			{
                if (!ChnPtzIsAble(curChn))
                {
                    printf("***Chn is Error Or Ptz is not Open***\n");
                    return;
                }

                //开始轨迹
                psPtzCtlInfo->nPos[curChn] = 0;

                SetCtlBtmStatusCtl(EM_PTZ_BTM_TRACK);
			}break;
			case 23:
			{
                if (!ChnPtzIsAble(curChn))
                {
                    printf("***Chn is Error Or Ptz is not Open***\n");
                    return;
                }
                
                //开始自动扫描
                psPtzCtlInfo->nPos[curChn] = 0;

                SetCtlBtmStatusCtl(EM_PTZ_BTM_SCAN);
			}break;
			case 24:
			{
                if (!ChnPtzIsAble(curChn))
                {
                    printf("***Chn is Error Or Ptz is not Open***\n");
                    return;
                }
                
                //开始巡航                
                ncurCirLine = this->pComboBox[1]->GetCount()?this->pComboBox[1]->GetCurSel():-1;                
                if (-1 == ncurCirLine)
                {
                    printf("CruiseLine is NULL\n");
                    return;
                }
                
                nFixCruiseNu = atoi(this->pComboBox[1]->GetString(ncurCirLine)) - 1;                
                if (-1 != nFixCruiseNu)
                {
                    psPtzCtlInfo->nPos[curChn] = nFixCruiseNu;
                    SetCtlBtmStatusCtl(EM_PTZ_BTM_TOUR);
                }
			}break;
			case 25:
			{
				//printf("hide\n");
		        HideWindow();				
			}break;
			case 26:
			{
				//printf("exit\n"); //退出
                pButton[nLastClkBtn]->Enable(TRUE);
				if (-1 != curChn)
                {
				    SetCurPreviewMode_CW(eMaxViewMode);//cw_preview
			        SwitchPreview(eMaxViewMode, 0);
                }
				BizSetZoomMode(0);
			
                this->Close();
				if(IsDwell)
				{
					usleep(500 * 1000);
					ControlPatrol(1);
					IsDwell = 0;
				}
			}break;
			default:
			break;
		}		
	}
}


void CPagePtzCtl::OnClkChnSel()
{
	int i = 0;
	BOOL bFind = FALSE;
	int nChIdx;
			
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < nRealRowBtn; i++)
	{
		if(i > 4)
		{
			nChIdx = i+22;
		}
		else
		{
			nChIdx = i;
		}
		
		if (pFocusButton == pButton[nChIdx])
		{
			bFind = TRUE;
			break;
		}
	}

	int nBtnIdx = i+nCurChPg*nRealRowBtn;
	u8  chn 	= nChMax;
	u32 cur 	= i;
	if(i == nRealRowBtn)
	{
		cur = curChn;
    }
	else
	{
		cur = nBtnIdx;
	}
						
	if (bFind)
	{
		#if 0
		printf("## switch channel %d\n", i);
		
		if(i != nRealRowBtn)
		{			
			SwitchPreview(EM_BIZPREVIEW_1SPLIT, nBtnIdx);

			int nTmp = curChn%nRealRowBtn; 
			if( nTmp>4 )
			{
				pButton[nTmp+22]->Enable(TRUE);				
			}
			else
			{			
				pButton[nTmp]->Enable(TRUE);
			}
			
			curChn = nBtnIdx;
			pButton[nChIdx]->Enable(FALSE);
		}
		else
		{
			SwitchPreview(eMaxViewMode, 0);			
		}
		#else
        //恢复上一个点击的按键可用
        /*
        int nTmp = curChn%nRealRowBtn;
        if( nTmp>4 )
        {
            nLastClkBtn = nTmp+22;			
        }
        else
        {
            nLastClkBtn = nTmp;
        }
        */
        pButton[nLastClkBtn]->Enable(TRUE);
			
        //点击哪个通道combobox更新对应通道选项
        if (i != nRealRowBtn)//nChMax
        {
            //选择通道事件,全频显示选中通道画面
            SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
            SwitchPreview(EM_BIZPREVIEW_1SPLIT, nBtnIdx);
			
			nLastClkBtn = nChIdx;
            
            //获得真实通道名号
            curChn = (u8)strtol(pFocusButton->GetText(), 0, 10) - 1;

            //选中对应通道时设置一下云台速率
            BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, (s16)pSlider->GetPos());

            //更新对应通道按键真实状态
            BtmStatusUpdate(curChn);
            
            //获得通道是否开启
            bizData_GetUartPara(curChn, &TabPara);

            //点击的通道号变为不可用
			pButton[nChIdx]->Enable(FALSE);
			//printf("***nCurChPg = %d, ButtomId = %d\n", nCurChPg, nChIdx);
            //
            char s[10] = {0};
            pComboBox[0]->RemoveAll();
            for (int j = 0; j<TOURPIONT_MAX; j++)
            {
                if (1 == psGuiPtzCount[cur].nPresetCount[j])
                {
                    sprintf(s, "%d", j+1);
                    pComboBox[0]->AddString(s);
                }
            }
            
            if (!pComboBox[0]->GetCount())
            {
                pComboBox[0]->AddString("&CfgPtn.NONE");
                pComboBox[0]->Enable(FALSE);
            }
            else
            {
                pComboBox[0]->Enable(TRUE);
            }
            pComboBox[0]->SetCurSel(0);

            pComboBox[1]->RemoveAll();
            for (int j = 0; j<TOURPATH_MAX; j++)
            {
                if (1 == psGuiPtzCount[cur].nTourPathCount[j])
                {
                    sprintf(s, "%d", j+1);
                    pComboBox[1]->AddString(s);
                }
            }
            
            if (!pComboBox[1]->GetCount())
            {
                pComboBox[1]->AddString("&CfgPtn.NONE");
                pComboBox[1]->Enable(FALSE);
            }
            else
            {
                pComboBox[1]->Enable(TRUE);
            }
            pComboBox[1]->SetCurSel(0);
        }
        else
        {
            SetCurPreviewMode_CW(eMaxViewMode);//cw_preview
            SwitchPreview(eMaxViewMode, 0);
            pButton[nRealRowBtn]->Enable(TRUE);//全部按键一直可用
            pComboBox[0]->RemoveAll();
            pComboBox[1]->RemoveAll();
        }
		#endif
	}
}

void CPagePtzCtl::OnSliderChange()
{
	//printf("OnSliderChange\n");
	char strVal[4] = {0};
	int value = pSlider->GetPos();
	sprintf(strVal,"%d",value);
    //printf("Slider value: %d\n", value);
	
	pStatic[7]->SetText(strVal);
    if (!ChnPtzIsAble(curChn))
    {
        printf("***Chn is Error Or Ptz is not Open***\n");
        return;
    }	
	
    BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, value);
}

void CPagePtzCtl::OnClkDirection()
{
	printf("OnClkDirection left\n");
	if(curChn < 0)
	{
		return ;
	}
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 5; i < 14; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		switch(i)
		{
			case 5:
			{
				//printf("up\n");
				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TILEUP, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_TILEUP;
			} break;
			case 6:
			{
				//printf("down\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TILEDOWN, 0); 
				m_lastDirect = EM_BIZPTZ_CMD_START_TILEDOWN;
			} break;
			case 7:
			{
				//printf("OnClkDirection left\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_PANLEFT, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_PANLEFT;
			} break;
			case 8:
			{
				//printf("right\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_PANRIGHT, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_PANRIGHT;
			} break;
			case 9:
			{
				//printf("left up\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_LEFTUP, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_LEFTUP;
			} break;
			case 10:
			{
				//printf("right up\n");
				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_RIGHTUP, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_RIGHTUP;
			} break;
			case 11:
			{
				//printf("left down\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_LEFTDOWN, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_LEFTDOWN;
			} break;
			case 12:
			{
				//printf("right down\n");

				BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_RIGHTDOWN, 0);
				m_lastDirect = EM_BIZPTZ_CMD_START_RIGHTDOWN;
			} break;
			case 13:
			{
				//printf("stop\n");

				BizPtzCtrl((u8)curChn, (EMBIZPTZCMD)(m_lastDirect+30), 0);
			} break;
			default:
			break;
		}		
	}
}

CPagePtzCtl::~CPagePtzCtl()
{

}

void CPagePtzCtl::SetPresetList()
{
	int chn = curChn;
	
	int sel = pComboBox[1]->GetCurSel();
	char* line = pComboBox[1]->GetString(sel);
	char* p = line + 4;
	int pathNo = 0;
	sscanf(p,"%d",&pathNo);

	SValue presetList[128];
	int presetReal = 0;
	GetPresetList(chn, pathNo, presetList, &presetReal, 128);

	pComboBox[0]->RemoveAll();
	for(int i=0; i<presetReal; i++)
	{
		pComboBox[0]->AddString(presetList[i].strDisplay);
	}

	pComboBox[0]->SetCurSel(0);
}


VD_BOOL CPagePtzCtl::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
        //默认是没有选中通道数,应该所有控制按钮显示为可用
        pButton[EM_PTZ_BTM_TRACK]->SetText(btnTextOK[0]);
        pButton[EM_PTZ_BTM_SCAN]->SetText(btnTextOK[1]);
        pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextOK[2]);
        
		BizSetZoomMode(1);
		if(GetDwellStartFlag())
		{
			IsDwell = 1;
			ControlPatrol(0);
		}
        
        m_bPlayBackHide = FALSE;
        m_bPlayBackPage = FALSE;
        curChn = -1;
        
        memset(psGuiPtzPara, 0, sizeof(SGuiPtzAdvancedPara));
        //memset(psGuiPtzCount, 0, sizeof(psGuiPtzCount)*(nChMax+1));

		int nChIdx;
		for (int i = 0; i < nRealRowBtn; i++)
		{
			if(i>4)
			{
				nChIdx = i+22;
			}
			else
			{
				nChIdx = i;
			}

			pButton[nChIdx]->Enable(TRUE);
		}
        
        for (int i = 0; i<nChMax; i++)
        {
            bizData_GetPtzAdvancedPara((uchar)i, &psGuiPtzPara[i]);
            memset(&psGuiPtzCount[i], 0, sizeof(psGuiPtzCount[i]));
            
            for (int j = 0; j<TOURPATH_MAX; j++)
            {
                if (psGuiPtzPara[i].sTourPath[j].nPathNo)
                {
                    psGuiPtzCount[i].nTourPathCount[j] = 1;
                }
            }
			for (int j = 0; j<TOURPIONT_MAX; j++)
            {
                if (psGuiPtzPara[i].nIsPresetSet[j])
                {
                    psGuiPtzCount[i].nPresetCount[j] = 1;
                }
            }
        }
        pComboBox[0]->RemoveAll();
        pComboBox[0]->AddString("&CfgPtn.NONE");
        pComboBox[0]->SetCurSel(0);
        pComboBox[0]->Enable(FALSE);
        
        pComboBox[1]->RemoveAll();
        pComboBox[1]->AddString("&CfgPtn.NONE");
        pComboBox[1]->SetCurSel(0);
        pComboBox[1]->Enable(FALSE);
        
        bLeftBtnCtlPtz = FALSE;
        nLastClkBtn = 0;
        m_CurPTZCmd = EM_BIZPTZ_CMD_NULL;
        m_lastDirect = EM_BIZPTZ_CMD_NULL;
	}
	
	return TRUE;
}

void CPagePtzCtl::SetCruiseStage(int flag)
{
	bincurisepage=flag;
}

int CPagePtzCtl::GetCruiseState()
{
	return bincurisepage;
}

VD_BOOL CPagePtzCtl::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px,py;
    int key;
	static int keyStatus=0;

	//printf("yg MsgProc msg: 0x%x\n", msg);
	switch(msg)
    {
    	case XM_KEYDOWN:		//按键按下   //cw_panel
    		{
			/*
    			key = wpa;
    			 if (m_bPlayBackHide)
    			 {
    			    switch(key)
                    	    {
        			    case KEY_RET:		//确认键
    		          	    {
    			                m_TmpRect.left = (m_RectScreen.right-636)/2;
    			                m_TmpRect.right = m_TmpRect.left + 636;
    			                m_TmpRect.top = m_RectScreen.bottom - 212 ;
    			                m_TmpRect.bottom = m_TmpRect.top + 182;
        	   			        this->SetRect(&m_TmpRect,TRUE);
        					    m_bPlayBackHide = FALSE;
        			 		    return 0;
    		            	}
        				break;
        				default:
        				return 0;
        			}
                	 }    	
			 else
			 */
    		 	if (wpa==KEY_RET)
    			{
	                CItem* temp = GetFocusItem();
	                if (temp == pComboBox[1]
	                     && pComboBox[1]->GetListBoxStatus())
	                {
	                    return CPageFloat::MsgProc(msg, wpa, lpa)?ReflushChnName():FALSE;
	                }
    						
    				if (keyStatus==0)
    				{
    					keyStatus=1;
					//printf("yg MsgProc goto PTZEnter\n");
    					goto PTZEnter;
    				}
    				else
    				{
    					keyStatus=0;
					//printf("yg MsgProc goto PTZDown\n");
    					goto PTZDown;
    				}
    			}
                
    			if (wpa==KEY_ESC)
                {
                    goto PTZESC;
                }
    			else
    			{
    				break;
    			}            
    		} break;
    	case XM_LBUTTONDOWN:
       	case XM_LBUTTONDBLCLK:
            //Modify by Lirl on Feb/13/2012
            //不管combox的listbox是否都显示出来,未覆盖完整的都将完全能够显示出来
	//printf("yg MsgProc XM_LBUTTONDOWN chn: %d\n", curChn);
	{
                CItem* temp = GetFocusItem();

                //when the mouse left buttom is down, if the listbox is't exist, and reflash name immediately
                if (temp == pComboBox[1]
                     && pComboBox[1]->GetListBoxStatus())
                {
                    //call the ReflushChnName() must be after call CPageFloat::MsgProc(msg, wpa, lpa)
                    return CPageFloat::MsgProc(msg, wpa, lpa)?ReflushChnName():FALSE;
                }
            }
            //end
            //printf("yg MsgProc PTZEnter\n");
		PTZEnter:
            if (m_bPlayBackHide)
            {
                m_TmpRect.left = (m_RectScreen.right-636)/2;
                m_TmpRect.right = m_TmpRect.left + 636;
                m_TmpRect.top = m_RectScreen.bottom - 212 ;
                m_TmpRect.bottom = m_TmpRect.top + 182;
            	
    		    this->SetRect(&m_TmpRect,TRUE);
    		    m_bPlayBackHide = FALSE;
				
    		    return 0;
            }
            else
            {                
                /*
                px = VD_HIWORD(lpa);
    			py = VD_LOWORD(lpa);
                CItem* temp = GetItemAt(px, py);
	            */
		        CItem* temp = GetFocusItem();
            	
                if (temp == pButton[5])
                { //up
                    //printf("***up\n");
                    m_CurPTZCmd = EM_BIZPTZ_CMD_START_TILEUP;
                }
                else if (temp == pButton[6])
                { //down
                    //printf("***down\n");
                    m_CurPTZCmd = EM_BIZPTZ_CMD_START_TILEDOWN;
                }
                else if (temp == pButton[7])
                { //left
                   //printf("***left\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_PANLEFT;
                }
                else if (temp == pButton[8])
                { //right
                   //printf("***right\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_PANRIGHT;
                }
                else if (temp == pButton[9])
                { //left up
                   //printf("***left up\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_LEFTUP;
                }
                else if (temp == pButton[10])
                { //right up
                   //printf("***right up\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_RIGHTUP;
                }
                else if (temp == pButton[11])
                { //left down
                   //printf("***left down\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_LEFTDOWN;
                }
                else if (temp == pButton[12])
                { //right down
                   //printf("***right down\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_RIGHTDOWN;
                }
                else if (temp == pButton[13])
                {  //stop
                   //printf("***stop\n");
                   return TRUE;
                   m_CurPTZCmd = EM_BIZPTZ_CMD_STOP_ALL;
                }
                else if (temp == pButton[14])
                {  //zoom --
                   //printf("***zoom --\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_ZOOMWIDE;
                }
                else if (temp == pButton[15])
                {  //zoom ++
                   //printf("***zoom ++\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_ZOOMTELE;
                }
                else if (temp == pButton[16])
                {  //focus --
                   //printf("***focus --\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_FOCUSNEAR;
                }
                else if (temp == pButton[17])

                {  //focus ++
                   //printf("***focus ++\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_FOCUSFAR;
                }
                else if (temp == pButton[18])
                {  //aperture --
                   //printf("***aperture --\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_IRISSMALL;
                }
                else if (temp == pButton[19])
                {  //aperture ++
                   //printf("***aperture ++\n");
                   m_CurPTZCmd = EM_BIZPTZ_CMD_START_IRISLARGE;
                }
                else if (temp == pSlider)
                { 
                    //slider
                    if (-1 == curChn)
                    {
                        MessgeBox();
                        return TRUE;
                    }
                    return CPageFloat::MsgProc(msg, wpa, lpa);
                }
                else
                {
                    return CPageFloat::MsgProc(msg, wpa, lpa);
                }

                if (-1 == curChn)
                {
                    MessgeBox();
                    return TRUE;
                }

		//printf("yg ptz MsgProc chn: %d, cmd: %d\n", curChn, m_CurPTZCmd);
                BizPtzCtrl((u8)curChn, m_CurPTZCmd, 0);
                m_lastDirect = m_CurPTZCmd;
                bLeftBtnCtlPtz = TRUE;
		//printf("yg MsgProc XM_LBUTTONDOWN 1\n");	
                //printf("***%d\n", CPageFloat::MsgProc(msg, wpa, lpa));
                //return CPageFloat::MsgProc(msg, wpa, lpa)?ReflushChnName():FALSE;
            }
            break;
        case XM_LBUTTONUP:
           PTZDown:
		//printf("yg MsgProc XM_LBUTTONUP, chn: %d, m_lastDirect: %d\n", curChn, m_lastDirect);
            {
                if (bLeftBtnCtlPtz)
                {
                    if ((int)m_lastDirect>=0 && (int)m_lastDirect<=253-30)
                    {
                        BizPtzCtrl((u8)curChn, (EMBIZPTZCMD)(m_lastDirect+30), 0);
                    }
                    bLeftBtnCtlPtz = FALSE;
                }
                /*
                CItem* temp = GetFocusItem();

                if (temp == pComboBox[1])
                {
                    if (0 == pComboBox[1]->GetListBoxStatus())
                    {
                        ReflushChnName();
                    }
                }
                */
            }
            break;
    	case XM_RBUTTONDOWN:
    	case XM_RBUTTONDBLCLK:       
		PTZESC:
    		if(m_bPlayBackHide)
    		{
    			//Show(TRUE);
    			//CRect rect(0,500,800,600);
    			//SetRect(&m_Rect,TRUE);
                
                m_TmpRect.left = (m_RectScreen.right-636)/2;
                m_TmpRect.right = m_TmpRect.left + 636;
                m_TmpRect.top = m_RectScreen.bottom - 212 ;
                m_TmpRect.bottom = m_TmpRect.top + 182;
                
    			this->SetRect(&m_TmpRect,TRUE);
    			m_bPlayBackHide = FALSE;

    			return 0;
    		}
    		else
    		{
                pButton[nLastClkBtn]->Enable(TRUE);
                if (-1 != curChn) 
				{
					SetCurPreviewMode_CW(eMaxViewMode);//cw_preview
    			    SwitchPreview(eMaxViewMode, 0);
                }
    			BizSetZoomMode(0);
                this->Close();
                
    			if(IsDwell)
    			{
    				usleep(500 * 1000);
    				ControlPatrol(1);
    				IsDwell = 0;
    			}
    		}
            break;
    	default:
    		break;
    }
    //printf("yg MsgProc over 1\n");
	return CPageFloat::MsgProc(msg, wpa, lpa);
}


void CPagePtzCtl::HideWindow()
{
    m_bPlayBackHide = TRUE;
    m_pDevGraphics = CDevGraphics::instance(m_screen);
    m_pDevGraphics->GetRect(&m_RectScreen);
    CRect rect(-10,-10,-10,-10);
    this->SetRect(&rect,TRUE);
}

BOOL CPagePtzCtl::ChnPtzIsAble(u32 nChn)
{
    if ((nChn >= 0) 
        && (nChn < nChMax) 
        && (TabPara.bEnable))
    {
        return TRUE;
    }

    return FALSE;
}

void CPagePtzCtl::MessgeBox()
{
    MessageBox("&CfgPtn.ChoosePTZChn", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
    return;
}

void CPagePtzCtl::BtmStatusUpdate(u32 nChn)
{
    //track button
    if (BtmEnable(EM_PTZ_BTM_TRACK) && !(BtmEnable(EM_PTZ_BTM_SCAN) & BtmEnable(EM_PTZ_BTM_TOUR)))
    {
        pButton[EM_PTZ_BTM_TRACK]->SetText(btnTextStop[0]);
    }
    else
    {
        pButton[EM_PTZ_BTM_TRACK]->SetText(btnTextOK[0]);
    }

    //scan button
    if (BtmEnable(EM_PTZ_BTM_SCAN) && !(BtmEnable(EM_PTZ_BTM_TRACK) & BtmEnable(EM_PTZ_BTM_TOUR)))
    {
        pButton[EM_PTZ_BTM_SCAN]->SetText(btnTextStop[1]);
    }
    else
    {
        pButton[EM_PTZ_BTM_SCAN]->SetText(btnTextOK[1]);
    }

    //tour button
    if (BtmEnable(EM_PTZ_BTM_TOUR) && !(BtmEnable(EM_PTZ_BTM_TRACK) & BtmEnable(EM_PTZ_BTM_SCAN)))
    {
        pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextStop[2]);
    }
    else
    {
        pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextOK[2]);
    }
}

void CPagePtzCtl::SetCtlBtmStatusCtl(EMPTZBTMCTL emBtmCtl)
{
    switch(emBtmCtl)
    {
        case EM_PTZ_BTM_GOTOTOUR:
            {
                if (TRUE == AllBtmEnable())
                {
					/*
					if (-1 == BizPtzCheckTouring(curChn))
					{
						return;
					}
					*/
                    psPtzCtlInfo->bitTour.set(curChn);
                    psPtzCtlInfo->bitTrack.reset(curChn);
                    psPtzCtlInfo->bitScan.reset(curChn);
					
                    pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextStop[2]);
                    BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TOUR, psPtzCtlInfo->nPos[curChn]);
                }
            } break;
        case EM_PTZ_BTM_TRACK:
            {
                if (1 == psPtzCtlInfo->bitTrack.test(curChn))
                {
                    if (BtmEnable(EM_PTZ_BTM_SCAN) & BtmEnable(EM_PTZ_BTM_TOUR))
                    {
                        if (-1 == BizPtzCheckTouring(curChn))
                        {
                            return;
                        }
                        psPtzCtlInfo->bitTrack.set(curChn);
                        psPtzCtlInfo->bitScan.reset(curChn);
                        psPtzCtlInfo->bitTour.reset(curChn);

                        pButton[EM_PTZ_BTM_TRACK]->SetText(btnTextStop[0]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_PATTERN, psPtzCtlInfo->nPos[curChn]);
                    }
                    else
                    {
                        psPtzCtlInfo->bitScan.set(curChn);
                        psPtzCtlInfo->bitTour.set(curChn);
                        
                        pButton[EM_PTZ_BTM_TRACK]->SetText(btnTextOK[0]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_STOP_PATTERN, psPtzCtlInfo->nPos[curChn]);
                    }                    
                }
            } break;
        case EM_PTZ_BTM_SCAN:
            {
                if (1 == psPtzCtlInfo->bitScan.test(curChn))
                {
                    if (BtmEnable(EM_PTZ_BTM_TRACK) & BtmEnable(EM_PTZ_BTM_TOUR))
                    {
						/*
						if (-1 == BizPtzCheckTouring(curChn))
						{
							return;
						}
						*/
                        psPtzCtlInfo->bitScan.set(curChn);
                        psPtzCtlInfo->bitTrack.reset(curChn);
                        psPtzCtlInfo->bitTour.reset(curChn);
						
                        pButton[EM_PTZ_BTM_SCAN]->SetText(btnTextStop[1]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TOUR_AUTO, psPtzCtlInfo->nPos[curChn]);
                    }
                    else
                    {
                        psPtzCtlInfo->bitTrack.set(curChn);
                        psPtzCtlInfo->bitTour.set(curChn);

                        pButton[EM_PTZ_BTM_SCAN]->SetText(btnTextOK[1]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_STOP_TOUR_AUTO, psPtzCtlInfo->nPos[curChn]);
                    }                    
                }
            } break;
        case EM_PTZ_BTM_TOUR:
            {
                if (1 == psPtzCtlInfo->bitTour.test(curChn))
                {
                    if (BtmEnable(EM_PTZ_BTM_TRACK) & BtmEnable(EM_PTZ_BTM_SCAN))
                    {
						/*
						if (-1 == BizPtzCheckTouring(curChn))
						{
							return;
						}
						*/
                        psPtzCtlInfo->bitTour.set(curChn);
                        psPtzCtlInfo->bitTrack.reset(curChn);
                        psPtzCtlInfo->bitScan.reset(curChn);
                        
                        pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextStop[2]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TOUR, psPtzCtlInfo->nPos[curChn]);
                    }
                    else
                    {
                        psPtzCtlInfo->bitTrack.set(curChn);
                        psPtzCtlInfo->bitScan.set(curChn);

                        pButton[EM_PTZ_BTM_TOUR]->SetText(btnTextOK[2]);
                        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_STOP_TOUR, psPtzCtlInfo->nPos[curChn]);
                    }                    
                }
            } break;
        default:
            break;
    }

    return;
}

BOOL CPagePtzCtl::BtmEnable(EMPTZBTMCTL emBtmCtl)
{
    switch(emBtmCtl)
    {
        case EM_PTZ_BTM_TRACK:
            return psPtzCtlInfo->bitTrack.test(curChn);
        case EM_PTZ_BTM_SCAN:
            return psPtzCtlInfo->bitScan.test(curChn);
        case EM_PTZ_BTM_TOUR:
            return psPtzCtlInfo->bitTour.test(curChn);
        default:
            break;
    }

    return FALSE;
}

BOOL CPagePtzCtl::AllBtmEnable()
{
    int ret = 1;
    ret &= psPtzCtlInfo->bitTrack.test(curChn);
    ret &= psPtzCtlInfo->bitScan.test(curChn);
    ret &= psPtzCtlInfo->bitTour.test(curChn);

    return ret;
}

BOOL CPagePtzCtl::ReflushChnName()
{
    if (4 == nChMax) //4路
    {
        pButton[2]->SetText(pButton[2]->GetText());
        pButton[3]->SetText(pButton[3]->GetText());
    }
    else if (8 == nChMax) //8路
    {
        //pButton[4]->SetText(pButton[4]->GetText());
        for (int i=27; i<29/*30*/; i++)
        {
            pButton[i]->SetText(pButton[i]->GetText());
        }
    }
    else if (16 == nChMax || 32 == nChMax) //16/32路
    {
        for (int i = 31; i < 34; i++)
        {
            pButton[i]->SetText(pButton[i]->GetText());
        }
    }
    else if (24 == nChMax) //24路
    {
        if (0 == nCurChPg) //第一页
        {
            for (int i = 31; i < 34; i++)
            {
                pButton[i]->SetText(pButton[i]->GetText());
            }
        }
    }

    return TRUE;
}


