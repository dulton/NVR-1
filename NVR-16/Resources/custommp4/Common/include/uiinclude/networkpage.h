#ifndef _NETWORKPAGE_H_
#define _NETWORKPAGE_H_

#include "ui.h"



#define IDC_PICTURE_NETWORK_ETH		IDD_DIALOG_NETWORK+1
#define IDC_PICTURE_NETWORK_PPPOE	IDD_DIALOG_NETWORK+2
#define IDC_PICTURE_NETWORK_DDNS	IDD_DIALOG_NETWORK+3

#define PIC_ETH		"data/pics/tools/eth.png"
#define PIC_PPPOE	"data/pics/tools/pppoe.png"
#define PIC_DDNS	"data/pics/tools/ddns.png"



#define IDC_STATIC_NETWORK_MAC				 IDD_DIALOG_ETHER+1
#define IDC_STATIC_NETWORK_IP				 IDD_DIALOG_ETHER+2
#define IDC_STATIC_NETWORK_SUBNET			 IDD_DIALOG_ETHER+3
#define IDC_STATIC_NETWORK_DNS				 IDD_DIALOG_ETHER+4
#define IDC_STATIC_NETWORK_SERVIP			 IDD_DIALOG_ETHER+5
#define IDC_STATIC_NETWORK_HTTPPORT			 IDD_DIALOG_ETHER+6

#define IDC_EDIT_NETWORK_MAC				 IDD_DIALOG_ETHER+7
#define IDC_EDIT_NETWORK_IP					 IDD_DIALOG_ETHER+8
#define IDC_EDIT_NETWORK_SUBNET				 IDD_DIALOG_ETHER+9
#define IDC_EDIT_NETWORK_DNS				 IDD_DIALOG_ETHER+10
#define IDC_EDIT_NETWORK_SERVIP				 IDD_DIALOG_ETHER+11
#define IDC_EDIT_NETWORK_HTTPPORT			 IDD_DIALOG_ETHER+12

#define IDC_STATIC_NETWORK_PORT				 IDD_DIALOG_ETHER+13
#define IDC_STATIC_NETWORK_GATEWAY			 IDD_DIALOG_ETHER+14
#define IDC_STATIC_NETWORK_MULTICAST		 IDD_DIALOG_ETHER+15
#define IDC_STATIC_NETWORK_SERVPORT			 IDD_DIALOG_ETHER+16

#define IDC_EDIT_NETWORK_PORT				 IDD_DIALOG_ETHER+17
#define IDC_EDIT_NETWORK_GATEWAY			 IDD_DIALOG_ETHER+18
#define IDC_EDIT_NETWORK_MULTICAST			 IDD_DIALOG_ETHER+19
#define IDC_EDIT_NETWORK_SERVPORT			 IDD_DIALOG_ETHER+20

#define IDC_STATIC_NETWORK_PPPOE			 IDD_DIALOG_ETHER+21
#define IDC_CHECK_NETWORK_PPPOEENABLE		 IDD_DIALOG_ETHER+22

#define IDC_STATIC_NETWORK_PPPOEUSER		 IDD_DIALOG_ETHER+23
#define IDC_EDIT_NETWORK_PPPOEUSER			 IDD_DIALOG_ETHER+26

#define IDC_BUTTON_NETWORK_OK				 IDD_DIALOG_ETHER+29
#define IDC_BUTTON_NETWORK_CANCEL			 IDD_DIALOG_ETHER+30

#define IDC_IPMACADDR_NETWORK_IP			 IDD_DIALOG_ETHER+31
#define IDC_IPMACADDR_NETWORK_SUBNET		 IDD_DIALOG_ETHER+32
#define IDC_IPMACADDR_NETWORK_GATEWAY		 IDD_DIALOG_ETHER+33
#define IDC_IPMACADDR_NETWORK_DNS			 IDD_DIALOG_ETHER+34
#define IDC_IPMACADDR_NETWORK_MULTICAST      IDD_DIALOG_ETHER+35
#define IDC_IPMACADDR_NETWORK_SERVIP		 IDD_DIALOG_ETHER+36
#define IDC_BUTTON_NETWORK_PWSET			 IDD_DIALOG_ETHER+37

