#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// 10000 ~ 20000 保留内部使用
#define EM_CUSTOM_DEVICE_FONT 10000

#ifdef WIN32
#include <io.h>
#include <stdlib.h>
#else
#include <unistd.h>
#endif

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <sys/mman.h>

#include "ToolCustom.h"
#include "ToolUpgrade.h"

#include "zconf.h"
#include "zlib.h"

#define TL_CUSTOM_MAGIC	0x882312FA
typedef struct {
	u32 nMageic; //magic值
	char strName[32];	//名字需要一定的约定
	u32 nCount; //定制文件数
} SCustomContainer;

#define BUF_SIZE	1024

FILE* gTmpFile = NULL; //定制用临时文件
FILE* gCustomFile = NULL; //定制输出文件
char gStrCustomFileName[256];
char gStrUpgradeFileName[256];
u8 gStrContainerBinDir[256]; // spliang, container.bin dir
u8 gnInit = 0, gnChanged = 0;

#define LANGUAGETOOL_VERSION 0x0001
typedef struct
{
	u8 nLanguageIdNum; //实际语言数
	u8 nLanguageIdDefault; //参考for nLanguageId 取值
	u8 nLanguageIdList[LANGUAGE_SET_NUM_MAX]; //参考for nLanguageId 取值
	u8 reserve[30];
	u16 nVersion;
} SLanguageTool;

#ifdef FLASHDEV
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//create by kong, 2011-11-9

//#define V20_FLASH_TEST_WR

#ifdef V20_FLASH_TEST_WR
struct mtd_info_user{
	unsigned int size;
	unsigned int erasesize;
};
struct erase_info_user{
	unsigned int start;
	unsigned int length;
};
#else
//#include <linux/types.h>
#include <mtd/mtd-user.h>
#endif

s32 GetContainerDev(char* strDev, u32 nMaxLen)
{
	char tmp_buf[256];
	int mtd_idx = 0;
	int fd, rtn;
	
	mtd_idx = 10;
	
	while(mtd_idx > 0)
	{
		sprintf(tmp_buf, "/dev/mtd/%d", mtd_idx);
		fd = open(tmp_buf, O_SYNC | O_RDWR);
		if(fd >= 0)
		{
			SCustomContainer sCustomContainer;
			rtn = read(fd, &sCustomContainer, sizeof(SCustomContainer));
			if(rtn ==  sizeof(SCustomContainer))
			{
				if(sCustomContainer.nMageic == TL_CUSTOM_MAGIC && (0 == strcmp("Container", sCustomContainer.strName)))
				{
					strcpy(strDev, tmp_buf);
					//printf("GetContainerDev %s\n", strDev);
					close(fd);
					return 0;
				}
			}
			close(fd);
		}
		mtd_idx--;
	}

	return -1;
}

s32 CustomExportContainer(char* strDev, char* strFileOut)
{
	if (strDev && strFileOut)
	{
		FILE* fpTmp = NULL;
		FILE* fpOut = NULL;
		s32 nRet, nLen = 0, i, nWrite = 0, nRead = 0;
		u8 Buf[BUF_SIZE];

		fpTmp = fopen(strDev, "rb");
		if (!fpTmp)
		{
			return -1;
		}

		fpOut = fopen(strFileOut, "wb");
		if (!fpOut)
		{
			fclose(fpOut);
			return -1;
		}

		SCustomContainer sCustContainer;

		nRet = fread(&sCustContainer, sizeof(char), sizeof(SCustomContainer),  fpTmp);
		if (nRet != sizeof(SCustomContainer))
		{
			fclose(fpTmp);
			fclose(fpOut);
			 return -1;
		}
		nRet = fwrite(&sCustContainer, sizeof(char), sizeof(SCustomContainer),  fpOut);
		if (nRet != sizeof(SCustomContainer))
		{
			fclose(fpTmp);
			fclose(fpOut);
			return -1;
		}

		for (i = 0; i < sCustContainer.nCount; i++)
		{
			SCustomInfo sCustInfo;
			
			nRet = fread(&sCustInfo, sizeof(char), sizeof(SCustomInfo),  fpTmp);
			if (nRet != sizeof(SCustomInfo))
			{
				fclose(fpTmp);
				fclose(fpOut);
				return -1;
			}

			nRet = fwrite(&sCustInfo, sizeof(char), sizeof(SCustomInfo),  fpOut);
			if (nRet != sizeof(SCustomInfo))
			{
				fclose(fpTmp);
				fclose(fpOut);
				return -1;
			}

			nLen += sCustInfo.nLen;
		}

		while (nLen > 0)
		{
			nRead = nLen > BUF_SIZE ? BUF_SIZE : nLen;
			nRet = fread(Buf, sizeof(char), nRead,  fpTmp);
			if (nRet != nRead)
			{
				fclose(fpTmp);
				fclose(fpOut);
				return -1;
			}

			nRet = fwrite(Buf, sizeof(char), nRead,  fpOut);
			if (nRet != nRead)
			{
				fclose(fpTmp);
				fclose(fpOut);
				return -1;
			}
			
			nWrite += nRead;
			nLen -= nRead;
		}
		
		fclose(fpOut);
		fclose(fpTmp);
		
		return 0;		
	}

	return -1;
}
#endif

s32 CopyData(FILE* fp_from, FILE* fp_to, s32 nSize);
s32 CopyFileTool(char* strFileFrom, char* strFileTo);
int AddCustom(FILE* fp_out, s32 nIndex, s32 nDataOffset, SCustomData* psCustomData, EMCUSTOMTYPE emType);

s32 CustomOpen(char* strFileName)
{
	if (gnInit)
	{
		return -1;
	}
	
	memset(gStrCustomFileName, 0, sizeof(gStrCustomFileName));
	
#ifdef FLASHDEV
	//printf("CustomOpen:before GetContainerDev\n");
	if(GetContainerDev(gStrCustomFileName, sizeof(gStrCustomFileName)))
	{
		printf("GetContainerDev failed\n");
		return -1;
	}
#else
	memset(gStrUpgradeFileName, 0, sizeof(gStrUpgradeFileName));
	
	sprintf(gStrUpgradeFileName, "%s.new", strFileName);
	
	//get container.bin dir, the same with UpgradeFile
	memset(gStrContainerBinDir, 0, sizeof(gStrContainerBinDir));
	int nFileLen = strlen(strFileName);
	while(strFileName[nFileLen-1] != '\\' && strFileName[nFileLen-1] != '/' && nFileLen > 0)
	{
		nFileLen--;
	}
	strncpy( gStrContainerBinDir, strFileName, nFileLen );
	
	CopyFileTool(strFileName, gStrUpgradeFileName);
	
	if (UpgradeFileOut(strFileName, EM_UPGRADE_FT_CONTAINER, gStrCustomFileName, sizeof(gStrCustomFileName)))
	{
		return -1;
	}
#endif
	
	gnInit = 1;
	
	return 0;
}

s32 CustomSave(void)
{
	#ifndef FLASHDEV
	if (gnChanged)
	{
		UpgradeFileRenew(gStrUpgradeFileName, EM_UPGRADE_FT_CONTAINER, gStrCustomFileName);
		gnChanged = 0;
	}
	#endif

	return 0;
}

s32 CustomClose(u8 nSave)
{
	if (0 == gnInit)
	{
		return -1;
	}
	#ifndef FLASHDEV
	if (nSave && gnChanged)
	{
		UpgradeFileRenew(gStrUpgradeFileName, EM_UPGRADE_FT_CONTAINER, gStrCustomFileName);
	}
	
	gnChanged = 0;
	#endif

	memset( gStrContainerBinDir, 0, sizeof(gStrContainerBinDir) );
	
	gnInit = 0;

	return 0;
}

