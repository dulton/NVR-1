#if 0
int   Deal_CTRL_CMD_LOGIN()
{
	ifly_link_online links;
	memset(&links,0,sizeof(links));
	int sel = 0;
	ifly_loginpara_t login;
	memcpy(&login,pbyMsgBuf,sizeof(ifly_loginpara_t));
	u32 dwIp = login.ipAddr;
	printf("name:%s,ip:%d.%d.%d.%d\n",login.username,(u8)dwIp,(u8)(dwIp>>8),(u8)(dwIp>>16),(u8)(dwIp>>24));

	int un=GetUserNum();	
	ifly_usermanage_t tem_user;
	if (un > 0)
	{
		for (sel = 0; sel < un; sel++)
		{
			GetUserInfo(&tem_user, sel);
			if (!strcmp(login.username, tem_user.name))
					
		}

		if (sel == un) {
			#ifdef ALARM_UPLOAD
			upload_alarm(8, 0, 1);
			#endif
			return CTRL_FAILED_USER;
		}

		/*******************/				//04-30  登录用户如无远程权限，则禁止登录主机段
		int k;
		for (k = 0; k < REMOTE_PRIVILIGE_NUM; ++k)
			if ('1' == tem_user.remote_privilege[k])
				

			if (k == REMOTE_PRIVILIGE_NUM) {
				#ifdef ALARM_UPLOAD
				upload_alarm(8, 0, 1);
				#endif	
				return CTRL_FAILED_PARAM;
			}

		/*******************/
		if (strcmp(login.loginpass, tem_user.password))
		{
			#ifdef ALARM_UPLOAD
			upload_alarm(8, 0, 1);
			#endif		
			printf("login failed because password is wrong\n");
			return CTRL_FAILED_PASSWORD;
		}
	
		/********************/
		if (strcmp(tem_user.mac_address, "00:00:00:00:00:00"))
		{
			if (strcmp(login.macAddr, tem_user.mac_address))
			{
				#ifdef ALARM_UPLOAD
				upload_alarm(8, 0, 1);
				#endif		
				printf("login failed because macaddr is wrong\n");
				return CTRL_FAILED_MACADDR;
			}
		}

		{
			links.link_online = un;
			memcpy(pbyAckBuf,&links,sizeof(links));
			
			int i;
			for(i=0;i<un;i++)
			{
				GetUserInfo(&tem_user, sel);
				
				ifly_remusr_limit quanxian;
				strncpy(quanxian.usrname,tem_user.name,sizeof(quanxian.usrname));
				strncpy(quanxian.remote_privilege,tem_user.remote_privilege,sizeof(quanxian.remote_privilege));
				
				memcpy(pbyAckBuf+sizeof(links)+i*sizeof(ifly_remusr_limit),&quanxian,sizeof(ifly_remusr_limit));
			}
			*pAckLen = sizeof(links)+un*sizeof(ifly_remusr_limit);
		}
		
	}

	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP << 4) | SLAVE_REMOTE_LOGIN);
	log.start = time(NULL);
	strcpy(log.loginfo, login.username);
	logCopy(log);

	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_LOGOFF()
{
	ifly_loginpara_t login;
	memcpy(&login,pbyMsgBuf,sizeof(ifly_loginpara_t));
	{
			*pAckLen = 0;
			printf("logout\n");
			return CTRL_SUCCESS;
	}
}
#endif

int   Deal_CTRL_CMD_GETDEVICEINFO()
{
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_DeviceInfo_t);
	}
	memcpy(pbyAckBuf,&deviceInfo,sizeof(ifly_DeviceInfo_t));

	printf("get device info deviceIP:%x,devicePort:%d,device_name:%s,device_mode:%s,chn=%d,maxAlarmInNum:%d\n",deviceInfo.deviceIP,ntohs(deviceInfo.devicePort),deviceInfo.device_name,deviceInfo.device_mode,deviceInfo.maxChnNum,deviceInfo.maxAlarmInNum);
}

int   Deal_CTRL_CMD_GETVIDEOPROPERTY()
{
	ifly_Video_Property_t videoproperty;
	videoproperty.videoEncType = 98;
	videoproperty.max_videowidth = htons(704);
	videoproperty.max_videoheight = htons(576);

	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_Video_Property_t);
	}
	memcpy(pbyAckBuf,&videoproperty,sizeof(ifly_Video_Property_t));
	printf("get video property\n");
}

int   Deal_CTRL_CMD_GETAUDIOPROPERTY()
{
	ifly_Audio_Property_t audioproperty;
	audioproperty.audioBitPerSample = 16;
	audioproperty.audioSamplePerSec = htons(8000);
	#ifdef USE_AUDIO_PCMU
	audioproperty.audioFrameSize = htons(642);
	audioproperty.audioEnctype = MEDIA_TYPE_PCMU;
	#else
	audioproperty.audioFrameSize = htons(168);
	audioproperty.audioEnctype = MEDIA_TYPE_PCMU;
	#endif

	audioproperty.audioEnctype = MEDIA_TYPE_ADPCM;
	audioproperty.audioFrameDurTime = htons(40); 
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_Audio_Property_t);
	}
	memcpy(pbyAckBuf,&audioproperty,sizeof(ifly_Audio_Property_t));
	printf("get audio property\n");
}

int   Deal_CTRL_CMD_GETVOIPPROPERTY()
{
	ifly_VOIP_Property_t voipproperty;
		voipproperty.VOIPBitPerSample = 16;
		voipproperty.VOIPFrameSize = htons(642);
		voipproperty.VOIPSamplePerSec = htons(8000);
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_VOIP_Property_t);
	}
	memcpy(pbyAckBuf,&voipproperty,sizeof(ifly_VOIP_Property_t));
	printf("get voip property\n");
}

int   Deal_CTRL_CMD_GETMDPROPERTY()
{
		ifly_MD_Property_t mdproperty;
		mdproperty.MDRow = 18;
		mdproperty.MDCol = 22;
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_MD_Property_t);
	}
	memcpy(pbyAckBuf,&mdproperty,sizeof(ifly_MD_Property_t));
	printf("get md property\n");
}

int   Deal_CTRL_CMD_STOPVIDEOMONITOR()
{
	u32 id;
	int i;
	int j,k;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_snd[i].linkid == id)
		{
			
		}
	}
	if(i>=STREAM_LINK_MAXNUM)
	{
		printf("stop monitor error\n", id);
		
	}

	printf("stop monitor, id=%d\n", id);
	for(j=0;j<MAX_CHN_NUM;j++)
	{
		for(k=0;k<EACH_STREAM_TCP_LINKS;k++)
		{
			if(g_snd[i].sockfd == INVALID_SOCKET)
			{
				
			}
			if(video_tcp_snd[j].sockfd[k] == g_snd[i].sockfd)
			{
				printf("chn=%d,sockfd=%d\n",j,g_snd[i].sockfd);
				video_tcp_snd[j].sockfd[k] =INVALID_SOCKET;
				//g_snd[i].bStart =0;
				close(g_snd[i].sockfd);
				memset(&g_snd[i], 0, sizeof(ifly_stearmsnd_t));
				g_snd[i].sockfd = INVALID_SOCKET;
				g_snd[i].linkid = 0;
				
			}
		}
	}
	
}

int   Deal_CTRL_CMD_STOPAUDIOMONITOR()
{
	u32 id;
	int i;
	int j,k;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_snd[i].linkid == id)
		{
			
		}
	}
	if(i>=STREAM_LINK_MAXNUM)
	{
		printf("stop audio error\n", id);
		
	}

	for(j=0;j<MAX_CHN_NUM;j++)
	{
		for(k=0;k<EACH_STREAM_TCP_LINKS;k++)
		{
			if(g_snd[i].sockfd == INVALID_SOCKET)
			{
				
			}
			if(audio_tcp_snd[j].sockfd[k] == g_snd[i].sockfd)
			{
				printf("audio chn=%d,sockfd=%d\n",j,g_snd[i].sockfd);
				audio_tcp_snd[j].sockfd[k] =INVALID_SOCKET;
				g_snd[i].bStart =0;
				close(g_snd[i].sockfd);
				memset(&g_snd[i], 0, sizeof(ifly_stearmsnd_t));
				g_snd[i].sockfd = INVALID_SOCKET;
				g_snd[i].linkid = 0;
				
			}
		}
	}

	printf("stop audio, id=%d\n", id);
}

int   Deal_CTRL_CMD_STOPVOIP()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_snd[i].linkid == id)
		{
			printf("stop voip, id=%d,i=%d,fd=%d\n", id,i,g_snd[i].sockfd);
			byVOIPStatus = 0;
			//g_snd[i].bStart =0;
			memset(&g_snd[i], 0, sizeof(ifly_stearmsnd_t));
			g_snd[i].sockfd = INVALID_SOCKET;
			g_snd[i].linkid = 0;
		}
	}
	set_audio_chn();
	tl_audio_disable(TL_AUDIO_CH_TALK_BACK);
}

int   Deal_CTRL_CMD_STOPDOWNLOAD()

int   Deal_CTRL_CMD_STOPFILEPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_snd[i].linkid == id)
		{
			g_snd[i].bStart =0;
			g_snd[i].linkid =0;
			close(g_snd[i].sockfd);
			memset(&g_snd[i], 0, sizeof(ifly_stearmsnd_t));
			g_snd[i].sockfd = INVALID_SOCKET;
			g_snd[i].linkid = 0;
			
		}
		
	}
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				//remote_player[i].byUse = 0;
				printf("stop remote_player sockfd=%d\n",remote_player[i].playback_sockfd);
				remote_player[i].byStop= 1;
				
			}
		}
	}
}

int   Deal_CTRL_CMD_STOPTIMEPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				//remote_player[i].byUse = 0;
				remote_player[i].byStop= 1;
				
			}
		}
	}
}

int   Deal_CTRL_CMD_FASTPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote fast play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		if(remote_player[i].byPause)
		{
			remote_player[i].rate = PLAYRATE_2;
			remote_player[i].byPause = 0;
		}
		else
		{
			if(remote_player[i].rate < PLAYRATE_2)
			{
				remote_player[i].rate = PLAYRATE_2;
			}
			else
			{
				remote_player[i].rate *= 2;
				if(remote_player[i].rate > PLAYRATE_MAX)
				{
					remote_player[i].rate = PLAYRATE_1;
				}
			}
		}
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SLOWPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote slow play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		if(remote_player[i].byPause)
		{
			remote_player[i].rate = PLAYRATE_1_2;
			remote_player[i].byPause = 0;
		}
		else
		{
			if(remote_player[i].rate > PLAYRATE_1_2)
			{
				remote_player[i].rate = PLAYRATE_1_2;
			}
			else
			{
				remote_player[i].rate *= 2;
				if(remote_player[i].rate < PLAYRATE_MIN)
				{
					remote_player[i].rate = PLAYRATE_1;
				}
			}
		}
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETPLAYRATE()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	int rate;
	memcpy(&rate,pbyMsgBuf+sizeof(id),sizeof(rate));
	rate = ntohl(rate);
	printf("remote play set rate:(id:0x%x),%d\n",id,rate);
	if(rate > PLAYRATE_MAX || rate < PLAYRATE_MIN)
	{
		return CTRL_FAILED_PARAM;
	}
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].rate = rate;
		if(remote_player[i].byPause) remote_player[i].byPause = 0;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PAUSEPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote pause play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].byPause = !remote_player[i].byPause;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_RESUMEPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote resume play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].byPause = 0;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SINGLEPLAY()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote sing play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		if(remote_player[i].byPause) remote_player[i].rate = PLAYRATE_SINGLE;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_FASTBACKPLAY()
{
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PLAYPREV()
int   Deal_CTRL_CMD_PLAYNEXT()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote next/prev play:(id:0x%x)\n",id);
	*pAckLen = 0;
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		if(event ==  Deal_CTRL_CMD_PLAYPREV) remote_player[i].bySkip = 1;
		else remote_player[i].bySkip = 2;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PLAYSEEK()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote seek play:(id:0x%x)\n",id);
	*pAckLen = 0;
	int seekPos;
	memcpy(&seekPos,pbyMsgBuf+sizeof(id),sizeof(seekPos));
	seekPos = ntohl(seekPos);
	printf("seekPos=%d\n",seekPos);
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].seekPos = seekPos;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PLAYMUTE()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote mute play:(id:0x%x)\n",id);
	*pAckLen = 0;
	u8 byMute;
	memcpy(&byMute,pbyMsgBuf+sizeof(id),sizeof(byMute));
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		//printf("PLAYMUTE i=%d,byuse=%d,hid.id=%d,id=%d\n",i,remote_player[i].byUse,remote_player[i].hid.id , id);
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].byMute = !(byMute==0);
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PLAYPROGRESS()
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	printf("remote progress play:(id:0x%x)\n",id);
	*pAckLen = 0;
	u8 byStatus;
	memcpy(&byStatus,pbyMsgBuf+sizeof(id),sizeof(byStatus));
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				
			}
		}
	}
	printf("sockfd=%d\n",cph->sockfd);
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].byStatus = !(byStatus==0);
		remote_player[i].cph = cph;
	}
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETNETWORK()
{
	ifly_NetWork_t net;
	ifly_NetWork_t networkparam;

	GetNetWorkParam(&net);

	memcpy(networkparam.mac_address,net.mac_address,sizeof(net.mac_address));
	networkparam.ip_address = (net.ip_address);
	networkparam.server_port = htons(net.server_port);
	networkparam.net_mask = (net.net_mask);
	networkparam.net_gateway = (net.net_gateway);
	networkparam.dns = (net.dns);
	networkparam.multicast_address = (net.multicast_address);
	//networkparam.admin_host = htonl(net.admin_host);
	//networkparam.host_port = htons(net.host_port);
	#ifdef SJJK_LANGTAO//20100506 SJJK_PORT
	//pw:26/05/2010
	ifly_reserved_t res;				//dong	100526
	GetReservedParam(&res);
	networkparam.mobile_port = htons(res.port2);
	#endif
	networkparam.http_port = htons(net.http_port);
	networkparam.flag_pppoe = net.flag_pppoe;
	memcpy(networkparam.pppoe_user_name,net.user_name,sizeof(net.user_name));
	memcpy(networkparam.pppoe_passwd,net.passwd,sizeof(net.passwd));
	networkparam.flag_dhcp = net.flag_dhcp&0x80;
	networkparam.ddnsserver = net.flag_ddns&0x7F;
	networkparam.flag_ddns = net.flag_ddns;
	memcpy(networkparam.ddns_domain,net.ddns_domain,sizeof(net.ddns_domain));
	memcpy(networkparam.ddns_user,net.ddns_user,sizeof(net.ddns_user));
	memcpy(networkparam.ddns_passwd,net.ddns_passwd,sizeof(net.ddns_passwd));
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_NetWork_t);
	}
	memcpy(pbyAckBuf,&networkparam,sizeof(ifly_NetWork_t));
	printf("get network\n");
}

