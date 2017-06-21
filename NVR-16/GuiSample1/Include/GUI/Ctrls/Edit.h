//
//  "$Id: Edit.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//    Description:    
//    Revisions:        Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_EDIT_H__
#define __CTRL_EDIT_H__

typedef enum
{
    #if defined SHBELL || defined ZXBELL
    ETS_NORMAL        = 0x0000,
    #endif
    editPassword    = 0x0001,
    
	edit_KI_NUMERIC = 0x0002,			//数字
	edit_KI_UPPERCASE = 0x0004,	//大写字母
	edit_KI_LOWERCASE = 0x0008,	//小写字母
	edit_KI_MARK = 0x0010,		//常用符号
	edit_KI_PINYIN = 0x0020,		//拼音输入
	edit_KI_CONTROL = 0x0040,		//ATM原始控制键
	edit_KI_NOCH = 0x8000,		//无中文
//    styleNoBorder    = 0x0002,
}EDITSTYLE;

struct EDIT_ITEM
{
    uchar len;                //字符或字符串字节数
};

typedef enum{
    ETB_NORMAL,
    ETB_SELECTED,
    ETB_DISABLED,
}EditBitmap;

typedef std::vector<EDIT_ITEM> EDIT_ITEM_VECTOR;

class CEdit : public CItem
{
    int     curpos;            //当前位置
    int     showpos;            //开始显示的位置
    int     word_pos;            //当前词的位置
    int     len_max;            //字符最大个数
    EDIT_ITEM_VECTOR items;    //词信息
    std::string input;        //用户输入
    CNumberPad* numpad;        //数字输入框
    int caret_cnt;            //插入符闪烁计数
    int caret_pos;            //插入符位置
    int last_char_with;        //最后一个字符的宽度
    VD_RECT caret_rect;        //插入符的区域
    VD_BOOL modifying;            //是否正在修改
    VD_COLORREF    m_BkColor;    //背景颜色
static    KEY_INPUT_TYPE input_type;//当前输入法类型
static    VD_BOOL m_bMarkPadShown;    //符号面板被自动打开
    CTRLPROC m_onChanged;
        
private:
    void DrawCaret();
    void UpdateCaretPos(VD_BOOL redraw = FALSE);
    int GetTextExtent(int pos, int len = 1024);
protected:
    static VD_BITMAP* bmp_etb_normal;
    static VD_BITMAP* bmp_etb_selected;
    static VD_BITMAP* bmp_etb_disabled;

public:
	VD_BOOL IsPadOpen();
    //CEdit控件不具有自动聚焦的属性,--by wangqin 20070316
    CEdit(VD_PCRECT pRect, CPage * pParent, int vlen=6, uint vstyle = 0, CTRLPROC onChanged = NULL);
    virtual ~CEdit();

    void ShowDropDown(VD_BOOL show);
    void SetFocus(VD_BOOL flag);
    void Draw();
    VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
    void Select(VD_BOOL flag);
    void Empty();
    int GetText(char *buf, int vlen);
    void SetText(const char *psz);
    void SetBkColor(VD_COLORREF color, VD_BOOL redraw = FALSE);
    uint GetStyle();
    VD_SIZE& GetDefaultSize(void);

    static void SetDefaultBitmap(EditBitmap region, VD_PCSTR name);
};
CEdit* CreateEdit(VD_PCRECT pRect, CPage * pParent, int vlen=6, uint vstyle = 0, CTRLPROC onChanged = NULL);

//add by nike.xie 20090907  //对域名编辑框做特殊处理
class CDDNSEdit: public CEdit
{
public:
    CDDNSEdit(VD_PCRECT pRect, CPage * pParent, int vlen=6, uint vstyle = 0, CTRLPROC onChanged = NULL);
    virtual ~CDDNSEdit();

    VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
};
//end

#endif //__CTRL_EDIT_H__
