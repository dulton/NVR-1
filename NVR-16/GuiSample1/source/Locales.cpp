#include "System/Locales.h"
// #include "System/AppConfig.h"
// #include "Configs/ConfigGeneral.h"
// #include "Configs/ConfigLocation.h"
#include "math.h"
#include "GUI/Pages/BizData.h"

#if 0
//modified log by wangqin
//20070413 增加对葡萄牙文的支持 StringsPt.txt
//20070515 增加对土耳其文的支持 StringsTr.txt
static char* strings_file[LANG_NR] = {
	"StringsChinese.txt", "StringsEnglish.txt", "StringsTChinese.txt", "StringsKorea.txt",
		"StringsJapanese.txt", "StringsGerman.txt", "StringsRussian.txt", "StringsFrance.txt",
		"StringsPortuguese.txt","StringsTurkey.txt","StringsSpanish.txt",
		"StringsItalian.txt","StringsPolish.txt","StringsPersian.txt","StringsThai.txt",
};
#else
//csp modify 20121224
static char* strings_file[LANG_NR] = {
	"StringsEnglish.txt", "StringsChinese.txt", "StringsTChinese.txt", "StringsKorea.txt",
		"StringsJapanese.txt", "StringsGerman.txt", "StringsRussian.txt", "StringsFrance.txt",
		"StringsPortuguese.txt","StringsTurkey.txt","StringsSpanish.txt",
		"StringsItalian.txt","StringsPolish.txt","StringsPersian.txt","StringsThai.txt",
		"StringsMagyar.txt","StringsSlovakia.txt","StringsVietnam.txt","StringsGreece.txt",
		"StringsHebrew.txt",
};
#endif

PATTERN_SINGLETON_IMPLEMENT(CLocales);