int   Deal_CTRL_CMD_SETNETWORK()
{
	ifly_NetWork_t networkparam;
	msgLen = sizeof(ifly_NetWork_t);
	memcpy(&networkparam,pbyMsgBuf,sizeof(ifly_NetWork_t));
	*pAckLen = 0;
	printf("set network\n");
	return remotenet(networkparam);
}

int   Deal_CTRL_CMD_GETVGASOLLIST()
{
	ifly_VGA_Solution vgasolution;
	memset(&vgasolution,0,sizeof(vgasolution));
#if defined(TIANMIN) && defined(TL_9508_D1_8M)
	vgasolution.vgapro[0].flashrate = 60;
	vgasolution.vgapro[0].width = htons(800);
	vgasolution.vgapro[0].height = htons(600);
#else
	vgasolution.vgapro[0].flashrate = 60;
	vgasolution.vgapro[0].width = htons(1280);
	vgasolution.vgapro[0].height = htons(1024);
	vgasolution.vgapro[1].flashrate = 60;
	vgasolution.vgapro[1].width = htons(1024);
	vgasolution.vgapro[1].height = htons(768);
	vgasolution.vgapro[2].flashrate = 60;
	vgasolution.vgapro[2].width = htons(800);
	vgasolution.vgapro[2].height = htons(600);
#endif
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_VGA_Solution);
	}
	memcpy(pbyAckBuf,&vgasolution,sizeof(ifly_VGA_Solution));
	printf("get vgasollist\n");
}

#if 1
int   Deal_CTRL_CMD_GETSYSLANGLIST:				//dong	100424
{
	ifly_SysLangList_t syslanglist;
	memset(&syslanglist,0,sizeof(syslanglist));
	syslanglist.max_langnum = SUPPORT_LANGUAGE_NUM;
	int i = 0;

	#ifdef LANGUAGE_CHS
		syslanglist.langlist[i++] = SYS_LANGUAGE_CHS;		//简体中文
	#endif
	#ifdef LANGUAGE_ENU
		syslanglist.langlist[i++] = SYS_LANGUAGE_ENU; 		//美式英文
	#endif
	#ifdef LANGUAGE_TCH
		syslanglist.langlist[i++] = SYS_LANGUAGE_CHT;		//繁体諫文
	#endif
	#ifdef LANGUAGE_GER
		syslanglist.langlist[i++] = SYS_LANGUAGE_GER;		//德语
	#endif
	#ifdef LANGUAGE_FRE
		syslanglist.langlist[i++] = SYS_LANGUAGE_FRE;		//法语
	#endif
	#ifdef LANGUAGE_SPA
		syslanglist.langlist[i++] = SYS_LANGUAGE_SPA;		//西班牙语
	#endif
	#ifdef LANGUAGE_ITA
		syslanglist.langlist[i++] = SYS_LANGUAGE_ITA;  		//意大利
	#endif
	#ifdef LANGUAGE_POR
		syslanglist.langlist[i++] = SYS_LANGUAGE_POR; 		//葡萄牙语
	#endif
	#ifdef LANGUAGE_RUS
		syslanglist.langlist[i++] = SYS_LANGUAGE_RUS;  		//俄语
	#endif
	#ifdef LANGUAGE_TUR
		syslanglist.langlist[i++] = SYS_LANGUAGE_TUR; 		//土耳其语
	#endif
	#ifdef LANGUAGE_TAI
		syslanglist.langlist[i++] = SYS_LANGUAGE_THA;		//泰国语
	#endif
	#ifdef LANGUAGE_JAP
		syslanglist.langlist[i++] = SYS_LANGUAGE_JAP;		//日语
	#endif
	#ifdef LANGUAGE_HAN
		syslanglist.langlist[i++] = SYS_LANGUAGE_HAN;		//韩语
	#endif	
	#ifdef LANGUAGE_POL
		syslanglist.langlist[i++] = SYS_LANGUAGE_POL;		//波兰语
	#endif
	#ifdef LANGUAGE_HEB	
		syslanglist.langlist[i++] = SYS_LANGUAGE_HEB;		//希伯来语Hebrew
	#endif
	#ifdef LANGUAGE_HUN
		syslanglist.langlist[i++] = SYS_LANGUAGE_HUN;		//匈牙利语Hungarian
	#endif
	#ifdef LANGUAGE_ROM
		syslanglist.langlist[i++] = SYS_LANGUAGE_ROM;		//罗马语Roma
	#endif
	#ifdef LANGUAGE_ARA
			syslanglist.langlist[i++] = SYS_LANGUAGE_ARA;			
	#endif
	
	if(0 == i)
		syslanglist.langlist[i] = -1;

//	printf("syslanglist.max_langnum=%d,,,max_langnum=%d\n",syslanglist.max_langnum,ntohl(syslanglist.max_langnum));	
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_SysLangList_t);
	}
	memcpy(pbyAckBuf,&syslanglist,sizeof(syslanglist));
	printf("Get Sub sys language!\n");		
}

#endif
int   Deal_CTRL_CMD_GETSYSPARAM()
{
	printf("~~~~~~~~~~get sysparam!\n");
	Web_Syspara_Init(pbyAckBuf,pAckLen);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETSYSPARAM()
{
	printf("set systemparam\n");
	ifly_SysParam_t systemparam;
	msgLen = sizeof(ifly_SysParam_t);
	memcpy(&systemparam,pbyMsgBuf,msgLen);
	*pAckLen = 0;
	remotesyspara(systemparam);
	/* 07-10-09-3 begin 远程配置*/
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"SysPara");
	logCopy(log);
	/* 07-10-09 end */
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETRECPARAM()
{
	printf("get recparam!\n");
	Web_Recconf_Init(pbyAckBuf,pAckLen,*pbyMsgBuf);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETRECPARAM()
{
	ifly_record_t rc;
	int chnno;
	ifly_RecordParam_t recinfo;
	memcpy(&recinfo,pbyMsgBuf,sizeof(recinfo));
	chnno = recinfo.channelno;
	GetRecordSet(&rc,chnno);
	rc.channel_no = chnno;
	if(rc.channel_no >= byTotalRecChn) return CTRL_FAILED_PARAM;
	//for(i=0;i<MAX_RECCHN_NUM;i++)
	{
		if(recinfo.code_type != rc.code_type)
		{
			EmRecState eCurState = tRecChn[chnno].eState;
			if(eCurState == RECSTATE_PREREC ||eCurState == RECSTATE_RUNNING || eCurState == RECSTATE_DELAY) //08-01-18
			{
				//ShowMsgBox(IDC_TEXT_RECINFOSWITCH,IDC_CAP_RECSET,NULL,TRUE);
				return CTRL_FAILED_BUSY;
			}			
		}
	}

	rc.code_type = recinfo.code_type;
	rc.bit_type = recinfo.bit_type;
	rc.bit_max = ntohl(recinfo.bit_max);
	rc.quality = recinfo.quality;
	rc.frame_rate = recinfo.frame_rate;
	rc.pre_record = ntohs(recinfo.pre_record);
	rc.post_record = ntohs(recinfo.post_record);
#ifdef IECTRL_NEW
	rc.video_resolution = recinfo.resolutionpos;
#endif	

	//wrchen 090225
	SetRecordSet(&rc,chnno);
	
	for (i = 0; i < MAX_CHN_NUM; i++)
	{
		if ((1 << i ) & (ntohl(recinfo.copy2chnmask)))
		{
			GetRecordSet(&rc,i);
			rc.code_type = recinfo.code_type;
			rc.bit_type = recinfo.bit_type;
			rc.bit_max = ntohl(recinfo.bit_max);
			rc.quality = recinfo.quality;
			rc.frame_rate = recinfo.frame_rate;
			rc.pre_record = ntohs(recinfo.pre_record);
			rc.post_record = ntohs(recinfo.post_record);
			SetRecordSet(&rc,i);
		}
	}
	*pAckLen = 0;
	printf("set recordparam\n");
	WriteConfig(&ConfManager);
	/*****************************/

	/*****************************/
	setRecChnParam(&tRecChn[chnno],&rc);

	venc_parameter_t venc_param;
	venc_param.frame_rate = rc.frame_rate;
	venc_param.bit_rate = rc.bit_max;
	venc_param.pic_level = rc.quality;
	venc_param.is_cbr = !rc.bit_type;
	
	tl_venc_set_para(chnno, 0, &venc_param);//0-主码流,1-次码流
	
	/* 07-10-09-8 begin 远程配置*/
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"RecSet");
	logCopy(log);
	/* 07-10-09 end */
	return CTRL_SUCCESS;
}

#if 0//csp modify
int Deal_CTRL_CMD_GETBITRATELIST()
{
	ifly_record_t rc;			
	ifly_bitRateList_t bitRateList;
	memset(&bitRateList,0,sizeof(bitRateList));
	
	int j;			
	
	GetRecordSet(&rc,*pbyMsgBuf);
	bitRateList.chn = rc.channel_no;
	bitRateList.videotype = rc.sub_bit_type;
	
	for(j = 0;j < 16;j++)
	{
		if(j < GetComboBoxItemCount(IDC_COMBO_RECORDSET_BITRATE))
		{
			bitRateList.bitratelist[j] = GetBitRateValue(j);
		}
	}
	
	//printf("bitRateList.chn=%d,,,videotype=%d\n",bitRateList.chn,bitRateList.videotype);	
	if(pAckLen)
	{
		*pAckLen = sizeof(bitRateList);
	}
	memcpy(pbyAckBuf,&bitRateList,sizeof(bitRateList));
	printf("Get Sub sys Bitrate!\n");		
}
#endif

int Deal_CTRL_CMD_GETSUBSTREAMPARAM()
{
	ifly_SubStreamParam_t substreamparam;
	ifly_record_t rc;
	GetRecordSet(&rc,*pbyMsgBuf);
	
	substreamparam.channelno = rc.channel_no;
	//substreamparam.sub_flag = rc.channelno;
	//yqluo 20101106
	if(rc.sub_video_resolution)
	{
		substreamparam.sub_flag = 0;//pw 2010/10/29 pc与我们相反
	}
	else
	{
		substreamparam.sub_flag = 1;//pw 2010/10/29 pc与我们相反	
	}
	printf("~~~~~~sub_flag:%d~~~~~~~~~rc.sub_video_resolution:%d\n", substreamparam.sub_flag, rc.sub_video_resolution);
	substreamparam.sub_bit_type = rc.sub_bit_type;
	substreamparam.sub_quality = rc.sub_quality;
	substreamparam.sub_framerate = rc.sub_framerate;
	substreamparam.sub_bitrate = htonl(rc.sub_bitrate);
	printf("sub chn=%d,bit type=%d,bit max=%d,frame=%d\n",substreamparam.channelno,substreamparam.sub_bit_type,ntohl(substreamparam.sub_bitrate),substreamparam.sub_framerate);	
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_SubStreamParam_t);
	}
	memcpy(pbyAckBuf,&substreamparam,sizeof(ifly_SubStreamParam_t));
	printf("Get Sub Stream Param!\n");
}