#define IDC_STATIC_NETWORK_SHIFTPROMPT		 IDD_DIALOG_ETHER+38

#define IDC_STATIC_NETWORK_DHCP				 IDD_DIALOG_ETHER+39
#define IDC_CHECK_NETWORK_DHCPENABLE		 IDD_DIALOG_ETHER+40

#define IDC_STATIC_NETWORK_DDNS				 IDD_DIALOG_ETHER+41
#define IDC_CHECK_NETWORK_DDNSENABLE		 IDD_DIALOG_ETHER+42

#define IDC_STATIC_NETWORK_DDNSDOMAIN		 IDD_DIALOG_ETHER+43
#define IDC_EDIT_NETWORK_DDNSDOMAIN			 IDD_DIALOG_ETHER+44
#define IDC_STATIC_NETWORK_DDNSUSER			 IDD_DIALOG_ETHER+45
#define IDC_EDIT_NETWORK_DDNSUSER			 IDD_DIALOG_ETHER+46

#define IDC_BUTTON_NETWORK_DDNSPWSET		 IDD_DIALOG_ETHER+47

#define IDC_STATIC_NETWORK_SHIFTPROMPT2		 IDD_DIALOG_ETHER+48
#define IDC_STATIC_NETWORK_SHIFTPROMPT3		 IDD_DIALOG_ETHER+49
#define IDC_INPUTBOX_NETWORK_DENO		 IDD_DIALOG_ETHER+50

#define IDC_STATIC_NETWORK_MULTIENABLE  IDD_DIALOG_ETHER+51
#define IDC_COMBO_NETWORK_MULTIENABLE  IDD_DIALOG_ETHER+52

#define IDC_STATIC_NETWORK_PORT2				 IDD_DIALOG_ETHER+53
#define IDC_EDIT_NETWORK_PORT2				 IDD_DIALOG_ETHER+54
#define IDC_STATIC_IP_ERROR		IDD_DIALOG_ETHER+55
#define IDC_EDIT_NETWORK_MAC_STATIC			IDD_DIALOG_ETHER+56

//#ifdef JUNMINGSHI_KAICONG
#define IDC_STATIC_NETWORK_UPNP				IDD_DIALOG_ETHER+57
#define IDC_CHECK_NETWORK_UPNP				IDD_DIALOG_ETHER+58
//#endif