s32 CustomDo(EMCUSTOMTYPE emType, SCustomData* psCustomData)
{
	s32 nRet = 0;
#ifndef FLASHDEV
	u8 strTmpName[256];
	SCustomContainer sCustomContainerOld;
	SCustomContainer sCustomContainer;
	SCustomInfo sCustemItem;
	s32 nCopyItem = 0, nDataOffset = 0;
	u32 i, nCancel = 0, nFoundItem = 0;

	if (emType == EM_CUSTOM_LOGO_START)
	{
		return -1000;
	}

	if (!psCustomData)
	{
		nCancel = 1;
	}
	
	//printf("CustomGenerate-----------------1\n");
	sprintf(strTmpName, "%s.tmp", gStrCustomFileName);	
	if (NULL == gTmpFile) //打开定制临时文件
	{		
		gTmpFile = fopen(strTmpName, "wb");
		if (!gTmpFile)
		{
			printf("open file(%s) error\n", strTmpName);
			nRet = -1;
			goto CustomDoErr;
		}
	}
	else
	{
		fseek(gTmpFile, 0, SEEK_SET);
	}

	if (NULL == gCustomFile) //如果定制文件已经存在，则保留原有定制
	{
		gCustomFile = fopen(gStrCustomFileName, "rb");
		if (!gCustomFile)
		{
		}	
	}
	else
	{
		fseek(gCustomFile, 0, SEEK_SET);
	}

	//printf("CustomGenerate-----------------2\n");

	memset(&sCustomContainerOld, 0, sizeof(SCustomContainer));
	memset(&sCustomContainer, 0, sizeof(SCustomContainer));
	
	sCustomContainer.nMageic = TL_CUSTOM_MAGIC;

	if (gCustomFile)
	{
		//printf("CustomGenerate-----------------3\n");
		nRet = fread(&sCustomContainerOld, sizeof(char), sizeof(SCustomContainer),  gCustomFile);
		if (nRet != sizeof(SCustomContainer)){
			memset(&sCustomContainerOld, 0, sizeof(SCustomContainer));
		} else {
			memcpy(&sCustomContainer, &sCustomContainerOld, sizeof(SCustomContainer));
			sCustomContainer.nCount = 0;
			strcpy(sCustomContainer.strName, "Container");
			
			i = 0;
			for (i = 0; i < sCustomContainerOld.nCount; i++)
			{
				fseek(gCustomFile, sizeof(SCustomContainer) + i * sizeof(SCustomInfo), SEEK_SET);
				nRet = fread(&sCustemItem, sizeof(char), sizeof(SCustomInfo), gCustomFile);
				if (sCustemItem.emType != emType)
				{
					nCopyItem++;
				}
			}

			if (nCancel)
			{
				sCustomContainer.nCount = nCopyItem;
			}
			else
			{
				sCustomContainer.nCount = nCopyItem + 1;
			}

			if (sCustomContainer.nCount > 0)
			{
				nRet = fwrite(&sCustomContainer, sizeof(char), sizeof(SCustomContainer), gTmpFile);
				if(nRet != sizeof(SCustomContainer)){
					printf("write file gTmpFile error 2\n");
					nRet = -2;
					goto CustomDoErr;
				}

				for (i = 0; i < sCustomContainer.nCount; i++)
				{
					nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), gTmpFile);
					if (nRet  != sizeof(SCustomInfo)){
						printf("write file gTmpFile error 3\n");
						nRet = -3;
						goto CustomDoErr;
					}
				}

				nDataOffset = sizeof(SCustomContainer) + sizeof(SCustomInfo) * sCustomContainer.nCount;
				nCopyItem = 0;
				for (i = 0; i < sCustomContainerOld.nCount; i++)
				{
					fseek(gCustomFile, sizeof(SCustomContainer) + i * sizeof(SCustomInfo), SEEK_SET);
					nRet = fread(&sCustemItem, sizeof(char), sizeof(SCustomInfo), gCustomFile);
					if (sCustemItem.emType != emType)
					{
						s32 nReadOffset = sCustemItem.nOffSet;
						sCustemItem.nOffSet = nDataOffset;
						fseek(gTmpFile, sizeof(SCustomContainer) + nCopyItem * sizeof(SCustomInfo), SEEK_SET);
						nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), gTmpFile);
						if (nRet != sizeof(SCustomInfo)){
							fclose(gCustomFile);
							gCustomFile = NULL;
							printf("write file gTmpFile error 4\n");
							nRet = -4;
							goto CustomDoErr;
						}

						if (sCustemItem.nLen > 0)
						{
							fseek(gCustomFile, nReadOffset, SEEK_SET);
							fseek(gTmpFile, sCustemItem.nOffSet, SEEK_SET);
							
							CopyData(gCustomFile, gTmpFile, sCustemItem.nLen);
							
							nDataOffset += sCustemItem.nLen;
						}
						nCopyItem++;
					}
					else
					{
						nFoundItem = 1;
					}
				}
			}
		}

		fclose(gCustomFile);
		gCustomFile = NULL;
	}

	if (0 == sCustomContainer.nCount )
	{
		if (nCancel && (0 == nFoundItem))
		{
			nRet = 0;
			goto CustomDoErr;
		}
		
		fseek(gTmpFile, 0, SEEK_SET);

		sCustomContainer.nCount = 1 - nCancel;
		strcpy(sCustomContainer.strName, "Container");
		nRet = fwrite(&sCustomContainer, sizeof(char), sizeof(SCustomContainer), gTmpFile);
		if (nRet != sizeof(SCustomContainer)){
			printf("write file gTmpFile error 5\n");
			nRet = -5;
			goto CustomDoErr;
		}

		if (sCustomContainer.nCount > 0)
		{
			nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), gTmpFile);
			if (nRet  != sizeof(SCustomInfo)){
				printf("write file gTmpFile error 6\n");
				nRet = -6;
				goto CustomDoErr;
			}
			nDataOffset = sizeof(SCustomContainer) + sizeof(SCustomInfo) * sCustomContainer.nCount;
		}
		//printf("CustomGenerate-----------------4\n");
	}

	if (0 == nCancel)
	{
		//printf("gTmpFile:%x nCopyItem=%d, nDataOffset=%d logo:%s\n", gTmpFile, nCopyItem, nDataOffset, strLogoFileName);
		AddCustom(gTmpFile, nCopyItem, nDataOffset, psCustomData, emType);
	}
	
	//printf("CustomGenerate-----------------5\n");
	
	fclose(gTmpFile);
	gTmpFile = NULL;

	CopyFileTool(strTmpName, gStrCustomFileName);

	remove(strTmpName);
	
	gnChanged = 1;
	//printf("CustomGenerate-----------------6\n");

	return 0;

CustomDoErr:
	
	if (gCustomFile)
	{
		fclose(gCustomFile);
		gCustomFile = NULL;
	}

	if (gTmpFile)
	{
		fclose(gTmpFile);
		gTmpFile = NULL;
	}

	remove(strTmpName);
	
	if (nRet)
	{
		printf("CustomDoErr\n");
	}
#endif		
	return nRet;
}

