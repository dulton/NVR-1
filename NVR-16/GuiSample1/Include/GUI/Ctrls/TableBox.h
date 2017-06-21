#ifndef _TABLEBOX_H_
#define _TABLEBOX_H_

#include <vector>

typedef std::vector<int> COL_WIDTH_VECTOR;
typedef std::vector<int> ROW_HEIGHT_VECTOR;

typedef enum
{
	EM_CAVITY,
	EM_SOLID,
}EM_TABLE_DRAWSTYLE;

class CTableBox:public CItem
{
protected:
	static VD_BITMAP*	bmp_tab; 
	//std::string	string;	//控件字符串

public:
	CTableBox(VD_PCRECT pRect, CPage * pParent, int col,int row, uint style = 0,EM_TABLE_DRAWSTYLE drawStyle = EM_SOLID);
	virtual ~CTableBox();

	void Draw();
	
	void SetColWidth(int col, int width);	//col = -1时均分
	void SetRowHeight(int row, int height); //row = -1时均分

	void GetTableRect(int col, int row, VD_PRECT pRect);

	void SetFrameColor( VD_COLORREF color);
private:
	int m_nCol;
	int m_nRow;
	
	COL_WIDTH_VECTOR m_vColW;
	ROW_HEIGHT_VECTOR m_vRowH;
	VD_COLORREF m_clorFrame;

	EM_TABLE_DRAWSTYLE m_drawStyle;


};
CTableBox* CreateTableBox(VD_PCRECT pRect, CPage * pParent, int col,int row,uint style = 0,EM_TABLE_DRAWSTYLE drawStyle = EM_SOLID);

#endif