CLocales::CLocales()
{
	//printf("CLocales::CLocales()>>>>>>>>>\n");
	
	m_iLanguage = -1;
	
	//load font file
	m_bFontValid = FALSE;
	
#if 1
//#ifdef ENC_SHOW_SMALL_FONT
  	m_bFontValidSmall = FALSE;           //zgzhit

    /*读取小字库头信息, 和小型ASCII点阵: zgzhit*/
	if (m_FileFontSmall.Open(FONT_BIN"/FontSmallEn.bin", CFile::modeRead)
		&& m_FileFontSmall.Read(&m_UFHSmall, sizeof(UCS_FONT_HEADER)) == sizeof(UCS_FONT_HEADER)
		&& m_UFHSmall.size == m_FileFontSmall.GetLength()
        && m_UFHSmall.blocks )
	   //&& strcmp(m_UFHSmall.magic, UCS_FONT_MAGIC) == 0)
	{
		m_bFontValidSmall = TRUE;
		m_pUFBSmall = new UCS_FONT_BLOCK[m_UFHSmall.blocks];
		m_FileFontSmall.Read(m_pUFBSmall, sizeof(UCS_FONT_BLOCK) * m_UFHSmall.blocks);
		m_sizeFontSmall.w = m_pUFBSmall[0].width;	//暂时只取第一个区点阵的宽度和高度
		m_sizeFontSmall.h = m_pUFBSmall[0].height;
		m_nFontBytesSmall = (m_sizeFontSmall.w + 7) / 8 * m_sizeFontSmall.h;
		m_pASCIIFontSmall = new uchar[m_nFontBytesSmall * 128 + 128];
		memset(m_pASCIIFontSmall, 0, m_nFontBytesSmall * 128 + 128);
		
		/*所有小型ASCII字体拷到缓冲: zgzhit*/
		for(uint i = 0; i < m_UFHSmall.blocks; i++)
		{
			if(m_pUFBSmall[i].end <= 0x80)
			{
				m_FileFontSmall.Seek(m_pUFBSmall[i].roffs, CFile::begin);
				m_FileFontSmall.Read(&m_pASCIIFontSmall[m_pUFBSmall[i].start * m_nFontBytesSmall], (m_pUFBSmall[i].end - m_pUFBSmall[i].start) * m_nFontBytesSmall);
				m_FileFontSmall.Seek(m_pUFBSmall[i].xoffs, CFile::begin);
				m_FileFontSmall.Read(&m_pASCIIFontSmall[ 128 * m_nFontBytesSmall + m_pUFBSmall[i].start], m_pUFBSmall[i].end - m_pUFBSmall[i].start);
			}
		}
	}
	else
	{
		printf("CLocales::CLocales Open FontSmallEn.bin File Failed!\n");
	}
//#endif
#endif
	//读取字库头信息, 和ASCII点阵, 先不读取其他字符点阵

	if(m_FileFont.Open(FONT_BIN"/Font.bin", CFile::modeRead)
		&& m_FileFont.Read(&m_UFH, sizeof(UCS_FONT_HEADER)) == sizeof(UCS_FONT_HEADER)
		&& m_UFH.size == m_FileFont.GetLength()
		&& m_UFH.blocks)
		//&& strcmp(m_UFH.magic, UCS_FONT_MAGIC) == 0 )
		//&& strcmp(m_UFH.magic, UCS_FONT_MAGIC) != 0 )
	{
		m_bFontValid = TRUE;
		m_pUFB = new UCS_FONT_BLOCK[m_UFH.blocks];
		m_FileFont.Read(m_pUFB, sizeof(UCS_FONT_BLOCK) * m_UFH.blocks);
		m_sizeFont.w = m_pUFB[0].width;	//暂时只取第一个区点阵的宽度和高度
		m_sizeFont.h = m_pUFB[0].height;
		m_nFontBytes = (m_sizeFont.w + 7)  / 8 * m_sizeFont.h;
		m_pASCIIFont = new uchar[m_nFontBytes * 128 + 128];
		memset(m_pASCIIFont, 0, m_nFontBytes * 128 + 128);
		
		printf("CLocales::Font w:%d h:%d,ucs blocks:%d,file size:%d\n",m_sizeFont.w,m_sizeFont.h,m_UFH.blocks,m_UFH.size);
		
		//所有ASCII字体拷到缓冲
		for(uint i = 0; i < m_UFH.blocks; i++)
		{
			//printf("CLocales::UCS_FONT_BLOCK[%d] codes:[%04x,%04x] w&h:[%dx%d] offset:[%d,%d]\n",
			//	i,m_pUFB[i].start,m_pUFB[i].end,m_pUFB[i].width,m_pUFB[i].height,m_pUFB[i].roffs,m_pUFB[i].xoffs);
			
			if(m_pUFB[i].end <= 0x80)
			{
				m_FileFont.Seek(m_pUFB[i].roffs, CFile::begin);
				m_FileFont.Read(&m_pASCIIFont[m_pUFB[i].start * m_nFontBytes], (m_pUFB[i].end - m_pUFB[i].start) * m_nFontBytes);
				m_FileFont.Seek(m_pUFB[i].xoffs, CFile::begin);
				m_FileFont.Read(&m_pASCIIFont[ 128 * m_nFontBytes + m_pUFB[i].start], m_pUFB[i].end - m_pUFB[i].start);
			}
		}
		//printf("CLocales::CLocales()222\n");
	}
	else
	{
		printf("CLocales::CLocales Open Font File Failed!\n");
		
		printf("##1.m_FileFont.Open:%d\n",m_FileFont.Open(FONT_BIN"/Font.bin", CFile::modeRead));//add for test by james.xu  on 090209
		printf("##2.m_FileFont.Read:%d,size:%d\n",m_FileFont.Read(&m_UFH, sizeof(UCS_FONT_HEADER)),sizeof(UCS_FONT_HEADER));//add for test by james.xu  on 090209
		printf("##3.m_UFH.size:%d,m_FileFont.GetLength():%d \n",m_UFH.size,m_FileFont.GetLength());//add for test by james.xu  on 090209
		printf("##4.m_UFH.blocks:%d \n",m_UFH.blocks);//add for test by james.xu  on 090209
		printf("##5.m_UFH.magic:%s, UCS_FONT_MAGIC:%s \n",m_UFH.magic, UCS_FONT_MAGIC);//add for test by james.xu  on 090209
	}
	
	m_iLanguage = -1;
	m_nCommonChars = 0;
	m_pCommonChars = NULL;
	m_pCommonFont = NULL;
	
	//sleep(3);
	
	printf("Language:%d###\n",GetLanguage());
	fflush(stdout);
	
	//设置语言, 一定要保证区域配置CConfigLocation已经加载
	//SetLanguage(CConfigLocation::getLatest().iLanguage);
	SetLanguage(GetLanguage());
	
	printf("CLocales::CLocales() end...\n");
	fflush(stdout);
}

