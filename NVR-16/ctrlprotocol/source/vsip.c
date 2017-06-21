
#include "vsip.h"

#ifdef CSVISION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SOCKHANDLE vsip_udp_socket = INVALID_SOCKET;
struct sockaddr_in vsip_serv;

VSIP_CAPABILITY vsip_capability;

GUID g_UnitGuid = {0,0,0,{0x00,0x00,0x00,0x70,0x18,0x30,0xc9,0x81}};

VSWC_PING_RESPOND_MSG	vsip_ping_resp_msg;
VSWC_D_GET_CONFIG_RESP	vsip_sys_pub_param;

struct VSWC_VCODEC_ENC_CONTEXT	m_VCodecEncContext[4];

void n2hVSIPHeader(VSWC_CONTROL_HEADER *pHeader)
{
	pHeader->m_Version	= ntohs(pHeader->m_Version);
	pHeader->m_TransNum = ntohs(pHeader->m_TransNum);
	pHeader->m_MsgLen	= ntohs(pHeader->m_MsgLen);
}

void h2nVSIPHeader(VSWC_CONTROL_HEADER *pHeader)
{
	pHeader->m_Version	= htons(pHeader->m_Version);
	pHeader->m_TransNum = htons(pHeader->m_TransNum);
	pHeader->m_MsgLen	= htons(pHeader->m_MsgLen);
}

void n2hGUID(GUID *pID)
{
	pID->Data1 = ntohl(pID->Data1);
	pID->Data2 = ntohs(pID->Data2);
	pID->Data3 = ntohs(pID->Data3);
}

void h2nGUID(GUID *pID)
{
	pID->Data1 = htonl(pID->Data1);
	pID->Data2 = htons(pID->Data2);
	pID->Data3 = htons(pID->Data3);
}

u16 VSIPInit()
{
	struct sockaddr_in local;
	
/*#ifdef WIN32
	{
		WSADATA wsaData;
		DWORD err;
		
		err = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if(err != 0)
		{
			return CTRL_FAILED_PARAM;
		}
	}
#endif*/
	
	vsip_udp_socket = socket(AF_INET,SOCK_DGRAM,0);
	
	{
		int optval = 1;
		if( SOCKET_ERROR == setsockopt(vsip_udp_socket, SOL_SOCKET, SO_BROADCAST,
			(char*)&optval, sizeof(optval)))
		{
			printf("SO_BROADCAST failed\n");
		}
		else
		{
			printf("SO_BROADCAST success\n");
		}
	}
	
	local.sin_family		= AF_INET;
	local.sin_addr.s_addr	= htonl(0);
	local.sin_port			= htons(VSWC_PORT);
	if(SOCKET_ERROR == bind(vsip_udp_socket,(struct sockaddr *)&local,sizeof(local)))
	{
		#ifdef WIN32
		closesocket(vsip_udp_socket);
		#else
		close(vsip_udp_socket);
		#endif
		vsip_udp_socket = INVALID_SOCKET;
		printf("bind failed\n");
		return CTRL_FAILED_BIND;
	}
	else
	{
		printf("bind success\n");
	}
	
	return CTRL_SUCCESS;
}

u16 SetVsipServAddr(u32 dwIp,u16 wPort)
{
	vsip_serv.sin_family = AF_INET;
	vsip_serv.sin_addr.s_addr = dwIp;
	vsip_serv.sin_port = htons(wPort);
	return CTRL_SUCCESS;
}

