
#ifndef __CTRL_RADIO_H__
#define __CTRL_RADIO_H__

class CRadio;
typedef std::vector<CRadio *> PRADIOGROUP;

class CRadio : public CItem
{
	//std::string	string;	//控件字符串
	CRadio		*m_pHead;	//第一个单选框
	int			m_iIndex;	//在组里的序号
	int			m_iCurSel;	//当前项, 只对第一个单选框有效
	PRADIOGROUP m_Group;//单选框组, 只对第一个单选框有效
	CTRLPROC	m_onChanged;
	VD_BITMAP* m_pRadioImageNormal;
	VD_BITMAP* m_pRadioImageSelect;
	VD_BITMAP* m_pRadioImagePush;
	VD_BITMAP* m_pRadioImageDisable;

public:
	CRadio(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz = NULL, CTRLPROC onChanged = NULL,uint style = 0);//onChanged只对首项有用
	CRadio(VD_PCRECT pRect, CPage * pParent, CRadio* pHead, VD_PCSTR psz = NULL,uint style = 0);
	virtual ~CRadio();

	void Draw();
	void Select(VD_BOOL flag);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	int SetCurSel(int sel);
	int GetCurSel();
	void Switch();
	VD_BOOL IsHead();
	VD_SIZE& GetDefaultSize(void);

	VD_BOOL SetBitmap(VD_BITMAP* pBmpNormal = NULL,	VD_BITMAP* pBmpSelect = NULL, VD_BITMAP* pBmpPush = NULL, VD_BITMAP* pBmpDisable = NULL);
	void DrawCtrlRadio(CTRL_STATE state= ctrlStateNormal);
};
CRadio* CreateRadio(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz = NULL, CTRLPROC onChanged = NULL,uint style = 0);
CRadio* CreateRadio(VD_PCRECT pRect, CPage * pParent, CRadio* pHead, VD_PCSTR psz = NULL,uint style = 0);

#endif //__CTRL_RADIO_H__