CLocales::~CLocales()
{
	
}

#if 1//csp modify 20131208
int CLocales::IsArabic(ushort code)
{
	typedef struct
	{
		ushort nStartCode;
		ushort nEndCode;
	}SUCZone;
	
	SUCZone LangZone[] = 
	{
		{0x0600, 0x06FF},
		{0x0750, 0x077F},
		{0xFB50, 0xFDFF},
		{0xFE70, 0xFEFF},
		{0xFE50, 0xFE6F},
	};
	
	int i = 0;
	int count = sizeof(LangZone)/sizeof(LangZone[0]);
	
	for(i = 0; i < count; i++)
	{
		if(code >= LangZone[i].nStartCode && code <= LangZone[i].nEndCode)
		{
			return 1;
		}
	}
	
	return 0;
}

int CLocales::IsHebrew(ushort code)
{
	typedef struct
	{
		ushort nStartCode;
		ushort nEndCode;
	}SUCZone;
	
	SUCZone LangZone[] = 
	{
		{0x0590, 0x05FF},
		{0xFB00, 0xFB4F},
	};
	
	int i = 0;
	int count = sizeof(LangZone)/sizeof(LangZone[0]);
	
	for(i = 0; i < count; i++)
	{
		if(code >= LangZone[i].nStartCode && code <= LangZone[i].nEndCode)
		{
			return 1;
		}
	}
	
	return 0;
}

int CLocales::ReverseString(char *str, int left, int right, int debug)
{
	ushort code = 0;//字符unicode
	int n = 0;
	int l = 0;//字符字节数
	
	std::list<unsigned char> uclist;
	
	for(n = left; n < right; n += l)
	{
		code = GetCharCode(&str[n], &l);
		if(l == 0)
		{
			break;
		}
		
		if(debug)
		{
			//printf("ReverseString:code=0x%04x,n=%d,l=%d\n",code,n,l);
		}
		
		int i = 0;
		for(i = l - 1; i >= 0; i--)
		{
			uclist.push_front(str[n+i]);
		}
	}
	
	int x = left;
	
	while(!uclist.empty())
	{
		unsigned char ch = uclist.front();
		uclist.pop_front();
		memcpy(&str[x],&ch,1);
		x++;
	}
		
	return 1;
}

int CLocales::ReloadString(char *str, int debug)
{
	int left = 0;
	int right = 0;
	unsigned char reverse = 0;
	unsigned char finished = 0;
	
	ushort code = 0;//字符unicode
	int n = 0;
	int l = 0;//字符字节数
	
	int end_space_num = 0;//需要反向的字符串结尾处连续的空格数
	
	if(str == NULL)
	{
		return 0;
	}
	
	for(n = 0; ; n += l)
	{
		code = GetCharCode(&str[n], &l);
		if(l == 0)
		{
			finished = 1;
			if(reverse)
			{
				right = n;
			}
			break;
		}
		
		if(debug)
		{
			printf("ReloadString-1:code=0x%04x,n=%d,l=%d\n",code,n,l);
		}
		
		if(IsArabic(code) || IsHebrew(code))
		{
			if(!reverse)
			{
				reverse = 1;
				left = right = n;
			}
			end_space_num = 0;
		}
		else
		{
			if(reverse)
			{
				if(code == 0x0020)//空格
				{
					end_space_num++;
				}
				else if(code == '(' || code == ')' || code == '[' || code == ']' || code == '-')
				{
					//end_space_num = 0;
					
					right = n;
					n += l;
					
					break;
				}
				else if(code < 0x80)//ASCII
				{
					//end_space_num = 0;
					
					right = n;
					n += l;
					
					break;
				}
				else
				{
					//end_space_num = 0;
					
					right = n;
					n += l;
					
					break;
				}
			}
			else
			{
				end_space_num = 0;
			}
		}
	}
	
	if(reverse)
	{
		ReverseString(str, left, right-end_space_num, debug);
	}
	
	if(!finished)
	{
		ReloadString(&str[n], debug);
	}
	
	return 1;
}

