/*
 * linux/fs/nls_cp936.c
 *
 * Charset cp936 translation tables.
 * This translation table was generated automatically, the
 * original table can be download from the Microsoft website.
 * (http://www.microsoft.com/typography/unicode/unicodecp.htm)
 */
#ifndef NLS_H
#define NLS_H

int UniTGb2312(unsigned short* pUnicode,int nUnicLen, char* pStr,int nStrLen);

int Gb2312ToUni(char* pSource,unsigned short* pTag,int nLenTag);



int Gb2312TUtf8(char* pSource,unsigned char* pUtf8,int nUtf8Len);

int utf8TGb2312(unsigned char* putf8,char* pStr,int nUtf8Len,int nStrLen);


int uni2char(const unsigned short uni,
			unsigned char *out, int boundlen);


int char2uni(const unsigned char *rawstring, int boundlen,
			unsigned short *uni);


int
utf8_mbtowc(unsigned short *p, const unsigned char *s, int n);

int
utf8_mbstowcs(unsigned short *pwcs, const unsigned char *s, int n);

int
utf8_wctomb(unsigned char *s, unsigned short wc, int maxlen);


int
utf8_wcstombs(unsigned char *s, const unsigned short *pwcs, int maxlen);




#endif