int   Deal_CTRL_CMD_SETSUBSTREAMPARAM()
{
	ifly_record_t rc;
	int chnno;
	ifly_SubStreamParam_t recinfo;
	memcpy(&recinfo,pbyMsgBuf,sizeof(recinfo));
	chnno = recinfo.channelno;
	GetRecordSet(&rc,chnno);
	rc.channel_no = chnno;
	if(rc.channel_no >= byTotalRecChn) return CTRL_FAILED_PARAM;

	rc.sub_bit_type = recinfo.sub_bit_type;
	rc.sub_quality = recinfo.sub_quality;
	rc.sub_framerate = recinfo.sub_framerate;
	rc.sub_bitrate = ntohl(recinfo.sub_bitrate);
	if(recinfo.sub_flag)
	{
		rc.sub_video_resolution = 0;
	}
	else
	{
		rc.sub_video_resolution = 1;			//pw 2010/10/29
	}

	printf("~~~~sub_video_resolution:%d, sub_flag:%d\n", rc.sub_video_resolution, recinfo.sub_flag);
	//wrchen 090225
	SetRecordSet(&rc,chnno);
	i = chnno;
	if(btmpSubVideoNetSnd[i])//wrchen 090915
	{
		 venc_parameter_t mvp;
		//printf("chn%d:open\n",i+1);
		if(getYFactor() == NTSC)
		{
			if(rc.sub_framerate==25)
			{
				rc.sub_framerate = 30;
			}
		}
		else
		{
			if(rc.sub_framerate==30)
			{
				rc.sub_framerate = 25;
			}
		}
		
		mvp.frame_rate = rc.sub_framerate;
		mvp.is_cbr = !rc.sub_bit_type;
		mvp.pic_level = rc.sub_quality;
		mvp.bit_rate = rc.sub_bitrate;
		tl_venc_minor_set_para(i, &mvp);
		
	}
	
	for (i = 0; i < MAX_CHN_NUM; i++)
	{
		if ((1 << i ) & (ntohl(recinfo.copy2chnmask)))
		{
			GetRecordSet(&rc,i);
			rc.sub_bit_type = recinfo.sub_bit_type;
			rc.sub_quality = recinfo.sub_quality;
			rc.sub_framerate = recinfo.sub_framerate;
			rc.sub_bitrate = ntohl(recinfo.sub_bitrate);
			if(recinfo.sub_flag)
			{
				rc.sub_video_resolution = 0;
			}
			else
			{
				rc.sub_video_resolution = 1;			//pw 2010/10/29
			}
			SetRecordSet(&rc,i);

			if(btmpSubVideoNetSnd[i])//wrchen 090915
			{
				 venc_parameter_t mvp;
				//printf("chn%d:open\n",i+1);
				if(getYFactor() == NTSC)
				{
					if(rc.sub_framerate==25)
					{
						rc.sub_framerate = 30;
					}
				}
				else
				{
					if(rc.sub_framerate==30)
					{
						rc.sub_framerate = 25;
					}
				}
				
				mvp.frame_rate = rc.sub_framerate;
				mvp.is_cbr = !rc.sub_bit_type;
				mvp.pic_level = rc.sub_quality;
				mvp.bit_rate = rc.sub_bitrate;
				tl_venc_minor_set_para(i, &mvp);
				
			}
		}

	}
	*pAckLen = 0;
	printf("set recordparam\n");
	WriteConfig(&ConfManager);
	/*****************************/

	/*****************************/
	
	

	/* 07-10-09-8 begin 远程配置*/
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"RecSet");
	logCopy(log);
	/* 07-10-09 end */
	return CTRL_SUCCESS;
}



int   Deal_CTRL_CMD_GETIMGPARAM()
{
	printf("get imageparam!\n");
	Web_Imag_Init(pbyAckBuf,pAckLen,*pbyMsgBuf);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETIMGPARAM()
{
		int k;
		ifly_image_t img;
		ifly_ImgParam_t setimgparam;
		memset(&setimgparam,0,sizeof(setimgparam));
		memcpy(&setimgparam,pbyMsgBuf,sizeof(ifly_ImgParam_t));
		
		int chnno = setimgparam.channel_no;
		GetImageSet(&img,chnno);
		img.channel_no = chnno;
		strncpy(img.channel_name, setimgparam.channelname,sizeof(img.channel_name));
		img.flag_mask= setimgparam.flag_mask;
		//img.flag_lost = setimgparam.lost;
		ifly_area_t area[MAX_COVER_REGION_NUM];
		memset(area,0,sizeof(area));
		for (k = 0; k < MAX_COVER_REGION_NUM; ++k)
		{
			area[k].channel_no = chnno;
			area[k].type = 1;
			area[k].x = ntohs(setimgparam.MaskInfo[k].x);
			area[k].y = ntohs(setimgparam.MaskInfo[k].y);
			area[k].w = ntohs(setimgparam.MaskInfo[k].width);
			area[k].h = ntohs(setimgparam.MaskInfo[k].height);
			//printf("k=%d,x=%d,y=%d,w=%d,h=%d\n",k,area[k].x,area[k].y,area[k].w,area[k].h);
		}
		SetTmpEnvelopArea(area, chnno);

		/**/
		if(chnno==0)
		{
			img.flag_time=setimgparam.flag_time;
			img.x_time = (ntohs(setimgparam.timepos_x)+7)/8*8;
			img.y_time = (ntohs(setimgparam.timepos_y)+7)/8*8;
		}
		img.flag_name=setimgparam.flag_name;
		img.x_name=(ntohs(setimgparam.chnpos_x)+7)/8*8;
		img.y_name=(ntohs(setimgparam.chnpos_y)+7)/8*8;				

		SetImageSet(&img,chnno);


		printf("recv  Deal_CTRL_CMD_IMAGSET_PARAM msg,chn:%d,time flag:%d,name flag:%d\n",setimgparam.channel_no,setimgparam.flag_time,setimgparam.flag_name);

		for (i = 0; i <MAX_CHN_NUM; i++)
		{
			GetImageSet(&img,i);
			if ((1 << i ) & (ntohl(setimgparam.copy2chnmask))||(i==chnno))
			{
				img.flag_mask = setimgparam.flag_mask;
				//img.flag_lost = setimgparam.lost;
				img.flag_name=setimgparam.flag_name;
				img.x_name=(ntohs(setimgparam.chnpos_x)+7)/8*8;
				img.y_name=(ntohs(setimgparam.chnpos_y)+7)/8*8;

				SetImageSet(&img,i);

				for (k = 0; k < MAX_COVER_REGION_NUM; ++k)
				{
					area[k].channel_no = i;
					area[k].type = 1;
					area[k].x = ntohs(setimgparam.MaskInfo[k].x);
					area[k].y = ntohs(setimgparam.MaskInfo[k].y);
					area[k].w = ntohs(setimgparam.MaskInfo[k].width);
					area[k].h = ntohs(setimgparam.MaskInfo[k].height);
				//	printf("k=%d,x=%d,y=%d,w=%d,h=%d\n",k,area[k].x,area[k].y,area[k].w,area[k].h);
				}
				//SetTmpEnvelopArea(area, i);
				if(i == 0)
				{
					//时间
					printf("time x=%d,y=%d\n",img.x_time,img.y_time);
					g_p_info_layer.time_info_pos.x = img.x_time;
					g_p_info_layer.time_info_pos.y = img.y_time;
					g_p_info_layer.b_time_disp = img.flag_time;
				}

				//通道名
				chnname_info_pos[i].x = (img.x_name/2+7)/8*8;
				chnname_info_pos[i].y = (img.y_name/2+7)/8*8;
				printf("name x=%d,y=%d\n",img.x_name,img.y_name);
				g_p_info_layer.chn_info_layer[i].chnname_pos.x = chnname_info_pos[i].x;
				#if defined(D1_8)//yqluo 20110217
					g_p_info_layer.chn_info_layer[i].chnname_pos.x = chnname_info_pos[i].x/2;
				#endif
				
				g_p_info_layer.chn_info_layer[i].chnname_pos.y = chnname_info_pos[i].y;
				#if defined(D1_8) || defined(HALF_D1) //yqluo 20110217
					g_p_info_layer.chn_info_layer[i].chnname_pos.y = chnname_info_pos[i].y/2;
				#endif
				
				#ifndef ONLY_UNICODE
				memcpy(g_p_info_layer.chn_info_layer[i].chnname,img.channel_name,sizeof(img.channel_name));
				#else
				g_p_info_layer.chn_info_layer[i].chnname[0]=0x20;
				memcpy(g_p_info_layer.chn_info_layer[i].chnname+1,img.channel_name,sizeof(img.channel_name));
				#endif

				printf("image set chn %d name pos(%d,%d)flag:%d\n",i+1,g_p_info_layer.chn_info_layer[i].chnname_pos.x,g_p_info_layer.chn_info_layer[i].chnname_pos.y,img.flag_name);
				g_p_info_layer.chn_info_layer[i].chnname_flag = img.flag_name;

				//遮盖
				//ifly_area_t area[MAX_COVER_REGION_NUM];
				//GetEnvelopArea(area,i);
				for(k=0;k<MAX_COVER_REGION_NUM;k++)
				{
					if(env_fd[i][k]!=-1)
					{
						//printf("tl_video_cover_destroy chn %d,reg:%d\n",i+1,k+1);
						tl_video_cover_destroy(env_fd[i][k]);
						env_fd[i][k] = -1;
					}
				}

				g_p_info_layer.chn_info_layer[i].envelop_flag = 0;
				//GetTmpEnvelopArea(area, i);
				envelop_info_num[i] = GetEnvelopValidArea(area,envelop_info_rect[i]);
				g_p_info_layer.chn_info_layer[i].envelop_num = envelop_info_num[i];
				
				g_p_info_layer.chn_info_layer[i].envelop_flag = img.flag_mask;
				SetEnvelopArea(area, i);
				if(img.flag_mask)
				{

					for(j=0;j<envelop_info_num[i];j++)
					{
						video_cover_atr_t atr;
						atr.color = 0x00E77D4A;
						atr.x = envelop_info_rect[i][j].x;
						atr.y = envelop_info_rect[i][j].y;
						atr.width= envelop_info_rect[i][j].w;
						atr.height= envelop_info_rect[i][j].h;
						printf("x=%d,y=%d,w=%d,h=%d\n",atr.x,atr.y,atr.width,atr.height);
						env_fd[i][j] = tl_video_cover_create(i,&atr);
					}
				}

				//亮度，对比度，灰度，色度
				video_image_para_t image_para;//wrchen 090120
				{
					//printf("chn%d img brihgt new %d\n",i+1,img.brightness);
					image_para.brightness= img.brightness;
					image_para.contrast= img.contrast;
					image_para.hue= img.hue;
					image_para.saturation= img.saturation;
					tl_video_set_img_para(i, &image_para);
				}

			}
		}
		*pAckLen = 0;
		printf("set imageparam\n");


		WriteConfig(&ConfManager);

		system_buzz_clear();
		
		/* 07-10-09-6 begin 远程配置*/
		ifly_loginfo_t log;
		log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
		log.start = time(NULL);
		sprintf(log.loginfo,"NetSet");
		logCopy(log);
		/* 07-10-09 end */
		return CTRL_SUCCESS;
	}

int   Deal_CTRL_CMD_GETPICADJ()
{
	int chn = 0;
	ifly_image_t imag;
	ifly_PicAdjust_t getpicadjust;
	memset(&getpicadjust,0,sizeof(getpicadjust));
	msgLen = sizeof(ifly_PicAdjust_t);
	memcpy(&getpicadjust,pbyMsgBuf,sizeof(ifly_PicAdjust_t));
	chn = getpicadjust.channel_no;
	GetImageSet(&imag,chn);
	if(0==getpicadjust.flag) getpicadjust.val = imag.brightness;
	else if(1==getpicadjust.flag) getpicadjust.val = imag.contrast;
	else if(2==getpicadjust.flag) getpicadjust.val = imag.hue;
	else if(3==getpicadjust.flag) getpicadjust.val = imag.saturation;
	else printf("err  Deal_CTRL_CMD_SETPICADJ\n");
	printf("chn=%d,flag=%d,val=%d\n",chn,getpicadjust.flag,getpicadjust.val);

	ifly_PicAdjust_t picadjust[16];
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_PicAdjust_t);
	}
	memcpy(pbyAckBuf,&getpicadjust,sizeof(ifly_PicAdjust_t));
	printf("get pic adjust\n");
}

int   Deal_CTRL_CMD_SETPICADJ()
{
	int chn = 0;
	ifly_image_t imag;
	ifly_PicAdjust_t setpicadjust;
	memset(&setpicadjust,0,sizeof(setpicadjust));
	msgLen = sizeof(ifly_PicAdjust_t);
	memcpy(&setpicadjust,pbyMsgBuf,sizeof(ifly_PicAdjust_t));
	chn = setpicadjust.channel_no;
	printf("chn=%d,flag=%d,val=%d\n",chn,setpicadjust.flag,setpicadjust.val);
	GetImageSet(&imag,chn);
	if(0==setpicadjust.flag) imag.brightness = setpicadjust.val;
	else if(1==setpicadjust.flag) imag.contrast = setpicadjust.val;
	else if(2==setpicadjust.flag) imag.hue = setpicadjust.val;
	else if(3==setpicadjust.flag) imag.saturation = setpicadjust.val;
	else printf("err  Deal_CTRL_CMD_SETPICADJ\n");
	video_image_para_t image_para;//wrchen 090120
	{
		//printf("chn%d img brihgt new %d\n",i+1,img.brightness);
		image_para.brightness= imag.brightness;
		image_para.contrast= imag.contrast;
		image_para.hue= imag.hue;
		image_para.saturation= imag.saturation;
		tl_video_set_img_para(chn, &image_para);
	}
	
	SetImageSet(&imag,chn);
	*pAckLen = 0;
	printf("set pic adjust\n");
	WriteConfig(&ConfManager);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETALARMINPARAM()
{
	ifly_alarm_ptz_t ptz[MAX_RECCHN_NUM];
	ifly_AlarmInParam_t alarminparam;
	ifly_alarm_in_t  alarmin;
	int id = *pbyMsgBuf;
	u32 tmp;
	
	memset(&alarminparam,0,sizeof(alarminparam));
	GetAlarmInEmailAlarm(&(alarminparam.flag_email),id); 
	GetAlarmInParam(&alarmin, id);
	GetAlarmPtzParam(ptz,id);

	alarminparam.inid = id;
	alarminparam.typein = alarmin.type;
	alarminparam.flag_deal = alarmin.flag_deal;
	alarminparam.flag_buzz = alarmin.flag_buzz;
	tmp = 0;
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		if('1'==alarmin.record_channel[i])
			tmp |= 1<<i;
	}
	alarminparam.triRecChn = htonl(tmp);
	printf("sizeof=%d,",sizeof(ifly_AlarmInParam_t));
	tmp = 0;
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	//for(i=0;i<2;i++)
	{
		if('1'==alarmin.alarm_out[i])
			tmp |= 1<<i;
	}
	alarminparam.triAlarmoutid = htonl(tmp);
	printf("get triAlarmoutid =0x%x\n",tmp);
	alarminparam.delay= htons(alarmin.delay_time);
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		alarminparam.AlarmInPtz[i].flag_preset = ptz[i].flag_preset;
		alarminparam.AlarmInPtz[i].preset = ptz[i].preset;
		alarminparam.AlarmInPtz[i].flag_cruise = ptz[i].flag_cruise;
		alarminparam.AlarmInPtz[i].cruise = ptz[i].cruise_no;
		alarminparam.AlarmInPtz[i].flag_track = ptz[i].flag_track;
	}
	
	
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_AlarmInParam_t);
	}
	memcpy(pbyAckBuf,&alarminparam,sizeof(ifly_AlarmInParam_t));
	printf("get alarm in param!\n");
	return CTRL_SUCCESS;

}