int CLocales::Utf8ToUnicode(char* pch, ushort* strUni, int nMaxLen)
{
	int len = 0, n = 0;
	
	while (*pch && len < nMaxLen)
	{
		strUni[len] = GetCharCode(pch, &n);
		if (0 == n)
		{
			break;
		}
		else
		{
			len++;
			pch += n;
		}
	}
	
	return len;
}

void CLocales::ReorderUniStr(ushort* displaystr, int len)
{
	ushort pch[len];
	int wait_pos = len - 1, i = 0, normal_char_num = 0;
	
	memcpy(pch, displaystr, sizeof(u16) * len);
	
	while (i < len && pch[i] < 128)
	{
		displaystr[i] = pch[i];
		i++;
	}
	
	while (i < len) 
	{
		if (pch[i] >= 128) 
		{
			if (normal_char_num > 0) 
			{
				memcpy(displaystr + wait_pos - normal_char_num + 1, pch + i - normal_char_num, normal_char_num * sizeof(u16));
				wait_pos -= normal_char_num;
				normal_char_num = 0;
			}
			displaystr[wait_pos--] = pch[i];
		} 
		else 
		{
			normal_char_num++;
		}
		i++;
	}
	
	if (normal_char_num > 0) 
	{
		memcpy(displaystr + wait_pos - normal_char_num + 1, pch + i - normal_char_num, normal_char_num * sizeof(u16));
		wait_pos -= normal_char_num;
		normal_char_num = 0;
	}
	
	i = 0;
	while (i < len)
	{
		switch(displaystr[i]) 
		{
		case '(':
			displaystr[i] = ')';
			break;
		case ')':
			displaystr[i] = '(';
			break;
		default: 
			break;
		}
		i++;
	}
	
	//printf("\n");
}
#endif