#ifdef FLASHDEV
s32 CustomChange(EMCUSTOMTYPE emType, SCustomData* psCustomData, char* strContainerFile)
{
	s32 nRet = 0;
	char strTmpName[256];
	SCustomContainer sCustomContainerOld;
	SCustomContainer sCustomContainer;
	SCustomInfo sCustemItem;
	s32 nCopyItem = 0, nDataOffset = 0;
	u32 i, nCancel = 0;
	FILE* TmpFile = NULL; //定制用临时文件
	FILE* CustomFile = NULL; //定制输出文件


	if (emType == EM_CUSTOM_LOGO_START)
	{
		return -1000;
	}

	if (!psCustomData)
	{
		nCancel = 1;
	}
	
	//printf("CustomGenerate-----------------1\n");
	sprintf(strTmpName, "%s.tmp", strContainerFile);	
	if (NULL == TmpFile) //打开定制临时文件
	{		
		TmpFile = fopen(strTmpName, "wb");
		if (!TmpFile)
		{
			printf("open file(%s) error\n", strTmpName);
			nRet = -1;
			goto CustomChangeErr;
		}
	}
	else
	{
		fseek(TmpFile, 0, SEEK_SET);
	}

	if (NULL == CustomFile) //如果定制文件已经存在，则保留原有定制
	{
		CustomFile = fopen(strContainerFile, "rb");
		if (!CustomFile)
		{
		}	
	}
	else
	{
		fseek(CustomFile, 0, SEEK_SET);
	}

	//printf("CustomGenerate-----------------2\n");

	memset(&sCustomContainerOld, 0, sizeof(SCustomContainer));
	memset(&sCustomContainer, 0, sizeof(SCustomContainer));
	
	sCustomContainer.nMageic = TL_CUSTOM_MAGIC;

	if (CustomFile)
	{
		//printf("CustomGenerate-----------------3\n");
		nRet = fread(&sCustomContainerOld, sizeof(char), sizeof(SCustomContainer),  CustomFile);
		if (nRet != sizeof(SCustomContainer)){
			memset(&sCustomContainerOld, 0, sizeof(SCustomContainer));
		} else {
			memcpy(&sCustomContainer, &sCustomContainerOld, sizeof(SCustomContainer));
			sCustomContainer.nCount = 0;
			strcpy(sCustomContainer.strName, "Container");
			
			i = 0;
			for (i = 0; i < sCustomContainerOld.nCount; i++)
			{
				fseek(CustomFile, sizeof(SCustomContainer) + i * sizeof(SCustomInfo), SEEK_SET);
				nRet = fread(&sCustemItem, sizeof(char), sizeof(SCustomInfo), CustomFile);
				if (sCustemItem.emType != emType)
				{
					nCopyItem++;
				}
			}

			if (nCancel)
			{
				sCustomContainer.nCount = nCopyItem;
			}
			else
			{
				sCustomContainer.nCount = nCopyItem + 1;
			}

			if (sCustomContainer.nCount > 0)
			{
				nRet = fwrite(&sCustomContainer, sizeof(char), sizeof(SCustomContainer), TmpFile);
				if(nRet != sizeof(SCustomContainer)){
					printf("write file gTmpFile error 2\n");
					nRet = -2;
					goto CustomChangeErr;
				}

				for (i = 0; i < sCustomContainer.nCount; i++)
				{
					nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), TmpFile);
					if (nRet  != sizeof(SCustomInfo)){
						printf("write file gTmpFile error 3\n");
						nRet = -3;
						goto CustomChangeErr;
					}
				}

				nDataOffset = sizeof(SCustomContainer) + sizeof(SCustomInfo) * sCustomContainer.nCount;
				nCopyItem = 0;
				for (i = 0; i < sCustomContainerOld.nCount; i++)
				{
					fseek(CustomFile, sizeof(SCustomContainer) + i * sizeof(SCustomInfo), SEEK_SET);
					nRet = fread(&sCustemItem, sizeof(char), sizeof(SCustomInfo), CustomFile);
					if (sCustemItem.emType != emType)
					{
						s32 nReadOffset = sCustemItem.nOffSet;
						sCustemItem.nOffSet = nDataOffset;
						fseek(TmpFile, sizeof(SCustomContainer) + nCopyItem * sizeof(SCustomInfo), SEEK_SET);
						nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), TmpFile);
						if (nRet != sizeof(SCustomInfo)){
							fclose(CustomFile);
							CustomFile = NULL;
							printf("write file gTmpFile error 4\n");
							nRet = -4;
							goto CustomChangeErr;
						}

						if (sCustemItem.nLen > 0)
						{
							fseek(CustomFile, nReadOffset, SEEK_SET);
							fseek(TmpFile, sCustemItem.nOffSet, SEEK_SET);
							
							CopyData(CustomFile, TmpFile, sCustemItem.nLen);
							
							nDataOffset += sCustemItem.nLen;
						}
						nCopyItem++;
					}
				}
			}
		}

		fclose(CustomFile);
		CustomFile = NULL;
	}

	if (0 == sCustomContainer.nCount )
	{
		fseek(TmpFile, 0, SEEK_SET);

		sCustomContainer.nCount = 1 - nCancel;
		strcpy(sCustomContainer.strName, "Container");
		nRet = fwrite(&sCustomContainer, sizeof(char), sizeof(SCustomContainer), TmpFile);
		if (nRet != sizeof(SCustomContainer)){
			printf("write file gTmpFile error 5\n");
			nRet = -5;
			goto CustomChangeErr;
		}

		if (sCustomContainer.nCount > 0)
		{
			nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), TmpFile);
			if (nRet  != sizeof(SCustomInfo)){
				printf("write file gTmpFile error 6\n");
				nRet = -6;
				goto CustomChangeErr;
			}
			nDataOffset = sizeof(SCustomContainer) + sizeof(SCustomInfo) * sCustomContainer.nCount;
		}
		//printf("CustomGenerate-----------------4\n");
	}

	if (0 == nCancel)
	{
		//printf("gTmpFile:%x nCopyItem=%d, nDataOffset=%d logo:%s\n", gTmpFile, nCopyItem, nDataOffset, strLogoFileName);
		AddCustom(TmpFile, nCopyItem, nDataOffset, psCustomData, emType);
	}
	
	//printf("CustomGenerate-----------------5\n");
	
	fclose(TmpFile);
	TmpFile = NULL;

	CopyFileTool(strTmpName, strContainerFile);

	remove(strTmpName);

	return 0;

CustomChangeErr:
	
	if (CustomFile)
	{
		fclose(CustomFile);
		CustomFile = NULL;
	}

	if (TmpFile)
	{
		fclose(TmpFile);
		TmpFile = NULL;
	}

	printf("CustomChangeErr\n");
	
	return nRet;
}

#endif	

#ifndef FLASHDEV
extern s32 CustomLanguageSet(SLanguageParam * psLangSet);
#endif

s32 CustomGenerate(EMCUSTOMTYPE emType, SCustomData* psCustomData)
{
	if (gnInit)
	{
		switch (emType)
		{
			case EM_CUSTOM_LOGO_START:
				return UpgradeFileRenew(gStrUpgradeFileName, EM_UPGRADE_FT_OTHER, psCustomData->strFilePath);
			case EM_CUSTOM_DEVICE_LANGUAGE:
				#ifndef FLASHDEV
				if (psCustomData)
				{
					if ((0 == psCustomData->sLanguageParam.nLanguageIdNum) || CustomLanguageSet(&psCustomData->sLanguageParam))
					{
						return -1;
					}
				}
				else
				#endif
				{
					break;
				}
			default:
				if (EM_CUSTOM_DEVICE_FONT == emType)
				{
					return -1;
				}
				return CustomDo(emType, psCustomData);
		}
	}

	return -1;
}

s32 CustomCancel(EMCUSTOMTYPE emType)
{
	if (EM_CUSTOM_ALL == emType)
	{
		SCustomInfo sCustomInfo;
		s32 i, nCount = CustomGetCount();
		for (i = 0; i < nCount; i++)
		{
			if (0 == CustomGetInfo(0, &sCustomInfo))
			{
				CustomDo(sCustomInfo.emType, NULL);
			}
		}
	}
	else
	{
		return CustomDo(emType, NULL);
	}

	return 0;
}

s32 CustomGetCount(void)
{
	s32 nRet;
#ifndef FLASHDEV
	FILE* fpTmp = NULL;
	SCustomContainer sCustomContainer;

	if (!gnInit)
	{
		return 0;
	}
	
	fpTmp = fopen(gStrCustomFileName, "rb");
	if (!fpTmp)
	{
		return 0;
	}	


	nRet = fread(&sCustomContainer, sizeof(char), sizeof(SCustomContainer),  fpTmp);
	fclose(fpTmp);
	if (nRet != sizeof(SCustomContainer))
	{
		return 0;
	}
	else
	{
		return sCustomContainer.nCount;
	}
#else
	s32 fd = open(gStrCustomFileName, O_SYNC | O_RDWR);
	if (fd >= 0)
	{
		SCustomContainer sCustomContainer;
		nRet = read(fd, &sCustomContainer, sizeof(SCustomContainer));
		if (nRet ==  sizeof(SCustomContainer))
		{
			if (sCustomContainer.nMageic == TL_CUSTOM_MAGIC && (0 == strcmp("Container", sCustomContainer.strName)))
			{
				close(fd);
				return sCustomContainer.nCount;
			}
		}
		close(fd);
	}

	return 0;
#endif
}

s32 CustomGetInfo(s32 nIndex, SCustomInfo* psCustomInfo)
{
	FILE* fpTmp = NULL;
	s32 nRet;

	if (!gnInit ||!psCustomInfo )
	{
		return -1;
	}

	fpTmp = fopen(gStrCustomFileName, "rb");
	if (!fpTmp)
	{
		return -1;
	}
	
	fseek(fpTmp, sizeof(SCustomContainer) + nIndex * sizeof(SCustomInfo), SEEK_SET);
	nRet = fread(psCustomInfo, sizeof(char), sizeof(SCustomInfo),  fpTmp);
	fclose(fpTmp);
	if (nRet != sizeof(SCustomInfo))
	{
		return -1;
	}
	else
	{
		if (EM_CUSTOM_DEVICE_FONT == psCustomInfo->emType)
		{
			 psCustomInfo->emType = EM_CUSTOM_DEVICE_GETFONT;
		}
		return 0;
	}
}

s32 CustomReadData(s32 nIndex, u32 nMaxLen, u8* pData, u32 *pDataLen);