int   Deal_CTRL_CMD_SETALARMINPARAM()
{
	//pw
 	#ifdef FLAGEMAIL
	int id = *pbyMsgBuf;
	#endif

	ifly_alarm_ptz_t ptz[MAX_RECCHN_NUM];
	ifly_alarm_in_t  alarmin;
	ifly_AlarmInParam_t setalarminparam;
	memset(&setalarminparam,0,sizeof(setalarminparam));
	memcpy(&setalarminparam,pbyMsgBuf,sizeof(ifly_AlarmInParam_t));
	printf("sizeof=%d,",sizeof(ifly_AlarmInParam_t));
	printf("set triAlarmoutid=0x%x\n",(ntohl(setalarminparam.triAlarmoutid)));

	GetAlarmInParam(&alarmin, setalarminparam.inid);
	GetAlarmPtzParam(ptz,setalarminparam.inid);

	alarmin.type = setalarminparam.typein ;
	alarmin.flag_deal = setalarminparam.flag_deal ;
	alarmin.flag_buzz = setalarminparam.flag_buzz;

	for(i=0;i<MAX_CHN_NUM;i++)
	{
		if((1 << i ) & (ntohl(setalarminparam.triRecChn)))
			alarmin.record_channel[i] = '1';
		else
			alarmin.record_channel[i] = '0';
	}
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	//for(i=0;i<2;i++)
	{
	printf("set triAlarmoutid=0x%x\n",(ntohl(setalarminparam.triAlarmoutid)));
		if((1 << i ) & (ntohl(setalarminparam.triAlarmoutid)))
			alarmin.alarm_out[i] = '1';
		else
			alarmin.alarm_out[i] = '0';
	}
	alarmin.delay_time = ntohs(setalarminparam.delay);

	
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		ptz[i].flag_preset = setalarminparam.AlarmInPtz[i].flag_preset;
		ptz[i].preset = setalarminparam.AlarmInPtz[i].preset;
		ptz[i].flag_cruise = setalarminparam.AlarmInPtz[i].flag_cruise;
		ptz[i].cruise_no = setalarminparam.AlarmInPtz[i].cruise;
		ptz[i].flag_track = setalarminparam.AlarmInPtz[i].flag_track;
	}

	SetAlarmInParam(&alarmin, setalarminparam.inid);
	SetAlarmPtzParam(ptz,setalarminparam.inid);

	//pw 2010/6/5
	#ifdef FLAGEMAIL
	SetAlarmInEmailAlarm(setalarminparam.flag_email, id);
	SetTmpAlarmInEmailAlarm(setalarminparam.flag_email, id); 
	#endif
	for (i = 0; i < MAX_ALARM_IN_NUM; i++)
	{
		if ((1 << i ) & (ntohl(setalarminparam.copy2AlarmInmask)))
		{
			SetAlarmInParam(&alarmin, i);
			SetAlarmPtzParam(ptz,i);
		}
	}
	*pAckLen = 0;
	printf("set alarm in param!\n");

	WriteConfig(&ConfManager);
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"AlarmSet");
	logCopy(log);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETALARMOUTPARAM()
{
	ifly_AlarmOutParam_t alarmoutparam;
	memset(&alarmoutparam,0,sizeof(alarmoutparam));
	ifly_alarm_out_t alarmout;
	int id = *pbyMsgBuf;
	GetAlarmOutParam(&alarmout, id);

	alarmoutparam.outid = id;
	alarmoutparam.typeout = alarmout.type;
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_AlarmOutParam_t);
	}
	memcpy(pbyAckBuf,&alarmoutparam,sizeof(ifly_AlarmOutParam_t));
	printf("get alarm out param!\n");
}

int   Deal_CTRL_CMD_SETALARMOUTPARAM()
{
	ifly_AlarmOutParam_t setalarmoutparam;
	memset(&setalarmoutparam,0,sizeof(setalarmoutparam));
	ifly_alarm_out_t alarmout;
	memcpy(&setalarmoutparam,pbyMsgBuf,sizeof(ifly_AlarmOutParam_t));
	alarmout.id = setalarmoutparam.outid;
	GetAlarmOutParam(&alarmout, setalarmoutparam.outid);
	alarmout.type = setalarmoutparam.typeout;

	SetAlarmOutParam(&alarmout, alarmout.id);
	
	for (i = 0; i < MAX_ALARM_OUT_NUM; i++)
	{
		if ((1 << i ) & (ntohl(setalarmoutparam.copy2AlarmOutmask)))
		{
			alarmout.id = i;
			SetAlarmOutParam(&alarmout, alarmout.id);
		}
	}
	*pAckLen = 0;
	printf("set alarm out pro!\n");

	WriteConfig(&ConfManager);
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"AlarmSet");
	logCopy(log);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETALARMNOTICYPARAM()
{
	ifly_AlarmNoticeParam_t alarmnoticeparam;
	GetEmailAlarm(alarmnoticeparam.alarm_email);
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_AlarmNoticeParam_t);
	}
	memcpy(pbyAckBuf,&alarmnoticeparam,sizeof(ifly_AlarmNoticeParam_t));
	printf("get alarm noticy param:%s!\n",alarmnoticeparam.alarm_email);
}

int   Deal_CTRL_CMD_SETALARMNOTICYPARAM()
{
	ifly_AlarmNoticeParam_t alarmnoticeparam;
	memcpy(&alarmnoticeparam,pbyMsgBuf,sizeof(ifly_AlarmNoticeParam_t));
	SetEmailAlarm(alarmnoticeparam.alarm_email);
	//pw 2010/9/29
	SetSendEmailAlarm(alarmnoticeparam.alarm_email);
	*pAckLen = 0;
	printf("set alarm noticy papam:%s!\n",alarmnoticeparam.alarm_email);
	WriteConfig(&ConfManager);
	return CTRL_SUCCESS;
}

#ifdef IECTRL_NEW
//2011-1-7   ie设置中图像参数还原默认设置//wangcong
int Deal_CTRL_CMD_RESETPICADJUST()
{
	u8 channel = 0;
	ifly_image_t imag;
	ifly_reset_picadjust_t reset_picadjust;
	memset(&reset_picadjust,0,sizeof(reset_picadjust));
	msgLen = sizeof(ifly_reset_picadjust_t);
	memcpy(&reset_picadjust,pbyMsgBuf,sizeof(ifly_reset_picadjust_t));
	channel = reset_picadjust.chn;
	printf("chn=%d\n",channel);
	GetImageSet(&imag,channel);
#if defined(HISHARP)
	if(TRUE == sys_is_pal())
	{
		if(reset_picadjust.adjtype & 0x04) imag.saturation = 110;//pal
	}
	else
	{
		if(reset_picadjust.adjtype & 0x04) imag.saturation = 120;//ntsc
	}
	if(reset_picadjust.adjtype & 0x01) imag.brightness = 128;
	if(reset_picadjust.adjtype & 0x08) imag.hue = 117;
	if(reset_picadjust.adjtype & 0x02) imag.contrast = 87;
#elif defined(HAOWEI)
	if(reset_picadjust.adjtype & 0x01) imag.brightness = 87;
	if(reset_picadjust.adjtype & 0x02) imag.contrast = 130;
	if(reset_picadjust.adjtype & 0x08) imag.hue = 117;
	if(reset_picadjust.adjtype & 0x04) imag.saturation = 110;
#else
#ifdef HISI_3515
	if(reset_picadjust.adjtype & 0x01) imag.brightness = 128;
	if(reset_picadjust.adjtype & 0x02) imag.contrast = 99;//pw 2010/10/22
	if(reset_picadjust.adjtype & 0x08) imag.hue = 128;
	if(reset_picadjust.adjtype & 0x04) imag.saturation = 128;
#else
	if(reset_picadjust.adjtype & 0x01) imag.brightness = 128;
	if(reset_picadjust.adjtype & 0x02) imag.contrast = 99;//pw 2010/10/22
	if(reset_picadjust.adjtype & 0x08) imag.hue = 128;
	if(reset_picadjust.adjtype & 0x04) imag.saturation = 128;
#endif
#endif
	else printf("err  Deal_CTRL_CMD_SETPICADJ\n");
	video_image_para_t image_para;//wrchen 090120
	{
		//printf("chn%d img brihgt new %d\n",i+1,img.brightness);
		image_para.brightness= imag.brightness;
		image_para.contrast= imag.contrast;
		image_para.hue= imag.hue;
		image_para.saturation= imag.saturation;
		tl_video_set_img_para(channel, &image_para);
	}
	SetImageSet(&imag,channel);
	WriteConfig(&ConfManager);
	printf("reset pic adjust\n");
	*pAckLen = 0;
	return CTRL_SUCCESS;
}

// 获得帧率列表
int Deal_CTRL_CMD_GETFRAMERATELIST()
{
	int i;
	u8 rate_ntsc[7] = {30,20,15,10,6,2,1};
	u8 rate_pal[7] =  {25,20,15,10,6,2,1};
	ifly_framerate_list_t framerate_list;
	ifly_record_t rc;
	int factor = getYFactor();
	GetRecordSet(&rc,*pbyMsgBuf);
	framerate_list.chn = *pbyMsgBuf;
	framerate_list.type = *(pbyMsgBuf + 1);
	for(i=0; i<10; i++)
	{
		if((i < 7) && (factor == NTSC))
		{
			framerate_list.framerate[i] = rate_ntsc[i];
		}
		else if(i < 7)
		{
			framerate_list.framerate[i] = rate_pal[i];
		}
		else
		{
			framerate_list.framerate[i] = 0;
		}
		
	}
	if (pAckLen)
	{
	
		*pAckLen = sizeof(ifly_framerate_list_t);
	}
	memcpy(pbyAckBuf,&framerate_list,sizeof(ifly_SubStreamParam_t));
	printf("Get frame list!\n");
	return CTRL_SUCCESS;
}


//获得支持的最大遮盖数
int   Deal_CTRL_CMD_GETMAX_IMGMASKNUM()
{
	u8 imgmasknum = 4;
	if(pAckLen)
	{
		*pAckLen = sizeof(u8);
	}
	*pbyAckBuf = imgmasknum;
	printf("Get max ingmask number!\n");
	return CTRL_SUCCESS;
}
	

//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
int  CTRL_GET_RESOLUTION_LIST()
{
	u8 video_sub[2] = {1,2};
	#ifdef TIANMIN
	u8 video[3] = {2,3,4};
	#else
	u8 video[2] = {2,4};
	#endif
	//u8 video_hd[3] = {2,3,4};
	ifly_videoresolu_list_t videoresolu_list;
	ifly_record_t rc;
	GetRecordSet(&rc,*pbyMsgBuf);
	videoresolu_list.chn = *pbyMsgBuf;
	videoresolu_list.type = *(pbyMsgBuf + 1);
//		videoresolu_list.type =0;
	printf("%d,sub chn=%d,bit type=%d\n",__LINE__,videoresolu_list.chn,videoresolu_list.type);	
	for(i=0; i<8; i++)
	{
		if(0 == videoresolu_list.type)
		{
				#ifdef TIANMIN
				if(i < 3)
				#else
				if(i < 2)
				#endif
				{
					videoresolu_list.videoresolu[i] = video[i];
				}
				else
				{
					videoresolu_list.videoresolu[i] = 0;
				}
		}
		else if(1 == videoresolu_list.type)
		{
			if(i < 2)
			{
				videoresolu_list.videoresolu[i] = video_sub[i];
			}
			else
			{
				videoresolu_list.videoresolu[i] = 0;
			}
		}
		else
			printf("The cmd of video resolution type is error\n");
		
	}	
	if (pAckLen)
	{
		*pAckLen = sizeof(videoresolu_list);
	}
	printf("%d,sub chn=%d,bit type=%d\n",__LINE__,videoresolu_list.chn,videoresolu_list.type);	
//	memcpy(pbyAckBuf,&videoresolu_list,sizeof(ifly_SubStreamParam_t));
	memcpy(pbyAckBuf,&videoresolu_list,sizeof(videoresolu_list));
	printf("Get video resolution list!\n");
	return CTRL_SUCCESS;
}

//pw 2010/7/14
#endif

//pw 2010/7/14
#ifdef EMAIL_OPT
int   Deal_CTRL_CMD_GETEMAILSMTP()
{
	ifly_AlarmEmail_SMTP_t alarm_mail_smtp;
	memset(&alarm_mail_smtp,0,sizeof(ifly_AlarmEmail_SMTP_t));
	//printf("GET ConfManager alarm_send_email=%s , send_email_pwd=%s , send_email_smtp=%s \n",ConfManager.config.reserved_param.alarm_send_email,ConfManager.config.reserved_param.send_email_pwd, ConfManager.config.reserved_param.send_email_smtp);
	GetSendEmailAlarm(alarm_mail_smtp.username);
	GetSendEmailPwd(alarm_mail_smtp.userpw);
	GetSendEmailSMTP(alarm_mail_smtp.SMTP_svr);
	//printf("GET alarm_mail_smtp.username=%s , alarm_mail_smtp.userpw=%s , alarm_mail_smtp.SMTP_svr=%s \n",alarm_mail_smtp.username,alarm_mail_smtp.userpw, alarm_mail_smtp.SMTP_svr);
	#ifdef SHENSHIYIN_SP2
	alarm_mail_smtp.smtp_port = htons(ConfManager.config.add_param.smtp_port);//yqluo 20101129
	#endif
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_AlarmEmail_SMTP_t);
	}
	memcpy(pbyAckBuf,&alarm_mail_smtp,sizeof(ifly_AlarmEmail_SMTP_t));
	printf("get alarm email smtp param\n");
}