#define IDC_STATIC_DDNS_ENABLE	IDD_DIALOG_DDNS+1
#define IDC_CHECK_DDNS_ENABLE		IDD_DIALOG_DDNS+2
#define IDC_STATIC_DDNS_TYPE		IDD_DIALOG_DDNS+3
#define IDC_COMBO_DDNS_TYPE		IDD_DIALOG_DDNS+4
#define IDC_STATIC_DDNS_USERNAME	IDD_DIALOG_DDNS+5
#define IDC_EDIT_DDNS_USERNAME	IDD_DIALOG_DDNS+6
#define IDC_STATIC_DDNS_PASSWORD	IDD_DIALOG_DDNS+7
#define IDC_EDIT_DDNS_PASSWORD	IDD_DIALOG_DDNS+8
#define IDC_STATIC_DDNS_URL		IDD_DIALOG_DDNS+9
#define IDC_EDIT_DDNS_URL			IDD_DIALOG_DDNS+10
#define IDC_BUTTON_DDNS_OK		IDD_DIALOG_DDNS+11
#define IDC_BUTTON_DDNS_CANCEL	IDD_DIALOG_DDNS+12
#define IDC_EDIT_DDNS_PASSWORD2	IDD_DIALOG_DDNS+13
#define IDC_STATIC_DDNS_PASSWORD2	IDD_DIALOG_DDNS+14
#define IDC_STATIC_DDNS_URL2		IDD_DIALOG_DDNS+15//20100618 zlb_las			//lonse_new
#define IDC_BUTTON_DDNS_APPLY_OK		IDD_DIALOG_DDNS+16//20100618 zlb_las		//lonse_new
#define IDC_STATIC_DDNS_APPLY_STATUS		IDD_DIALOG_DDNS+17//20100618 zlb_las		//lonse_new
#ifdef JUNMINGSHI_DDNS
#define IDC_BUTTON_DDNS_CANCEL_OK		IDD_DIALOG_DDNS+18//20100618 zlb_las		//lonse_new
#endif
#ifdef PEOPLE_DDNS
#define IDC_STATIC_DDNS_HWID        			 IDD_DIALOG_DDNS+19
#define IDC_EDIT_DDNS_HWID			 IDD_DIALOG_DDNS+20
#endif
//pw 2010/7/31
#define IDC_STATIC_DDNS_APPLY_USERNAME	IDD_DIALOG_APPLY_DDNS+1
#define IDC_EDIT_DDNS_APPLY_USERNAME		IDD_DIALOG_APPLY_DDNS+2
#define IDC_STATIC_DDNS_APPLY_PASSWORD	IDD_DIALOG_APPLY_DDNS+3
#define IDC_EDIT_DDNS_APPLY_PASSWORD	IDD_DIALOG_APPLY_DDNS+4
//#define IDC_BUTTON_DDNS_APPLY_OK			IDD_DIALOG_APPLY_DDNS+5
#define IDC_BUTTON_DDNS_APPLY_CANCEL		IDD_DIALOG_APPLY_DDNS+6
#define IDC_BUTTON_DDNS_APPLY_DDNS		IDD_DIALOG_APPLY_DDNS+15	

#define IDC_STATIC_PPPOE_USERNAME	IDD_DIALOG_PPPOE+1
#define IDC_EDIT_PPPOE_USERNAME	IDD_DIALOG_PPPOE+2
#define IDC_STATIC_PPPOE_PASSWORD	IDD_DIALOG_PPPOE+3
#define IDC_EDIT_PPPOE_PASSWORD	IDD_DIALOG_PPPOE+4
#define IDC_STATIC_PPPOE_ENABLE	IDD_DIALOG_PPPOE+5
#define IDC_CHECK_PPPOE_ENABLE	IDD_DIALOG_PPPOE+6
#define IDC_EDIT_PPPOE_IP	IDD_DIALOG_PPPOE+7
#define IDC_STATIC_PPPOE_IP	IDD_DIALOG_PPPOE+8
#define IDC_STATIC_PPPOE_SUBNET	IDD_DIALOG_PPPOE+9
#define IDC_EDIT_PPPOE_SUBNET	IDD_DIALOG_PPPOE+10
#define IDC_STATIC_PPPOE_GATEWAY	IDD_DIALOG_PPPOE+11
#define IDC_EDIT_PPPOE_GATEWAY	IDD_DIALOG_PPPOE+12

#define IDC_STATIC_PPPOE_DNS	IDD_DIALOG_PPPOE+13
#define IDC_EDIT_PPPOE_DNS	IDD_DIALOG_PPPOE+14
#define IDC_BUTTON_PPPOE_OK	IDD_DIALOG_PPPOE+15
#define IDC_BUTTON_PPPOE_CANCEL	IDD_DIALOG_PPPOE+16

#define IDC_STATIC_PPPOE_PASSWORD2	IDD_DIALOG_PPPOE+17
#define IDC_EDIT_PPPOE_PASSWORD2		IDD_DIALOG_PPPOE+18

BOOL CreateNetworkPage();
BOOL ShowNetworkPage();
BOOL CeateEtherPage();
BOOL ShowEtherPage();
BOOL CeatePPPOEPage();
BOOL ShowPPPOEPage();
BOOL CeateDDNSPage();
BOOL ShowDDNSPage();

//pw 2010/7/31
#ifdef LONGANSHI_DDNS 								//dong	100308	addapplyddns
BOOL CeateDDNSApplyPage();
BOOL ShowDDNSApplyPage();
#endif

#endif

