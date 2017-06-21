#ifndef _RECFILETIMEBOX_H_
#define _RECFILETIMEBOX_H_


#include <vector>


typedef struct
{
	int starttime;	//sec
	int endtime; //sec
}RecTimeData;

typedef std::vector<RecTimeData> RECTIME_LIST;

typedef struct
{
	std::string szItem;
	RECTIME_LIST recTime;
}RecItem;
typedef std::vector<RecItem> REC_ITEM_LIST;


class CRecFileTimeBox:public CItem
{
public:
	CRecFileTimeBox(VD_PCRECT pRect, CPage * pParent, CTRLPROC onClkTrack = NULL, uint style = 0);
	~CRecFileTimeBox();

	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);

	int InsertItem(int index, VD_PCSTR string, VD_BOOL redraw = FALSE);
	void DeleteItem(int index, VD_BOOL redraw = FALSE) ;
	void RemoveAllItem(VD_BOOL redraw = FALSE);

	void AddItemTimeData(int index, RecTimeData* pData, VD_BOOL redraw = FALSE);

	void SetTrackTimePos(int timepos);
	int GetTrackTimePos();

private:
	void DrawFrame();
	void DrawItem();

	void DrawTrack(int timepos);

	REC_ITEM_LIST m_items;
	int m_nTrackTimePos;

	double m_dbSecPerPix;
	double m_hourSplitWidth;
	double m_hourwidth;

	//std::vector<VD_COLORREF> m_TrackOld;

	CTRLPROC m_vProc;

};
CRecFileTimeBox* CreateRecTimeBox(VD_PCRECT pRect, CPage * pParent, CTRLPROC onClkTrack = NULL, uint style = 0);
#endif