int   Deal_CTRL_CMD_SETEMAILSMTP()
{
	ifly_AlarmEmail_SMTP_t alarm_mail_smtp;
	memcpy(&alarm_mail_smtp,pbyMsgBuf,sizeof(ifly_AlarmEmail_SMTP_t));
	//printf("SET alarm_mail_smtp.username=%s , alarm_mail_smtp.userpw=%s , alarm_mail_smtp.SMTP_svr=%s \n",alarm_mail_smtp.username,alarm_mail_smtp.userpw, alarm_mail_smtp.SMTP_svr);
	//SetSendEmailAlarm(alarm_mail_smtp.username);
	//pw 2010/9/29
	//SetEmailAlarm(alarm_mail_smtp.username);
	SetSendEmailPwd(alarm_mail_smtp.userpw);
	SetSendEmailSMTP(alarm_mail_smtp.SMTP_svr);
	//printf("SET ConfManager alarm_send_email=%s , send_email_pwd=%s , send_email_smtp=%s \n",ConfManager.config.reserved_param.alarm_send_email,ConfManager.config.reserved_param.send_email_pwd, ConfManager.config.reserved_param.send_email_smtp);
	#ifdef SHENSHIYIN_SP2
	ConfManager.config.add_param.smtp_port = ntohs(alarm_mail_smtp.smtp_port);//yqluo 20101129
	#endif
	*pAckLen = 0;
	WriteConfig(&ConfManager);
	printf("get alarm email smtp param\n");
}

#endif

int   Deal_CTRL_CMD_GETRECSCHPARAM: 
{
	memcpy(temp,pbyMsgBuf,sizeof(pbyMsgBuf));
	printf("get rec sch param!\n");
	printf("sizeof=%d,%d\n",sizeof(ifly_RecordSCH_t),sizeof(ifly_RecordSCH_t));
	Web_RecStart_Init(pbyAckBuf,pAckLen,(int)temp[0],(int)temp[1]);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETRECSCHPARAM()
{
	ifly_RecordSCH_t setrecordsch;
	ifly_valid_time_t rt[DAYS_OF_WEEK][SEGMENTS_OF_DAY];
	s32  week;
	int  chnno;
	int  tmp_week;

	memset(&setrecordsch,0,sizeof(setrecordsch));
	memcpy(&setrecordsch,pbyMsgBuf,sizeof(ifly_RecordSCH_t));

	*pAckLen = 0;
	printf("set rec sch param!");
	printf("copy2Weekday=0x%x,",setrecordsch.copy2Weekday);
	printf("copy2Chnmask=0x%x,0x%x,sizeof=%d\n",(ntohl(setrecordsch.copy2Chnmask)),(setrecordsch.copy2Chnmask),sizeof(ifly_RecordSCH_t));
	chnno = setrecordsch.chn;
	GetRecordTime((ifly_valid_time_t *)rt,chnno);
	week = (setrecordsch.weekday+1)%DAYS_OF_WEEK;
	for(i=0;i<SEGMENTS_OF_DAY;i++)
	{
		rt[week][i].start_time = ntohl(setrecordsch.TimeFiled[i].starttime);
		rt[week][i].end_time = ntohl(setrecordsch.TimeFiled[i].endtime);
		rt[week][i].type = setrecordsch.TimeFiled[i].flag_sch;
		if(setrecordsch.TimeFiled[i].flag_md) rt[week][i].type |=(1<<1);
		if(setrecordsch.TimeFiled[i].flag_alarm) rt[week][i].type |=(1<<2);
	}
	for (j = 0; j < DAYS_OF_WEEK; j++)
	{
		if ((1 << j) & setrecordsch.copy2Weekday)
		{
			printf("set sch cp j=%d",j);
			tmp_week = (j+1)%DAYS_OF_WEEK;
			printf(" tmp_week=%d\n",tmp_week);
			memcpy(rt[tmp_week],rt[week],sizeof(rt[week]));
		}
	}
	SetRecordTime((ifly_valid_time_t *)rt,chnno);
	setRecChnTime(&tRecChn[chnno],rt);
	for (i = 0; i < MAX_CHN_NUM; i++)
	{
		if ((1 << i ) & (ntohl(setrecordsch.copy2Chnmask)))
		{
			printf("set sch cp i=%d\n",i);
			chnno = i;
			SetRecordTime((ifly_valid_time_t *)rt,chnno);
			setRecChnTime(&tRecChn[chnno],rt);
		}
	}
	WriteConfig(&ConfManager);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETMDPARAM()
{
	ifly_MDParam_t mdparam;
	int chn = *pbyMsgBuf;
	ifly_image_t img_set;
	ifly_md_area_t md_area;
	u32 tmp = 0;

	
	GetMDArea(&md_area,chn);
	GetImageSet(&img_set,chn);
	GetMDEmailAlarm(&(mdparam.flag_email),chn);
	mdparam.chn = chn;
	mdparam.flag_buzz = img_set.flag_buzz;
	mdparam.delay = htons(img_set.flag_delay);
	mdparam.sense = img_set.flag_motion;
	memcpy(mdparam.block,md_area.flag_block,sizeof(md_area.flag_block));
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		if('1'==img_set.record_channel[i])
		tmp |= 1<<i;
	}
	mdparam.trigRecChn = htonl(tmp);
	tmp = 0;
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	{
		if('1'==img_set.alarm_out[i])
		tmp |= 1<<i;
	}
	mdparam.trigAlarmOut = htonl(tmp);
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_MDParam_t);
	}
	memcpy(pbyAckBuf,&mdparam,sizeof(ifly_MDParam_t));
	printf("get md param!\n");
}

int   Deal_CTRL_CMD_SETMDPARAM()
{
	ifly_MDParam_t setmdparam;
	ifly_image_t img_set;
	ifly_md_area_t md_area;
	ifly_record_t rs;

	memset(&setmdparam,0,sizeof(setmdparam));
	memcpy(&setmdparam,pbyMsgBuf,sizeof(ifly_MDParam_t));
	int chn = setmdparam.chn;

	
	GetMDArea(&md_area,chn);
	GetImageSet(&img_set,chn);
	img_set.flag_buzz = setmdparam.flag_buzz;
	img_set.flag_delay = ntohs(setmdparam.delay);
	img_set.flag_motion = setmdparam.sense;
	memcpy(md_area.flag_block,setmdparam.block,sizeof(md_area.flag_block));

	printf("md chn=%d,rec:%x,aout:%x\n",chn,ntohl(setmdparam.trigRecChn),ntohl(setmdparam.trigAlarmOut));
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		printf("rec:%x&%x=%x\n",(1<<i),(ntohl(setmdparam.trigRecChn)),((1<<i)&(ntohl(setmdparam.trigRecChn)))?1:0);
		img_set.record_channel[i] = 48+(((1<<i)&(ntohl(setmdparam.trigRecChn)))?1:0);
	}
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	{
		printf("md:%x&%x=%x\n",(1<<i),(ntohl(setmdparam.trigAlarmOut)),((1<<i)&(ntohl(setmdparam.trigAlarmOut)))?1:0);
		img_set.alarm_out[i] = 48+(((1<<i)&(ntohl(setmdparam.trigAlarmOut)))?1:0);
	}
	SetMDEmailAlarm(setmdparam.flag_email,chn);
	SetMDArea(&md_area,chn);
	SetImageSet(&img_set,chn);

	if(img_set.flag_motion)
	{
		GetRecordSet(&rs, chn);
		setmd(chn,rs.video_resolution);
	}
	else
	{
		md_flag[chn] = img_set.flag_motion;
	}


	WriteConfig(&ConfManager);
	*pAckLen = 0;
	printf("set md param!\n");
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETVIDEOLOSTPARAM()
{
	ifly_VideoLostParam_t videolostparam;
	int chn = *pbyMsgBuf;
	ifly_dispose_t vl_set;
	u32 tmp = 0;

	
	GetVideoLoseSet(&vl_set,chn);
	GetVLEmailAlarm(&(videolostparam.flag_email),chn);
	GetVLEmailAlarm(&(videolostparam.flag_email),chn);
	
	videolostparam.chn = chn;
	videolostparam.flag_buzz = vl_set.flag_buzz;

	for(i=0;i<MAX_CHN_NUM;i++)
	{
		if('1'==vl_set.record_channel[i])
		tmp |= 1<<i;
	}
	videolostparam.trigRecChn = htonl(tmp);
	tmp = 0;
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	{
		if('1'==vl_set.alarm_out[i])
		tmp |= 1<<i;
	}
	videolostparam.trigAlarmOut = htonl(tmp);

	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_VideoLostParam_t);
	}
	memcpy(pbyAckBuf,&videolostparam,sizeof(ifly_VideoLostParam_t));
	printf("get video lost param!\n");
}

int   Deal_CTRL_CMD_SETVIDEOLOSTPARAM: 	
{
	ifly_dispose_t vl_set;
	ifly_VideoLostParam_t setvideolostparam;
	memset(&setvideolostparam,0,sizeof(setvideolostparam));

	memcpy(&setvideolostparam,pbyMsgBuf,sizeof(ifly_VideoLostParam_t));

	int chn = setvideolostparam.chn;

	GetVideoLoseSet(&vl_set,chn);
	
	vl_set.flag_buzz = setvideolostparam.flag_buzz;
	for(i=0;i<MAX_CHN_NUM;i++)
	{
		//pw 2010/6/5
		vl_set.record_channel[i] = 48+(((1<<i)&ntohl(setvideolostparam.trigRecChn))?1:0);             //dong       100603
			}
	for(i=0;i<MAX_ALARM_OUT_NUM;i++)
	{
		//pw 2010/6/5
		vl_set.alarm_out[i] = 48+(((1<<i)&ntohl(setvideolostparam.trigAlarmOut))?1:0);           //dong       100603
	}
	SetVLEmailAlarm(setvideolostparam.flag_email,chn);
	SetVideoLoseSet(&vl_set,chn);
	WriteConfig(&ConfManager);

	*pAckLen = 0;
	printf("set video lost param!\n");
}

int   Deal_CTRL_CMD_GETVIDEOBLOCKPARAM()
{
                //dong       100603
                ifly_VideoBlockParam_t videoblockparam;
                int chn = *pbyMsgBuf;
                ifly_dispose_t vb_set;
                u32 tmp = 0;  
				GetVBEmailAlarm(&(videoblockparam.flag_email),chn); 
                GetVideoCoverSet(&vb_set,chn);
                GetVBEmailAlarm(&(videoblockparam.flag_email),chn);                      
                videoblockparam.chn = chn;
                videoblockparam.flag_buzz = vb_set.flag_buzz;
                for(i=0;i<MAX_CHN_NUM;i++)
                {
                         if('1'==vb_set.record_channel[i])
                         tmp |= 1<<i;
                }
                videoblockparam.trigRecChn = htonl(tmp);
                tmp = 0;
                for(i=0;i<MAX_ALARM_OUT_NUM;i++)
                {
                         if('1'==vb_set.alarm_out[i])
                         tmp |= 1<<i;
                }
                videoblockparam.trigAlarmOut = htonl(tmp);//dong       100603

	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_VideoBlockParam_t);
	}
	//dong 100603
	memcpy(pbyAckBuf, &videoblockparam, sizeof(ifly_VideoBlockParam_t));			          
}

int   Deal_CTRL_CMD_SETVIDEOBLOCKPARAM()
{
	ifly_VideoBlockParam_t setvideoblockparam;
	memset(&setvideoblockparam,0,sizeof(setvideoblockparam));
	msgLen = sizeof(ifly_VideoBlockParam_t);
	memcpy(&setvideoblockparam,pbyMsgBuf,sizeof(ifly_VideoBlockParam_t));
	memcpy(&videoblockparam[setvideoblockparam.chn],&setvideoblockparam,sizeof(ifly_VideoBlockParam_t));

	//dong       100603                                                                                                          
            ifly_dispose_t vb_set;
            int chn = setvideoblockparam.chn;
            GetVideoCoverSet(&vb_set,chn);                    
            vb_set.flag_buzz = setvideoblockparam.flag_buzz;
            for(i=0;i<MAX_CHN_NUM;i++)
            {
                     vb_set.record_channel[i] = 48+(((1<<i)&ntohl(setvideoblockparam.trigRecChn))?1:0);
            }
            for(i=0;i<MAX_ALARM_OUT_NUM;i++)
            {
                     vb_set.alarm_out[i] = 48+(((1<<i)&ntohl(setvideoblockparam.trigAlarmOut))?1:0);       
            }
            SetVBEmailAlarm(setvideoblockparam.flag_email,chn);
            SetVideoCoverSet(&vb_set,chn);
            WriteConfig(&ConfManager);
	*pAckLen = 0;
	printf("set video block param!\n");
}

