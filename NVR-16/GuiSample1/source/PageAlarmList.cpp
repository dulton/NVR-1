#include "GUI/Pages/PageAlarmList.h"
#include "text.h"
#include "geom.h"

using namespace std;

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

void RefreshList(u8 page);

void StartTimer();
void StopTimer();

CPageAlarmList::CPageAlarmList(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon /*= NULL*/, CPage * pParent /*= NULL*/)
:CPageFrame(pRect, psz, icon, pParent)
{
	//SetRect(CRect(0, 0, pageMainFrame_w, pageMainFrame_h/*370*/), FALSE);
	//SetRect(CRect(0, 0, 480, 320/*370*/), FALSE);
	
	SetMargin(0, 0, 0, 0);
	CRect tabRect = CLIENT_RECT(CTRL_MARGIN, CTRL_MARGIN, CLIENT_WIDTH - CTRL_MARGIN * 2, CTRL_HEIGHT*TABLE_ROWS);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	m_table = CreateTableBox(tabRect, this, TABLE_COLUMNS, TABLE_ROWS);
	m_table->SetColWidth(0, 130);
	
	const char *columnHeaders[TABLE_COLUMNS] = 
	{
		TEXT_FAUILURE_TYPE,
		TEXT_DETAIL
	};
	
	CRect rt;
	CStatic *header;
	for(int i = 0; i < TABLE_COLUMNS; ++i)
	{
		m_table->GetTableRect(i, 0, &rt);
		header = CreateStatic(RECT_OFFSET(rt, tabRect), this, columnHeaders[i]);
		header->SetTextAlign(VD_TA_LEFT);
		header->SetBkColor(VD_RGB(67, 77, 87));
		header->SetTextAlign(VD_TA_XLEFT);
		header->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	}
	
	for(int i = 0; i< TABLE_ITEM_ROWS; ++i)
	{
		alarm_list_item item;
		
		m_table->GetTableRect(0, i + 1, &rt);
		item.type = CreateStatic(RECT_OFFSET(rt, tabRect), this, "");
		
		m_table->GetTableRect(1, i + 1, &rt);
		rt.right -= SCROLLBAR;
		item.detail = CreateStatic(RECT_OFFSET(rt, tabRect), this, "");
		
		m_items.push_back(item);
	}
	
	m_scrollBar = CreateScrollBar(
		RECT(tabRect.right - SCROLLBAR, tabRect.top + CTRL_HEIGHT, SCROLLBAR, tabRect.Height() - CTRL_HEIGHT),
		this, scrollbarY, 0, MAX_ITEMS, 1, (CTRLPROC)&CPageAlarmList::OnScroll);
	
	rt = CLIENT_RECT(CLIENT_WIDTH - (BUTTON_WIDTH + CTRL_MARGIN),
		CLIENT_HEIGHT - (CTRL_HEIGHT + CTRL_MARGIN), BUTTON_WIDTH, CTRL_HEIGHT);
	CreateButton(rt, this, TEXT_EXIT, (CTRLPROC)&CPageAlarmList::OnClickBt)
		->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
}

CPageAlarmList::~CPageAlarmList()
{
	
}

void CPageAlarmList::OnScroll()
{
	u8 page = m_scrollBar->GetPos();
	if(m_currentPage == page)
	{
		return;
	}
	m_currentPage = page;
	RefreshList(page);
}

void CPageAlarmList::InsertItem(int row, VD_PCSTR type, VD_PCSTR detail)
{
	m_items[row].type->SetText(type);
	m_items[row].detail->SetText(detail);
	m_items[row].type->Show(TRUE);
	m_items[row].detail->Show(TRUE);
}

void CPageAlarmList::DeleteItem(int row)
{
	m_items[row].type->SetText("");
	m_items[row].detail->SetText("");
	m_items[row].type->Show(FALSE);
	m_items[row].detail->Show(FALSE);
}

void CPageAlarmList::ClearItems(int page)
{
	for(int i = 0; i < TABLE_ITEM_ROWS; i++)
	{
		DeleteItem(i);
	}
}

VD_BOOL CPageAlarmList::UpdateData(UDM mode)
{
	if(mode == UDM_OPEN)
	{
		m_currentPage = 0;
		m_scrollBar->SetPos(m_currentPage);
		
		RefreshList(m_currentPage);
		
		StartTimer();
	}
	else if(mode == UDM_CLOSED)
	{
		StopTimer();
		//yaogang modify 20141106
		//用户已经查看了报警信息
		//窗口关闭前去除报警已经恢复的源信息
		//printf("yg AlarmList close \n");
		RefreshAllStatus();
	}
	return true;
}

void CPageAlarmList::OnClickBt()
{
	StopTimer();
	this->Close();
}

VD_BOOL CPageAlarmList::MsgProc(uint msg, uint wpa, uint lpa)
{
	/*switch(msg)
	{
	case XM_MOUSEWHEEL:
		m_scrollBar->MsgProc(msg, wpa, lpa);
		break;
	default:
		break;
	}*/
	return CPage::MsgProc(msg, wpa, lpa);
}

