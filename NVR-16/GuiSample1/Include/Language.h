#ifndef __LANGUAGE_API_H__
#define __LANGUAGE_API_H__

#ifdef __cplusplus
extern "C" {
#endif
#define NOTSET					-1
#define ENGLISH					0			//英语							==
#define CHINESE_S				1			//简体中文					==
#define CHINESE_T				2			//繁体中文					==
#define KOREAN					3		    //韩语
#define JAPANESE				4			//日语							==
#define GERMAN					5			//德语							==
#define RUSSIAN					6			//俄语							==
#define FRENCH                  7			//法语                          			==
#define PORTUGAL				8			//葡萄牙文
#define TURKEY					9			//土耳其文
#define SPANISH                 10          //西班牙语					==
#define ITALIAN					11			//意大利语					==
#define POL						12			//波兰
#define POS						13			//波斯
#define THAI					14			//泰语

//csp modify 20121224
#define MAGYAR					15			//匈牙利语
#define SLOVAKIA				16			//斯洛伐克语
#define VIETNAM					17			//越南语
#define GREECE					18			//希腊语
//lcy add
#define HEBREW                  19          //希伯来语

//csp modify 20121224
//#define LANG_NR				15			//支持语言总数
//#define LANG_NR				16			//支持语言总数
//#define LANG_NR				17			//支持语言总数
//#define LANG_NR				18			//支持语言总数
//#define LANG_NR				19			//支持语言总数
#define LANG_NR					20			//支持语言总数

#ifdef __cplusplus
}
#endif

#endif  //__LANGUAGE_API_H__

