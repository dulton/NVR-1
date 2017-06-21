#include "common_basetypes.h"
#include "kdvsys.h"
#include "public.h"

s32 PublicIniGetString(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s8* strOutValue, s32 nMaxStrLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strOutValue) || (nMaxStrLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetRegKeyString(strFilePath, strSection, strItem, "default", strOutValue, nMaxStrLen))
	{
		//fprintf(stderr, "Get String failed! function: %s\n", __FUNCTION__);//csp modify
		return -1;
	}
	
	return 0;	
}

s32 PublicIniGetValue(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s32* nOutValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetRegKeyInt(strFilePath, strSection, strItem, 0, (SDWORD *)nOutValue))//csp modify
	{
		//fprintf(stderr, "Get Value<%s,%s,%s> failed! function: %s\n", strFilePath, strSection, strItem, __FUNCTION__);//csp modify
		return -1;
	}
	
	return 0;	
}

s32 PublicIniGetValueU64(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT u64* nOutValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetRegKeyU64(strFilePath, strSection, strItem, 0, nOutValue))
	{
		fprintf(stderr, "Get Value failed! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	return 0;	
}

s32 PublicIniGetArrayStrings(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s8* strOutValue[], PARAOUT s32* nItems, s32 nTableLen,s32 nMaxStrLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strOutValue) || (NULL == nItems) || (nTableLen <= 0) || (nMaxStrLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 num = 0;
	s8 tmp[100] = {0};
	while(1)
	{
		memset(tmp, 0, sizeof(tmp));
		//PUBPRT("HERE");
		if(0 == PublicIniGetArrayString(strFilePath, strSection, strItem, (u8)num, tmp, nMaxStrLen))
		{
			if(num < nTableLen)
			{
			    //PUBPRT("HERE");
			    //strOutValue += nMaxStrLen;
                         //strcpy(strOutValue, tmp);
                         //printf("len=%d temp=%s\n", strlen(tmp), tmp);
                         //printf("len=%d strOutValue[%d]=%p\n", strlen(tmp), num, strOutValue[num]);
		            strcpy(strOutValue[num], tmp);
			}				
		}
		else
		{
				break;
		}
		//PUBPRT("HERE");
		num++;
	}
	*nItems = num;
	
	return 0;	
}

s32 PublicIniGetArrayValues(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s32* nOutValue[], PARAOUT s32* nItems, s32 nTableLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue) || (NULL == nItems) || (nTableLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 num = 0;
	s32 tmp = 0;
	while(1)
	{
		tmp = 0;
		if(0 == PublicIniGetArrayValue(strFilePath, strSection, strItem, (u8)num, &tmp))
		{
			if(num < nTableLen)
			{
				*nOutValue[num] = tmp;
			}				
		}
		else
		{
				break;
		}
		num++;
	}
	*nItems = num;
	
	return 0;
}

s32 PublicIniGetArrayValuesU64(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT u64** nOutValue, PARAOUT s32* nItems, s32 nTableLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue) || (NULL == nItems) || (nTableLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 num = 0;
	u64 tmp = 0;
	while(1)
	{
		tmp = 0;
		if(0 == PublicIniGetArrayValueU64(strFilePath, strSection, strItem, (u8)num, &tmp))
		{
			if(num < nTableLen)
			{
				*nOutValue[num] = tmp;
			}				
		}
		else
		{
				break;
		}
		num++;
	}
	*nItems = num;
	
	return 0;
}

s32 PublicIniGetArrayString(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT s8* strOutValue, s32 nMaxStrLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strOutValue) || (nMaxStrLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetStringFromTableWithKey(strFilePath, strSection, strItem, nIndex+1, strOutValue, nMaxStrLen))
	{
		//fprintf(stderr, "Get String failed! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	return 0;
}

s32 PublicIniGetArrayValue(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT s32* nOutValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetValueFromTableWithKey(strFilePath, strSection, strItem, nIndex+1, (SDWORD *)nOutValue))//csp modify
	{
		//fprintf(stderr, "Get Value failed! function: %s strFilePath:%s strSection:%s strItem:%s nIndex=%d\n"
		//	, __FUNCTION__, strFilePath, strSection, strItem, nIndex);
		return -1;
	}
	
	return 0;
}

s32 PublicIniGetArrayValueU64(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT u64* nOutValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == nOutValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(FALSE == GetU64ValueFromTableWithKey(strFilePath, strSection, strItem, nIndex+1, nOutValue))
	{
		//fprintf(stderr, "Get Value failed! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	return 0;
}