void CLocales::SetLanguage(int index)
{
	char path[VD_MAX_PATH];
	int i, n;
	uint length;
	std::list<ushort> common_list;
	std::list<ushort>::iterator pi;
	ushort code = 0;//unicode
	char *buf;
	char *p;
	char *key;
	char *value;
	
	//csp modify 20130323
	//char *alias_key = NULL;
	
	//printf("SetLanguage-1...\n");
	//fflush(stdout);
	
	if(!m_bFontValid || index == m_iLanguage)
	{
		return;
	}
	
	//printf("SetLanguage-2\n");
	//fflush(stdout);
	
	if(m_iLanguage >= 0)//if language already be set, release first.
	{
		m_FileStrings.UnLoad();
		
		//csp modify 20130323
		if(m_pCommonChars != NULL)
		{
			delete []m_pCommonChars;
			m_pCommonChars = NULL;
		}
		
		//csp modify 20130323
		if(m_pCommonFont != NULL)
		{
			delete []m_pCommonFont;
			m_mapStrings.clear();
		}
	}
	
	m_iLanguage = index;
	
	// 先加载指定目录，再加载特殊目录。为了升级语言使用
	sprintf(path, "%s/%s", FONT_DIR, strings_file[index]);
	//printf("file:%s, func:%s, line:%d, path:%s\n", __FILE__, __FUNCTION__, __LINE__, path);
	
	//printf("SetLanguage-3,path=%s\n",path);
	//fflush(stdout);
	
	if(!(buf = (char *)m_FileStrings.Load(path)))
	{
		printf("Load Language File Failed! Now Load the first Language surport\n");
		char i = 0;
		for(i = 0; i < LANG_NR; i++)
		{
			sprintf(path, "%s/%s", FONT_DIR, strings_file[i]);
			buf = (char *)m_FileStrings.Load(path);
			if((buf) && (buf[0]==(char)0xEF && buf[1]==(char)0xBB && buf[2]==(char)0xBF))
			{
				//是否有必要保存系统语言配置
				break;
			}
		}
		if(i >= LANG_NR)
		{
			printf("Load first Language File Failed! Now Load from Default Directory\n");
			//sprintf(path, "%s/%s", FONT_DEFAULT_DIR, strings_file[index]);
			sprintf(path, "%s/%s", FONT_DIR, "StringsEn.txt");
			if(!(buf = (char *)m_FileStrings.Load(path)))
			{
				printf("Load Default Language File Failed!\n");
				return;
			}
		}
	}
	
	//printf("SetLanguage-4\n");
	//fflush(stdout);
	
	//检查文件格式是否UTF-8
	if(buf[0] != (char)0xEF || buf[1] != (char)0xBB || buf[2] != (char)0xBF)
	{
		printf("CLocales::SetLanguage Strings File Format Error!\n");
		m_FileStrings.UnLoad();
		return;
	}
	
	//printf("SetLanguage-5\n");
	//fflush(stdout);
	
	//csp modify 20130729
	{
		char i = 0;
		for(i = 0; i < LANG_NR; i++)
		{
			char path[VD_MAX_PATH];
			sprintf(path, "%s/%s", FONT_DIR, strings_file[i]);
			
			//测试文件是否存在
			if(0 == access(path, 0))
			{
				remove(path);
			}
		}
	}
	
	//扫描字符串文件, 提取其中的常用字符
	length = m_FileStrings.GetLength();
	p = buf;
	while((code = GetCharCode(p, &n)))
	{
		p += n;
		if(code >= 0x80)//非ASCII码
		{
			common_list.push_back(code);
		}
		if(p >= buf + length)
		{
			break;
		}
	}
	if(!code)//解析出错
	{
		printf("CLocales::SetLanguage Parse Strings File Failed!\n");
	}
	
	//printf("SetLanguage-6.0...\n");
	//fflush(stdout);
	
	//排序,去掉重复的字符.
	common_list.sort();
	
	//printf("SetLanguage-6.1\n");
	//fflush(stdout);
	
	common_list.unique();
	
	//printf("SetLanguage-6.2\n");
	//fflush(stdout);
	
	if((n = common_list.size()))
	{
		m_nCommonChars = n;
		m_pCommonChars = new ushort[n];
		m_pCommonFont = new uchar[n * m_nFontBytes + n];
		printf("n=%d,m_nFontBytes=%d,m_pCommonChars=0x%08x,m_pCommonFont=0x%08x\n",n,m_nFontBytes,m_pCommonChars,m_pCommonFont);
		
		for(i = 0, pi = common_list.begin(); pi != common_list.end(); pi++, i++)
		{
			m_pCommonChars[i] = *pi;
			m_pCommonFont[n * m_nFontBytes + i] = GetOneFontFromFile(m_pCommonChars[i], &m_pCommonFont[i * m_nFontBytes]);
		}
	}
	
	//printf("SetLanguage-7\n");
	//fflush(stdout);
	
	//提取字符串, 并按键值排列
	p = buf;
	key = p;
	value = NULL;
	while(*p)
	{
		if(*p == '=')
		{
			if(!value)
			{
				*p = '\0';
				value = p + 1;
			}
		}
		else if(*p == '\n')
		{
			if(value)
			{
				*p = '\0';
				if(*(p-1) == '\r')//windows format
				{
					*(p-1) = '\0';
				}
				
				#if 0//csp modify 20130323
				if(alias_key && !strncmp(key, "titles", 6))//处理各ODM版本标题不同的问题
				{
					if(strncmp(key, alias_key, 9))
					{
						goto jump_over;
					}
					i = 6;
					while(1)
					{
						key[i] = key[i+3];
						if(!key[i])
						{
							break;
						}
						i++;
					}
				}
				#endif
				
				#if 1//csp modify 20131208
				//printf("#################ReloadString:index=%d,key=%s#################\n",index,key);
				
				if(index == POS || index == HEBREW)
				//if(index == HEBREW)
				{
					int debug = 0;
					if(strstr(key,"CfgPtn.20s") || strstr(key,"CfgPtn.Size"))
					{
						//debug = 1;
						//printf("Reload:(%s,%s)\n",key,value);
					}
					
					ReloadString(value, debug);
					
					if(debug)
					{
						int n = 0;
						int l = 0;
						
						for(n = 0; ; n += l)
						{
							code = GetCharCode(&value[n], &l);
							if(l == 0)
							{
								break;
							}
							
							if(debug)
							{
								printf("ReloadString-2:code=0x%04x,n=%d,l=%d\n",code,n,l);
							}
						}
					}
				}
				/*else if(index == POS)
				{
					//printf("波斯语...阿拉伯语...\n");
					
					ushort strUni[512] = {0};
					int nStrUniLen = 0;
					
					memset(strUni, 0, sizeof(strUni));
					
					//reorder
					//ReloadString(value, 0);
					
					//utf8->ucs2
					nStrUniLen = Utf8ToUnicode(value, strUni, sizeof(strUni) / sizeof(ushort));
					//printf("nStrUniLen=%d\n", nStrUniLen);
					
					//reorder
					ushort* pStrUni = strUni;
					ReorderUniStr(pStrUni, nStrUniLen);
					
					//reshape 连写变化后的unicode编码
					int tmplen = nStrUniLen;
					arabic_reshape(&nStrUniLen, strUni, ar_nothing);
					//printf("arabic_reshape nStrUniLen=%d\n", nStrUniLen);
					nStrUniLen = tmplen;
					
					//reorder
					//ushort* pStrUni = strUni;
					//ReorderUniStr(pStrUni, nStrUniLen);
					
					m_mapStrings.insert(MAPSTRING::value_type(key,(char *)strUni));
					goto jump_over;
				}*/
				#endif
				
				m_mapStrings.insert(MAPSTRING::value_type(key,value));
jump_over:
				value = NULL;
			}
			key = p + 1;
		}
		p++;
		if(p >= buf + length)
		{
			break;
		}
	}
	
	//printf("SetLanguage-8\n");
	//fflush(stdout);
	
	//return;
}
//装载一个字符的字体, 传入参数为UCS-2编码, 返回值为字体宽度
uchar CLocales::GetOneFontFromFile(ushort code, uchar *p)
{
	int start = 0;
	int end = m_UFH.blocks;
	int mid;
	uchar wx;

	while(1)
	{
		mid = (start + end)/2;
		if(mid == start)break;
		if(m_pUFB[mid].start <= code)
		{
			start = mid;
		}
		else
		{
			end = mid;
		}
	}
	if(code >= m_pUFB[mid].end) //字库中也没有, 显示未知字符.
	{
		//trace("Unknown code = %04x\n", code);
		if(p)
		{
			memset(p, 0xff, m_nFontBytes);
		}
		return m_sizeFont.w / 2;
	}

	if(p)
	{
		m_FileFont.Seek(m_pUFB[mid].roffs + (code - m_pUFB[mid].start) * m_nFontBytes, CFile::begin);
		m_FileFont.Read(p, m_nFontBytes);
	}
	m_FileFont.Seek(m_pUFB[mid].xoffs + (code - m_pUFB[mid].start), CFile::begin);
	m_FileFont.Read(&wx, 1);
	return wx;
}