u16 GetVsipGuid(GUID *pGuid)
{
	if(pGuid == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	*pGuid = g_UnitGuid;
	return CTRL_SUCCESS;
}

u16 SetVsipGuid(char *pMac)
{
	char chSeps[8] = ":";
	char *pchToken = NULL;
	char *pchEndPos = NULL;
	
	if(pMac == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	
	g_UnitGuid.Data1 = 0;
	g_UnitGuid.Data2 = 0;
	g_UnitGuid.Data3 = 0;
	memset(g_UnitGuid.Data4,0,sizeof(g_UnitGuid.Data4));
	
	pchToken = strtok(pMac,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[2] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[3] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[4] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[5] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[6] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	g_UnitGuid.Data4[7] = (unsigned char)strtoul(pchToken,&pchEndPos,16);
	
	return CTRL_SUCCESS;
}

u16 SetVsipCapability(VSIP_CAPABILITY *pCapability)
{
	if(pCapability == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	vsip_capability = *pCapability;
	
	if(vsip_capability.byVideoDecNum > 0)
	{
		vsip_capability.byVideoDecNum = 0;
	}
	
	if(vsip_capability.byAudioEncNum > 1)
	{
		vsip_capability.byAudioEncNum = 1;
	}
	vsip_capability.byAudioEncNum = 0;
	
	if(vsip_capability.byAudioDecNum > 0)
	{
		vsip_capability.byAudioDecNum = 0;
	}
	
	if(vsip_capability.bySerialPortNum > 2)
	{
		vsip_capability.bySerialPortNum = 2;
	}
	
	if(vsip_capability.byInputPinNum > 6)
	{
		vsip_capability.byInputPinNum = 6;
	}
	
	if(vsip_capability.byOutputPinNum > 3)
	{
		vsip_capability.byOutputPinNum = 3;
	}
	
	return CTRL_SUCCESS;
}

unsigned short CountVsipCapability()
{
	int count = 0;
	count += vsip_capability.byVideoEncNum;//video enc
	count += vsip_capability.byVideoDecNum;
	count += vsip_capability.bySerialPortNum;//serial por
	count += vsip_capability.byInputPinNum;//input pin
	count += vsip_capability.byOutputPinNum;//output pin
	count += vsip_capability.byAudioEncNum;
	count += vsip_capability.byAudioDecNum;
	count += vsip_capability.byVideoInputPortNum;//md
	return count;
}

u16 GetVsipPingRespMsg(VSWC_PING_RESPOND_MSG *pMsg)
{
	if(pMsg == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	*pMsg = vsip_ping_resp_msg;
	
	pMsg->m_Header.m_Version	= VSWC_PROTOCOL_VERSION;
	pMsg->m_Header.m_MsgType 	= VSWC_MSG_TYPE_D_PING_RESP;
	pMsg->m_Header.m_TransNum	= 0;
	pMsg->m_Header.m_MsgLen		= sizeof(VSWC_PING_RESPOND_MSG);//35;
	h2nVSIPHeader(&pMsg->m_Header);
	
	pMsg->m_UnitIP 				= pMsg->m_UnitIP;
	pMsg->m_VswcPort			= htons(pMsg->m_VswcPort);
	pMsg->m_ConnType			= pMsg->m_ConnType;
	
	memcpy(&(pMsg->m_UnitGUID),&g_UnitGuid,sizeof(GUID));
	h2nGUID(&(pMsg->m_UnitGUID));
	
	pMsg->m_ManuID				= 0;
	pMsg->m_UnitType			= 0;
	pMsg->m_Status				= 0;
	
	return CTRL_SUCCESS;
}

u16 SetVsipPingRespMsg(VSWC_PING_RESPOND_MSG *pMsg)
{
	if(pMsg == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	vsip_ping_resp_msg = *pMsg;
	return CTRL_SUCCESS;
}

u16 GetVsipSysPubParam(VSWC_D_GET_CONFIG_RESP *pMsg)
{
	if(pMsg == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	*pMsg = vsip_sys_pub_param;
	
	//pMsg->VSWCHeader
	
	pMsg->m_DeviceGUID	= g_UnitGuid;
	h2nGUID(&pMsg->m_DeviceGUID);
	
	pMsg->m_232Baud		= ntohs(pMsg->m_232Baud);
	pMsg->m_485Baud		= ntohs(pMsg->m_485Baud);

	pMsg->m_CenterIP	= pMsg->m_CenterIP;
	pMsg->m_Mask		= pMsg->m_Mask;
	pMsg->m_Gate		= pMsg->m_Gate;
	
	pMsg->m_DeviceType	= pMsg->m_DeviceType;
	
	return CTRL_SUCCESS;
}

u16 SetVsipSysPubParam(VSWC_D_GET_CONFIG_RESP *pMsg)
{
	if(pMsg == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	vsip_sys_pub_param = *pMsg;
	vsip_sys_pub_param.m_DeviceGUID = g_UnitGuid;
	return CTRL_SUCCESS;
}

u16 SendVsipDataToServ(char *buf,int len)
{
	if(vsip_udp_socket != INVALID_SOCKET && vsip_serv.sin_addr.s_addr != 0)
	{
		int ret = sendto(vsip_udp_socket,buf,len,0,(struct sockaddr *)&vsip_serv,sizeof(vsip_serv));
		if(ret != len)
		{
			return CTRL_FAILED_NETSND;
		}
		return CTRL_SUCCESS;
	}
	return CTRL_FAILED_NOINIT;
}

u16 SendPingRespMsgToServ()
{
	VSWC_PING_RESPOND_MSG PingResp;
	GetVsipPingRespMsg(&PingResp);
	return SendVsipDataToServ((char *)&PingResp,sizeof(PingResp));
}

int RespondADConfig(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;	
	unsigned int				ConfigValue;	
	
	SendOffset	= 0;	
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	////////////1.输入制式	
	Config.Id	= htons(CONFIG_VIDEO_INFO_ANALOG_FORMAT);
	ConfigValue	= htonl((unsigned int)VSWC_VIDEO_ANALOG_FORMAT_PAL);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////3.亮度	
	Config.Id	= htons(CONFIG_VIDEO_ATTRIBUT_BRIGHTNESS);
	ConfigValue	= htonl((unsigned int)4);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////4.对比度	
	Config.Id	= htons(CONFIG_VIDEO_ATTRIBUT_CONTRAST);
	ConfigValue	= htonl((unsigned int)8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////5.饱和度	
	Config.Id	= htons(CONFIG_VIDEO_ATTRIBUT_SATURATION);
	ConfigValue	= htonl((unsigned int)16);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//////////6.色彩
	Config.Id	= htons(CONFIG_VIDEO_ATTRIBUT_HUE);
	ConfigValue	= htonl((unsigned int)32);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

///////////////////////////////////////////////////////////////////////
int RespondVideoConfig(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int				ConfigValue;
	
	SendOffset	= 0;
	
	ListCount	= htons(9);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	SendOffset += RespondADConfig(RespBuf + SendOffset);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	
	////////////2.分辨率	
	Config.Id	= htons(CONFIG_VIDEO_COMPRESSION_DATA_FORMAT);
	ConfigValue	= htonl((unsigned int)VSWC_VIDEO_FORMAT_CIF);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////7.帧率
	Config.Id	= htons(CONFIG_VIDEO_COMPRESSION_FRAME_RATE);
	ConfigValue	= htonl(25);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////8.码率
	Config.Id	= htons(CONFIG_VIDEO_COMPRESSION_BIT_RATE);
	ConfigValue	= htonl((unsigned int)1548);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////9.量化参数
	Config.Id	= htons(CONFIG_VIDEO_COMPRESSION_QUANTIZATION);
	ConfigValue	= htonl((unsigned int)9);//图像质量:4-9;4最好
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

int RespondAudioEncConfig(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int                ConfigValue;
	
	SendOffset	= 0;
	
	ListCount	= htons(4);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	////////////1.编码格式	
	Config.Id	= htons(CONFIG_AUDIO_COMPRESSION_DATA_FORMAT);
	ConfigValue	= htonl((unsigned int)AUDIO_FORMAT_PCM);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////2.采样频率	
	Config.Id	= htons(CONFIG_AUDIO_COMPRESSION_SAMPLING_RATE);
	ConfigValue	= htonl(8000);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////3.通道数	
	Config.Id	= htons(CONFIG_AUDIO_COMPRESSION_CHANNEL);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////.数据位	
	Config.Id	= htons(CONFIG_AUDIO_COMPRESSION_SAMPLE_BITS);
	ConfigValue	= htonl(8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

int RespondAudioDecConfig(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int                ConfigValue;
	
	SendOffset	= 0;
	
	ListCount	= htons(4);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	////////////1.编码格式	
	Config.Id	= htons(CONFIG_AUDIO_COMPRESSION_DATA_FORMAT);
	ConfigValue	= htonl((unsigned int)AUDIO_FORMAT_PCM);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////2.采样频率	
	Config.Id		= htons(CONFIG_AUDIO_COMPRESSION_SAMPLING_RATE);
	ConfigValue		= htonl(8000);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////3.通道数	
	Config.Id		= htons(CONFIG_AUDIO_COMPRESSION_CHANNEL);
	ConfigValue		= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////.数据位	
	Config.Id		= htons(CONFIG_AUDIO_COMPRESSION_SAMPLE_BITS);
	ConfigValue		= htonl(8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

int Respond485Config(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int                ConfigValue;
	
	SendOffset					= 0;
	
	ListCount					= htons(4);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	////////////1.波特率
	Config.Id	= htons(CONFIG_SERIAL_PORT_BAUD_RATE);
	ConfigValue	= htonl((unsigned int)4800);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////2.停止位
	Config.Id	= htons(CONFIG_SERIAL_PORT_STOP_BITS);
	ConfigValue	= htonl((unsigned int)1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////3.校验位
	Config.Id	= htons(CONFIG_SERIAL_PORT_PARITY);
	ConfigValue	= htonl((unsigned int)0);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////.数据位
	Config.Id	= htons(CONFIG_SERIAL_PORT_DATA_BITS);
	ConfigValue	= htonl(8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

int Respond232Config(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int                ConfigValue;
	
	SendOffset					= 0;
	
	ListCount					= htons(4);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	////////////1.波特率
	Config.Id	= htons(CONFIG_SERIAL_PORT_BAUD_RATE);
	ConfigValue	= htonl((unsigned int)9600);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////2.停止位
	Config.Id	= htons(CONFIG_SERIAL_PORT_STOP_BITS);
	ConfigValue	= htonl((unsigned int)1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////3.校验位
	Config.Id	= htons(CONFIG_SERIAL_PORT_PARITY);
	ConfigValue	= htonl((unsigned int)0);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	////////////.数据位
	Config.Id	= htons(CONFIG_SERIAL_PORT_DATA_BITS);
	ConfigValue	= htonl(8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	return SendOffset;
}

int RespondMotionDetectionConfig(unsigned char *RespBuf,unsigned char chn)
{
	VSWC_MEDIA_CONTROL_CONFIG	Config;
	int							SendOffset;
	unsigned short				ListCount;
	unsigned int                ConfigValue;						
	char						*pbyStr = "0;116;10;12;10;12;10;12;10;12;10;12;10;12;10;12;10;116";
	unsigned short				StrLen = htons((unsigned short)strlen(pbyStr));
	
	SendOffset					= 0;
	
	ListCount					= htons(11);//htons(12);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	
	//1
	Config.Id	= htons(CONFIG_MOTD_UPPER_THRESHOLD);
	ConfigValue	= htonl((unsigned int)16);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//2
	Config.Id	= htons(CONFIG_MOTD_LOWER_THRESHOLD);
	ConfigValue	= htonl((unsigned int)8);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//3
	Config.Id	= htons(CONFIG_MOTD_NB_FRAMES);
	ConfigValue	= htonl((unsigned int)5);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//4
	Config.Id	= htons(CONFIG_MOTD_VECT_LENGTH);
	ConfigValue	= htonl(10);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//5
	Config.Id	= htons(CONFIG_MOTD_LEFT_TO_RIGHT);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//6
	Config.Id	= htons(CONFIG_MOTD_RIGHT_TO_LEFT);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//7
	Config.Id	= htons(CONFIG_MOTD_UP_TO_DOWN);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//8
	Config.Id	= htons(CONFIG_MOTD_DOWN_TO_UP);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	Config.ValueType = VSWC_VALUE_TYPE_UINT;
	
	//9
	Config.Id	= htons(CONFIG_MOTD_STATE);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//10
	Config.Id	= htons(CONFIG_MOTD_MASK_ENABLED);
	ConfigValue	= htonl(1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	
	//11
	Config.ValueType = VSWC_VALUE_TYPE_STRING;
	Config.Id	= htons(CONFIG_MOTD_MASK_STRING);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&StrLen,sizeof(StrLen));
	SendOffset += sizeof(StrLen);
	memcpy(RespBuf + SendOffset,pbyStr,ntohs(StrLen));
	SendOffset += ntohs(StrLen);
	
	#if 0
	//12
	Config.Id	= htons(CONFIG_MOTD_INPUT_INDEX);
	ConfigValue	= htonl(chn+1);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&ConfigValue,sizeof(ConfigValue));
	SendOffset += sizeof(ConfigValue);
	#endif
	
	return SendOffset;
}

int RespondProprietary(unsigned char *RespBuf)
{
	VSWC_MEDIA_CONTROL_PROPRIETARY_CONFIG	Config;
	int										SendOffset;
	unsigned short							ListCount;
	unsigned int							value;
	
	SendOffset	= 0;
	
	ListCount	= htons(5);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	Config.ValueType		= VSWC_VALUE_TYPE_UINT;
	Config.SectionId		= htons(VSWC_NETWORK_SECTIONID);
	Config.NameSize			= htonl(0);
	Config.HelpStringSize	= htonl(0);
	//////////NETWORK : 1.IP ADDRESS
	Config.Id				= htons(VSWC_IP_ADDRESS_ID);
	value					= vsip_ping_resp_msg.m_UnitIP;
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&value,sizeof(value));
	SendOffset += sizeof(value);
	memcpy(RespBuf + SendOffset,&Config.SectionId,sizeof(Config.SectionId));
	SendOffset += sizeof(Config.SectionId);
	memcpy(RespBuf + SendOffset,&Config.NameSize,sizeof(Config.NameSize));
	SendOffset += sizeof(Config.NameSize);
	memcpy(RespBuf + SendOffset,&Config.HelpStringSize,sizeof(Config.HelpStringSize));
	SendOffset += sizeof(Config.HelpStringSize);
	
	//////////NETWORK : 2.NET MASK
	Config.Id			= htons(VSWC_SUBNET_ID);
	value				= vsip_sys_pub_param.m_Mask;
	printf("**************RespondProprietary,NET MASK:0x%08x\n",vsip_sys_pub_param.m_Mask);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&value,sizeof(value));
	SendOffset += sizeof(value);
	memcpy(RespBuf + SendOffset,&Config.SectionId,sizeof(Config.SectionId));
	SendOffset += sizeof(Config.SectionId);
	memcpy(RespBuf + SendOffset,&Config.NameSize,sizeof(Config.NameSize));
	SendOffset += sizeof(Config.NameSize);
	memcpy(RespBuf + SendOffset,&Config.HelpStringSize,sizeof(Config.HelpStringSize));
	SendOffset += sizeof(Config.HelpStringSize);
	
	//////////NETWORK : 3.GATE WAY
	Config.Id			= htons(VSWC_GATEWAY_ID);
	value				= vsip_sys_pub_param.m_Gate;
	printf("**************RespondProprietary,Gateway:0x%08x\n",vsip_sys_pub_param.m_Gate);
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&value,sizeof(value));
	SendOffset += sizeof(value);
	memcpy(RespBuf + SendOffset,&Config.SectionId,sizeof(Config.SectionId));
	SendOffset += sizeof(Config.SectionId);
	memcpy(RespBuf + SendOffset,&Config.NameSize,sizeof(Config.NameSize));
	SendOffset += sizeof(Config.NameSize);
	memcpy(RespBuf + SendOffset,&Config.HelpStringSize,sizeof(Config.HelpStringSize));
	SendOffset += sizeof(Config.HelpStringSize);
	
	//////////VSIP : 4.VSIP DISCOVERY IP
	Config.SectionId	= htons(VSWC_CENTER_SECTIONID);
	Config.Id			= htons(VSWC_DISCOVERYIP_ID);
	value				= vsip_sys_pub_param.m_CenterIP;
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&value,sizeof(value));
	SendOffset += sizeof(value);
	memcpy(RespBuf + SendOffset,&Config.SectionId,sizeof(Config.SectionId));
	SendOffset += sizeof(Config.SectionId);
	memcpy(RespBuf + SendOffset,&Config.NameSize,sizeof(Config.NameSize));
	SendOffset += sizeof(Config.NameSize);
	memcpy(RespBuf + SendOffset,&Config.HelpStringSize,sizeof(Config.HelpStringSize));
	SendOffset += sizeof(Config.HelpStringSize);
	
	//////////VIDEO ENCODER : 5.RATE CONTROL MODE
	Config.SectionId	= htons(VSWC_VENCODER_SECTIONID);
	Config.Id			= htons(VSWC_RATE_CONTROL_MODE_ID);
	value				= htonl((unsigned int)(1));
	memcpy(RespBuf + SendOffset,&Config.Id,sizeof(Config.Id));
	SendOffset += sizeof(Config.Id);
	memcpy(RespBuf + SendOffset,&Config.ValueType,sizeof(Config.ValueType));
	SendOffset += sizeof(Config.ValueType);
	memcpy(RespBuf + SendOffset,&value,sizeof(value));
	SendOffset += sizeof(value);
	memcpy(RespBuf + SendOffset,&Config.SectionId,sizeof(Config.SectionId));
	SendOffset += sizeof(Config.SectionId);
	memcpy(RespBuf + SendOffset,&Config.NameSize,sizeof(Config.NameSize));
	SendOffset += sizeof(Config.NameSize);
	memcpy(RespBuf + SendOffset,&Config.HelpStringSize,sizeof(Config.HelpStringSize));
	SendOffset += sizeof(Config.HelpStringSize);
	
	return SendOffset;
}

int RespondOtherConfig(unsigned char *RespBuf)
{
	int							SendOffset;
	unsigned short				ListCount;
	
	SendOffset	= 0;
	
	ListCount	= htons(0);
	memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
	SendOffset += sizeof(ListCount);
	
	return SendOffset;	
}

int SetEntityConfig(unsigned char *ReqBuf,unsigned short EntityID,unsigned short ConfigID,unsigned char ValueType,unsigned short CapaAppID)
{
	struct VSWC_AD_CONTEXT 			*m_pADContext;
	struct VSWC_VCODEC_ENC_CONTEXT 	*m_pVCodecContext;
	struct VSWC_VCODEC_ENC_NOTIFY 	*m_pVCodecNotify;
	struct MOTION_DETECTION_CONTEXT *m_pMotionContext;
	int 							m_ADIICAddr;
	int								m_VCIICAddr;
	unsigned char					m_ADOffset;
	unsigned char					m_VCodecOffset;
	unsigned int 					ConfigValue;
	//unsigned char					Offset;
	//unsigned char 				OutValue;
	int 							MotionIICAddr;
	
	if(EntityID == GUID_VIDEO)
	{
		m_pADContext 			= &m_ADContext1;
		m_ADIICAddr  			= IIC_ADDR+8;
		m_ADOffset				= 8;
		
		m_VCIICAddr				= IIC_ADDR+8;
		m_VCodecOffset			= 28;
		m_pVCodecContext		= &m_VCodecEncContext[0];
		m_pVCodecNotify			= &m_EncNotify;
	}
	else if(EntityID == GUID_VIDEO+2)
	{
		m_pADContext 			= &m_ADContext2;
		m_ADIICAddr  			= IIC_ADDR+10;
		m_ADOffset				= 0;
		
		m_VCIICAddr				= IIC_ADDR+8;
		m_VCodecOffset			= 136;
		m_pVCodecContext		= &m_VCodecEncContext[1];
		m_pVCodecNotify			= &m_EncNotify2;
	}
	else if(EntityID == GUID_VIDEO+3)
	{
		m_pADContext 			= &m_ADContext3;
		m_ADIICAddr  			= IIC_ADDR+12;//guchanghai
		m_ADOffset				= 200;
		
		m_VCIICAddr				= IIC_ADDR+10;
		m_VCodecOffset			= 20;
		m_pVCodecContext		= &m_VCodecEncContext[2];
		m_pVCodecNotify			= &m_EncNotify3;
	}
	else if(EntityID == GUID_VIDEO+4)
	{
		m_pADContext 			= &m_ADContext4;
		m_ADIICAddr  			= IIC_ADDR+12;//guchanghai
		m_ADOffset				= 220;
		
		m_VCIICAddr				= IIC_ADDR+10;
		m_VCodecOffset			= 128;
		m_pVCodecContext		= &m_VCodecEncContext[3];			
		m_pVCodecNotify			= &m_EncNotify4;
	}
	else if(EntityID == GUID_MOTION_DETECTION)
	{
		if(CapaAppID==0)
		{
			m_pMotionContext	= &m_MotionContext;
			MotionIICAddr		= IIC_ADDR+2;
		}
		else if(CapaAppID==2)
		{
			m_pMotionContext	= &m_MotionContext3;
			MotionIICAddr		= IIC_ADDR+4;
		}
		else if(CapaAppID==3)
		{
			m_pMotionContext	= &m_MotionContext4;
			MotionIICAddr		= IIC_ADDR+6;
		}	
		else if(CapaAppID==4)
		{
			m_pMotionContext	= &m_MotionContext5;
			MotionIICAddr		= IIC_ADDR+14;
		}	
	}	
	
	switch (ConfigID)
	{
	case CONFIG_IO_PIN_STATE://设置开关量输出
		{
			//unsigned char writeromcontence[2];
			//unsigned char value = 0xff;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = ntohl(ConfigValue);
			
			if(ConfigValue == 1)//EntityID：输出闭合
			{
				if(EntityID == 0x07)//第一路输出
				{
					//writeromcontence[0]=0x02;
					//writeromcontence[1]=0xff;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 1 start\n");
				}
				else if(EntityID == 0x17)//第二路输出
				{
					//writeromcontence[0]=0x03;
					//writeromcontence[1]=0xff;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 2 start\n");
				}
				else if(EntityID == 0x27)//第三路输出
				{
					//writeromcontence[0]=0x04;
					//writeromcontence[1]=0xff;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 3 start\n");
				}
			}
			else if(ConfigValue == 0)//EntityID：输出断开
			{
				if(EntityID == 0x07)//第一路输出
				{
					//writeromcontence[0]=0x02;
					//writeromcontence[1]=0x00;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 1 stop\n");
				}
				else if(EntityID == 0x17)//第二路输出
				{
					//writeromcontence[0]=0x03;
					//writeromcontence[1]=0x00;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 2 stop\n");
				}
				else if(EntityID == 0x27)//第三路输出
				{
					//writeromcontence[0]=0x04;
					//writeromcontence[1]=0x00;
					//MyWriteMcu(0x60,writeromcontence,2);
					printf("alarm out 3 stop\n");
				}	
			}
		}
		break;
	case CONFIG_VIDEO_COMPRESSION_FRAME_RATE://设置编码帧率
		{
			unsigned short FrameRate;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = ntohl(ConfigValue);
			FrameRate = (unsigned short)ConfigValue;
			
			if(FrameRate != m_pVCodecContext->m_FrameRate)//帧率改变
			{
				m_pVCodecContext->m_FrameRate		= FrameRate;
				m_pVCodecNotify->m_FrameRateNotify	= 1;
				//MyWriteEEPROM(m_VCIICAddr,&Offset,(char *)&(m_pVCodecContext->m_FrameRate),sizeof(m_pVCodecContext->m_FrameRate));
			}
			
			printf("frame rate=%d\n",FrameRate);
		}
		break;
	case CONFIG_VIDEO_COMPRESSION_BIT_RATE://设置编码码率
		{
			unsigned short BitRate;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			BitRate = (unsigned short)ConfigValue;
			
			if(BitRate != m_pVCodecContext->m_BitRate)//码率改变
			{
				m_pVCodecContext->m_BitRate			= BitRate;
				m_pVCodecNotify->m_BitRateNofity	= 1;
				
				/*if(m_pVCodecContext->m_BitRate>=500)
					m_pVCodecContext->m_FrameRate = 25;
				else if((m_pVCodecContext->m_BitRate<500)&&(m_pVCodecContext->m_BitRate>400))
					m_pVCodecContext->m_FrameRate = 20;
				else if((m_pVCodecContext->m_BitRate<400)&&((m_pVCodecContext->m_BitRate>300)))
					m_pVCodecContext->m_FrameRate = 15;
				else if((m_pVCodecContext->m_BitRate<300)&&(m_pVCodecContext->m_BitRate>150))
					m_pVCodecContext->m_FrameRate = 10;
				else 
					m_pVCodecContext->m_FrameRate = 5;*/					
				
				//MyWriteEEPROM(m_VCIICAddr,&Offset,(char *)&(m_pVCodecContext->m_BitRate),sizeof(m_pVCodecContext->m_BitRate));
			}
			
			printf("bit rate=%d,frame rate=%d\n",BitRate,m_pVCodecContext->m_FrameRate);
		}
		break;
	case CONFIG_VIDEO_COMPRESSION_QUANTIZATION://设置最大量化参数
		{
			unsigned char MaxQuantizer;
					
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			MaxQuantizer = (unsigned char)ConfigValue;
			
			if(MaxQuantizer != m_pVCodecContext->m_MaxQuantizer)//最大量化参数改变
			{
				m_pVCodecContext->m_MaxQuantizer	= MaxQuantizer;
				m_pVCodecNotify->m_MaxQuanyNotify	= 1;
				//MyWriteEEPROM(m_VCIICAddr,&Offset,(char *)&(m_pVCodecContext->m_MaxQuantizer),sizeof(m_pVCodecContext->m_MaxQuantizer));
			}
			
			printf("MaxQuantizer=%d\n",MaxQuantizer);
		}
		break;
	case CONFIG_VIDEO_COMPRESSION_MIN_QUANTIZATION://设置最小量化参数
		{
			unsigned char MinQuantizer;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			MinQuantizer = (unsigned char)ConfigValue;
			
			if(MinQuantizer != m_pVCodecContext->m_MinQuantizer)//最小量化参数改变
			{
				m_pVCodecContext->m_MinQuantizer	= MinQuantizer;
				m_pVCodecNotify->m_MinQuantNotify	= 1;					
				//MyWriteEEPROM(m_VCIICAddr,&Offset,(char *)&(m_pVCodecContext->m_MinQuantizer),sizeof(m_pVCodecContext->m_MinQuantizer));
			}
			
			printf("MinQuantizer=%d\n",MinQuantizer);
		}
		break;
	case CONFIG_VIDEO_INFO_ANALOG_FORMAT://设置图像制式
		{
			unsigned char Analog;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Analog = (unsigned char)ConfigValue;
			
			if(Analog != m_pADContext->m_VideoStandard)//图像制式改变
			{
				m_pADContext->m_VideoStandard	= Analog;
				m_pVCodecNotify->m_AnalogNotify = 1;	
				///.....................别的动作--->api函数调用
				//MyWriteEEPROM(m_ADIICAddr,&Offset,(char *)&(m_pADContext->m_VideoStandard),sizeof(m_pADContext->m_VideoStandard));
				//m_bNeedReboot = 1;
				//reboot(4);
			}
			
			printf("Analog=%d\n",Analog);
		}
		break;
	case CONFIG_VIDEO_COMPRESSION_DATA_FORMAT://设置图像的分辨率
		{
			unsigned char Resolution;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Resolution = (unsigned char)ConfigValue;
			
			printf("Resolution=%d\n",Resolution);
		}
        break;
	case CONFIG_VIDEO_ATTRIBUT_BRIGHTNESS://设置亮度
		{
			unsigned short Brightness;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Brightness = (unsigned char)ConfigValue;
			
			if(Brightness != m_pADContext->m_Brightness)//亮度改变
			{
				m_pADContext->m_Brightness = Brightness;
				//MyWriteEEPROM(m_ADIICAddr,&Offset,(char *)&(m_pADContext->m_Brightness),sizeof(m_pADContext->m_Brightness));
			}
			
			printf("Brightness=%d\n",Brightness);
		}
		break;
	case CONFIG_VIDEO_ATTRIBUT_HUE://设置色彩
		{
			unsigned short Hue;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Hue = (unsigned char)ConfigValue;
			
			if(Hue != m_pADContext->m_Hue)//色彩改变
			{
				m_pADContext->m_Hue = Hue;
				//SetVideoInColor(0,m_ADContext1.m_Brightness,m_ADContext1.m_Contrast,m_ADContext1.m_Saturate, m_ADContext1.m_Hue);
				///.....................别的动作,调用api函数改变色彩
				//MyWriteEEPROM(m_ADIICAddr,&Offset,(char *)&(m_pADContext->m_Hue),sizeof(m_pADContext->m_Hue));
			}
			
			printf("Hue=%d\n",Hue);
		}
		break;
	case CONFIG_VIDEO_ATTRIBUT_CONTRAST://设置对比度
		{
			unsigned short Contrast;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Contrast = (unsigned char)ConfigValue;
			
			if(Contrast != m_pADContext->m_Contrast)//对比度改变
			{
				m_pADContext->m_Contrast = Contrast;
				//SetVideoInColor(0,m_ADContext1.m_Brightness,m_ADContext1.m_Contrast,m_ADContext1.m_Saturate, m_ADContext1.m_Hue);
				///.....................别的动作,调用api函数改变对比度
				//MyWriteEEPROM(m_ADIICAddr,&Offset,(char *)&(m_pADContext->m_Contrast),sizeof(m_pADContext->m_Contrast));
			}
			
			printf("Contrast=%d\n",Contrast);
		}
		break;
	case CONFIG_VIDEO_ATTRIBUT_SATURATION://设置饱和度
		{
			unsigned short Saturate;
			
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			Saturate = (unsigned char)ConfigValue;
			
			if(Saturate != m_pADContext->m_Saturate)//饱和度改变
			{
				m_pADContext->m_Saturate = Saturate;
				//SetVideoInColor(0,m_ADContext1.m_Brightness,m_ADContext1.m_Contrast,m_ADContext1.m_Saturate, m_ADContext1.m_Hue);
				///.....................别的动作,调用api函数改变饱和度
				//MyWriteEEPROM(m_ADIICAddr,&Offset,(char *)&(m_pADContext->m_Saturate),sizeof(m_pADContext->m_Saturate));
			}
			
			printf("Saturate=%d\n",Saturate);
		}
		break;
	case CONFIG_SERIAL_PORT_BAUD_RATE://设置串口波特率
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(EntityID == GUID_232)
			{
				unsigned char ConfigData[2];
				ConfigData[0] = 0x02;
				ConfigData[1] = ConfigValue/1200;
				m_232Context.m_BaudRate = ConfigValue;
				printf("RS232 baudrate=%d\n",ConfigValue);
				//RS232Cfg(0x60,ConfigData,2);
				//Offset = MANAGE_SIZE+SERIAL_SIZE+BaudRate_Offset;					
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&(m_232Context.m_BaudRate),sizeof(m_232Context.m_BaudRate));
			}
			else if(EntityID == GUID_485)
			{
				unsigned char ConfigData[2];
				ConfigData[0] = 0x02;
				ConfigData[1] = ConfigValue/1200;
				m_485Context.m_BaudRate = ConfigValue;
				printf("RS485 baudrate=%d\n",ConfigValue);
				//kprintf("485 serial baud %d ok\n",m_485Context.m_BaudRate);
				//iic_print_num(ConfigData[1]);
				//semTake(m_IICMutex,WAIT_FOREVER);
				//RS485Cfg(0x62,ConfigData,2);
				//initSerial(1,m_485Context.m_BaudRate,1);
				//semGive(m_IICMutex);
				//Offset = MANAGE_SIZE+BaudRate_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&(m_485Context.m_BaudRate),sizeof(m_485Context.m_BaudRate));
			}
		}
		break;
		case CONFIG_SERIAL_PORT_STOP_BITS:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(EntityID == GUID_232)
			{
				m_232Context.m_StopBit = ConfigValue;
				printf("RS232 stop bits=%d\n",ConfigValue);
				//调用api函数设置232的停止位
				//Offset = MANAGE_SIZE+SERIAL_SIZE+StopBit_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_232Context.m_StopBit,sizeof(m_232Context.m_StopBit));	
			}
			else if(EntityID == GUID_485)
			{
				m_485Context.m_StopBit = ConfigValue;
				printf("RS485 stop bits=%d\n",ConfigValue);
				//调用api函数设置485的停止位
				//Offset = MANAGE_SIZE+StopBit_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_485Context.m_StopBit,sizeof(m_485Context.m_StopBit));					
			}
		}
		break;
	case CONFIG_SERIAL_PORT_PARITY:
		{
			char CVlaue;
			
			memcpy(&CVlaue,ReqBuf,sizeof(CVlaue));
			
			if(EntityID == GUID_232)
			{
				m_232Context.m_CheckNum = CVlaue;
				printf("RS232 check bit=%d\n",CVlaue);
				//调用api函数设置232的校验位
				//Offset = MANAGE_SIZE+SERIAL_SIZE+CheckNum_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_232Context.m_CheckNum,sizeof(m_232Context.m_CheckNum));					
			}
			else if(EntityID == GUID_485)
			{
				m_485Context.m_CheckNum = CVlaue;
				printf("RS485 check bit=%d\n",CVlaue);
				//调用api函数设置485的校验位
				//Offset = MANAGE_SIZE+CheckNum_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_485Context.m_CheckNum,sizeof(m_485Context.m_CheckNum));	
			}
		}
		break;
	case CONFIG_SERIAL_PORT_DATA_BITS:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(EntityID == GUID_232)
			{
				m_232Context.m_DataBits = ConfigValue;
				printf("RS232 data bits=%d\n",ConfigValue);
				//调用api函数设置232的数据位
				//Offset = MANAGE_SIZE+SERIAL_SIZE+Databits_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_232Context.m_DataBits,sizeof(m_232Context.m_DataBits));						
			}
			else if(EntityID == GUID_485)
			{
				m_485Context.m_DataBits = ConfigValue;
				printf("RS485 data bits=%d\n",ConfigValue);
				//调用api函数设置485的数据位
				//Offset = MANAGE_SIZE+Databits_Offset;
				//MyWriteEEPROM(IIC_ADDR+12,&Offset,(char *)&m_485Context.m_DataBits,sizeof(m_485Context.m_DataBits));	
			}
		}
		break;
	case CONFIG_MOTD_UPPER_THRESHOLD:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_UpperThreshhold)
			{
				m_pMotionContext->m_UpperThreshhold = ConfigValue;
				//Offset = MD_UPPERTHRESHHOLD_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_UpperThreshhold=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_LOWER_THRESHOLD:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_LowerThreshhold)
			{
				m_pMotionContext->m_LowerThreshhold = ConfigValue;
				//Offset = MD_LOWERTHRESHHOLD_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_LowerThreshhold=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_NB_FRAMES:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_ConsecutiveFrame)
			{
				m_pMotionContext->m_ConsecutiveFrame = ConfigValue;
				//Offset = MD_CONSECUTIVEFRAME_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_ConsecutiveFrame=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_VECT_LENGTH:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_VectorThreshhold)
			{
				m_pMotionContext->m_VectorThreshhold = ConfigValue;
				//Offset = MD_VECTORTHRESHHOLD_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_VectorThreshhold=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_LEFT_TO_RIGHT:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_FromL2R)
			{
				m_pMotionContext->m_FromL2R = ConfigValue;
				//Offset = MD_FROML2R_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_FromL2R=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_RIGHT_TO_LEFT:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_FromR2L)
			{
				m_pMotionContext->m_FromR2L = ConfigValue;
				//Offset = MD_FROMR2L_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_FromR2L=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_UP_TO_DOWN:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_FromT2B)
			{
				m_pMotionContext->m_FromT2B = ConfigValue;
				//Offset = MD_FROMT2B_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_FromT2B=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_DOWN_TO_UP:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_FromB2T)
			{
				m_pMotionContext->m_FromB2T = ConfigValue;
				//Offset = MD_FROMB2T_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_FromB2T=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_STATE:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_bMotionDetection)
			{
				m_pMotionContext->m_bMotionDetection = ConfigValue;
				//Offset = MD_ENABLE_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_bMotionDetection=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_MASK_ENABLED:
		{
			memcpy(&ConfigValue,ReqBuf,sizeof(ConfigValue));
			ConfigValue = htonl(ConfigValue);
			
			if(ConfigValue != m_pMotionContext->m_bMaskEnable)
			{
				m_pMotionContext->m_bMaskEnable = ConfigValue;
				//Offset = MD_MASKENABLE_Offset;
				//MyWriteEEPROM(m_MotionIICAddr,&Offset,(char *)&(m_Value),sizeof(m_Value));
			}
			
			printf("m_bMaskEnable=%d\n",ConfigValue);
		}
		break;
	case CONFIG_MOTD_MASK_STRING:////该处需要注意一下
		{
			int  i;
			char *pSrc;
			char LastChar;
			unsigned short MaskLen;
			
			memcpy(&MaskLen,ReqBuf,sizeof(MaskLen));
			pSrc = ReqBuf + sizeof(MaskLen);
			
			///////////////保证屏蔽串不溢出，并且最后一个字符不是';'
			if(MaskLen > 210)
			{
				MaskLen		= 210;
				LastChar	= pSrc[210];
				if(LastChar == ';')
				{
					MaskLen = 209;
				}
			}
			
			if(MaskLen != m_pMotionContext->m_MaskStrLen)
			{
				m_pMotionContext->m_MaskStrLen = MaskLen;
			}
			if(MaskLen)
			{
				memcpy(m_pMotionContext->m_MotionMaskStr,pSrc,MaskLen);
			}
			for(i=MaskLen;i<210;i++)
			{
				m_pMotionContext->m_MotionMaskStr[i] = ',';
			}
			
			printf("MaskLen=%d\n",MaskLen);
		}
		break;
	default:
		break;
	}
	
	return 0;
}

extern u16 DealVSIPCommand(unsigned char MsgType,unsigned char *msg,unsigned char *RespBuf,VSWC_CONTROL_HEADER *pRespHeader);

u16 RecvVsipMsg()
{
	int ret;
	
	unsigned char  msg[2048];
	unsigned char  RespBuf[2048];
	unsigned short SendOffset;
	
	VSWC_CONTROL_HEADER Header;
	VSWC_CONTROL_HEADER RespHeader;
	
	SOCKHANDLE s = vsip_udp_socket;
	
	struct sockaddr_in from;
	int addrlen = sizeof(from);
	
	struct sockaddr_in to;
	
	ret = recvfrom(s,msg,sizeof(msg),0,(struct sockaddr *)&from,&addrlen);
	if(ret < 0)
	{
#ifdef WIN32
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);
		printf("recv failed,ret=%d,errcode=%d,errmsg=%s,from(%s,%d)\n",ret,WSAGetLastError(),(LPCTSTR)lpMsgBuf,inet_ntoa(from.sin_addr),ntohs(from.sin_port));
#else
		printf("recv failed,ret=%d,errcode=%d,errmsg=%s,from(%s,%d)\n",ret,errno,strerror(errno),inet_ntoa(from.sin_addr),ntohs(from.sin_port));
#endif
		return CTRL_FAILED_NETRCV;
	}
	else
	{
		//printf("recv %d bytes from(%s,%d)\n",ret,inet_ntoa(from.sin_addr),ntohs(from.sin_port));
	}
	
	/*if(ret < sizeof(Header) || from.sin_addr.s_addr != vsip_serv.sin_addr.s_addr)
	{
		return CTRL_FAILED_VERIFY;
	}*/
	
	to.sin_family			= AF_INET;
	to.sin_addr.s_addr		= from.sin_addr.s_addr;
	to.sin_port				= from.sin_port;
	
	memcpy(&Header,msg,sizeof(Header));
	n2hVSIPHeader(&Header);
	//printf("m_TransNum=0x%04x,m_Version=0x%04x,m_MsgType=(%d,0x%02x)\n",Header.m_TransNum,Header.m_Version,Header.m_MsgType,Header.m_MsgType);
	
	RespHeader.m_Version	= VSWC_PROTOCOL_VERSION;
	RespHeader.m_TransNum	= Header.m_TransNum | 0x8000;
	//RespHeader.m_MsgType	= 0;
	//RespHeader.m_MsgLen	= 0;
	
	switch(Header.m_MsgType)
	{
	case VSWC_MSG_TYPE_D_PING_REQ:
		{
			VSWC_PING_RESPOND_MSG PingResp;
			VSWC_PING_REQUEST_MSG PingReq;
			
			GetVsipPingRespMsg(&PingResp);
			
			memcpy(&PingReq,msg,sizeof(PingReq));
			PingReq.m_CenterIp		= ntohl(PingReq.m_CenterIp);
			PingReq.m_CenterPort	= ntohs(PingReq.m_CenterPort);
			printf("CenterIp=0x%08x,CenterPort=0x%04x\n",PingReq.m_CenterIp,PingReq.m_CenterPort);
			
			if(htonl(PingReq.m_CenterIp) == inet_addr("255.255.255.255"))
			{
				to.sin_addr.s_addr	= inet_addr("255.255.255.255");//需添加到"255.255.255.255"的路由或默认网关
				to.sin_port			= PingReq.m_CenterPort;
			}
			
			printf("D_PING_REQ 1,sizeof(PingResp)=%d\n",sizeof(PingResp));
			
			PingResp.m_Header.m_TransNum = htons(RespHeader.m_TransNum);
			//printf("TransNum=0x%04x,ConnType=%d\n",RespHeader.m_TransNum,PingResp.m_ConnType);
			
			ret = sendto(s,(char *)&PingResp,sizeof(PingResp),0,(struct sockaddr *)&to,sizeof(to));
			
			printf("D_PING_REQ 2,ret=%d\n",ret);
			
			if(ret<0)
			{
				#ifndef WIN32
				printf("D_PING_REQ 3,ret=%d,errcode=%d,errstr=%s,target:(%s,%d)\n",ret,errno,strerror(errno),inet_ntoa(to.sin_addr),ntohs(to.sin_port));
				#endif
			}
		}
		break;
	case VSWC_MSG_TYPE_D_GETCONFIG_REQ://需要确定成员变量的先后顺序
		{
			VSWC_D_GET_CONFIG_REQ  ConfigReq;
			VSWC_D_GET_CONFIG_RESP ConfigResp;
			
			printf("D_GETCONFIG_REQ 1!!!!!!!!!!\n");
			
			memcpy(&ConfigReq,msg,sizeof(ConfigReq));
			if((ConfigReq.m_DeviceGUID.Data4[0] == g_UnitGuid.Data4[0])&&
				(ConfigReq.m_DeviceGUID.Data4[1] == g_UnitGuid.Data4[1])&&
				(ConfigReq.m_DeviceGUID.Data4[2] == g_UnitGuid.Data4[2])&&
				(ConfigReq.m_DeviceGUID.Data4[3] == g_UnitGuid.Data4[3])&&
				(ConfigReq.m_DeviceGUID.Data4[4] == g_UnitGuid.Data4[4])&&
				(ConfigReq.m_DeviceGUID.Data4[5] == g_UnitGuid.Data4[5])&&
				(ConfigReq.m_DeviceGUID.Data4[6] == g_UnitGuid.Data4[6])&&
				(ConfigReq.m_DeviceGUID.Data4[7] == g_UnitGuid.Data4[7]))
			{
				RespHeader.m_MsgType 	= VSWC_MSG_TYPE_D_GETCONFIG_RESP;
				RespHeader.m_MsgLen		= sizeof(ConfigResp);
				h2nVSIPHeader(&RespHeader);
				ConfigResp.VSWCHeader	= RespHeader;
				
				ConfigResp.m_DeviceType = vsip_sys_pub_param.m_DeviceType;
				ConfigResp.m_DeviceGUID = g_UnitGuid;
				h2nGUID(&ConfigResp.m_DeviceGUID);
				
				ConfigResp.m_Mask		= vsip_sys_pub_param.m_Mask;
				ConfigResp.m_Gate		= vsip_sys_pub_param.m_Gate;
				ConfigResp.m_CenterIP	= vsip_sys_pub_param.m_CenterIP;
				
				ConfigResp.m_485Baud	= htons(vsip_sys_pub_param.m_485Baud);
				ConfigResp.m_232Baud	= htons(vsip_sys_pub_param.m_232Baud);
				
				ret = sendto(s,(char *)&ConfigResp,sizeof(ConfigResp),0,(struct sockaddr *)&to,sizeof(to));
				
				printf("D_GETCONFIG_REQ 2,GateWay=0x%08x,Net Mask=0x%08x,m_MsgLen=%d,ret=%d!!!!!!!!!!\n",ConfigResp.m_Gate,ConfigResp.m_Mask,sizeof(ConfigResp),ret);
			}
		}
		break;
	case VSWC_MSG_TYPE_IC_CAPABILITIES_REQ:
		{
			VSWC_CAPABILITY Capa;
			unsigned short  count;
			int i;
			
			printf("Capability!!!!!!!!!!\n");
			
			SendOffset = sizeof(RespHeader);
			
			//count = htons(4+2+4+1+4);//4 video enc;2 serial port;4 input pin;1 output pin;4 md
			count = CountVsipCapability();
			count = htons(count);
			
			memcpy(RespBuf + SendOffset,&count,sizeof(count));
			SendOffset += sizeof(count);
			
			for(i=0;i<vsip_capability.byVideoEncNum;i++)
			{
				Capa.m_CapaType			= VSWC_ENTITY_VIDEOENCODER;
				Capa.m_GUID				= g_UnitGuid;
				Capa.m_GUID.Data1		= htonl(GUID_VIDEO+((i)?(i+1):(i)));
				Capa.m_ManID			= 0;
				Capa.m_Ver				= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			if(vsip_capability.bySerialPortNum > 1)
			{
				Capa.m_CapaType			= VSWC_ENTITY_SERIALPORT;
				Capa.m_GUID				= g_UnitGuid;
				Capa.m_GUID.Data1		= htonl(GUID_232);
				Capa.m_ManID			= 0;
				Capa.m_Ver				= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			if(vsip_capability.bySerialPortNum > 0)
			{
				Capa.m_CapaType			= VSWC_ENTITY_SERIALPORT;
				Capa.m_GUID				= g_UnitGuid;
				Capa.m_GUID.Data1		= htonl(GUID_485);
				Capa.m_ManID			= 0;
				Capa.m_Ver				= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			for(i=0;i<vsip_capability.byInputPinNum;i++)
			{
				Capa.m_CapaType		= VSWC_ENTITY_INPUTPIN;
				Capa.m_GUID			= g_UnitGuid;
				if(i == 0) Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN);
				else if(i == 1) Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN2);
				else if(i == 2) Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN3);
				else if(i == 3) Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN4);
				else if(i == 4) Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN5);
				else Capa.m_GUID.Data1 = htonl(GUID_INPUTPIN6);
				Capa.m_ManID		= 0;
				Capa.m_Ver			= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			for(i=0;i<vsip_capability.byOutputPinNum;i++)
			{
				Capa.m_CapaType		= VSWC_ENTITY_OUTPUTPIN;
				Capa.m_GUID			= g_UnitGuid;
				if(i == 0) Capa.m_GUID.Data1 = htonl(GUID_OUTPUTPIN);
				else if(i == 1) Capa.m_GUID.Data1 = htonl(GUID_OUTPUTPIN2);
				else Capa.m_GUID.Data1 = htonl(GUID_OUTPUTPIN3);
				Capa.m_ManID		= 0;
				Capa.m_Ver			= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			for(i=0;i<vsip_capability.byVideoInputPortNum;i++)
			{
				Capa.m_CapaType			= VSWC_ENTITY_MOTION_DETECTOR;
				Capa.m_GUID				= g_UnitGuid;
				Capa.m_GUID.Data1		= htonl(GUID_MOTION_DETECTION);
				Capa.m_GUID.Data2		= htons((unsigned short)((i)?(i+1):(i)));
				Capa.m_ManID			= 0;
				Capa.m_Ver				= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			for(i=0;i<vsip_capability.byAudioEncNum;i++)
			{
				Capa.m_CapaType			= VSWC_ENTITY_AUDIOENCODER;
				Capa.m_GUID				= g_UnitGuid;
				Capa.m_GUID.Data1		= htonl(GUID_AUDIO_ENCODER+i);
				Capa.m_ManID			= 0;
				Capa.m_Ver				= 0;
				memcpy(RespBuf + SendOffset,&Capa,sizeof(Capa));
				SendOffset += sizeof(Capa);
			}
			
			RespHeader.m_MsgType	= VSWC_MSG_TYPE_IC_CAPABILITIES_RESP;
			RespHeader.m_MsgLen		= SendOffset;
			h2nVSIPHeader(&RespHeader);
			memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
			
			ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
		}
		break;
	case VSWC_MSG_TYPE_MC_GETCONFIG_REQ:
		{
			VSWC_MEDIA_CONTROL_GETCONFIGREQUEST_MSG ReqMsg;
			unsigned short ListCount;
			unsigned short ValueSize;
			unsigned char  ValueType;
			
			memcpy(&ReqMsg,msg,sizeof(ReqMsg));
			//n2hGUID(&ReqMsg.guid);
			ReqMsg.ListCount	= ntohs(ReqMsg.ListCount);
			ReqMsg.ConfigId		= ntohs(ReqMsg.ConfigId);
			printf("Enter Get Config......ListCount=%d,ConfigId=%d\n",ReqMsg.ListCount,ReqMsg.ConfigId);
			
			RespHeader.m_MsgType = VSWC_MSG_TYPE_MC_GETCONFIG_RESP;
			
			SendOffset = sizeof(RespHeader);
			
			//h2nGUID(&ReqMsg.guid);
			memcpy(RespBuf + SendOffset,&ReqMsg.guid,sizeof(ReqMsg.guid));
			SendOffset += sizeof(ReqMsg.guid);
			
			n2hGUID(&ReqMsg.guid);
			
			if(ReqMsg.ListCount != 0)
			{
				if(ReqMsg.ConfigId == CONFIG_SYSTEM_DEVICE_TYPE)
				{
					printf("Device type!\n");
					
					ListCount = htons(1);
					memcpy(RespBuf + SendOffset,&ListCount,sizeof(ListCount));
					SendOffset += sizeof(ListCount);
					
					ReqMsg.ConfigId = htons(ReqMsg.ConfigId);//注意此处可能有问题,是否需要转序?
					memcpy(RespBuf + SendOffset,&ReqMsg.ConfigId,sizeof(ReqMsg.ConfigId));
					SendOffset += sizeof(ReqMsg.ConfigId);
					
					ValueType = VSWC_VALUE_TYPE_STRING;
					memcpy(RespBuf + SendOffset,&ValueType,sizeof(ValueType));
					SendOffset += sizeof(ValueType);
					
					ValueSize = htons((unsigned short)(strlen("XF1500T4")));	
					memcpy(RespBuf + SendOffset,&ValueSize,sizeof(ValueSize));
					SendOffset += sizeof(ValueSize);
					
					memcpy(RespBuf + SendOffset,"XF1500T4",strlen("XF1500T4"));
					SendOffset += strlen("XF1500T4");
					
					RespHeader.m_MsgLen = SendOffset;
					h2nVSIPHeader(&RespHeader);
					memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
					
					ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
					if(ret != SendOffset)
					{
						printf("send error %d\n",ret);
					}
					else
					{
						printf("SendOffset=%d\n",SendOffset);
					}
				}
			}
			else
			{
				unsigned int	CapaID		= ReqMsg.guid.Data1;
				unsigned short	CapaAppID	= ReqMsg.guid.Data2;
				
				if(CapaID == GUID_VIDEO)
				{
					printf("video 1\n");
					SendOffset += RespondVideoConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_VIDEO + 2)
				{
					printf("video 2\n");
					SendOffset += RespondVideoConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_VIDEO + 3)
				{
					printf("video 3\n");
					SendOffset += RespondVideoConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_VIDEO + 4)
				{
					printf("video 4\n");
					SendOffset += RespondVideoConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_485)
				{
					printf("rs485\n");
					SendOffset += Respond485Config(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_232)
				{
					printf("rs232\n");
					SendOffset += Respond232Config(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_AUDIO_ENCODER)
				{
					printf("Enter audio encoder.....\n");
					SendOffset += RespondAudioEncConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_AUDIO_DECODER)
				{
					printf("Enter audio decoder.....\n");
					SendOffset += RespondAudioDecConfig(RespBuf + SendOffset);
				}
				else if(CapaID == GUID_MOTION_DETECTION)
				{
					printf("Enter Motion D.....\n");
					if(CapaAppID==0)
					{
						printf("motion id is 0 %d\n",CapaAppID);
						SendOffset += RespondMotionDetectionConfig(RespBuf + SendOffset,0);
					}
					else if(CapaAppID==2)
					{
						printf("motion id is 1 %d\n",CapaAppID);
						SendOffset += RespondMotionDetectionConfig(RespBuf + SendOffset,1);
					}
					else if(CapaAppID==3)
					{
						printf("motion id is 1 %d\n",CapaAppID);
						SendOffset += RespondMotionDetectionConfig(RespBuf + SendOffset,2);
					}
					else if(CapaAppID==4)
					{
						printf("motion id is 1 %d\n",CapaAppID);
						SendOffset += RespondMotionDetectionConfig(RespBuf + SendOffset,3);
					}
				}
				else
				{
					SendOffset += RespondOtherConfig(RespBuf + SendOffset);
				}
				
				RespHeader.m_MsgLen = SendOffset;
				h2nVSIPHeader(&RespHeader);
				memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
				
				printf("CapaID 0x%08x SendOffset %d\n",CapaID,SendOffset);
				
				ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
				if(ret != SendOffset)
				{
					printf("get config : send error %d\n",ret);
				}
			}
		}
		break;
	case VSWC_MSG_TYPE_MC_GETPROPCONFIG_REQ:
		{
			VSWC_MEDIA_CONTROL_GETPROPCONFIGREQUEST_MSG ReqMsg;
			
			printf("*********RespondProprietary\n");
			
			memcpy(&ReqMsg,msg,sizeof(ReqMsg));
			ReqMsg.ListCount		= ntohs(ReqMsg.ListCount);
			
			RespHeader.m_MsgType	= VSWC_MSG_TYPE_MC_GETPROPCONFIG_RESP;
			
			SendOffset = sizeof(RespHeader);
			
			memcpy(RespBuf + SendOffset,&ReqMsg.guid,sizeof(ReqMsg.guid));
			SendOffset += sizeof(ReqMsg.guid);
			
			if(ReqMsg.ListCount == 0)//获得全部对外的私有属性
			{
				SendOffset += RespondProprietary(RespBuf + SendOffset);
			}
			else
			{
				
			}
			
			RespHeader.m_MsgLen = SendOffset;
			h2nVSIPHeader(&RespHeader);
			memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
			
			ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
			if(ret != SendOffset)
			{
				printf("get propconfig : send error %d\n",ret);
			}
			else
			{
				printf("VSWC_MSG_TYPE_MC_GETPROPCONFIG_REQ success\n");
			}
		}
		break;
	case VSWC_MSG_TYPE_E_SUBSCRIBE_REQ:
		{
			VSWC_EVENT_SUBSCRIBE_REQUEST_MSG	ReqMsg;
			unsigned int						ErrorCode;
			unsigned char						RespMsg = VSWC_MSG_TYPE_E_SUBSCRIBE_REQ;
			
			memcpy(&ReqMsg,msg,sizeof(ReqMsg));
			
			m_CenterContext.m_AlarmCenter	= ReqMsg.m_AlarmCenterIP;
			m_CenterContext.m_AlarmPort		= ReqMsg.m_AlarmPort;
			
			{
				struct in_addr in;
				in.s_addr = ReqMsg.m_AlarmCenterIP;
				printf("report ip:%s,port:%d\n",inet_ntoa(in),ntohs(ReqMsg.m_AlarmPort));
			}
			
			RespHeader.m_MsgType = VSWC_MSG_TYPE_ER_ERROR;
			
			SendOffset = sizeof(RespHeader);
			
			memcpy(RespBuf + SendOffset,&RespMsg,sizeof(RespMsg));
			SendOffset += sizeof(RespMsg);
			
			ErrorCode = htons(0);
			memcpy(RespBuf + SendOffset,&ErrorCode,sizeof(ErrorCode));
			SendOffset += sizeof(ErrorCode);
			
			RespHeader.m_MsgLen = SendOffset;
			h2nVSIPHeader(&RespHeader);
			memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
			
			ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
			if(ret == SendOffset)
			{
				printf("Send OK!!!!!!!!\n");
			}
		}
		break;
	case VSWC_MSG_TYPE_MC_SETCONFIG_REQ:
		{
			VSWC_SETCONFIGREQUEST_HEAD_MSG		ReqMsg;
			VSWC_MEDIA_CONTROL_CONFIG			cfg;
			unsigned int                     	ErrorCode;
			unsigned short						ConfigID;
			unsigned char                      	ValueType;
			unsigned int 						EntityID;
			unsigned short 						CapaAppID;
			unsigned char						RespMsg;
			
			printf("hehe,VSWC_MSG_TYPE_MC_SETCONFIG_REQ\n");
			
			memcpy(&ReqMsg,msg,sizeof(ReqMsg));
			
			////获得功能单元的ID
			n2hGUID(&ReqMsg.guid);
			EntityID	= ReqMsg.guid.Data1;
			CapaAppID	= ReqMsg.guid.Data2;
			
			ReqMsg.ListCount = ntohs(ReqMsg.ListCount);
			
			memcpy(&cfg,msg+sizeof(ReqMsg),sizeof(cfg));
			ConfigID	= ntohs(cfg.Id);
			ValueType	= cfg.ValueType;
			
			//m_bConfigMotionD = 1;
			SetEntityConfig(msg+sizeof(ReqMsg)+sizeof(cfg),(unsigned short)EntityID,ConfigID,ValueType,CapaAppID);
			//m_bConfigMotionD = 0;
			
			RespHeader.m_MsgType = VSWC_MSG_TYPE_ER_ERROR;
			
			SendOffset = sizeof(RespHeader);
			
			RespMsg = VSWC_MSG_TYPE_MC_SETCONFIG_REQ;
			memcpy(RespBuf + SendOffset,&RespMsg,sizeof(RespMsg));
			SendOffset += sizeof(RespMsg);
			
			ErrorCode = htons(0);
			memcpy(RespBuf + SendOffset,&ErrorCode,sizeof(ErrorCode));
			SendOffset += sizeof(ErrorCode);
			
			RespHeader.m_MsgLen = SendOffset;
			h2nVSIPHeader(&RespHeader);
			memcpy(RespBuf,&RespHeader,sizeof(RespHeader));
			
			ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
			if(ret != SendOffset)
			{
				printf("command : send error %d\n",ret);
			}
		}
		break;
	default:
		{
			printf("*********MsgType:%d\n",Header.m_MsgType);
			RespHeader.m_MsgLen = 0;
			DealVSIPCommand(Header.m_MsgType,msg,RespBuf,&RespHeader);
			SendOffset = ntohs(RespHeader.m_MsgLen);
			ret = sendto(s,(char *)RespBuf,SendOffset,0,(struct sockaddr *)&to,sizeof(to));
		}
		break;
	}
	
	return CTRL_SUCCESS;
}

#endif
