#include "biz_ptz.h"
#include "biz_config.h"

void PtzDeal(u32* pNums, u8** pData);

s32 PtzInit(u8 nEnable, SBizPtz* psInitPara)
{
	printf("PtzInit -1\n");
	if(nEnable)
	{
		SPtzInitPara sPtzInitPara;
		
		sPtzInitPara.nChnNum = psInitPara->nChnNum;
		sPtzInitPara.pDevPath = psInitPara->pDevPath;
		sPtzInitPara.pfUpdateProtocol = PtzDeal;
		printf("PtzInit -2\n");
		if(0 != ModPtzInit(&sPtzInitPara))
		{
		    return -1;
		}
	}
	
    return 0;
}

void PtzDeal(u32* pNums, u8** pData)
{
	SBizManager* psBizManager = &g_sBizManager;
	int i;
	
	if(pNums && pData)
	{
		psBizManager->nPtzProtocolNum = *pNums;
		psBizManager->pProtocolList = pData;
		//printf("PtzDeal nPtzProtocolNum=%d\n", *pNums);
		//for(i = 0; i <  *pNums; i++)
		//{
		//	printf("%s\n", pData[i]);
		//}
		//printf("\n");
	}
}

//csp modify 20130323
s32 PtzInitPara(u8 nChn, SBizPtzPara* psBizPtzPara)
{
	SBizManager* psBizManager = &g_sBizManager;
	
	if(psBizPtzPara && psBizPtzPara->nProtocol < psBizManager->nPtzProtocolNum)
	{
		SModPtzParam sModPtzPara;
		int i, j, k;
		
		memset(&sModPtzPara, 0, sizeof(SModPtzParam));
		sModPtzPara.nChn = nChn;
		sModPtzPara.emBaudRate = psBizPtzPara->nBaudRate;
		sModPtzPara.emCheckType = psBizPtzPara->nCheckType;
		sModPtzPara.emDataBit = psBizPtzPara->nDataBit;
		sModPtzPara.emFlowCtrlType = psBizPtzPara->nFlowCtrlType;
		sModPtzPara.emStopBit = psBizPtzPara->nStopBit;
		sModPtzPara.nCamAddr = psBizPtzPara->nCamAddr;
		sModPtzPara.nEnable = psBizPtzPara->nEnable;
		strcpy(sModPtzPara.nProtocolName, psBizManager->pProtocolList[psBizPtzPara->nProtocol]);
		for(i = 0; i < sizeof(psBizPtzPara->sAdvancedPara.nIsPresetSet); i++)
		{
			j = i / 64;
			k = i % 64;
			if(psBizPtzPara->sAdvancedPara.nIsPresetSet[i])
			{
				sModPtzPara.sTourPresetPara.nPresetId[j] |= ((u64)1 << k);
			}
			else
			{
				sModPtzPara.sTourPresetPara.nPresetId[j] &= ~((u64)1 << k);
			}
		}
		
		for(i = 0; i < sizeof(psBizPtzPara->sAdvancedPara.sTourPath) / sizeof(SBizTourPath); i++)
		{
			u8 nPathNo = psBizPtzPara->sAdvancedPara.sTourPath[i].nPathNo;
			if(nPathNo > 0)
			{
				nPathNo -= 1;
				j = nPathNo / 64;
				k = nPathNo % 64;
				
				sModPtzPara.sTourPresetPara.nTourId[j] |= ((u64)1 << k);
				for(j = 0; j < sizeof(psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint) / sizeof(SBizTourPoint) ; j++)
				{
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nStayTime = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nDwellTime;
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nPresetId = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nPresetPos;
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nSpeed = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nSpeed;
				}
			}
		}
		
		return ModPtzInitParam(&sModPtzPara);
	}
	
	return -1;
}

s32 PtzSetPara(u8 nChn, SBizPtzPara* psBizPtzPara)
{
	SBizManager* psBizManager = &g_sBizManager;
	
	if(psBizPtzPara && psBizPtzPara->nProtocol < psBizManager->nPtzProtocolNum)
	{
		SModPtzParam sModPtzPara;
		int i, j, k;
		
		memset(&sModPtzPara, 0, sizeof(SModPtzParam));
		sModPtzPara.nChn = nChn;
		sModPtzPara.emBaudRate = psBizPtzPara->nBaudRate;
		sModPtzPara.emCheckType = psBizPtzPara->nCheckType;
		sModPtzPara.emDataBit = psBizPtzPara->nDataBit;
		sModPtzPara.emFlowCtrlType = psBizPtzPara->nFlowCtrlType;
		sModPtzPara.emStopBit = psBizPtzPara->nStopBit;
		sModPtzPara.nCamAddr = psBizPtzPara->nCamAddr;
		sModPtzPara.nEnable = psBizPtzPara->nEnable;
		strcpy(sModPtzPara.nProtocolName, psBizManager->pProtocolList[psBizPtzPara->nProtocol]);
		for(i = 0; i < sizeof(psBizPtzPara->sAdvancedPara.nIsPresetSet); i++)
		{
			j = i / 64;
			k = i % 64;
			if(psBizPtzPara->sAdvancedPara.nIsPresetSet[i])
			{
				sModPtzPara.sTourPresetPara.nPresetId[j] |= ((u64)1 << k);
			}
			else
			{
				sModPtzPara.sTourPresetPara.nPresetId[j] &= ~((u64)1 << k);
			}
		}
		
		for(i = 0; i < sizeof(psBizPtzPara->sAdvancedPara.sTourPath) / sizeof(SBizTourPath); i++)
		{
			u8 nPathNo = psBizPtzPara->sAdvancedPara.sTourPath[i].nPathNo;
			if(nPathNo > 0)
			{
				nPathNo -= 1;
				j = nPathNo / 64;
				k = nPathNo % 64;
				
				sModPtzPara.sTourPresetPara.nTourId[j] |= ((u64)1 << k);
				for(j = 0; j < sizeof(psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint) / sizeof(SBizTourPoint) ; j++)
				{
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nStayTime = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nDwellTime;
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nPresetId = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nPresetPos;
					sModPtzPara.sTourPresetPara.nTourPresetId[nPathNo][j].nSpeed = psBizPtzPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nSpeed;
				}
			}
		}
		
		return ModPtzSetParam(&sModPtzPara);
	}
	
	return -1;
}

s32 PtzCtrl(SBizPtzCtrl* psBizPtzCtrl)
{
	if(psBizPtzCtrl)
	{
		u32 nId = (u32)psBizPtzCtrl->nId;
		return ModPtzCtrl(psBizPtzCtrl->nChn, psBizPtzCtrl->emCmd, &nId);
	}
	
	return -1;
}

s32 PtzCheckIsTouring(u8 nChn)
{
	return PtzCheckTouring(nChn);
}

