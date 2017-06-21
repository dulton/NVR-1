#include "GUI/Pages/PagePtzTrack.h"
#include "Biz.h"
#include "GUI/Pages/BizData.h"


#define ROW_CHN_MAX	10
#define ROW_BTN_MAX	(ROW_CHN_MAX+1) // 一排最多的按钮个数
#define NEW_CHN_BTN_IDX	19			// 保持原有按钮的id顺序，新加的按钮序号往后增加

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
	PTZ_BUTTON_HIDE,
	PTZ_BUTTON_EXIT,
	PTZ_BUTTON_TRIANGLE,
	PTZ_BUTTON_NUM,
};


char* ptzTrackShortcutBmpName[PTZ_BUTTON_NUM][2] = {    
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


static int GetMaxChnNum()
{ 
    return GetVideoMainNum(); 
}

int bizData_GetUartPara(uchar nChn, STabParaUart* pTabPara);

CPagePtzTrack::CPagePtzTrack( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent)
{
	nChMax = GetMaxChnNum();
    m_CurPTZCmd = EM_BIZPTZ_CMD_NULL;
    m_lastDirect = EM_BIZPTZ_CMD_NULL;
	
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
			eMaxViewMode = EM_BIZPREVIEW_25SPLITS;
			//eMaxViewMode = EM_BIZPREVIEW_24SPLITS;
		} break;
		case 32:
		{
			eMaxViewMode = EM_BIZPREVIEW_36SPLITS;
		} break;
		default:
		{
			eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
		} break;
	}

	
	curChn = 0;
    RecFlag = PatFlag = 2;
	IsDwell = 0;

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


	CRect rtTable;
	rtTable.left = 0;
	rtTable.top = 0;
	rtTable.right = m_Rect.Width();
	rtTable.bottom = rtTable.top + 44;
	#if 0
	pTable0 = CreateTableBox(rtTable, this, 3, 1);
	pTable0->SetColWidth(0, 123);
	pTable0->SetColWidth(1, 190);
	#endif
	
	rtTable.left = 0;
	rtTable.top = m_Rect.Height()-34;
	rtTable.right = m_Rect.Width();
	rtTable.bottom = rtTable.top + 34;
	pTable1 = CreateTableBox(rtTable, this, 1, 1);

	
	rtTable.left = 265;
	rtTable.top = 60;
	rtTable.right = m_Rect.Width()-10;
	rtTable.bottom = rtTable.top + 80;
	pTable2 = CreateTableBox(rtTable, this, 1, 1);

    //Draw SepLine
    pDrawLine = CreateStatic(CRect(1,42,m_Rect.Width()-1,44), this, "");
	pDrawLine->SetBkColor(VD_RGB(102,102,102));
	