int   Deal_CTRL_CMD_GETPTZPARAM()
{
	Web_Yuntai_Init(pbyAckBuf,pAckLen,*pbyMsgBuf);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETPTZPARAM()
{
	ifly_PTZParam_t setptzparam;
	memset(&setptzparam,0,sizeof(setptzparam));
	memcpy(&setptzparam,pbyMsgBuf,sizeof(setptzparam));
	remoteyuntai(setptzparam);
	*pAckLen = 0;
	/* 07-10-09-4 begin 远程配置*/
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_CONFIG);
	log.start = time(NULL);
	sprintf(log.loginfo,"PTZSet");
	logCopy(log);
	/* 07-10-09 end */
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETPRESET()
{
	ifly_PtzPresetCtr_t ptzpresetctrl;
	msgLen = sizeof(ifly_PtzPresetCtr_t);
	memcpy(&ptzpresetctrl,pbyMsgBuf,sizeof(ifly_PtzPresetCtr_t));
	*pAckLen = 0;
	if (ptzpresetctrl.option == 0)
	{
		printf("add presetpoint!\n");

		int presetpos;
		presetpos = ptzpresetctrl.presetpoint;
		if((presetpos<1)||(presetpos>128))
		{
			return CTRL_FAILED_PARAM;
		}
		else
		{
			#ifdef CRUISE_INDEX_PRESETPOS
			wait_cruise_stop();
			#endif
			
			ifly_yuntai_t yt;
			TCamInfo tCamInfo;
			TCamData tCamData;
			int ret;
			u8 chn = ptzpresetctrl.chn;
			GetYunTaiParam(&yt,chn);
			if(set_speed(PTZfd, yt.baud_rate) == -1)
			{
				printf("Set speed Error\n");
				//exit(-1);
			}
			if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
			{
				printf("Set Parity Error\n");
				//exit(-1);
			}
			tCamInfo.m_byType = yt.protocol;
			tCamInfo.m_dwId = yt.address;
			tCamInfo.m_byCmdId = CAM_COMMAND_SETPRESET;
			tCamInfo.m_abyParam[PRESET_INDEX_POS] = presetpos;
			tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
			GetCamProtocol(&tCamInfo,&tCamData);
			ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
			return CTRL_SUCCESS;
		}
	}
	if (ptzpresetctrl.option == 1)
	{
		printf("del presetpoint!\n");
		int presetpos;
		presetpos = ptzpresetctrl.presetpoint;
		if((presetpos<1)||(presetpos>128))
		{
			return CTRL_FAILED_PARAM;
		}
		else
		{
			#ifdef CRUISE_INDEX_PRESETPOS
			wait_cruise_stop();
			#endif
			
			ifly_yuntai_t yt;
			TCamInfo tCamInfo;
			TCamData tCamData;
			int ret;
			u8 chn = ptzpresetctrl.chn;
			GetYunTaiParam(&yt,chn);
			if(set_speed(PTZfd, yt.baud_rate) == -1)
			{
				printf("Set speed Error\n");
				//exit(-1);
			}
			if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
			{
				printf("Set Parity Error\n");
				//exit(-1);
			}
			tCamInfo.m_byType = yt.protocol;
			tCamInfo.m_dwId = yt.address;
			tCamInfo.m_byCmdId = CAM_COMMAND_CLRPRESET;
			tCamInfo.m_abyParam[PRESET_INDEX_POS] = presetpos;
			tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
			GetCamProtocol(&tCamInfo,&tCamData);
			ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
			return CTRL_SUCCESS;
		}
	}
	if (ptzpresetctrl.option == 2)
	{
		printf("go presetpoint!\n");
		int presetpos;
		presetpos = ptzpresetctrl.presetpoint;
		if((presetpos<1)||(presetpos>128))
		{
			return CTRL_FAILED_PARAM;
		}
		else
		{
			#ifdef CRUISE_INDEX_PRESETPOS
			wait_cruise_stop();
			#endif
			
			ifly_yuntai_t yt;
			TCamInfo tCamInfo;
			TCamData tCamData;
			int ret;
			u8 chn = ptzpresetctrl.chn;
			GetYunTaiParam(&yt,chn);
			if(set_speed(PTZfd, yt.baud_rate) == -1)
			{
				printf("Set speed Error\n");
				//exit(-1);
			}
			if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
			{
				printf("Set Parity Error\n");
				//exit(-1);
			}
			tCamInfo.m_byType = yt.protocol;
			tCamInfo.m_dwId = yt.address;
			tCamInfo.m_byCmdId = CAM_COMMAND_SHOTPRESET;
			tCamInfo.m_abyParam[PRESET_INDEX_POS] = presetpos;
			tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
			tCamInfo.m_byCtrlWay = CAM_CTRLWAY_GENERAL;
			GetCamProtocol(&tCamInfo,&tCamData);
			ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
			return CTRL_SUCCESS;
		}
	}
}

int   Deal_CTRL_CMD_GETCRUISEPARAM()
{
	u8   chn;
	int  path_no;
	ifly_cruise_path_t path[MAX_CRUISE_PATH_NUM];
	ifly_PtzCruisePathParam_t getptzcruisepathparam;
	memset(&getptzcruisepathparam,0,sizeof(ifly_PtzCruisePathParam_t));
	chn = *pbyMsgBuf;
	path_no = *(pbyMsgBuf+1);
	GetCruisePath(path,chn);
	getptzcruisepathparam.chn = chn;
	getptzcruisepathparam.cruise_path = path_no;
	if(path_no == 0 || path_no > MAX_CRUISE_PATH_NUM)
	{
	return CTRL_FAILED_PARAM;
	}
	int i;
	for(i=0;i<MAX_CRUISE_POS_NUM;i++)
	{
	getptzcruisepathparam.Cruise_point[i].preset = path[path_no-1].cruise_pos[i].preset_no;
	getptzcruisepathparam.Cruise_point[i].weeltime = path[path_no-1].cruise_pos[i].dwell_time;
	getptzcruisepathparam.Cruise_point[i].rate = path[path_no-1].cruise_pos[i].cruise_speed;
	getptzcruisepathparam.Cruise_point[i].flag_add = 0;
	}
	if (pAckLen)
	{
	*pAckLen = sizeof(ifly_PtzCruisePathParam_t);
	}
	memcpy(pbyAckBuf,&getptzcruisepathparam,sizeof(ifly_PtzCruisePathParam_t));
	printf("get cruise param!\n");
	return CTRL_SUCCESS;
	}
	
int   Deal_CTRL_CMD_SETCRUISEPARAM:	
{
	#ifdef CRUISE_INDEX_PRESETPOS
	wait_cruise_stop();
	#endif
	
	ifly_PtzCruisePathParam_t setptzcruisepathparam;
	memset(&setptzcruisepathparam,0,sizeof(setptzcruisepathparam));
	msgLen = sizeof(ifly_PtzCruisePathParam_t);
	memcpy(&setptzcruisepathparam,pbyMsgBuf,sizeof(ifly_PtzCruisePathParam_t));
	*pAckLen = 0;

	ifly_yuntai_t yt;
	TCamInfo tCamInfo;
	TCamData tCamData;
	int  ret;
	u8   chn;
	char strbuf[16];
	int  path_no;
	ifly_cruise_path_t path[MAX_CRUISE_PATH_NUM];

	chn = setptzcruisepathparam.chn;
	GetYunTaiParam(&yt,chn);
	
	GetCruisePath(path,chn);

	#ifndef CRUISE_INDEX_PRESETPOS
	if(set_speed(PTZfd, yt.baud_rate) == -1)
	{
		printf("Set speed Error\n");
		//exit(-1);
	}
	if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
	{
		printf("Set Parity Error\n");
		//exit(-1);
	}
	
	tCamInfo.m_byType = yt.protocol;
	tCamInfo.m_dwId = yt.address;
	tCamInfo.m_byCmdId = CAM_COMMAND_INSERTCRUISEPOS;
	tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
	#endif
	path_no = setptzcruisepathparam.cruise_path;
	//printf("path_no=%d\n",path_no);
	if(path_no == 0 || path_no > MAX_CRUISE_PATH_NUM)
	{
		return CTRL_FAILED_PARAM;
	}
	#ifndef CRUISE_INDEX_PRESETPOS
	tCamInfo.m_abyParam[CRUISE_PATH_POS] = path_no;
	#endif
	
	int i;
	for(i=0;i<MAX_CRUISE_POS_NUM;i++)
	{
		if(1==setptzcruisepathparam.Cruise_point[i].flag_add) //add 1
		{
			path[path_no-1].cruise_pos[i].cruise_no=i+1;
			path[path_no-1].cruise_pos[i].preset_no=setptzcruisepathparam.Cruise_point[i].preset;
			path[path_no-1].cruise_pos[i].dwell_time=setptzcruisepathparam.Cruise_point[i].weeltime;
			path[path_no-1].cruise_pos[i].cruise_speed=setptzcruisepathparam.Cruise_point[i].rate;
		}
		else if(2==setptzcruisepathparam.Cruise_point[i].flag_add) //del 2
		{
			path[path_no-1].cruise_pos[i].cruise_no		= 0xff;
			path[path_no-1].cruise_pos[i].preset_no		= 0;
			path[path_no-1].cruise_pos[i].dwell_time		= 0;
			path[path_no-1].cruise_pos[i].cruise_speed	= 0;
		}
	}
	#ifndef CRUISE_INDEX_PRESETPOS
	for(i=0;i<MAX_CRUISE_POS_NUM;i++)
	{
		tCamInfo.m_abyParam[4+i*4+CRUISE_INDEX_POS] = path[path_no-1].cruise_pos[i].cruise_no;
		tCamInfo.m_abyParam[4+i*4+PRESET_INDEX_POS] = path[path_no-1].cruise_pos[i].preset_no;
		tCamInfo.m_abyParam[4+i*4+DWELL_TIME_POS]	= path[path_no-1].cruise_pos[i].dwell_time;
		tCamInfo.m_abyParam[4+i*4+CRUISE_SPEED_POS] = path[path_no-1].cruise_pos[i].cruise_speed;
	}
	
	GetCamProtocol(&tCamInfo,&tCamData);
	ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	#endif
	SetCruisePath(path,chn);

	WriteConfig(&ConfManager);
	
	printf("set cruise param!\n");
}

int   Deal_CTRL_CMD_CTRLCRUISEPATH()
{
	#ifdef CRUISE_INDEX_PRESETPOS
	wait_cruise_stop();
	#endif
	ifly_CruisePathCtr_t cruisepathctrl;
	memcpy(&cruisepathctrl,pbyMsgBuf,sizeof(cruisepathctrl));
	msgLen = sizeof(ifly_CruisePathCtr_t);
	*pAckLen = 0;

	ifly_yuntai_t yt;
	TCamInfo tCamInfo;
	TCamData tCamData;
	int  ret;
	u8   chn;
	int  path_no;
	

	chn = cruisepathctrl.chn;
	GetYunTaiParam(&yt,chn);
	#ifndef CRUISE_INDEX_PRESETPOS
	if(set_speed(PTZfd, yt.baud_rate) == -1)
	{
		printf("Set speed Error\n");
		//exit(-1);
	}
	if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
	{
		printf("Set Parity Error\n");
		//exit(-1);
	}
	#endif
	path_no = cruisepathctrl.cruisepath;
	//printf("path_no=%d\n",path_no);
	if(path_no == 0 || path_no > MAX_CRUISE_PATH_NUM)
	{
		return CTRL_FAILED_PARAM;
	}
	#ifndef CRUISE_INDEX_PRESETPOS
	tCamInfo.m_byType = yt.protocol;
	tCamInfo.m_dwId = yt.address;
	if (cruisepathctrl.flagoption == 0)
	{
		tCamInfo.m_byCmdId = CAM_COMMAND_STOPCRUISE;
		printf("stop cruise path!\n");
	}
	else if (cruisepathctrl.flagoption == 1)
	{
		tCamInfo.m_byCmdId = CAM_COMMAND_STARTCRUISE;
		printf("start cruise path!\n");
	}
	tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
	tCamInfo.m_abyParam[CRUISE_PATH_POS] = path_no;

	GetCamProtocol(&tCamInfo,&tCamData);
	ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	#else
	if (cruisepathctrl.flagoption == 1) {
		cruise_path = path_no;
		cruise_chn = chn;
		if (0 == cruise_flag) cruise_flag = 1;
	}
	else if (cruisepathctrl.flagoption == 0) {
		cruise_path = path_no;
		cruise_chn = chn;
		cruise_flag = 0;
		}
	#endif
}

int   Deal_CTRL_CMD_CTRLPTZTRACK: 		
{
	#ifdef CRUISE_INDEX_PRESETPOS
	wait_cruise_stop();
	#endif
	ifly_TrackCtr_t trackctrl;
	msgLen = sizeof(ifly_TrackCtr_t);
	memcpy(&trackctrl,pbyMsgBuf,sizeof(ifly_TrackCtr_t));
	*pAckLen = 0;
	if (trackctrl.flagoption == 0)
	{
		printf("start record track!\n");

		ifly_yuntai_t yt;
		TCamInfo tCamInfo;
		TCamData tCamData;
		int ret;
		u8 chn;
		
		chn = trackctrl.chn;
		GetYunTaiParam(&yt,chn);
		
		if(set_speed(PTZfd, yt.baud_rate) == -1)
		{
			printf("Set speed Error\n");
			//exit(-1);
		}
		if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
		{
			printf("Set Parity Error\n");
			//exit(-1);
		}
		
		tCamInfo.m_byType = yt.protocol;
		tCamInfo.m_dwId = yt.address;
		tCamInfo.m_byCmdId = CAM_COMMAND_STARTPATTERN;
		tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
		
	}
	if (trackctrl.flagoption == 1)
	{
		printf("stop record track!\n");
		ifly_yuntai_t yt;
		TCamInfo tCamInfo;
		TCamData tCamData;
		int ret;
		u8 chn;
		
		chn = trackctrl.chn;
		GetYunTaiParam(&yt,chn);
		
		if(set_speed(PTZfd, yt.baud_rate) == -1)
		{
			printf("Set speed Error\n");
			//exit(-1);
		}
		if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
		{
			printf("Set Parity Error\n");
			//exit(-1);
		}
		
		tCamInfo.m_byType = yt.protocol;
		tCamInfo.m_dwId = yt.address;
		tCamInfo.m_byCmdId = CAM_COMMAND_STOPPATTERN;
		tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	}
	if (trackctrl.flagoption == 2)
	{
		printf("start track!\n");
		ifly_yuntai_t yt;
		TCamInfo tCamInfo;
		TCamData tCamData;
		int ret;
		u8 chn;
		
		chn = trackctrl.chn;
		GetYunTaiParam(&yt,chn);
		
		if(set_speed(PTZfd, yt.baud_rate) == -1)
		{
			printf("Set speed Error\n");
			//exit(-1);
		}
		if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
		{
			printf("Set Parity Error\n");
			//exit(-1);
		}
		
		tCamInfo.m_byType = yt.protocol;
		tCamInfo.m_dwId = yt.address;
		tCamInfo.m_byCmdId = CAM_COMMAND_RUNPATTERN;
		tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	}
	if (trackctrl.flagoption == 3)
	{
		printf("stop track!\n");
		ifly_yuntai_t yt;
		TCamInfo tCamInfo;
		TCamData tCamData;
		int ret;
		u8 chn;
		
		chn = trackctrl.chn;
		GetYunTaiParam(&yt,chn);
		
		if(set_speed(PTZfd, yt.baud_rate) == -1)
		{
			printf("Set speed Error\n");
			//exit(-1);
		}
		if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
		{
			printf("Set Parity Error\n");
			//exit(-1);
		}
		
		tCamInfo.m_byType = yt.protocol;
		tCamInfo.m_dwId = yt.address;
		tCamInfo.m_byCmdId = CAM_COMMAND_RUNPATTERN;
		tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
		//只有这样才能停止轨迹
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVERIGHT;
		tCamInfo.m_abyParam[PAN_SPEED_POS] = 1;
		tCamInfo.m_abyParam[TILT_SPEED_POS] = 1;
		tCamInfo.m_byCtrlWay = CAM_CTRLWAY_GENERAL;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
		
		tCamInfo.m_byCmdId = CAM_COMMAND_STOPALL;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	}
}

int   Deal_CTRL_CMD_GETHDDINFO()
{
	int k;
	ifly_hddInfo_t hddinfo;
	memset(&hddinfo,0,sizeof(hddinfo));
	k= *pbyMsgBuf;
	if(k<MAX_HDD_NUM)
	{
		if(hddmanager.hinfo[k].is_disk_exist)
		{
			get_disk_info(&hddmanager,k);

			//pw 2010/6/11
			hddinfo.capability = htonl(hddmanager.hinfo[k].total/1024);
			hddinfo.freesize = htonl(hddmanager.hinfo[k].free/1024);
			hddinfo.hdd_exist = 1;
			hddinfo.hdd_index = k;
		}
		memcpy(pbyAckBuf,&hddinfo,sizeof(ifly_hddInfo_t));
		
		if (pAckLen)
		{
			*pAckLen = sizeof(ifly_hddInfo_t);
		}
		printf("get hdd info succ k=%d!, total:%ld, free:%ld\n",k, hddmanager.hinfo[k].total, hddmanager.hinfo[k].free);
		return CTRL_SUCCESS;
	}
	else
	{
		printf("get hdd info err k=%d!\n",k);
		return CTRL_FAILED_PARAM;
	}
}

int   Deal_CTRL_CMD_GETUSERINFO()
{
	ifly_userNumber_t usernumInfo;
	memset(&usernumInfo,0,sizeof(usernumInfo));
	int un=GetUserNum();	
	ifly_usermanage_t tem_user;
	un =min(un, 8);
	for(i=0;i<un;++i)
	{
		GetUserInfo(&tem_user, i);
		strncpy(usernumInfo.userNum[i].name,tem_user.name,sizeof(usernumInfo.userNum[i].name));
		strncpy(usernumInfo.userNum[i].passwd,tem_user.password,sizeof(usernumInfo.userNum[i].passwd));
		strncpy(usernumInfo.userNum[i].macaddr,tem_user.mac_address,sizeof(usernumInfo.userNum[i].macaddr));

		usernumInfo.userNum[i].lcamer = tem_user.local_privilege[0] -48;
		usernumInfo.userNum[i].lrec = tem_user.local_privilege[1] -48;
		usernumInfo.userNum[i].lplay = tem_user.local_privilege[2] -48;
		usernumInfo.userNum[i].lsetpara = tem_user.local_privilege[3] -48;
		usernumInfo.userNum[i].llog = tem_user.local_privilege[4] -48;
		usernumInfo.userNum[i].ltool = tem_user.local_privilege[5] -48;

		usernumInfo.userNum[i].rcamer = tem_user.remote_privilege[0] -48;
		usernumInfo.userNum[i].rrec = tem_user.remote_privilege[1] -48;
		usernumInfo.userNum[i].rplay = tem_user.remote_privilege[2] -48;
		usernumInfo.userNum[i].rsetpara = tem_user.remote_privilege[3] -48;
		usernumInfo.userNum[i].rlog = tem_user.remote_privilege[4] -48;
		usernumInfo.userNum[i].rtool = tem_user.remote_privilege[5] -48;
		usernumInfo.userNum[i].rpreview = tem_user.remote_privilege[6] -48;
		usernumInfo.userNum[i].ralarm = tem_user.remote_privilege[7] -48;
		usernumInfo.userNum[i].rvoip = tem_user.remote_privilege[8] -48;
		//strncpy(usernumInfo.userNum[i].local_privilege,tem_user.local_privilege,sizeof(usernumInfo.userNum[i].local_privilege));
		//strncpy(usernumInfo.userNum[i].remote_privilege,tem_user.remote_privilege,sizeof(usernumInfo.userNum[i].remote_privilege));
		
	}
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_userNumber_t);
	}
	memcpy(pbyAckBuf,&usernumInfo,sizeof(ifly_userNumber_t));
	printf("get user info!\n");
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETUSERINFO()
{
//	ifly_userNumber_t usernumInfo;
	ifly_userMgr_t usermanger;
	memset(&usermanger,0,sizeof(usermanger));
	msgLen = sizeof(ifly_userMgr_t);
	memcpy(&usermanger,pbyMsgBuf,sizeof(ifly_userMgr_t));
	*pAckLen = 0;
	if (usermanger.flagOption == 0)
	{
		printf("AddUser!\n");
		
		{
		ifly_usermanage_t User;
		
		//added by kmli 2007-05-30
		//防止用户名为空或跟数据库中已存在的用户名重名
	//	if(!strcmp(usermanger.userInfo.name,"admin") || !strcmp(usermanger.userInfo.name,""))
	//	{
	//		return CTRL_FAILED_PARAM;
	//	}
		
		int k;
		ifly_usermanage_t tem_user;
		int un=GetUserNum();	
		if(un>=MAX_USER_NUM)
		{
			return CTRL_FAILED_USER_MAX;//wrchen 080529
		}

		for(k=1;k<un;k++)
		{
			GetUserInfo(&tem_user, k);
			if(!strcmp(usermanger.userInfo.name,tem_user.name))
			{
				return CTRL_FAILED_USER_SAME;
			}
		}
		remote_user(&User,usermanger.userInfo);
		SetUserInfo(&User, un);
		SetUserNum(++un);
		WriteConfig(&ConfManager);
		return CTRL_SUCCESS;
	}
	}
	if (usermanger.flagOption == 1)
	{
		printf("EditUser!\n");
		ifly_usermanage_t User;
		remote_user(&User,usermanger.userInfo);//在函数remote_user中添加了语音对讲权限2007-06-26
		int un=GetUserNum();	
		ifly_usermanage_t tem_user;
		int k;
		for(k=1;k<un;k++)
		{
			GetUserInfo(&tem_user, k);
			if(!strcmp(User.name,tem_user.name))
			{
				SetUserInfo(&User, k);
				WriteConfig(&ConfManager);
				return CTRL_SUCCESS;
				//
			}
		}

		return CTRL_FAILED_USER;
	}
	if (usermanger.flagOption == 2)
	{
		printf("DelUser!\n");
		int un=GetUserNum();	
		ifly_usermanage_t tem_user,userinfo2;
		int i,k;
		for(k=1;k<un;k++)
		{
			GetUserInfo(&tem_user, k);
			if(!strcmp(usermanger.userInfo.name,tem_user.name))
			{
				for(i=k+1;i<un;i++)
				{
					GetUserInfo(&userinfo2,i);
					SetUserInfo(&userinfo2, i-1);
				}
				SetUserNum(--un);
				WriteConfig(&ConfManager);
				return CTRL_SUCCESS;
			}
		}
		return CTRL_FAILED_USER;
	}
}