ushort CLocales::GetCharCode(VD_PCSTR pch, int *pn)
{
	uchar ch;//sigle char
	ushort code = 0;//unicode
	int flag = 0;//0 - empty, 1 - 1 char to finish unicode, 2 - 2 chars to finish unicode, -1 - error
	
	*pn = 0;
	while((ch = (uchar)*pch))
	{
		pch++;
		if(ch & 0x80)
		{
			if((ch & 0xc0) == 0xc0)
			{
				if((ch & 0xe0) == 0xe0)
				{
					if((ch & 0xf0) == 0xf0)//ucs-4?
					{
						break;
					}
					if(flag)
					{
						break;
					}
					*pn = 3;
					flag = 2;
					code |= ((ch & 0x0f) << 12);
				}
				else
				{
					if(flag)
					{
						break;
					}
					*pn = 2;
					flag = 1;
					code |= ((ch & 0x1f) << 6);
				}
			}
			else
			{
				if(flag == 0)
				{
					break;
				}
				else if(flag == 1)//unicode finished
				{
					code |= (ch & 0x3f);
					break;
				}
				else
				{
					code |= ((ch & 0x3f) << 6);
				}
				flag--;
			}
		}
		else//ASCII
		{
			if(flag)
			{
				break;
			}
			*pn = 1;
			code = ch;
			break;
		}
	}
	
	if(ch == 0)
	{
		code = 0;
	}
	
	return code;
}

