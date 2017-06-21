#ifndef _PAGE_NETWORK_CONFIG_H_
#define _PAGE_NETWORK_CONFIG_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>
#include "common_basetypes.h"

#define _RZ_P2P_

#ifdef _RZ_P2P_
typedef enum
{
	P2P_TX_HEART = 'h',//"h"表示心跳帧
	P2P_TX_CMD = 'd',//"d"表示通讯数据帧
	P2P_TX_OVER = 'o',
	P2P_TX_LOST = 'E',
}EmP2PTxType;
#define TX_HEAD_LEN	3
#define TX_DATA_LEN	20
#define TX_TAIL_LEN	2
//头部三字节为0x58,0x44,0x43,尾部两字节为0x52,0x5A
typedef struct
{
	u8 head[TX_HEAD_LEN];
	u8 leixing;//参考EmP2PTxType
	u8 shuju[TX_DATA_LEN];
	u8 jiaoyan;//为类型和数据部分的和的模256校验
	u8 weibu[TX_TAIL_LEN];
}Tongxun;
#endif

#ifdef _RZ_P2P_
#define NETWORKCFG_SUBPAGES	5
#define NETWORKCFG_BTNNUM	8
#else
#define NETWORKCFG_SUBPAGES	4
#define NETWORKCFG_BTNNUM	7
#endif

/*
#define NETWORKCFG_MAXSUB   2

typedef struct tagMAINFRAME_ITEM
{
	std::vector<CItem*> SubItems[NETWORKCFG_MAXSUB];
	int			cursubindex;
}MAINFRAME_ITEM;
*/

/*
 配置接口存取网络参数结构
*/

typedef struct _sGuiNetAdvancePara
{
	char	szSmtpServer[64];
	// 发件人邮箱
	char	szSendMailBox[64];
	char	szSMailPasswd[64];

    u8		nSSLFlag;

	// 收件人邮箱
	char	szReceiveMailBox[64];
	char	szReceiveMailBox2[64];
	char	szReceiveMailBox3[64];

	// 附加图片
	char	szSnapPic[64];
	
	// 抓图间隔
	u32		nSnapIntvl;
	
} SGuiNetAdvancePara;

typedef struct _sGuiNetConfigPara
{
	// 基本参数
	u8     	Version[8];			    // 8字节的版本信息
	char   	HostName[16];           // 主机名
	u32  	HostIP;                 // IP 地址
	u32  	Submask;                // 子网掩码
	u32  	GateWayIP;              // 网关 IP
	u32  	DNSIP;                  // DNS IP
	u32  	DNSIPAlt;               // DNS IP
	u8 		MAC[18];				// MAC Address

    // 外部接口
    u32   	AlarmServerIP;          // 报警中心IP
    u16 	AlarmServerPort;        // 报警中心端口
    u32   	SMTPServerIP;           // SMTP server IP
    u16 	SMTPServerPort;         // SMTP server port
	
    u32   	LogServerIP;            // Log server IP
    u16 	LogServerPort;          // Log server port

    // 本机服务端口
    u16 	HttpPort;               	// HTTP服务端口号
    u16 	MobilePort;              	// 手机监控端口号
    
    u16 	TCPMaxConn;             	// TCP 最大连接数
    u16 	TCPPort;                	// TCP 侦听端口

	u8		nSubStreamMax;				// 子码流数上限
	u8		nSendBufNumPerChn;			// 每通道发送buff数
	u32		nFrameSizeMax;				// 帧buff大小上限
	
	// 网路子码流
	u8		nVideoMediaType;			// 视频编码类型
	u8		nAudioMediaType;			// 预览音频编码类型
	u8		nVoipMediaType;				// 对讲音频编码类型
	u8		nAudioSampleMode;			// 音频采样模式

    u16 	UDPPort;                	// UDP 侦听端口

    u8		McastFlag;
    u16 	McastPort;              	// 组播端口
    u32 	McastIP;                	// 组播IP
    
	u8		PPPOEFlag;					// pppoe 使能开关
	u8		PPPOEUser[64];
	u8		PPPOEPasswd[64];
	
	u8		DhcpFlag;					// Dhcp 使能开关
	
	u8		DDNSFlag;					// DDNS 控制位 0 closed, 1, 2, 3 other active sel
	u8		DDNSDomain[64];
	u8		DDNSUser[64];
	u8		DDNSPasswd[64];
	u16		UpdateIntvl;				// DDNS	IP更新间隔
	
    // 其他
    u8    	MonMode;                	// 监视协议 TCP|UDP|MCAST
    u8    	PlayMode;               	// 回放协议 TCP|UDP|MCAST
    u8    	AlmSvrStat;             	// 报警中心状态 <开/关>
	
	// 高级配置
    SGuiNetAdvancePara	sAdancePara;
	
	//csp modify 20130321
	u8		UPNPFlag;
} SGuiNetPara;