s32 CustomUncompress(char* strFileDst, u8* pData, u32 nDataLen)
{
	s32 nRet, nLen, nCur = 0, nPos = 0, nHeadLen = 4;
	u32* pnDataLen = (u32*)pData;
	unsigned long nUnzipSize = *pnDataLen;//前4字节是压缩前文件长度
	u8* nDataBuf = malloc(nUnzipSize);
	unsigned long nZipLen = nDataLen - nHeadLen;
	FILE* fp_in = NULL;

	if (!nDataBuf)
	{
		printf("malloc failed nUnzipSize=%lu\n", nUnzipSize);
		return -1;
	}

	//printf("CustomUncompress before uncompress\n");

	nRet = uncompress((Bytef *)nDataBuf, (unsigned long *)&nUnzipSize, (Bytef *)(pData + nHeadLen), (unsigned long)(nZipLen));
	if (nRet != Z_OK)
	{
		printf("uncompress Error! nRet=%d\n", nRet);
		free(nDataBuf);
		nDataBuf = NULL;
		return -1;
	}

	//printf("CustomCompress uncompress ok size=%lu\n", nUnzipSize);
	
	fp_in = fopen(strFileDst, "wb");
	if (!fp_in){
		printf("CustomCompress open file %s error\n", strFileDst);
		
		free(nDataBuf);
		nDataBuf = NULL;

		return -1;
	}

	nLen = nUnzipSize;
	nPos = 0;
	nCur = 0;
	while (nLen > 0)
	{
		nCur = nLen > BUF_SIZE ? BUF_SIZE : nLen;

		nCur = fwrite(nDataBuf + nPos, sizeof(char), nCur, fp_in);
		if (nCur <= 0)
		{
			fclose(fp_in);

			printf("CustomGenerateFont fwrite file %s error\n", strFileDst);
			free(nDataBuf);
			nDataBuf = NULL;
			return -1;
		}
		
		nLen -= nCur;
		nPos += nCur;
	}

	fclose(fp_in);

	free(nDataBuf);
	nDataBuf = NULL;

	return nUnzipSize;
}

s32 CustomGetFile(s32 nIndex, char* strFileOut)
{
	if (gnInit && strFileOut)
	{
		SCustomInfo sCustomInfo;
		u32 nDataLen = 0;
		s32 nRet;
	
		if (0 == CustomGetInfo(nIndex, &sCustomInfo))
		{
			u8 *nData = malloc(sCustomInfo.nLen);
			if (!nData)
			{
				printf("CustomGetFile malloc(%d) failed\n", sCustomInfo.nLen);
				return -1;
			}
			
			//printf("file len:%u\n", (sCustomInfo.nLen));
			
			switch(sCustomInfo.emType)
			{
				case EM_CUSTOM_LOGO_APP:
					//printf("EM_CUSTOM_LOGO_APP:%s,file len:%u\n",strFileOut,(sCustomInfo.nLen));
					return CustomReadData(nIndex, sCustomInfo.nLen, nData, &nDataLen);
				default:
					if (EM_CUSTOM_DEVICE_GETFONT == sCustomInfo.emType || EM_CUSTOM_PANEL_REMOTE_CTRL == sCustomInfo.emType || (sCustomInfo.emType >= EM_CUSTOM_TRANSLATE_BASE && sCustomInfo.emType <= EM_CUSTOM_TRANSLATE_TOP))
					{
						nRet = CustomReadData(nIndex, sCustomInfo.nLen, nData, &nDataLen);
						//printf("CustomGetFile:CustomReadData nRet = %d, nDataLen = %u\n", nRet, nDataLen);
						if(nRet > 0)
						{
							//printf("CustomGetFile:%s\n", strFileOut);
							nRet = CustomUncompress(strFileOut, nData, nDataLen);
							if(nData)
							{
								free(nData);
								nData = NULL;
							}
							
							return nRet;
						}
					}
					break;
			}
			
			if (nData)
			{
				free(nData);
				nData = NULL;
			}
		}
	}
	
	return -1;
}

s32 CustomGetFile2(EMCUSTOMTYPE emType, char* strFileOut)
{
	u32 nCount = CustomGetCount();
	s32 i;

	for (i = 0; i < nCount; i++)
	{
		SCustomInfo sCustInfo;
		if (0 == CustomGetInfo(i, &sCustInfo) && emType == sCustInfo.emType)
		{
			return CustomGetFile(i, strFileOut);
		}
	}

	return -1;
}

s32 CustomGetLogo(s32 nIndex, u32 nMaxLen, u8* pData, u32* pDataLen) //获得logo文件数据使用
{
	if (gnInit)
	{
		SCustomInfo sCustomInfo;
		u32 nLen = 0;
		char strTmpFile[256];
		struct stat stbuf;
		FILE* fp;
		s32 nRet;

		if (!pData || !pDataLen)
		{
			return -1;
		}
		
		if (0 == CustomGetInfo(nIndex, &sCustomInfo))
		{
			switch (sCustomInfo.emType)
			{
				case EM_CUSTOM_LOGO_APP:
					return CustomReadData(nIndex, nMaxLen, pData, pDataLen);
				case EM_CUSTOM_LOGO_START:
					if (0 == UpgradeFileOut(gStrUpgradeFileName, EM_UPGRADE_FT_OTHER, strTmpFile, sizeof(strTmpFile)))
					{
						if (stat(strTmpFile, &stbuf))
						{
							printf("CustomGetLogo start logo stat file error\n");
							return -1;
						}
						
						if (stbuf.st_size > nMaxLen)
						{
							printf("recieve buf is not enough nMaxLen:%u, need len:%u\n", (unsigned int)nMaxLen, (unsigned int)stbuf.st_size);
							return -1;
						}
						
						fp = fopen(strTmpFile, "rb");
						if (!fp)
						{
							return -1;
						}

						while (nLen < stbuf.st_size && !feof(fp))
						{
							nRet = fread(pData + nLen, sizeof(char), stbuf.st_size - nLen,  fp);
							if (nRet <= 0)
							{
								break;
							}
							nLen += nRet;
						}
						*pDataLen = nLen;
						
						fclose(fp);
						
						return 0;
					}
				default:
					break;
			}
		}
	}
	return -1;
}

s32 CustomGetData(s32 nIndex, SCustomData* psCustomData)
{
	SCustomInfo sCustomInfo;
	u32 nLen = 0;

	if (!psCustomData)
	{
		return -1;
	}
	
	if (0 == CustomGetInfo(nIndex, &sCustomInfo))
	{
		EMCUSTOMTYPE emType = sCustomInfo.emType;
		switch (emType)
		{
			case EM_CUSTOM_DEVICE_MODEL:
			case EM_CUSTOM_DEVICE_IP:
			case EM_CUSTOM_DEVICE_MAC:
			case EM_CUSTOM_DEVICE_GATEWAY:
				memset(psCustomData->strData, 0, sizeof(psCustomData->strData));
				CustomReadData(nIndex, sizeof(psCustomData->strData), psCustomData->strData, &nLen);
				break;
			case EM_CUSTOM_DEVICE_VIDEOSTAND:
				memset(&psCustomData->nVideoStandard, 0, sizeof(u8));
				CustomReadData(nIndex, sizeof(u8), &psCustomData->nVideoStandard, &nLen);			
				break;
			case EM_CUSTOM_DEVICE_LANGUAGE:
				memset(&psCustomData->sLanguageParam, 0, sizeof(SLanguageParam));
				CustomReadData(nIndex, sizeof(SLanguageParam), (void *)&psCustomData->sLanguageParam, &nLen);			
				break;
			case EM_CUSTOM_LOGO_APP:
			case EM_CUSTOM_DEVICE_FONT:
			case EM_CUSTOM_DEVICE_GETFONT:
			default:
				if ((emType >= EM_CUSTOM_VIDENC_MAIN_BASE && emType <= EM_CUSTOM_VIDENC_MAIN_TOP) || (emType >= EM_CUSTOM_VIDENC_SUB_BASE && emType <= EM_CUSTOM_VIDENC_SUB_TOP))
				{
					memset(&psCustomData->sVidEncParam, 0, sizeof(SVidEncParam));
					CustomReadData(nIndex,  sizeof(SVidEncParam), (void *)&psCustomData->sVidEncParam, &nLen);		
					break;
				}
				else if (emType >= EM_CUSTOM_PREVIEW_COLOR_BASE && emType <= EM_CUSTOM_PREVIEW_COLOR_TOP)
				{
					memset(&psCustomData->sVidPreviewColor, 0, sizeof(SVidPreviewColor));
					CustomReadData(nIndex,  sizeof(SVidPreviewColor), (void *)&psCustomData->sVidPreviewColor, &nLen);				
					break;
				}
				return -1;
		}
		return 0;
	}
	
	return -1;
}