int   Deal_CTRL_CMD_SETRESTORE()
{
	*pAckLen = 0;
	printf("resume cmd got\n"); 

//pw 2010/9/6
#if defined(CFG_SAVE_IN_FLASH)
	ErareConfig(&ConfManager);
#else
	char path[32];
	sprintf(path,DEV_EEPROM);
	remove(path);
	sync();
#endif
	
	SetWndText2(IDC_STATIC_UPDATESUCC_TEXT,IDC_TEXT_SETSUCREBOOT);
	SetSuccFlag(2);
	//ShowWnd(IDD_DIALOG_UPDATESUCC,TRUE);
	fb_resume_nomouse();
	ShowMsgBox(IDC_TEXT_SETSUCREBOOT,IDD_DIALOG_RESUME,NULL,FALSE);//wrchen 090901
	fb_resume_mouse();
	
	OnUpdateSuccOK(0);
	return CTRL_SUCCESS; 
}

int   Deal_CTRL_CMD_CLEARALARM:			
{
	*pAckLen = 0;
	printf("clear alarm!\n");
	ClearAlarm();//2007-06-13
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_GETSYSTIME()
{
	ifly_sysTime_t getsystemtime;
	memset(&getsystemtime,0,sizeof(getsystemtime));
	getsystemtime.systemtime = htonl(time(NULL));
	memcpy(pbyAckBuf,&getsystemtime,sizeof(getsystemtime));
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_sysTime_t);
	}
	memcpy(pbyAckBuf,&getsystemtime,sizeof(ifly_sysTime_t));
	time_t t = ntohl(getsystemtime.systemtime);
	printf("get systime time:%s!\n",ctime(&t));
}

int   Deal_CTRL_CMD_SETSYSTIME()
{
	if(APP_MODE_PLAY == byAppMode)
	{
		return CTRL_FAILED_PARAM;
	}
	int i;
	ifly_sysTime_t setsystemtime;
	memset(&setsystemtime,0,sizeof(setsystemtime));
	for(i=0;i<byTotalRecChn;i++)
	{
		EmRecState eCurState = tRecChn[i].eState;
		if(eCurState == RECSTATE_RUNNING || eCurState == RECSTATE_DELAY)
		{
			//ShowMsgBox(IDC_TEXT_RECONDINGNOSETTIME,IDC_CAP_SYSTIME,NULL,TRUE);
			//ShowMsgBox("系统正在录像,此时不能设置时间!","系统时间",NULL,TRUE);
			return CTRL_FAILED_PARAM;
		}
	}
	
	memcpy(&setsystemtime,pbyMsgBuf,sizeof(ifly_sysTime_t));
	time_t t = ntohl(setsystemtime.systemtime);
	stime(&t);
	write_rtc(t,1);
	*pAckLen = 0;
	printf("set system time:%s!\n",ctime(&t));
	return CTRL_SUCCESS;
}

#ifdef _DAYTIME_
int   Deal_CTRL_CMD_USEUTCTIME()
{
	ifly_utctime_t isUseUTC;
	memset(&isUseUTC,0,sizeof(ifly_utctime_t));
	isUseUTC.bUseUTC = 1;//1
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_utctime_t);
	}
	memcpy(pbyAckBuf,&isUseUTC,sizeof(ifly_utctime_t));
	printf("use UTC time\n");
}
#endif

int   Deal_CTRL_CMD_GETSYSINFO()
{
	ifly_sysparam_t sys;
	GetSysParam(&sys);
	ifly_sysinfo_t systeminfo;
	memset(&systeminfo,0,sizeof(systeminfo));
	strcpy(systeminfo.devicename,sys.device_name);
	strcpy(systeminfo.devicemodel,sys.device_model);
	strcpy(systeminfo.deviceser,sys.device_serial);
	strcpy(systeminfo.version,sys.version);
	if (pAckLen)
	{
		*pAckLen = sizeof(ifly_sysinfo_t);
	}
	memcpy(pbyAckBuf,&systeminfo,sizeof(ifly_sysinfo_t));
	printf("get systime info!\n");
}

int   Deal_CTRL_CMD_SHUTDOWN()
{
	
	//do some protection work; 
	*pAckLen = 0;
	printf("remote shutdown!\n");
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_POWEROFF);
	log.start = time(NULL);
	memset(log.loginfo, 0, sizeof(log.loginfo));
	logCopy(log);

	rec_stop_reason |= REC_STOP_POWEROFF; 
	
	// do nothing; 
	sleep(5); 
	//SendToPanel(0x0040);
	SendToPanel(0x0401);
}