//编码参数
typedef struct 
{
	u32 nBitRate; // 位率(单位为Kb)
	u8 nFrameRate; // 帧率
	u32 nGop; // 关键帧间隔
	u32 nMinQP; // (暂不支持)
	u32 nMaxQP; // (暂不支持)

	u8 nVideoResolution;  // 视频分辨率(EMBIZENCRESOLUTION)
	u8 nEncodeType; // 视频编码类型(EMBIZENCTYPE数字,暂时只支持固定取值98,H.264) 不支持设置
	u8 nBitRateType; // 位率类型(EMBIZENCBITRATETYPE)
	u8 nPicLevel; // 图像质量(EMBIZENCPICLEVEL)
} SGuiNetStreamPara;

class CPageNetworkConfig :public CPageFrame
{
public:
	CPageNetworkConfig(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageNetworkConfig();
	
	VD_PCSTR GetDefualtTitleCenter();
	
	VD_BOOL UpdateData( UDM mode );
	//VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	void SwitchDHCP();
	void LoadPara( int curID );
	void CtrlEnable(VD_BOOL flag);
	void SetDynNetPara(uint ip, uint submsk, uint gw, uint dns1);
	void SetNetStatus(uchar type, uchar state, uchar rslt); // 0 dhcp 1 pppoe 2 mail 3 ddns; 0 suc, 1 fail 
	int  GetProtocolValue(int sel);//cw_ddns
	
protected:
	
private:
	void LoadMailCfg(SGuiNetPara* pCfg);
	void UpdateDomain();
	BOOL Digital(const char *s);
	void MessgeBox(int index);
	
	void ShowRegistButton( void );
	void TestMailInfo(uchar idx, uchar act, uchar rslt);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	CStatic* 		pStatus;
	CScrollBar* 	pScrollbar;
	SGuiNetStreamPara*	pTabPara;
	int             nRealRow;
	int             nPageNum;
	int             nHideRow;
	int             nCurPage;
	
	void 			OnTrackMove0();
	void			AdjHide();
	
	int	nChMax;
	
	int	m_nLastSel;
	
	int	curID;
	
	void OnCombox3();
	void OnCheckBox3();
	void OnCombSel();
	//void OnClkConfigItem();
	void SwitchPage( int mainID,int subID);
	void ShowSubPage(int mainID,int subID, BOOL bShow);

	void OnClickSubPage();
	void OnClickBtn();
	void OnTrackMove();

	CButton* pButton[NETWORKCFG_BTNNUM+2];
	CTableBox* pTable;
	CStatic* pInfoBar;
	CStatic* pIconInfo;

	void SetInfo(char* szInfo);
	void ClearInfo();
	//CButton* pItemButton[3];

	//"网络"子页面0
	void InitPage00();
	void OnEditChange00();
	void OnTrackMove00();
	int lastPos00;
	CStatic* pStatic00[9];
	CEdit* pEdit00[2+1];//20111024 mobile port
	CIPAddressCtrl* pIpCtrl00[5];
	CCheckBox* pCheckBox00;
	CTableBox* pTable00;
	CTableBox* pTable01;
	CScrollBar* pScrollbar00;
	
	//"网络"子页面1
	void InitPage01();
	void OnTest01();
	
	CCheckBox* pCheckBox01;
	CEdit* pEdit01[2];
	
	//csp modify
	//CStatic* pStatic01[2];
	CStatic* pStatic01[3];
	
	CButton* pButton01;
	
	//csp modify
	CStatic* pStaticpppoetitle[4];
	CIPAddressCtrl* pIpCtrlpppoecontext[4];
	//unsigned char pppoe_flag;
	u8 pppoe_flag;
	
	//csp modify 20130321
	CStatic* pStaticupnptitle[1];
	CCheckBox* pCheckBoxupnp[1];
	void OnCheckBoxupnp();
	
	//"码流"子页
	void InitPage1();
	CStatic* pStatic1[12+2];
	CComboBox* pComboBox1[25+5*2];
	CCheckBox* pCheckBox1;
	CTableBox* pTable10;
	CTableBox* pTable11;
	void OnCheckBox01();
	
	void OnEditChange2();
	//"Email"子页0
	void InitPage20();
	void OnTest20();
	void OnTrackMove20();
	int lastPos20;
	CStatic* pStatic20[8];
	CEdit* pEdit20[8];
	CCheckBox* pCheckBox20;
	CButton* pButton20;
	CTableBox* pTable20;
	CTableBox* pTable21;
	CScrollBar* pScrollbar20;
	
	//"Email"子页1
	void InitPage21();
	CStatic* pStatic21[3];
	CEdit* pEdit21[2];
	
	//"更多设置"子页
	void InitPage3();
	void OnTest3();
	void OnEditChange3();
	CStatic* pStatic3[6];
	CCheckBox* pCheckBox3;
	CComboBox* pComboBox3[2];
	CEdit* pEdit3[3];
	CButton* pButton3;
	CTableBox* pTable3;
	void LoadButtomCtl();
	
	unsigned char m_IsLoader;
	
#ifdef _RZ_P2P_
	void InitPage4();
	
	void OnCheckBox4();
	
	void p2pregister();
	
	void modifymac(char *macaddr);
	void setnetwork();
	
	void mtdopmac(char *mac);
	void mtdopuuid(char *uid);
	
	CTableBox* pTable4;
	CCheckBox* pCheckBox4;
	CStatic* pStatic4[7];
	CEdit* pEdit4[3];
	CButton* pButton4;
#endif
};

#endif //_PAGE_NETWORK_CONFIG_H_