s32 CustomGetData2(EMCUSTOMTYPE emType, SCustomData* psCustomData) //该接口不支持EM_CUSTOM_LOGO_APP/EM_CUSTOM_LOGO_START类型数据
{
	u32 nCount = CustomGetCount();
	s32 i;

	for (i = 0; i < nCount; i++)
	{
		SCustomInfo sCustInfo;
		if (0 == CustomGetInfo(i, &sCustInfo) && emType == sCustInfo.emType)
		{
			return CustomGetData(i, psCustomData);
		}
	}

	return -1;
}

s32 CustomReadData(s32 nIndex, u32 nMaxLen, u8* pData, u32 *pDataLen)
{
	FILE* fpTmp = NULL;
	SCustomInfo sCustomInfo;
	
	if (!gnInit ||!pDataLen || !pData)
	{
		return -1;
	}
	printf("%s-gStrCustomFileName: %s\n", __func__, gStrCustomFileName);
	
	fpTmp = fopen(gStrCustomFileName, "rb");
	if (!fpTmp)
	{
		return -1;
	}

	if (0 == CustomGetInfo(nIndex, &sCustomInfo))
	{
		s32 nDataLen = sCustomInfo.nLen, nRet, nReadLen = 0;
		if (nMaxLen < sCustomInfo.nLen)
		{
			return -1;
		}		
	
		fseek(fpTmp, sCustomInfo.nOffSet, SEEK_SET);

		while (nDataLen)
		{
			nRet = fread(pData + nReadLen, sizeof(char), nDataLen, fpTmp);
			if (nRet <= 0)
			{
				break;
			}
			nDataLen -= nRet;
			nReadLen += nRet;
		}
		
		fclose(fpTmp);

		*pDataLen = nReadLen;
		
		return *pDataLen;
	}

	return -1;
}

s32 CopyData(FILE* fp_from, FILE* fp_to, s32 nSize)
{
	u8 buf[BUF_SIZE];
	s32 nRet;

	if (fp_from && fp_to)
	{
		s32 nLen;
		while (nSize > 0)
		{
			nLen = (nSize > BUF_SIZE) ? BUF_SIZE : nSize;
			nSize -= nLen;
			nRet = fread(buf, sizeof(char), nLen, fp_from);
			if (nRet != nLen)
			{
				printf("CopyData read file error\n");
				return -1;
			}
			nRet = fwrite(buf, sizeof(char), nLen, fp_to);
			if (nRet != nLen)
			{
				printf("CopyData write file error\n");
				return -1;
			}
		}

		return 0;
	}

	return -1;
}

s32 CopyFileTool(char* strFileFrom, char* strFileTo)
{
	FILE* fpFrom;
	FILE* fpTo;
	struct stat stbuf;
	s32 nRet;

	if (!strFileFrom || !strFileTo)
	{
		return -1;
	}
	
	if (stat(strFileFrom, &stbuf))
	{
		printf("AddLogoCustom stat file error\n");
		return -1;
	}

	fpFrom = fopen(strFileFrom, "rb");
	if (!fpFrom){
		printf("CopyFileTool open file %s error\n", strFileFrom);
		return -2;
	}

	fpTo = fopen(strFileTo, "wb");
	if (!fpTo){
		printf("CopyFileTool open file %s error\n", strFileTo);
		fclose(fpFrom);
		return -2;
	}

	nRet = CopyData(fpFrom, fpTo, stbuf.st_size);

	fclose(fpFrom);
	fclose(fpTo);

	return nRet;
}

s32 CustomCompress(char* strFileSrc, char* strFileDst)
{
	struct stat stbuf;

	if (stat(strFileSrc, &stbuf))
	{
		printf("CustomCompress stat file error\n");
		return -1;
	}
	else
	{
		
		s32 nRet, nLen, nCur = 0, nPos = 0, nHeadLen = 4;
		u8* nBufZip = malloc(stbuf.st_size + nHeadLen);
		u8* nDataBuf = malloc(stbuf.st_size);
		unsigned long nZipLen = 0;
		FILE* fp_in = NULL;
		u32* pnLen = (u32 *)nBufZip;

		if(!nBufZip)
		{
			return -1;
		}
		if(!nDataBuf)
		{
			return -1;
		}

		//printf("CustomCompress 2\n");

		fp_in = fopen(strFileSrc, "rb");
		if (!fp_in){
			printf("CustomCompress open file %s error\n", strFileSrc);
			if (nBufZip)
			{
				free(nBufZip);
				nBufZip = NULL;
			}

			if (nDataBuf)
			{
				free(nDataBuf);
				nDataBuf = NULL;
			}

			return -1;
		}
		nLen = stbuf.st_size;

		while (nLen > 0)
		{
			nCur = nLen > BUF_SIZE ? BUF_SIZE : nLen;

			nCur = fread(nDataBuf + nPos, sizeof(char), nCur, fp_in);
			if (nCur <= 0)
			{
				fclose(fp_in);
				printf("CustomCompress fread file %s error\n", strFileSrc);
				if (nBufZip)
				{
					free(nBufZip);
					nBufZip = NULL;
				}
				
				if (nDataBuf)
				{
					free(nDataBuf);
					nDataBuf = NULL;
				}
				return -1;
			}
			
			nLen -= nCur;
			nPos += nCur;
		}
		fclose(fp_in);

		//remove(strFileSrc);

		//printf("CustomCompress before compress\n");

		nZipLen = stbuf.st_size;

		nRet = compress((Bytef *)(nBufZip + nHeadLen), (unsigned long*)&nZipLen, (Bytef *)nDataBuf, (unsigned long)stbuf.st_size);
		if (nRet != Z_OK)
		{
			printf("compress Error! nRet=%d\n", nRet);
			if (nBufZip)
			{
				free(nBufZip);
				nBufZip = NULL;
			}
			
			if (nDataBuf)
			{
				free(nDataBuf);
				nDataBuf = NULL;
			}
			return -1;
		}

		//printf("CustomCompress compress ok\n");
		
		fp_in = fopen(strFileDst, "wb");
		if (!fp_in){
			printf("CustomCompress open file %s error\n", strFileDst);
			if (nBufZip)
			{
				free(nBufZip);
				nBufZip = NULL;
			}
			
			if (nDataBuf)
			{
				free(nDataBuf);
				nDataBuf = NULL;
			}
			return -1;
		}

		*pnLen = stbuf.st_size;

		nLen = nZipLen + nHeadLen;
		nPos = 0;
		nCur = 0;
		while (nLen > 0)
		{
			nCur = nLen > BUF_SIZE ? BUF_SIZE : nLen;

			nCur = fwrite(nBufZip + nPos, sizeof(char), nCur, fp_in);
			if (nCur <= 0)
			{
				fclose(fp_in);
				
				printf("CustomGenerateFont fwrite file %s error\n", strFileDst);
				if (nBufZip)
				{
					free(nBufZip);
					nBufZip = NULL;
				}
				
				if (nDataBuf)
				{
					free(nDataBuf);
					nDataBuf = NULL;
				}
				return -1;
			}
			
			nLen -= nCur;
			nPos += nCur;
		}

		fclose(fp_in);

		if (nBufZip)
		{
			free(nBufZip);
			nBufZip = NULL;
		}
		
		if (nDataBuf)
		{
			free(nDataBuf);
			nDataBuf = NULL;
		}
	}

	return 0;
}

