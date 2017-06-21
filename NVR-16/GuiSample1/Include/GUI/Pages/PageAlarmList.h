#ifndef _PAGE_ALARM_LIST_H_
#define _PAGE_ALARM_LIST_H_

#include "PageFrame.h"

using namespace std;

#undef TABLE_COLUMNS
#undef TABLE_ROWS
#undef TABLE_ITEM_ROWS
#undef MAX_ITEMS

#define TABLE_COLUMNS 2
#define TABLE_ROWS 11
#define TABLE_ITEM_ROWS (TABLE_ROWS-1)

#define MAX_ITEMS 19//20

struct alarm_list_item
{
	CStatic *type, *detail;
};

class CPageAlarmList : public CPageFrame
{
public:
	CPageAlarmList(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon = NULL, CPage * pParent = NULL);
	~CPageAlarmList();
	
	VD_BOOL UpdateData(UDM mode);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	
	void InsertItem(int row, VD_PCSTR type, VD_PCSTR detail);
	void DeleteItem(int row);
	void ClearItems(int page);
	
	void OnClickBt();
	
	CTableBox *m_table;
	CScrollBar *m_scrollBar;
	vector<alarm_list_item> m_items;
	int m_currentPage;
	
private:
	void OnScroll();
};

#endif // !_PAGE_ALARM_LIST_H_