VD_BOOL CLocales::GetFontSize(VD_SIZE *pSize, FONTSIZE fontsize)
{
	if(!pSize)
	{
		return FALSE;
	}

	if (fontsize == FONTSIZE_NORMAL)
	{
		*pSize = m_sizeFont;
	}
	else if (fontsize == FONTSIZE_SMALL)
	{
		*pSize = m_sizeFontSmall;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

//二分发查找常用汉字
int CLocales::FindCommonChar(ushort code)
{
	if(!m_pCommonChars)
	{
		return -1;
	}
	
	int high = m_nCommonChars - 1, low = 0, mid;
	
	while(low <= high)
	{
		mid = (low + high) / 2;
		if(m_pCommonChars[mid] < code)
		{
			low = mid + 1;
		}
		else if(m_pCommonChars[mid] > code)
		{
			high = mid - 1;
		}
		else
		{
			return mid;
		}
	}
	
	return -1;
}
//得到字符的宽度, 如果p不为NULL, 则用点阵填充
uchar CLocales::GetCharRaster(ushort code, uchar* p /* = NULL */, FONTSIZE fontsize)
{
	int index;

	if (fontsize == FONTSIZE_NORMAL)
	{
		if(!m_bFontValid)
		{
			return 0;
		}

		if(code < 0x80)//ASCII字符
		{
			if(p)
			{
				memcpy(p, m_pASCIIFont + code * m_nFontBytes, m_nFontBytes);
			}
			return m_pASCIIFont[128 * m_nFontBytes + code];
		}

		//其他字符
		if((index = FindCommonChar(code)) >= 0) //在常用字列表中,直接取值
		{
			if(p)
			{
				memcpy(p, &m_pCommonFont[index * m_nFontBytes], m_nFontBytes);
			}
			return m_pCommonFont[m_nCommonChars * m_nFontBytes + index];
		}
		else //不在,读文件
		{
			return GetOneFontFromFile(code, p);
		}
	}
	else if (fontsize == FONTSIZE_SMALL)
	{
		if(!m_bFontValidSmall)
		{
			return 0;
		}

		if(code < 0x80)//ASCII字符
		{
			if(p)
			{
				memcpy(p, m_pASCIIFontSmall + code * m_nFontBytesSmall, m_nFontBytesSmall);
			}
			return m_pASCIIFontSmall[128 * m_nFontBytesSmall + code];
		}
	}
	return 0;
}

VD_PCSTR LOADSTR(VD_PCSTR key)
{
	MAPSTRING::iterator pi;
	pi = g_Locales.m_mapStrings.find(key);
	if(pi != g_Locales.m_mapStrings.end())
	{
		return (*pi).second;
	}
	else
	{
		//printf("Load string '%s' failed!!!\n", key);
		return key;
	}
}

/*!
\b Description		:	通过&字符进行字符串解析，便于在构造函数的时候不调用宏LOADSTRING\n
\b Revisions		:
- 2007-04-06		wangqin		Create
- 2007-04-12		wangqin		modified
*/
VD_PCSTR GetParsedString(VD_PCSTR key)
{
	if (*key == '&')
	{
		return LOADSTR(key+1);
	}
	else
		return key;
}


int CLocales::GetTextExtent(VD_PCSTR str, int len/*=1024*/, FONT_STYLE fs /*= FS_NORMAL*/)
{
	if (!str){
		return 0;
	}
	ushort code;//字符unicode
	int n;
	int w;//字符宽度
	int l;//字符字节数
	int width1 = 0;//字符串宽度
	FONT_STYLE	fontStyle = fs;
	if (len>(int)strlen(str))
	{
		len = strlen(str);
	}
	for (n = 0; n < len; n += l, width1 += w)
	{
		code = GetCharCode(&str[n], &l);
		if(l == 0)
		{
			break;
		}

		if(fontStyle == FS_SMALL)
		{
			w = GetCharRaster(code, NULL, FONTSIZE_SMALL);
		}
		else
		{
			w = GetCharRaster(code);
		}

	}
	return width1;
}