int AddCustom(FILE* fp_out, s32 nIndex, s32 nDataOffset, SCustomData* psCustomData, EMCUSTOMTYPE emType)
{
	SCustomInfo sCustemItem;
	FILE* fp_in = NULL;
	struct stat stbuf;
	u8 buf[BUF_SIZE];
	s32 nLen = 0, nRet = 0;
	char* strCustomFile = NULL;
	char strMyFile[256];
	
	if (!fp_out)
	{
		return -1;
	}
	
	memset(buf, 0, BUF_SIZE);
	
	switch (emType)
	{
		case EM_CUSTOM_LOGO_APP:
			strCustomFile = psCustomData->strFilePath;
			if (!strCustomFile)
			{
				return -1;
			}
			break;
		case EM_CUSTOM_DEVICE_FONT:
			strCustomFile = psCustomData->strFilePath;
			if (!strCustomFile)
			{
				return -1;
			}			
			break;;
		case EM_CUSTOM_DEVICE_MODEL:
		case EM_CUSTOM_DEVICE_IP:
		case EM_CUSTOM_DEVICE_MAC:
		case EM_CUSTOM_DEVICE_GATEWAY:
			nLen = strlen((char *)psCustomData->strData) + 1;
			memcpy(buf, psCustomData->strData, nLen);
			break;
		case EM_CUSTOM_DEVICE_VIDEOSTAND:
			nLen = sizeof(u8);
			memcpy(buf, &psCustomData->nVideoStandard, nLen);			
			break;
		case EM_CUSTOM_DEVICE_LANGUAGE:
			nLen = sizeof(SLanguageParam);
			memcpy(buf, &psCustomData->sLanguageParam, nLen);			
			break;
		default:
			if ((emType >= EM_CUSTOM_VIDENC_MAIN_BASE && emType <= EM_CUSTOM_VIDENC_MAIN_TOP) || (emType >= EM_CUSTOM_VIDENC_SUB_BASE && emType <= EM_CUSTOM_VIDENC_SUB_TOP))
			{
				nLen = sizeof(SVidEncParam);
				memcpy(buf, &psCustomData->sVidEncParam, nLen);			
				break;
			}
			else if (emType >= EM_CUSTOM_PREVIEW_COLOR_BASE && emType <= EM_CUSTOM_PREVIEW_COLOR_TOP)
			{
				nLen = sizeof(SVidPreviewColor);
				memcpy(buf, &psCustomData->sVidPreviewColor, nLen);				
				break;
			} 
			else if (emType >= EM_CUSTOM_TRANSLATE_BASE && emType <= EM_CUSTOM_TRANSLATE_TOP)
			{
				sprintf(strMyFile, "LangTrans%d.zip", emType);
				strCustomFile = strMyFile;
				if (0 == CustomCompress(psCustomData->strFilePath, strMyFile))
				{
					break;
				}
			}
			else if (EM_CUSTOM_PANEL_REMOTE_CTRL == emType)
			{
				sprintf(strMyFile, "panelremote%d.zip", emType);
				strCustomFile = strMyFile;
				if (0 == CustomCompress(psCustomData->strFilePath, strMyFile))
				{
					break;
				}
			}
			
			return -1;
	}

	memset(&sCustemItem, 0, sizeof(SCustomInfo));
	if (strCustomFile)
	{
		if (stat(strCustomFile, &stbuf))
		{
			printf("AddCustom stat file error\n");
			return -1;
		}
		sCustemItem.nLen = stbuf.st_size;
	}
	else
	{
		sCustemItem.nLen = nLen;
	}
	

	sCustemItem.emType = emType;
	sCustemItem.nOffSet = nDataOffset;
	fseek(fp_out, sizeof(SCustomContainer) + sizeof(SCustomInfo) * nIndex, SEEK_SET);
	nRet = fwrite(&sCustemItem, sizeof(char), sizeof(SCustomInfo), fp_out);
	if (nRet != sizeof(SCustomInfo))
	{
		printf("AddCustom write file error 1\n");
		nRet = -1;
		goto AddCustomErr;
	}

	fseek(fp_out, nDataOffset, SEEK_SET);
	if (strCustomFile)
	{
		fp_in = fopen(strCustomFile, "rb");
		if (!fp_in){
			printf("AddCustom open file %s error\n", strCustomFile);
			nRet = -2;
			goto AddCustomErr;
		}

		while(!feof(fp_in))
		{
			nLen = fread(buf, sizeof(char), BUF_SIZE, fp_in);
			if (nLen == 0)
			{
				break;
			}
			else if (nLen < 0)
			{
				printf("AddCustom read file error\n");
				nRet = -3;
				goto AddCustomErr;
			}
			nRet = fwrite(buf, sizeof(char), nLen, fp_out);
			if (nRet != nLen)
			{
				printf("AddCustom write file error\n");
				nRet = -4;
				goto AddCustomErr;
			}
		}
		fclose(fp_in);
	}
	else
	{
		nRet = fwrite(buf, sizeof(char), nLen, fp_out);
		if (nRet != nLen)
		{
			printf("AddCustom write file error\n");
			nRet = -5;
			goto AddCustomErr;
		}
	}

	if (strCustomFile == strMyFile)
	{
		remove(strCustomFile);
	}
	
	return 0;

AddCustomErr:

	if (strCustomFile == strMyFile)
	{
		remove(strCustomFile);
	}

	return nRet;
}

#ifndef FLASHDEV
#define TOTAL_CHAR_NUM 65536
#define EACH_LANG_ZONE_NUM 8
//字体名称
#define FONT_STYLE_SONG "Song"

typedef struct
{
	u16 nStartCode;
	u16 nEndCode;
} SUCZone;

