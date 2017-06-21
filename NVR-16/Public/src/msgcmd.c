#include "common_msgcmd.h"

SMsgCmdHdr CreateMsgCmd(u32 nCmdSize)
{
	SMsgCmdHdr psMsgCmdHdr = NULL;
	
	psMsgCmdHdr = (SMsgCmdHdr)malloc(sizeof(SMsgCmd));
	if (psMsgCmdHdr)
	{
		memset((void *)psMsgCmdHdr, 0, sizeof(SMsgCmd));
		
		sem_init(&psMsgCmdHdr->wsem, 0, 1);
		sem_init(&psMsgCmdHdr->cmdsem, 0, 0);
		sem_init(&psMsgCmdHdr->acksem, 0, 0);
		
		psMsgCmdHdr->nCmdSize = nCmdSize;
		
		if (nCmdSize > 0)
		{
			psMsgCmdHdr->pData = malloc(nCmdSize);
			if (NULL == psMsgCmdHdr->pData)
			{
				free(psMsgCmdHdr);
				return NULL;
			}
		}
		
		return psMsgCmdHdr;
	}
	else
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);			
	}
	
	return NULL;
}

s32 DestroyMsgCmd(SMsgCmdHdr psMsgCmdHdr)
{
	if (psMsgCmdHdr)
	{
		if (psMsgCmdHdr->pData)
		{
			free(psMsgCmdHdr->pData);
			psMsgCmdHdr->pData = NULL;
		}
		
		free(psMsgCmdHdr);
	}
	
	return 0;	
}

s32 WriteMsgCmd(SMsgCmdHdr psMsgCmdHdr, const void* pData)
{
	if(psMsgCmdHdr->nCmdSize > 0 && NULL == pData)
	{
		return -1;
	}
	
	sem_wait(&psMsgCmdHdr->wsem);
	
	//PUBPRT("HERE");
	
	if(psMsgCmdHdr->nCmdSize > 0)
	{
		memcpy(psMsgCmdHdr->pData, pData, psMsgCmdHdr->nCmdSize);
	}
	
	//PUBPRT("HERE");
	
	sem_post(&psMsgCmdHdr->cmdsem);
	
	//PUBPRT("HERE");
	
	sem_wait(&psMsgCmdHdr->acksem);
	
	//PUBPRT("HERE");
	
	sem_post(&psMsgCmdHdr->wsem);
	
	//PUBPRT("HERE");
	
	return 0;
}

s32 ReadMsgCmd(SMsgCmdHdr psMsgCmdHdr, void* pData)
{
	if(NULL == psMsgCmdHdr || (psMsgCmdHdr->nCmdSize > 0 && NULL == pData))
	{
		return -1;
	}
	
	s32 ret;
	ret = sem_trywait(&psMsgCmdHdr->cmdsem);
	if(0 == ret)
	{
		if(psMsgCmdHdr->nCmdSize > 0)
		{
			memcpy(pData, psMsgCmdHdr->pData, psMsgCmdHdr->nCmdSize);
		}
	}
	
	return ret;
}

s32 AckMsgCmd(SMsgCmdHdr psMsgCmdHdr)
{
	if(NULL == psMsgCmdHdr)
	{
		printf("No msg dealed!\n");
		return -1;
	}
	
	sem_post(&psMsgCmdHdr->acksem);
	
	return 0;
}