#if 1

	int nStart;
	int nBtnNum;
	int nWidth 		= pRect->right - pRect->left;
	int nBtnWidth 	= pBmpPtzChn_normal->width;
	if(nChMax < ROW_BTN_MAX)
	{	
		nCurChPg = 0;
		nChPgNum = 1;
		nLastHideCh = 0;
	
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
	rtTmp.left = nStart; //124;
	rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	rtTmp.top = 2;
	rtTmp.bottom = rtTmp.top + pBmpPtzChn_normal->height;
	
	int i = 0;
	for(i=0; i<nBtnNum; i++)
	{
		int nBtnIdx = 0;
		if(i>4)
		{
			nBtnIdx = i+NEW_CHN_BTN_IDX;
		}
		else
		{
			nBtnIdx = i;
		}
		
		if(i == nBtnNum-1)
		{
			if(i>=ROW_CHN_MAX)
			{
				rtTmp.left = nWidth-pBmpPtzChn_normal->width-1;
				rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
			}
			
			pButton[nBtnIdx] = CreateButton(rtTmp, this, "&CfgPtn.All" , (CTRLPROC)&CPagePtzTrack::OnClkChnSel, NULL, buttonNormalBmp);
            //屏蔽掉ALL按键
            pButton[nBtnIdx]->Show(FALSE, FALSE);
		}
		else
		{
			char chnID[4] = {0};
			sprintf(chnID,"%d",i+1);
			pButton[nBtnIdx] = CreateButton(rtTmp, this, chnID , (CTRLPROC)&CPagePtzTrack::OnClkChnSel, NULL, buttonNormalBmp);
		}
		
		pButton[nBtnIdx]->SetBitmap(pBmpPtzChn_normal, pBmpPtzChn_select, pBmpPtzChn_select, pBmpPtzChn_push);

		rtTmp.left = rtTmp.right-1;
		rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	}

	//printf("ptz track 0\n");
	
	if(nChMax >= ROW_BTN_MAX)
	{
		pBmpButtonNormal[PTZ_BUTTON_NUM-1] = VD_LoadBitmap(ptzTrackShortcutBmpName[PTZ_BUTTON_NUM-1][0]);
		pBmpButtonSelect[PTZ_BUTTON_NUM-1] = VD_LoadBitmap(ptzTrackShortcutBmpName[PTZ_BUTTON_NUM-1][1]);
		
		pButton[i+NEW_CHN_BTN_IDX] = CreateButton(CRect(
                						nWidth-pBmpPtzChn_normal->width-6-pBmpButtonNormal[PTZ_BUTTON_NUM-1]->width, 
                						rtTmp.top , 
                						nWidth-pBmpPtzChn_normal->width-6, 
                						rtTmp.top+pBmpButtonNormal[PTZ_BUTTON_NUM-1]->height), 
                						this, NULL, (CTRLPROC)&CPagePtzTrack::OnBtnDown, NULL, buttonNormalBmp);

		pButton[i+NEW_CHN_BTN_IDX]->SetBitmap(pBmpButtonNormal[PTZ_BUTTON_NUM-1], pBmpButtonSelect[PTZ_BUTTON_NUM-1], pBmpButtonSelect[PTZ_BUTTON_NUM-1]);
	}
	
	//printf("ptz track 1\n");
#else

	CRect rtTmp;
	rtTmp.left = 124;
	rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	rtTmp.top = 2;
	rtTmp.bottom = rtTmp.top + pBmpPtzChn_normal->height;
	
	int i = 0;
	for(i=0; i<5; i++)
	{
		if(i == 4)
		{
			pButton[i] = CreateButton(rtTmp, this, "&CfgPtn.All" , (CTRLPROC)&CPagePtzTrack::OnClkChnSel, NULL, buttonNormalBmp);
		}
		else
		{
			char chnID[4] = {0};
			sprintf(chnID,"%d",i+1);
			pButton[i] = CreateButton(rtTmp, this, chnID , (CTRLPROC)&CPagePtzTrack::OnClkChnSel, NULL, buttonNormalBmp);
		}
		
		pButton[i]->SetBitmap(pBmpPtzChn_normal, pBmpPtzChn_select, pBmpPtzChn_select, pBmpPtzChn_push);
			
		rtTmp.left = rtTmp.right + 1;
		rtTmp.right = rtTmp.left + pBmpPtzChn_normal->width;
	}

#endif

	//utton[curChn]->Enable(FALSE);


	pBmpButtonNormal[0] = VD_LoadBitmap(ptzTrackShortcutBmpName[0][0]);
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	int baseX = 8;
	int baseY = rtTmp.bottom +5;
	
	rtTmp.left = baseX;
	rtTmp.right = baseX + pBmpButtonNormal[0]->width;
	rtTmp.top = baseY;
	rtTmp.bottom = baseY + pBmpButtonNormal[0]->height;
	pStatic[0] =  CreateStatic(rtTmp, this, pBmpButtonNormal[0],0);

	//up
	pBmpButtonNormal[1] = VD_LoadBitmap(ptzTrackShortcutBmpName[1][0]);
	ReplaceBitmapBits(pBmpButtonNormal[1], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[1] = VD_LoadBitmap(ptzTrackShortcutBmpName[1][1]);
	ReplaceBitmapBits(pBmpButtonSelect[1], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+43; //53;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[1]->width;
	rtTmp.top =  baseY+12;//65;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[1]->height;
	pButton[5] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[5]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);

	//down
	pBmpButtonNormal[2] = VD_LoadBitmap(ptzTrackShortcutBmpName[2][0]);
	ReplaceBitmapBits(pBmpButtonNormal[2], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[2] = VD_LoadBitmap(ptzTrackShortcutBmpName[2][1]);
	ReplaceBitmapBits(pBmpButtonSelect[2], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+43; //53;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[2]->width;
	rtTmp.top =  baseY+74;//127;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[2]->height;
	pButton[6] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[6]->SetBitmap(pBmpButtonNormal[2], pBmpButtonSelect[2], pBmpButtonSelect[2]);

	//left
	pBmpButtonNormal[3] = VD_LoadBitmap(ptzTrackShortcutBmpName[3][0]);
	ReplaceBitmapBits(pBmpButtonNormal[3], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[3] = VD_LoadBitmap(ptzTrackShortcutBmpName[3][1]);
	ReplaceBitmapBits(pBmpButtonSelect[3], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+12; //22;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[3]->width;
	rtTmp.top =  baseY+42; //95;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[3]->height;
	pButton[7] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[7]->SetBitmap(pBmpButtonNormal[3], pBmpButtonSelect[3], pBmpButtonSelect[3]);

	//right
	pBmpButtonNormal[4] = VD_LoadBitmap(ptzTrackShortcutBmpName[4][0]);
	ReplaceBitmapBits(pBmpButtonNormal[4], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[4] = VD_LoadBitmap(ptzTrackShortcutBmpName[4][1]);
	ReplaceBitmapBits(pBmpButtonSelect[4], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+73; //83;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[4]->width;
	rtTmp.top =  baseY+42; //95;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[4]->height;
	pButton[8] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[8]->SetBitmap(pBmpButtonNormal[4], pBmpButtonSelect[4], pBmpButtonSelect[4]);

	//left up
	pBmpButtonNormal[5] = VD_LoadBitmap(ptzTrackShortcutBmpName[5][0]);
	ReplaceBitmapBits(pBmpButtonNormal[5], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[5] = VD_LoadBitmap(ptzTrackShortcutBmpName[5][1]);
	ReplaceBitmapBits(pBmpButtonSelect[5], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+23; //33;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[5]->width;
	rtTmp.top =  baseY+21; //74;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[5]->height;
	pButton[9] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[9]->SetBitmap(pBmpButtonNormal[5], pBmpButtonSelect[5], pBmpButtonSelect[5]);

	//right up
	pBmpButtonNormal[6] = VD_LoadBitmap(ptzTrackShortcutBmpName[6][0]);
	ReplaceBitmapBits(pBmpButtonNormal[6], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[6] = VD_LoadBitmap(ptzTrackShortcutBmpName[6][1]);
	ReplaceBitmapBits(pBmpButtonSelect[6], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+63;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[6]->width;
	rtTmp.top =  baseY+21;//74;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[6]->height;
	pButton[10] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[10]->SetBitmap(pBmpButtonNormal[6], pBmpButtonSelect[6], pBmpButtonSelect[6]);

	//left down
	pBmpButtonNormal[7] = VD_LoadBitmap(ptzTrackShortcutBmpName[7][0]);
	ReplaceBitmapBits(pBmpButtonNormal[7], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[7] = VD_LoadBitmap(ptzTrackShortcutBmpName[7][1]);
	ReplaceBitmapBits(pBmpButtonSelect[7], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+22;//32;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[7]->width;
	rtTmp.top =  baseY+64;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[7]->height;
	pButton[11] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[11]->SetBitmap(pBmpButtonNormal[7], pBmpButtonSelect[7], pBmpButtonSelect[7]);
			
	//right down	
	pBmpButtonNormal[8] = VD_LoadBitmap(ptzTrackShortcutBmpName[8][0]);
	ReplaceBitmapBits(pBmpButtonNormal[8], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[8] = VD_LoadBitmap(ptzTrackShortcutBmpName[8][1]);
	ReplaceBitmapBits(pBmpButtonSelect[8], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+63;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[8]->width;
	rtTmp.top =  baseY+64;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[8]->height;
	pButton[12] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[12]->SetBitmap(pBmpButtonNormal[8], pBmpButtonSelect[8], pBmpButtonSelect[8]);

	//stop
	pBmpButtonNormal[9] = VD_LoadBitmap(ptzTrackShortcutBmpName[9][0]);
	ReplaceBitmapBits(pBmpButtonNormal[9], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[9] = VD_LoadBitmap(ptzTrackShortcutBmpName[9][1]);
	ReplaceBitmapBits(pBmpButtonSelect[9], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	rtTmp.left = baseX+42;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[9]->width;
	rtTmp.top =  baseY+42;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[9]->height;
	pButton[13] = CreateButton(rtTmp, this, NULL, NULL, 0, buttonNormalBmp);
	pButton[13]->SetBitmap(pBmpButtonNormal[9], pBmpButtonSelect[9], pBmpButtonSelect[9]);
		


		
	pBmpButtonNormal[10] = VD_LoadBitmap(ptzTrackShortcutBmpName[10][0]);
	ReplaceBitmapBits(pBmpButtonNormal[10], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[10] = VD_LoadBitmap(ptzTrackShortcutBmpName[10][1]);
	ReplaceBitmapBits(pBmpButtonSelect[10], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonNormal[11] = VD_LoadBitmap(ptzTrackShortcutBmpName[11][0]);
	ReplaceBitmapBits(pBmpButtonNormal[11], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	pBmpButtonSelect[11] = VD_LoadBitmap(ptzTrackShortcutBmpName[11][1]);
	ReplaceBitmapBits(pBmpButtonSelect[11], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	char* staticText[7] = {
		"&CfgPtn.Zoom",
		"&CfgPtn.Focus",
		"&CfgPtn.Iris",
		"&CfgPtn.Preset",
		"&CfgPtn.Track",
		"&CfgPtn.Speed",
		"5",
	};
	
	rtTmp.left = 118;//73;
	rtTmp.right = rtTmp.left + pBmpButtonNormal[10]->width;
	rtTmp.top =  54;//117;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
	int j = 0;
	for(i=14; i<17; i++)
	{
		pButton[i+j] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzTrack::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+j]->SetBitmap(pBmpButtonNormal[10], pBmpButtonSelect[10], pBmpButtonSelect[10]);

		rtTmp.left = rtTmp.right;
		rtTmp.right = rtTmp.left + 80;
		pStatic[i-13] = CreateStatic(rtTmp, this, staticText[i-14]);
		pStatic[i-13]->SetTextAlign(VD_TA_CENTER);

		rtTmp.left = rtTmp.right;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[11]->width;

		pButton[i+j+1] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzTrack::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+j+1]->SetBitmap(pBmpButtonNormal[11], pBmpButtonSelect[11], pBmpButtonSelect[11]);

		rtTmp.left = 118;//73;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[10]->width;
		rtTmp.top =  rtTmp.top + pBmpButtonNormal[10]->height +8;//117;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[10]->height;
		j++;
	}


	pStatic[4] = CreateStatic(CRect(285,50,345,70), this, staticText[4]);
	pStatic[4]->SetTextAlign(VD_TA_CENTER);

	

	char* btnText[2] = {
		"&CfgPtn.StartRecord",
		"&CfgPtn.StartTrack",

	};
	
	rtTmp.left = 275;
	rtTmp.top = 75;
	rtTmp.right = m_Rect.Width() - 20;
	rtTmp.bottom = rtTmp.top + 25 - 1;
	
	for(i=0; i<2; i++)
	{
		pButton[i+20] = CreateButton(rtTmp, this, btnText[i], (CTRLPROC)&CPagePtzTrack::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+20]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);

		rtTmp.top = rtTmp.bottom + 7;
		rtTmp.bottom = rtTmp.top + 25 - 1;		
	}
	

	rtTmp.left = 30;
	rtTmp.right = rtTmp.left + 50;
	rtTmp.top = m_Rect.Height()- 27;
	rtTmp.bottom = rtTmp.top + 20;
	pStatic[5] = CreateStatic(rtTmp, this, staticText[5]);
	pStatic[5]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left = rtTmp.right;
	rtTmp.right = rtTmp.left + 15;
	pStatic[6] = CreateStatic(rtTmp, this, staticText[6]);
	pStatic[6]->SetTextAlign(VD_TA_CENTER);
	
	rtTmp.left = 120;//73;
	rtTmp.right = rtTmp.left + 128;
	rtTmp.top = m_Rect.Height()- 27;
	rtTmp.bottom = rtTmp.top + 20;

	pSlider = CreateSliderCtrl(rtTmp,this,1, 10, (CTRLPROC)&CPagePtzTrack::OnSliderChange, sliderNoNum);
    pSlider->SetPos(5);


	
	for(i=0; i<2; i++)
	{
		pBmpButtonNormal[i+12] = VD_LoadBitmap(ptzTrackShortcutBmpName[i+12][0]);
		ReplaceBitmapBits(pBmpButtonNormal[i+12], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
		pBmpButtonSelect[i+12] = VD_LoadBitmap(ptzTrackShortcutBmpName[i+12][1]);
		ReplaceBitmapBits(pBmpButtonSelect[i+12], VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

		rtTmp.left = 360 + i*pBmpButtonNormal[i+12]->width +10 ;//73;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[i+12]->width;
		rtTmp.top = m_Rect.Height()- 30;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[i+12]->height;

		pButton[i+22] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePtzTrack::OnClkOtherBtn, NULL, buttonNormalBmp);
		pButton[i+22]->SetBitmap(pBmpButtonNormal[i+12], pBmpButtonSelect[i+12], pBmpButtonSelect[i+12]);
	}
	

}

void CPagePtzTrack::ShowBtn()
{
	int nChIdx;
	
	for(int i=0; i<nRealRowBtn; i++)
	{
		char szCh[4]={0};
		sprintf(szCh, "%d", i+nCurChPg*ROW_CHN_MAX+1);
		nChIdx = i;
		if(i>4)
		{
            nChIdx += NEW_CHN_BTN_IDX;
		}
		
		pButton[nChIdx]->SetText(szCh);
		pButton[nChIdx]->Show(TRUE,TRUE);
	}
	
	if(nCurChPg == nChPgNum-1 && nChPgNum > 1)
	{		
		for(int i=0; i<nLastHideCh; i++)
		{
			nChIdx = i+ROW_CHN_MAX-nLastHideCh;
			if(nChIdx>4)
			{
			    nChIdx += NEW_CHN_BTN_IDX;
			}
			
			pButton[nChIdx]->Show(FALSE,TRUE);
		}
	}
}
	
void CPagePtzTrack::OnBtnDown()
{
	nCurChPg += 1;
	if(nCurChPg >= nChPgNum)
	{
		nCurChPg = 0;
	}

	int nBtnIdx = curChn%nRealRowBtn;
	if(curChn != nCurChPg*nRealRowBtn+nBtnIdx )
	{
		nBtnIdx = (nBtnIdx>4)?(nBtnIdx+NEW_CHN_BTN_IDX):nBtnIdx;
		pButton[nBtnIdx]->Enable(TRUE);
	}
	else
	{
		nBtnIdx = (nBtnIdx>4)?(nBtnIdx+NEW_CHN_BTN_IDX):nBtnIdx;
		pButton[nBtnIdx]->Enable(FALSE);
	}
	
	ShowBtn();
	
	return;
}


void CPagePtzTrack::OnClkOtherBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 14; i < 24; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

    char* btnText[2] = {
		"&CfgPtn.StartRecord",
		"&CfgPtn.StartTrack",

	};

    char* btnText1[2] = {
		"&CfgPtn.StopRecord",
		"&CfgPtn.StopTrack",

	};

	u8 			nChn;
	EMBIZPTZCMD emCmd = EM_BIZPTZ_CMD_NULL;
	s16 		nId; 

	if (bFind)
	{
		nChn = curChn;
		nId  = 0;
		
		switch(i)
		{
			case 20:
			{
                if (!TabPara.bEnable)
                {
                    printf("***Ptz Enable is not Open***\n");
                    return;
                }
                //开始录制
                if (RecFlag) 
                {
                    ++RecFlag;
                    if (1 == (RecFlag%2))
                    {
    				    //printf("start record \n");		
    				    emCmd = EM_BIZPTZ_CMD_STARTREC_PATTERN;
                        pButton[i]->SetText(btnText1[0]);
                        PatFlag = 0;
                    }
                    else
                    {
                        //printf("stop record \n");
                        emCmd = EM_BIZPTZ_CMD_STOPREC_PATTERN;
                        pButton[i]->SetText(btnText[0]);
                        PatFlag = 2;
                    }
                }
			}break;
			case 21:
			{
                if (!TabPara.bEnable)
                {
                    printf("***Ptz Enable is not Open***\n");
                    return;
                }
                //开始轨迹
                if (PatFlag)
                {
                    ++PatFlag;
                    if (1 == (PatFlag%2))
                    {
    				    //printf("start track \n");				
    				    emCmd = EM_BIZPTZ_CMD_START_PATTERN;
                        pButton[i]->SetText(btnText1[1]);
                        RecFlag = 0;
                    }
                    else
                    {
                        //printf("stop trak \n");
                        emCmd = EM_BIZPTZ_CMD_STOP_PATTERN;
                        pButton[i]->SetText(btnText[1]);
                        RecFlag = 2;
                    }
                }
				
			}break;
			case 22:
			{
				//printf("hide\n");
                HideWindow();
			}break;
			case 23:
			{
				//printf("exit\n");
				u8 chn = curChn%nRealRowBtn;
				if( chn>4 )
        		{
        			pButton[chn+NEW_CHN_BTN_IDX]->Enable(TRUE);				
        		}
        		else
        		{			
        			pButton[chn]->Enable(TRUE);
        		}
				SetSystemLockStatus(1);//cw_lock
				BizSetZoomMode(0);

				this->Close();
				SwitchPreview(eMaxViewMode, 0);
				
				this->m_pParent->Open();
				if(IsDwell)
				{
					usleep(500 * 1000);
					ControlPatrol(1);
					IsDwell = 0;
				}
				SetSystemLockStatus(0);
			}break;
			default:
			    break;
		}
		if (emCmd != EM_BIZPTZ_CMD_NULL)
		{
		    BizPtzCtrl(nChn, emCmd, nId);
		}
	}
}


void CPagePtzTrack::OnClkChnSel()
{
	int i = 0;
	BOOL bFind = FALSE;
	
	int nChIdx;
			
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < nRealRowBtn+1; i++)
	{
		if(i > 4)
		{
			nChIdx = i+NEW_CHN_BTN_IDX;
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

	if (bFind)
	{
	#if 1
		//printf("## switch channel %d\n", i);
    	int nTmp = curChn%nRealRowBtn; 
		if( nTmp>4 )
		{
			pButton[nTmp+NEW_CHN_BTN_IDX]->Enable(TRUE);				
		}
		else
		{			
			pButton[nTmp]->Enable(TRUE);
		}
		
		if(i != nRealRowBtn)
		{
			SwitchPreview(EM_BIZPREVIEW_1SPLIT, nBtnIdx);

			curChn = nBtnIdx;
            bizData_GetUartPara(curChn, &TabPara);
			pButton[nChIdx]->Enable(FALSE);
		}
		else
		{
			SwitchPreview(eMaxViewMode, 0);			
		}
	#else
		switch(i)
		{
			case 0:
			{
				printf("Chn 1\n");
				pButton[curChn]->Enable(TRUE);
				curChn = 0;
				pButton[curChn]->Enable(FALSE);
				
			}break;
			case 1:
			{
				printf("Chn 2\n");
				pButton[curChn]->Enable(TRUE);
				curChn = 1;
				pButton[curChn]->Enable(FALSE);
				
			}break;
			case 2:
			{
				printf("Chn 3\n");
				pButton[curChn]->Enable(TRUE);
				curChn = 2;
				pButton[curChn]->Enable(FALSE);
				
			}break;
			case 3:
			{
				printf("Chn 4\n");
				pButton[curChn]->Enable(TRUE);
				curChn = 3;
				pButton[curChn]->Enable(FALSE);
				
			}break;
			case 4:
			{
				printf("Chn All\n");
				pButton[curChn]->Enable(TRUE);
				curChn = 4;
				pButton[curChn]->Enable(FALSE);

			}break;
			default:
			break;
		}
	#endif
		
	}

}

void CPagePtzTrack::OnSliderChange()
{
	//printf("OnSliderChange \n");

	char strVal[4] = {0};
	int value = pSlider->GetPos();
	sprintf(strVal,"%d",value);
    //printf("Slider value: %d\n", value);

	pStatic[6]->SetText(strVal);
    BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, value);
	
}


void CPagePtzTrack::OnClkDirection()
{
	
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
					
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_TILEUP, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_TILEUP;
	
			}break;
			case 6:
			{
				//printf("down\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_TILEDOWN, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_TILEDOWN;
	
			}break;
			case 7:
			{
				//printf("left\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_PANLEFT, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_PANLEFT;
		
				
			}break;
			case 8:
			{
				//printf("right\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_PANRIGHT, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_PANRIGHT;
	
			}break;
			case 9:
			{
				//printf("left up\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_LEFTUP, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_LEFTUP;


			}break;
			case 10:
			{
				//printf("right up\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_RIGHTUP, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_RIGHTUP;
	
			}break;
			case 11:
			{
				//printf("left down\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_LEFTDOWN, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_LEFTDOWN;
	
			}break;
			case 12:
			{
				//printf("right down\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_START_RIGHTDOWN, 0); 
                //m_lastDirect = EM_BIZPTZ_CMD_START_RIGHTDOWN;
		
				
			}break;
			case 13:
			{
				//printf("stop\n");
				//BizPtzCtrl(curChn, EM_BIZPTZ_CMD_STOP_ALL, 0);
                //BizPtzCtrl(curChn, (EMBIZPTZCMD)(m_lastDirect + 30), 0);
	
			}break;
			default:
			break;
		}
		
	}

}

CPagePtzTrack::~CPagePtzTrack()
{

}

VD_BOOL CPagePtzTrack::UpdateData(UDM mode)
{
    if (UDM_OPEN == mode)
    { 
        BizSetZoomMode(1);
		if(GetDwellStartFlag())
		{
			IsDwell = 1;
			ControlPatrol(0);
		}
        
        SwitchPreview(EM_BIZPREVIEW_1SPLIT, curChn);
        bizData_GetUartPara(curChn, &TabPara);

		nCurChPg = curChn/nRealRowBtn;
		if(nCurChPg == 0)
		{
			nCurChPg = nChPgNum;
		}
		else
		{
			nCurChPg--;
		}
	
		OnBtnDown();
        m_CurPTZCmd = EM_BIZPTZ_CMD_NULL;
        m_lastDirect = EM_BIZPTZ_CMD_NULL;

        BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, (s16)pSlider->GetPos());
    }
    return TRUE;
}

void CPagePtzTrack::HideWindow()
{
    m_bPlayBackHide = TRUE;
    m_pDevGraphics = CDevGraphics::instance(m_screen);
    m_pDevGraphics->GetRect(&m_RectScreen);
    CRect rect(-10,-10,-10,-10);
    this->SetRect(&rect,TRUE);
}

void CPagePtzTrack::ParentSendCh(uchar nCh)
{
    curChn = nCh;
}

VD_BOOL CPagePtzTrack::MsgProc(uint msg, uint wpa, uint lpa)
{
	//printf("CPageFrame::MsgProc\n");
	static int keyStatus=0;
    u8 lock_flag = 0;  //cw_shutdown
    GetSystemLockStatus(&lock_flag);
    if(lock_flag)
    {
        return FALSE;
    }
	int px,py;
    
	switch(msg)
	{
	case XM_KEYDOWN:  //cw_ptz
		if (wpa==KEY_RET)
		{
			if (keyStatus==0)
			{
				keyStatus=1;
				goto TRACKEnter;
			}
			else
			{
				keyStatus=0;
				goto TRACKDown;
			}
		}
		if (wpa==KEY_PTZ)
		{ 
		    return FALSE;
		}
		if (wpa==KEY_MENU)
		{ 
		    goto TRACKESC;
		}
		if (wpa==KEY_REC)
		{ 
		    goto TRACKESC;
		}
		if (wpa==KEY_PLAY)
		{ 
		    goto TRACKESC;
		}
		if (wpa==KEY_ESC)
		{ 
		    goto TRACKESC;
		}
		else
		{
			return CPage::MsgProc(msg, wpa, lpa);
        }
	case XM_LBUTTONDOWN:
    case XM_LBUTTONDBLCLK:
        {
	    TRACKEnter:
            if (m_bPlayBackHide)
            {
                m_TmpRect.left = (m_RectScreen.right-436)/2;
                m_TmpRect.right = m_TmpRect.left + 436;
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
                else 
                {
                    return CPageFloat::MsgProc(msg, wpa, lpa);
                }
                /*
                if (-1 == curChn) {
                    MessgeBox();
                    return TRUE;
                }
                */
                
                BizPtzCtrl((u8)curChn, m_CurPTZCmd, 0);
                m_lastDirect = m_CurPTZCmd;
                bLeftBtnCtlPtz = TRUE;
            }
        break;
    case XM_LBUTTONUP:
	TRACKDown:
        {
            if (bLeftBtnCtlPtz) 
            {
                if ((int)m_lastDirect>=0 && (int)m_lastDirect<=253-30)
                {
                    BizPtzCtrl((u8)curChn, (EMBIZPTZCMD)(m_lastDirect+30), 0);
                }
            }
            bLeftBtnCtlPtz = FALSE;
        }
        break;
	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:
        {
	    TRACKESC://cw_ptz
            //((CPagePtzConfigFrameWork *)(this->GetParent()))->SetOpenFlag(TRUE);
            if (m_bPlayBackHide)
            {
                m_TmpRect.left = (m_RectScreen.right-436)/2;
                m_TmpRect.right = m_TmpRect.left + 436;
                m_TmpRect.top = m_RectScreen.bottom - 212 ;
                m_TmpRect.bottom = m_TmpRect.top + 182;
            
			    this->SetRect(&m_TmpRect,TRUE);
			    m_bPlayBackHide = FALSE;

			    return 0;
            }
            else
            {
            	SetSystemLockStatus(1);//cw_lock
                SwitchPreview(eMaxViewMode, 0);
				BizSetZoomMode(0);
				
				u8 chn = curChn%nRealRowBtn;
                if( chn>4 )
        		{
        			pButton[chn+NEW_CHN_BTN_IDX]->Enable(TRUE);				
        		}
        		else
        		{			
        			pButton[chn]->Enable(TRUE);
        		}
                this->Close();
				this->m_pParent->Open();
				
				if(IsDwell)
				{
					usleep(500 * 1000);
					ControlPatrol(1);
					IsDwell = 0;
				}
				SetSystemLockStatus(0);
            }
        }
		break;
	    default:
		    break;
	    }
	}

	return CPage::MsgProc(msg, wpa, lpa);
}


