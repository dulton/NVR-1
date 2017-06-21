owsp_ret_t owspParseData(char *buf, u_int32 buf_len, owsp_command_t *p_cmd)
{
       TLV_HEADER	*tlvHeaderTemp = NULL;
	TLV_HEADER	  tlvHeader;

	int     Headerror = 0;
	int     iDealResultError = 0;
	int	  iCount = 0;
	int	  iFlag = 0;
	
	const  int iHeaderLen = sizeof(OwspPacketHeader);
	
	u_int32	ulResLen = 0;
	u_int32	ulReadLen = 0;
        printf("owsp Parse Data\n");
	
	Lock();

	pubHeaderTemp = (OwspPacketHeader  *)m_TempBuffer;

	pubHeader.packet_length = ntohl(pubHeaderTemp->packet_length);
	pubHeader.packet_seq = pubHeaderTemp->packet_seq;

	OWSP_DBGPP("\n OWSP Recv Message Len %ld, Sequence %ld\n",pubHeader.packet_length, pubHeader.packet_seq);

	ulResLen = pubHeader.packet_length - 4;
	ulReadLen =iHeaderLen;

	while(ulResLen > 0)
	{
		tlvHeaderTemp = (TLV_HEADER *)(m_TempBuffer + ulReadLen);
		tlvHeader.tlv_len = tlvHeaderTemp->tlv_len;
		tlvHeader.tlv_type = tlvHeaderTemp->tlv_type;
		ulResLen -= sizeof(TLV_HEADER);
		ulReadLen += sizeof(TLV_HEADER);

		++iCount;

		OWSP_DBGPPP("TLV Sequece %d, Type %d, TLV Len %d\n", iCount, tlvHeader.tlv_type, tlvHeader.tlv_len);

		switch(tlvHeader.tlv_type)
		{
			case TLV_T_VERSION_INFO_REQUEST:
				iFlag = Version_Info_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			case TLV_T_LOGIN_REQUEST:
				iFlag = User_Login_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			case TLV_T_SENDDATA_REQUEST:
				break;
			case TLV_T_SUSPENDSENDDATA_REQUEST:
				break;
			case TLV_T_DEVICE_FORCE_EXIT:
				break;
			case TLV_T_CONTROL_REQUEST:
				printf("**********************control PTZ*********************\n");
				iFlag = Ptz_Control_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			case TLV_T_RECORD_REQUEST:
				break;
			case TLV_T_DEVICE_SETTING_REQUEST:
				break;
			case TLV_T_DEVICE_RESET:
				break;
			case TLV_T_CHANNLE_REQUEST:
				iFlag = Switch_Channel_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			case TLV_T_SUSPEND_CHANNLE_REQUEST:
				iFlag = Suspend_Channel_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			case TLV_T_VALIDATE_REQUEST:
			case TLV_T_DVS_INFO_REQUEST:
				break;
			case TLV_T_PHONE_INFO_REQUEST:
				iFlag = Phone_Info_Request(ulReadLen);
				if(-1 == iFlag)
				{
					iDealResultError = 1;
				}
				
				break;
			default:
				Headerror = 1;
				if(m_ConnectSocket)
				{
					close(m_ConnectSocket);
				}
				m_ConnectSocket = 0;
				
				break;
		}

		if(1 == iDealResultError)
		{
			if(m_ConnectSocket)
			{
				close(m_ConnectSocket);
			}
			m_ConnectSocket = 0;
			
			break;
		}

		ulResLen -= tlvHeader.tlv_len;
		ulReadLen += tlvHeader.tlv_len;
	}

	if((Headerror==1) || (1 == iDealResultError))
	{
		UnLock( );
		
		return 	-1;
	}

	UnLock( );
    
	return	0;
}