static SUCZone LangZone[LANGUAGE_ID_NUM][TOTAL_CHAR_NUM] = {
	//0					1				2				3				4				5				6					7
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 0 LANGUAGE_ID_ENG
	{{0x2FFE, 0x319f}, {0x3400, 0x4DBF}, {0x4E00, 0x9FBF}, {0xF900, 0xFAFF}, {0xFF00, 0xFFEF}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 1 LANGUAGE_ID_SCH
	{{0x2FFE, 0x319f}, {0x3400, 0x4DBF}, {0x4E00, 0x9FBF}, {0xF900, 0xFAFF}, {0xFF00, 0xFFEF}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 2 LANGUAGE_ID_TCH
	{{0x2FFE, 0x319f}, {0x3400, 0x4DBF}, {0x4E00, 0x9FBF}, {0xF900, 0xFAFF}, {0xAC00, 0xD7AF}, {0x1100, 0x11FF}, {0x3130, 0x318F}, {0xFF00, 0xFFEF}}, // 3 LANGUAGE_ID_HAN
	{{0x2FFE, 0x30FF}, {0x3190, 0x319f}, {0x31F0, 0x31FF}, {0x3400, 0x4DBF}, {0x4E00, 0x9FBF}, {0xF900, 0xFAFF}, {0xFF00, 0xFFFF}, {0x2100, 0x214F}}, // 4 LANGUAGE_ID_JAP
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 5 LANGUAGE_ID_GER
	{{0x0400, 0x04FF}, {0x2100, 0x214f}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 6 LANGUAGE_ID_RUS
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 7 LANGUAGE_ID_FRE
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 8 LANGUAGE_ID_POR
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 9 LANGUAGE_ID_TUR
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 10 LANGUAGE_ID_SPA
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 11 LANGUAGE_ID_ITA
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 12 LANGUAGE_ID_POL
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 13 LANGUAGE_ID_POS
	{{0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}}, // 14 LANGUAGE_ID_THAI
};

static SUCZone LangZoneCommon = {0x0000, 0x007f};
static SUCZone LangZoneCommon2 = {0x0081, 0x02ff};

#define CHARACTER_SPACE_COMMON 2

static u8 nCharacterSpace[LANGUAGE_ID_NUM] = {
	CHARACTER_SPACE_COMMON, // 0 英语
	(CHARACTER_SPACE_COMMON + 1), // 1 简中
	(CHARACTER_SPACE_COMMON + 1), // 2 繁中
	(CHARACTER_SPACE_COMMON + 1), // 3 韩语
	(CHARACTER_SPACE_COMMON + 1), // 4 日语
	CHARACTER_SPACE_COMMON, // 5 德语
	CHARACTER_SPACE_COMMON, // 6 俄语
	CHARACTER_SPACE_COMMON, // 7 法语
	CHARACTER_SPACE_COMMON, // 8 葡萄牙语
	CHARACTER_SPACE_COMMON, // 9 土耳其语
	CHARACTER_SPACE_COMMON, // 10 西班牙语
	CHARACTER_SPACE_COMMON, // 11 意大利语
	CHARACTER_SPACE_COMMON, // 12 波兰语
	CHARACTER_SPACE_COMMON, // 13 波斯语
	CHARACTER_SPACE_COMMON, // 14 泰语
};

s32 SetFontSelect(SUCZone *psUcZone, u8 *pnSelect)
{
	if (psUcZone && pnSelect)
	{
		if (psUcZone->nEndCode > 0 && psUcZone->nEndCode >= psUcZone->nStartCode)
		{
			memset(&pnSelect[psUcZone->nStartCode], 1, psUcZone->nEndCode - psUcZone->nStartCode + 1);
			return 0;
		}
	}

	return -1;
}

void GetFontSelect(u8 *pnSelect)
{
	SLanguageTool* psLangTool;
	SCustomData sCustData;
	u32 nLangId, i, j;

	memset(pnSelect, 0, TOTAL_CHAR_NUM);

	if (CustomGetData2(EM_CUSTOM_DEVICE_LANGUAGE, &sCustData))
	{
		return;
	}
	else
	{
		psLangTool = (SLanguageTool *)&sCustData.sLanguageParam;
		if (psLangTool->nVersion != LANGUAGETOOL_VERSION)
		{
			return;
		}
	}
	
	SetFontSelect(&LangZoneCommon, pnSelect);
	SetFontSelect(&LangZoneCommon2, pnSelect);
	
	for (i = 0; i < psLangTool->nLanguageIdNum && i < LANGUAGE_SET_NUM_MAX; i++)
	{
		nLangId = psLangTool->nLanguageIdList[i];
		if (LANGUAGE_ID_NUM > nLangId)
		{
			for (j = 0; j < EACH_LANG_ZONE_NUM; j++)
			{
				if (SetFontSelect(&LangZone[nLangId][j], pnSelect))
				{
					break;
				}
				else
				{
					printf("GetFontSelect LangZone %d ok\n", nLangId);
				}
			}
		}
		
	}
}

s32 GetBlockCount(u8 *pnSelect, s32* pnBlkCount, s32* pnCharCount)
{
	s32  i, nLastSelect = 0;

	if (pnCharCount && pnBlkCount)
	{
		*pnCharCount = 0;
		*pnBlkCount = 0;
		
		for (i = 0; i < TOTAL_CHAR_NUM; i++)
		{
			if (nLastSelect != pnSelect[i])
			{
				if (0 == nLastSelect)
				{
					*pnBlkCount += 1;
				}

				nLastSelect = pnSelect[i];
			}

			if (pnSelect[i])
			{
				*pnCharCount += 1;
			}

		}

		if (0 == *pnBlkCount)
		{
			return -1;
		}

		return 0;
	}

	return -1;
}

s32 GetNextZone(SUCZone *psUcZone, u8 *pnSelect, u16 *pnStart)
{
	if (pnStart && psUcZone)
	{
		s32 nCount = 0, i = *pnStart, nLastSelect = 0;

		for (; i < TOTAL_CHAR_NUM; i++)
		{
			if (nLastSelect != pnSelect[i])
			{
				if (0 == nLastSelect)
				{
					nCount++;
					psUcZone->nStartCode = i;
				}
				else
				{
					psUcZone->nEndCode = i - 1;
					*pnStart = i;
					return 0;
				}

				nLastSelect = pnSelect[i];
			}
		}

		if (nCount)
		{
			psUcZone->nEndCode = i -1;
			*pnStart = i;
			return 0;
		}
	}
	
	return -1;
}

//Custom for languages and fonts---------------------------------
#define UCS_FONT_MAGIC "tongli ucs font"

#define MAX_FONT_BLOCK_NUM 20

typedef struct
{
	s8  magic[16];	//标志
	u32 size;		//字库总大小
	u32 blocks;	//块数
} UCS_FONT_HEADER;

typedef struct
{
	u16 start;		//编码起始值
	u16 end;		//编码结束值
	u16 width;		//点阵宽度
	u16 height;	//点阵高度
	u32 roffs;	//字体点阵数据偏移
	u32 xoffs;	//字体扩展数据偏移
} UCS_FONT_BLOCK;

u16 GetCharacterSpace(u16 nCode)
{
	int i, j;

	if (nCode >= LangZoneCommon.nStartCode && nCode <= LangZoneCommon.nEndCode)
	{
		return CHARACTER_SPACE_COMMON;
	}

	if (nCode >= LangZoneCommon2.nStartCode && nCode <= LangZoneCommon2.nEndCode)
	{
		return CHARACTER_SPACE_COMMON;
	}
	
	
	for (i = 0; i < LANGUAGE_ID_NUM; i++)
	{
		for (j = 0; j < TOTAL_CHAR_NUM; j++)
		{
			if ( LangZone[i][j].nStartCode == LangZone[i][j].nEndCode)
			{
				break;
			}
			
			if (nCode >= LangZone[i][j].nStartCode && nCode <= LangZone[i][j].nEndCode)
			{
				return nCharacterSpace[i];
			}
		}
	}
	
	return 0;
}

int GetRasterWidth(FILE* fdot, u8* pRasterWidth, int len, u16 start, u16 width, u16 height, u16 nSpace)
{
	int nRowSize = ((width + 7) >> 3), nFontBufSize = nRowSize * height;
	int i = 0, j = 0, k = 0, l =0;
	u8 raster[128], nRasterWidth = 0, nMaxRasterWidth = 0;
	u16 nRealSpace = nSpace;

	memset(pRasterWidth, 0, len);

	while (i < len && !feof(fdot))
	{
		if (nFontBufSize == fread(raster, 1, nFontBufSize, fdot))
		{
			nRasterWidth = 0;
			for (j = 0; j < height; j++)
			{
				for (l = 0; l < nRowSize; l++)
				{
					for (k = 0; k < 8; k++)
					{
						if (*(raster + nRowSize * j + l) & (0x80>>k))
						{
							nRasterWidth = (nRasterWidth < (l * 8 + k)) ?  (l * 8 + k) : nRasterWidth;
						}
					}
				}
			}

			nMaxRasterWidth = nMaxRasterWidth < nRasterWidth ? nRasterWidth : nMaxRasterWidth;
			if (0x20 == start + i)
			{
				pRasterWidth[i] = nMaxRasterWidth + nSpace;
			}
			else
			{
				nRealSpace = GetCharacterSpace(start + i);
				if (0 == nRealSpace)
				{
					nRealSpace = nSpace;
				}
				pRasterWidth[i] = nRasterWidth + ((nRasterWidth > 0) ? nRealSpace : 0);
			}
			//printf("Unicode:%04x nRasterWidth:%d\n", start + i, pRasterWidth[i]);
		}
		else
		{
			return -1;
		}
		
		i++;
	}

	return 0;
}

extern s32 GetNextZone(SUCZone *psUcZone, u8 *pnSelect, u16 *pnStart);
extern s32 GetBlockCount(u8 *pnSelect, s32* pnBlkCount, s32* pnCharCount);

int CustomFont(u8* strDzkFile, u8* strFontFile, u8 *pnSelect, u8 nW, u8 nH)
{
	u8 dotBuf[256];
	s32 nBlkCnt = 0, nRet, nCharCount = 0, nTmp, nOft;
	u32 len, wlen;
	UCS_FONT_HEADER ucsHead;
	FILE* fdot = NULL;
	FILE *ftl = NULL;
	u16 nCur = 0;
	s32 nRaster = ((nW + 7) / 8) * nH;
	
	if (!strDzkFile || !strFontFile || !pnSelect || nRaster < 0)
	{
		return -1;
	}
	
	nRet = GetBlockCount(pnSelect, &nBlkCnt, &nCharCount);
	if (nRet < 0)
	{
		return -1;
	}

	//打开点阵字库文件
	
	fdot = fopen(strDzkFile, "rb");
	if (NULL == fdot)
	{
		printf("Open dotfont file %s failed\n", strDzkFile);
		return -1;
	}

	//创建通立字体文件
	ftl = fopen(strFontFile, "wb");
	if (NULL == ftl)
	{
		fclose(fdot);
		printf("Open tongli font file %s failed\n", strFontFile);
		return -1;
	}
	
		
	//写UCS_FONT_HEADER
	ucsHead.size = sizeof(UCS_FONT_HEADER) + sizeof(UCS_FONT_BLOCK) * nBlkCnt
		+ nCharCount * nRaster +  nCharCount;
	ucsHead.blocks = nBlkCnt;
	strcpy(ucsHead.magic, UCS_FONT_MAGIC);
	
	if (sizeof(UCS_FONT_HEADER) != fwrite(&ucsHead, 1, sizeof(UCS_FONT_HEADER), ftl))
	{
		printf("write UCS_FONT_HEADER failed\n");
		fclose(fdot);
		fclose(ftl);
		return -1;
	}
	//printf("CustomFont****************1\n");

	nCur = 0;
	nTmp = 0;
	nOft = sizeof(UCS_FONT_HEADER) + sizeof(UCS_FONT_BLOCK) * nBlkCnt;
	while (nTmp < nBlkCnt)
	{
		SUCZone sZone;
		UCS_FONT_BLOCK ucsBlock;//写UCS_FONT_BLOCK
		u16 nZoneCharNum = 0;

		if (0 == GetNextZone(&sZone, pnSelect, &nCur))
		{
			nZoneCharNum = sZone.nEndCode - sZone.nStartCode + 1;
			
			ucsBlock.start = sZone.nStartCode;
			ucsBlock.end = sZone.nEndCode + 1;
			ucsBlock.width = nW;
			ucsBlock.height = nH;
			ucsBlock.roffs = nOft;
			ucsBlock.xoffs = nOft + nZoneCharNum * nRaster;
			
			nOft = ucsBlock.xoffs + nZoneCharNum;

			if (sizeof(UCS_FONT_BLOCK) != fwrite(&ucsBlock, 1, sizeof(UCS_FONT_BLOCK), ftl))
			{
				printf("write UCS_FONT_BLOCK failed\n");
				fclose(fdot);
				fclose(ftl);
				return -1;
			}
		}
		else
		{
			printf("GetZone Failed\n");
			fclose(fdot);
			fclose(ftl);
			return -1;
		}

		nTmp++;
	}

	//printf("CustomFont****************2\n");
	//读点整字库文件并写点阵字库
	nCur = 0;
	nTmp = 0;
	while (nTmp < nBlkCnt)
	{
		SUCZone sZone;
		u16 nZoneCharNum = 0;
		//printf("CustomFont****************3.1\n");
		if (0 == GetNextZone(&sZone, pnSelect, &nCur))
		{
			s32 nReal, nOk = 0;

			nZoneCharNum = sZone.nEndCode - sZone.nStartCode + 1;
			len = nZoneCharNum * nRaster;
			
			fseek(fdot, sZone.nStartCode * nRaster, SEEK_SET);
			//printf("CustomFont****************3.2\n");
			while (len > 0)
			{
				wlen = (len >= sizeof(dotBuf)) ? sizeof(dotBuf) : len;
				nReal = fread(dotBuf, 1, wlen, fdot);
				if (nReal > 0)
				{
					nOk += nReal;
					if (nReal != wlen)
					{
						printf("nReal != wlen nReal=%d wlen=%u\n", nReal, wlen);
						fseek(fdot, nOk, SEEK_SET);
					}

					wlen = nReal;
					nReal = fwrite(dotBuf, 1, wlen, ftl);
					if (wlen != nReal)
					{
						printf("write failed for dot file nReal=%d wlen=%u\n", nReal, wlen);
						fclose(fdot);
						fclose(ftl);
						return -1;
					}

					len -= wlen;
				}
				else
				{
					printf("read err fdot wlen=%d len=%d\n", wlen, len);
					fclose(fdot);
					fclose(ftl);
					return -1;
				}
			}
			//printf("CustomFont****************3.3\n");
			if (nZoneCharNum > 0)
			{
				//printf("CustomFont****************3.5\n");
				u8* raster = malloc(nZoneCharNum);
				if (!raster)
				{
					return -1;
				}
				//printf("CustomFont****************3.6\n");
				
				fseek(fdot, sZone.nStartCode * nRaster, SEEK_SET);

				if (0 != GetRasterWidth(fdot, raster, nZoneCharNum, sZone.nStartCode, nW, nH, CHARACTER_SPACE_COMMON))
				{
					printf("GetRasterWidth failed\n");
					fclose(fdot);
					fclose(ftl);
					if (raster)
					{
						free(raster);
						raster = NULL;
					}
					return -1;
				}
				//printf("CustomFont****************3.7\n");

				wlen = nZoneCharNum;
				if  (nZoneCharNum > 0)
				{
					//printf("CustomFont****************3.7.1\n");
					if (wlen != fwrite(raster, 1, nZoneCharNum, ftl))
					{
						printf("write failed for extw\n");
						fclose(fdot);
						fclose(ftl);
						if (raster)
						{
							free(raster);
							raster = NULL;
						}
						return -1;
					}
					//printf("CustomFont****************3.7.2\n");
				}
				//printf("CustomFont****************3.8\n");
				
				if (raster)
				{
					free(raster);
					raster = NULL;
				}	
			}
			//printf("CustomFont****************3.4\n");
		}
		else
		{
			printf("GetZone Failed\n");
			
			fclose(fdot);
			fclose(ftl);

			return -1;
		}

		nTmp++;
	}
	

	//关闭文?
	fclose(fdot);
	fclose(ftl);

	return 0;
}

s32 CustomGenerateFont(SLanguageParam * psLangSet, u8 nW, u8 nH, u8 nSize, u8 *strFontStyle)
{
	if (strFontStyle && psLangSet)
	{
		int i, j, nLangId;

		u8 nCharSelect[TOTAL_CHAR_NUM];
		u8 nCharSelectLast[TOTAL_CHAR_NUM];

		memset(nCharSelect, 0, sizeof(nCharSelect));
		memset(nCharSelectLast, 0, sizeof(nCharSelect));
		
		SetFontSelect(&LangZoneCommon, nCharSelect);
		SetFontSelect(&LangZoneCommon2, nCharSelect);

		for (i = 0; i < psLangSet->nLanguageIdNum && i < LANGUAGE_SET_NUM_MAX; i++)
		{
			nLangId = psLangSet->nLanguageIdList[i];
			if (LANGUAGE_ID_NUM > nLangId)
			{
				for (j = 0; j < EACH_LANG_ZONE_NUM; j++)
				{
					if (SetFontSelect(&LangZone[nLangId][j], nCharSelect))
					{
						break;
					}
					else
					{
						//printf("SetFontSelect LangZone %d ok\n", nLangId);
					}
				}
			}
		}

		for (i = 0; i < LANGUAGE_ID_NUM; i++)
		{
			for (j = 0; j < psLangSet->nLanguageIdNum && j < LANGUAGE_SET_NUM_MAX; j++)
			{
				nLangId = psLangSet->nLanguageIdList[j];
				if (nLangId == i)
				{
					break;
				}
			}

			if ((j >= psLangSet->nLanguageIdNum) || (j >= LANGUAGE_SET_NUM_MAX))
			{
				CustomCancel(EM_CUSTOM_TRANSLATE_BASE + i);
			}
		}

		GetFontSelect(nCharSelectLast);

		if(1)// (memcmp(nCharSelectLast, nCharSelect, TOTAL_CHAR_NUM))
		{
			u8 strFontDzk[256], strFontBin[256], strFontZip[256];
			//重新生成字库
			sprintf(strFontDzk, "FONT%dX%d_%d_%s.DZK", nW, nH, nSize, strFontStyle);
			sprintf(strFontBin, "FONT%dX%d_%d_%s.bin", nW, nH, nSize, strFontStyle);
			sprintf(strFontZip, "FONT%dX%d_%d_%s.zip", nW, nH, nSize, strFontStyle);
			
			printf("Language Select Changed\n");
			
			if (0 == CustomFont(strFontDzk, strFontBin, nCharSelect, nW, nH))
			{
				//printf("after CustomFont\n");
				SCustomData sCustData;
				s32 nRet = CustomCompress(strFontBin, strFontZip);
				//printf("after CustomCompress nRet=%d\n", nRet);				
				if (0 == nRet)
				{
					strcpy(sCustData.strFilePath, strFontZip);
					
					nRet = CustomDo(EM_CUSTOM_DEVICE_FONT, &sCustData);

					remove(strFontZip);

					remove(strFontBin);

					return nRet;
				}

				remove(strFontBin);
			}
			
			return -1;
		}

		return 0;
	}

	return -1;
}

s32 CustomSetFont(u8 nW, u8 nH, u8 nSize, u8 *strFontStyle)
{
	return 0;
}

s32 CustomLanguageSet(SLanguageParam * psLangSet)
{

	if (gnInit && psLangSet)
	{
		SLanguageTool* psLangTool = (SLanguageTool *)psLangSet;

		psLangTool->nVersion = LANGUAGETOOL_VERSION;
		
		return CustomGenerateFont(psLangSet, 20, 20, 12, FONT_STYLE_SONG);
		//CustomSetFont(u8 nW, u8 nH, u8 nSize, u16 nCharNumPerSegment);
	}

	return -1;
}
#endif

