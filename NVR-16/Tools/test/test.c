#include "common_basetypes.h"
#include "string.h"
#include "ToolCustom.h"
#include "ToolUpgrade.h"

int main(int argc, char **argv)
{
	#ifdef PACK
		return UpgradeFileGenerate(argc, argv);
	#elif defined(FLASH)
		UpgradeConvertToFlashFile("mainboard2nd.bin");
	#elif defined(UNPACK)
		UpgradeFileDestruct("mainboard2nd.bin");
	#elif defined(CUSTOM)
	if ( 0 == CustomOpen("mainboard2nd.bin"))
	{
		#if 0
		#if 1
		CustomCancel(EM_CUSTOM_ALL);
		#else
		CustomCancel(EM_CUSTOM_LOGO_START);
		CustomCancel(EM_CUSTOM_LOGO_APP);
		CustomCancel(EM_CUSTOM_DEVICE_MODEL);
		CustomCancel(EM_CUSTOM_DEVICE_IP);
		CustomCancel(EM_CUSTOM_DEVICE_MAC);
		#endif
		#else
		SCustomData sCustomData;
		
		printf("_______________0\n");
		memset(&sCustomData, 0, sizeof(sCustomData));
		strcpy(sCustomData.strFilePath, "logo.jpg");
		CustomGenerate(EM_CUSTOM_LOGO_START, &sCustomData);

		printf("_______________1\n");
		memset(&sCustomData, 0, sizeof(sCustomData));
		strcpy(sCustomData.strFilePath, "logo.jpg");
		CustomGenerate(EM_CUSTOM_LOGO_APP, &sCustomData);

		printf("_______________2\n");
		strcpy(sCustomData.strData, "TL-R9624T");
		CustomGenerate(EM_CUSTOM_DEVICE_MODEL, &sCustomData);

		printf("_______________3\n");
		strcpy(sCustomData.strData, "192.168.20.240");
		CustomGenerate(EM_CUSTOM_DEVICE_IP, &sCustomData);

		printf("_______________4\n");
		strcpy(sCustomData.strData, "00:25:49:88:9a:66");
		CustomGenerate(EM_CUSTOM_DEVICE_MAC, &sCustomData);

		sCustomData.sLanguageParam.nLanguageIdDefault = LANGUAGE_ID_ENG;
		sCustomData.sLanguageParam.nLanguageIdNum = 2;
		sCustomData.sLanguageParam.nLanguageIdList[0] = LANGUAGE_ID_ENG;
		sCustomData.sLanguageParam.nLanguageIdList[1] = LANGUAGE_ID_SCH;
		#if 0
		sCustomData.sLanguageParam.nLanguageIdList[2] = LANGUAGE_ID_TCH;
		sCustomData.sLanguageParam.nLanguageIdList[3] = LANGUAGE_ID_HAN;
		sCustomData.sLanguageParam.nLanguageIdList[4] = LANGUAGE_ID_JAP;
		sCustomData.sLanguageParam.nLanguageIdList[5] = LANGUAGE_ID_RUS;
		sCustomData.sLanguageParam.nLanguageIdList[6] = LANGUAGE_ID_ENG;
		sCustomData.sLanguageParam.nLanguageIdList[7] = LANGUAGE_ID_FRE;
		sCustomData.sLanguageParam.nLanguageIdList[8] = LANGUAGE_ID_POR;
		sCustomData.sLanguageParam.nLanguageIdList[9] = LANGUAGE_ID_TUR;
		sCustomData.sLanguageParam.nLanguageIdList[10] = LANGUAGE_ID_SPA;
		sCustomData.sLanguageParam.nLanguageIdList[11] = LANGUAGE_ID_ITA;
		#endif

		CustomGenerate(EM_CUSTOM_DEVICE_LANGUAGE, &sCustomData);

		strcpy(sCustomData.strFilePath, "StringsEn.txt");
		CustomGenerate(EM_CUSTOM_TRANSLATE_ENG, &sCustomData);

		strcpy(sCustomData.strFilePath, "StringsCn.txt");
		CustomGenerate(EM_CUSTOM_TRANSLATE_SCH, &sCustomData);
			
		CustomSave();
		#endif
	
		CustomClose(1);
	}
	#elif defined(FLASHDEV)
		int count, i;
		SCustomData sCustomData;
		SCustomInfo sCustInfo;
		#if 0
		UpgradeToFlash("mainboard2nd.bin");
		#else
		UpgradeStartLogo("startlogo_new.jpg", NULL, "/dev/mtd/1", 0, 0x20000);
		
		UpgradeAppLogo("applogo_new.jpg", NULL, "/dev/mtd/5", 0, 0x100000);
		
		CustomOpen(NULL);
		count = CustomGetCount();

		for (i = 0; i < count; i++)
		{
			if (0 == CustomGetInfo(i, &sCustInfo))
			{
				printf("CustomGetInfo : type(%d) nLen(%d) nOffSet(%d)\n", sCustInfo.emType, sCustInfo.nLen, sCustInfo.nOffSet);
			}
			
			if (0 == CustomGetData(i, &sCustomData))
			{
				printf("CustomGetData : %s\n", sCustomData.strData);
			}
		}
		
		printf("count=%d\n", count);
		#endif
	#endif

	return 0;
}