int   Deal_CTRL_CMD_REBOOT()
{
	*pAckLen = 0;
	printf("remote reboot!\n");
	
	/* 07-10-09-11 begin 远程重启*/
	ifly_loginfo_t log;
	log.type = (u8)((MASTER_REMOTE_OP<<4)|SLAVE_REMOTE_REBOOT);
	log.start = time(NULL);
	memset(log.loginfo,0,sizeof(log.loginfo));
	logCopy(log);
	/* 07-10-09 end */

	//做好重启前的保护工作
	rec_stop_reason |= REC_STOP_RESTARTUP;
	
	fb_resume_nomouse();
	ShowMsgBox(IDC_TEXT_TYPESUCROBOOT, IDC_CAP_SYSPARA, NULL, TRUE);
	fb_resume_mouse();
	
	usleep(3* 1000 * 1000);
#ifdef NEW_HARDWARE
	clear_lights();
#else
	SendToPanel(0x101001ff);//清除指示灯
#endif

	reboot(RB_AUTOBOOT);
	//iflytek_system("reboot -f");
	
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_PTZCTRL()
{
	#ifdef CRUISE_INDEX_PRESETPOS
	wait_cruise_stop();
	#endif
	
	int ret = 0;
	ifly_yuntai_t yt;
	TCamInfo tCamInfo;
	TCamData tCamData;
	msgLen = sizeof(ifly_PtzCtrl_t);
	memcpy(&ptzctrl,pbyMsgBuf,sizeof(ifly_PtzCtrl_t));
	*pAckLen = 0;
	printf("ptz chn=%d cmd=%d,reserved=%d\n",ptzctrl.chn,ptzctrl.cmd,ptzctrl.reserved);

	GetYunTaiParam(&yt,ptzctrl.chn);


	if(set_speed(PTZfd, yt.baud_rate) == -1)
	{
		printf("Set speed Error\n");
		//exit(-1);
	}
	if(set_Parity(PTZfd, yt.data_bit, yt.stop_bit, 'N') == -1)
	{
		printf("Set Parity Error\n");
		//exit(-1);
	}
	
	tCamInfo.m_byType = yt.protocol;
	tCamInfo.m_dwId = yt.address;
	switch (ptzctrl.cmd)
	{
	case 0()
	#if 0
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVELEFT;
		tCamInfo.m_abyParam[PAN_SPEED_POS] = 1;//MAX_PAN_SPEED;
		tCamInfo.m_abyParam[TILT_SPEED_POS] = 1;//MAX_PAN_SPEED;
		tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
		tCamInfo.m_byCtrlWay = CAM_CTRLWAY_GENERAL;
		GetCamProtocol(&tCamInfo,&tCamData);
		ret = write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	#endif
		
		tCamInfo.m_byCmdId = CAM_COMMAND_STOPALL;
		printf("stop ptz!\n");
		
	case 1()
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVEUP;
		tCamInfo.m_abyParam[TILT_SPEED_POS] = r_ptz_speed;
		printf("ptz up!\n");
		
	case 2()
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVEDOWN;
		tCamInfo.m_abyParam[TILT_SPEED_POS] = r_ptz_speed;
		printf("ptz down!\n");
		
	case 3()
		tCamInfo.m_abyParam[PAN_SPEED_POS] = r_ptz_speed;
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVELEFT;
		printf("ptz left!\n");
		
	case 4()
		tCamInfo.m_abyParam[PAN_SPEED_POS] = r_ptz_speed;
		tCamInfo.m_byCmdId = CAM_COMMAND_MOVERIGHT;
		printf("ptz right!\n");
		
	case 5()
		tCamInfo.m_byCmdId = CAM_COMMAND_AUTOSCAN;
		printf("ptz auto scan!\n");
		
	case 6()
		tCamInfo.m_byCmdId = CAM_COMMAND_ZOOMTELE;
		tCamInfo.m_abyParam[ZOOM_SPEED_POS] = r_ptz_speed;
		printf("ptz zoom +!\n");
		
	case 7()
		tCamInfo.m_byCmdId = CAM_COMMAND_ZOOMWIDE;
		tCamInfo.m_abyParam[ZOOM_SPEED_POS] = r_ptz_speed;
		printf("ptz zoom -!\n");
		
	case 8()
		tCamInfo.m_byCmdId = CAM_COMMAND_FOCUSFAR;
		tCamInfo.m_abyParam[FOCUS_SPEED_POS] = r_ptz_speed;
		printf("ptz focus +!\n");
		
	case 9()
		tCamInfo.m_byCmdId = CAM_COMMAND_FOCUSNEAR;
		tCamInfo.m_abyParam[FOCUS_SPEED_POS] = r_ptz_speed;
		printf("ptz focus -!\n");
		
	case 10()
		tCamInfo.m_byCmdId = CAM_COMMAND_IRISOPEN;
		printf("ptz iris +!\n");
		
	case 11()
		tCamInfo.m_byCmdId = CAM_COMMAND_IRISCLOSE;
		printf("ptz iris -!\n");
		
	case 12()
		tCamInfo.m_byCmdId = CAM_COMMAND_LIGHTOPEN;
		tCamInfo.m_abyParam[LIGHT_POS] = 2;//灯光辅助号为2
		printf("ptz light open!\n");
		
	case 13()
		tCamInfo.m_byCmdId =CAM_COMMAND_LIGHTCLOSE;
		tCamInfo.m_abyParam[LIGHT_POS] = 2;//灯光辅助号为2
		printf("ptz light close!\n");
		
	case 14()
		tCamInfo.m_byCmdId = CAM_COMMAND_WIPEROPEN;
		tCamInfo.m_abyParam[WIPER_POS] = 1;//雨刷辅助号为1
		printf("ptz wipe open!\n");
		
	case 15()
		tCamInfo.m_byCmdId = CAM_COMMAND_WIPERCLOSE;
		tCamInfo.m_abyParam[WIPER_POS] = 1;//雨刷辅助号为1
		printf("ptz wipe close!\n");
		
	case 16()
		r_ptz_speed = getspeed(0);
		printf("ptz speed +!\n");
		return CTRL_SUCCESS;
		
	case 17()
		r_ptz_speed = getspeed(1);
		printf("ptz speed 0!\n");
		return CTRL_SUCCESS;
		
	case 18()
		r_ptz_speed = getspeed(2);
		printf("ptz speed -!\n");
		return CTRL_SUCCESS;
		
	case 19()
		tCamInfo.m_byCmdId = CAM_COMMAND_SETAUX;
		tCamInfo.m_abyParam[AUX_INDEX_POS] = ptzctrl.reserved;
		printf("ptz aux open!\n");
		
	case 20()
		tCamInfo.m_byCmdId = CAM_COMMAND_CLRAUX;
		tCamInfo.m_abyParam[AUX_INDEX_POS] = ptzctrl.reserved;
		printf("ptz aux close!\n");
		
	default()
		printf("ptz err cmd= %d!\n",ptzctrl.cmd);
		return CTRL_FAILED_COMMAND;
		
	}
	tCamInfo.m_abyParam[SENDER_ADDR_POS] = 0;
	tCamInfo.m_byCtrlWay = CAM_CTRLWAY_GENERAL;
	GetCamProtocol(&tCamInfo,&tCamData);
	#if 0
	{
		int i;
		printf("PTZfd=%d,ptzchn=%d,pd:\n",PTZfd,ptzctrl.chn);
		for(i=0;i<tCamData.m_dwLen;i++)
		printf("%02x ",tCamData.m_abyData[i]);
		printf("\n");
		
	}
	#endif
	if(tCamData.m_dwLen)
	{
		write(PTZfd,tCamData.m_abyData,tCamData.m_dwLen);
	}
}

int   Deal_CTRL_CMD_GETMANUALREC()
{
	printf("get record state ok!\n");
	Remote_HandRec_Init(pbyAckBuf,pAckLen);
	return CTRL_SUCCESS;
}

int   Deal_CTRL_CMD_SETMANUALREC:		
{
	ifly_ManualRecord_t manualrecord;
	memcpy(&manualrecord,pbyMsgBuf,sizeof(ifly_ManualRecord_t));
	*pAckLen = 0;
	printf("set record state ok!\n");
	return remote_handrec(manualrecord);
}

int   Deal_CTRL_CMD_LOGSEARCH()
{
	ifly_search_log_t para_info;
	memset(&para_info, 0, sizeof(ifly_search_log_t));

	memcpy(&para_info,pbyMsgBuf,sizeof(para_info));
	SearchLog(para_info,pbyAckBuf,pAckLen);
	return CTRL_SUCCESS;
	printf("log search!\n");
}

int   Deal_CTRL_CMD_RECFILESEARCH()
{
	int sum = 0;
	search_param_t localsearch;
	ifly_recsearch_param_t remotesearch;
	ifly_search_desc_t desc;
	ifly_recfileinfo_t info;
	
#if 1
	//pw 2010/6/25
	#if defined(REC_RETENTION_DAYS)
	ifly_reserved_t sp;
	GetReservedParam(&sp);
	
	time_t currtime;			
	currtime = time(NULL) - sp.flag_timeoverwrt * 24 * 3600;
	//pw 2010/9/27
	if(sp.rec_del_t <= currtime + sp.flag_timeoverwrt * 24 * 3600)
	{
		currtime = (currtime >= sp.rec_del_t ? currtime:sp.rec_del_t);
	}
	#endif
#endif
	
	memcpy(&remotesearch,pbyMsgBuf,sizeof(remotesearch));
	localsearch.channel_no= ntohs(remotesearch.channel_mask);
	localsearch.type = ntohs(remotesearch.type_mask);
	localsearch.start_time = ntohl(remotesearch.start_time);
	localsearch.end_time = ntohl(remotesearch.end_time);
	remotesearch.startID = ntohs(remotesearch.startID);
	remotesearch.max_return = ntohs(remotesearch.max_return);
	
	//pw 2010/6/25
	#if defined(REC_RETENTION_DAYS)
	if(sp.flag_timeoverwrt > 0) 
	{
		localsearch.start_time= (localsearch.start_time >= currtime ? localsearch.start_time : currtime );

		if(localsearch.end_time <= localsearch.start_time) 
		{
			localsearch.end_time = localsearch.start_time;
		}
	}
	#endif
	
	real_rs_rec_file_nums = search_all_rec_file(&hddmanager,&localsearch,all_rs_rec_file_info,MAX_SEARCH_NUM);
	printf("channel_no=0x%x,type=%d,start=%d,end=%d,real_rs_rec_file_nums=%d\n",
	localsearch.channel_no,
	localsearch.type,
	localsearch.start_time,
	localsearch.end_time,
	real_rs_rec_file_nums);
	if(real_rs_rec_file_nums < 0)
	{
		real_rs_rec_file_nums = MAX_SEARCH_NUM;
	}
	if(remotesearch.startID > real_rs_rec_file_nums)
	{
		desc.sum = htons(real_rs_rec_file_nums);
		desc.startID = htons(0);
		desc.endID = htons(0);
		*pAckLen = sizeof(desc);
		memcpy(pbyAckBuf,&desc,sizeof(desc));
		printf(" Deal_CTRL_CMD_RECSEARCH 2,(%d,%d)\n",remotesearch.startID,real_rs_rec_file_nums);
		return CTRL_SUCCESS;
	}
	desc.startID = htons(remotesearch.startID);
	desc.sum = htons(real_rs_rec_file_nums);
	//第0行是列名
	for(i=remotesearch.startID;i<(real_rs_rec_file_nums+1);++i)
	{
		if(i>=remotesearch.startID+remotesearch.max_return) 
		sum++;
		//info.channel_no = (remotesearch.channel_mask);
		//pw 2010/6/10
		printf("channel_no:%d\n", all_rs_rec_file_info[i-1].channel_no);
		info.channel_no = (all_rs_rec_file_info[i-1].channel_no - 1);
		info.type = (all_rs_rec_file_info[i-1].type);
		info.start_time = htonl(all_rs_rec_file_info[i-1].start_time);
		info.end_time = htonl(all_rs_rec_file_info[i-1].end_time);
		info.image_format = (all_rs_rec_file_info[i-1].image_format);
		info.stream_flag = (all_rs_rec_file_info[i-1].stream_flag);
		info.size =htonl( all_rs_rec_file_info[i-1].size);
		get_rec_file_name(&all_rs_rec_file_info[i-1],info.filename,&info.offset);
		printf("filename:%s\n", info.filename);
		info.offset = htonl(info.offset);
		//printf("file name:%s\n",info.filename);
		memcpy(pbyAckBuf+sizeof(desc)+(i-remotesearch.startID)*sizeof(ifly_recfileinfo_t),
			&info,sizeof(ifly_recfileinfo_t));
	}
	desc.endID = htons(remotesearch.startID+sum-1);
	memcpy(pbyAckBuf,&desc,sizeof(desc));
	*pAckLen = sum*sizeof(ifly_recfileinfo_t)+sizeof(desc);
	printf("sum=%d,start=%d,end=%d,len=%d\n",desc.sum,desc.startID,desc.endID,*pAckLen);
}

#if 0//#ifdef ALARM_UPLOAD//csp modify
int   Deal_CTRL_CMD_ALARMUPLOADCENTER()
{
	msgLen = sizeof(unsigned char);
	u8 tmp = 0;
	memcpy(&tmp,pbyMsgBuf,sizeof(unsigned char));
	*pAckLen = 0;
	for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
	{
		if(cph == g_AlarmUploadCenter[i].g_cph)
		{
			if (1 == tmp) {
				return CTRL_SUCCESS;
			} else {
				write_upload_alarm(0xffff0000+i);//set 0
				return CTRL_SUCCESS;
			}
		}
	}

	if (0 == tmp) return CTRL_SUCCESS;

	for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
	{
		if(NULL == g_AlarmUploadCenter[i].g_cph)
		{
			g_AlarmUploadCenter[i].g_cph = cph;
			write_upload_alarm(0xffff0100+i);//set 1
			
		}
	}

	if(i>=MAX_ALARM_UPLOAD_NUM)
	{
		printf("open close alarm upload center err!\n");
		return CTRL_FAILED_RESOURCE;
	}
	printf("open close alarm upload center ok!\n");
}
#endif

