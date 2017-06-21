
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "ez_libs/ezutil/str_opr.h"

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t l_count_c(char *src, const char __c)
{

	char *p;

	if (!src || !*src || !__c)
		return 0;

	p = src;

	while ((*p != '\0')
	       && (*p == __c))
	{
		p++;
	}

	return (size_t)(p-src);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t r_count_c(char *src, const char __c)
{
	char *p;
	char *p_tail;

	if (!src || !*src || !__c)
		return 0;

	p_tail = src + strlen(src)-1;
	p=p_tail;

	while ( (p>=src)
	        && (*p == __c) )
	{
		p--;
	}
	return (size_t)(p_tail-p);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 * l_trim_c - 
 * src:
 * trimmer: 
 * src_len:
 * Desc
 */
size_t l_trim_c(char *src, const char trimmer, size_t src_len)
{
	size_t pass_len;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !trimmer)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	pass_len = l_count_c(src, trimmer);

	if (pass_len>0)
	{
		src_len -= pass_len;
		memmove(src, src+pass_len, src_len);
		src[src_len] = '\0';
	}

	return (size_t)(pass_len);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t r_trim_c(char *src, const char trimmer, size_t src_len)
{
	char *p_tail;
	size_t pass_len = 0;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !trimmer)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	p_tail = src + src_len -1;

	while ( (p_tail>=src)
	        && (*p_tail==trimmer) )
	{
		*p_tail-- = '\0';
		pass_len++;
	}

	/* get right part */
	if (src_len<real_len)
		*(src+src_len) = '\0';

	return (size_t)(pass_len);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t trim_c(char *src, const char trimmer, size_t src_len)
{
	size_t pass_len;

	pass_len  = l_trim_c(src, trimmer, src_len);
	pass_len += r_trim_c(src, trimmer, src_len);

	return pass_len;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t trim_blank(char *src, size_t src_len)
{
	size_t pass_len;
	size_t real_len;
	char *p = src;

	/* check input*/
	if (!src || !*src)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	while ((*p != '\0')
	       && ( (*p == ' ')
	            || (*p == '\t')
	            || (*p == '\r')
	            || (*p == '\n') ) )
	{
		p++;
	}

	pass_len = p-src;
	src_len = src_len-pass_len;

	memmove(src, p, src_len);
	src[src_len] = '\0';

	p = src + src_len - 1;
	while (p >= src)
	{
		if ( (*p == ' ')
		     || (*p == '\t')
		     || (*p == '\r')
		     || (*p == '\n') )
		{
			*p-- = 0;
			pass_len++;
		}
		else
		{
			break;
		}
	}

	return (size_t)(pass_len);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t replace_c(
				 char *src, 
				 const char fromc, 
				 const char toc, 
				 size_t src_len)
{
	size_t real_len;
	size_t num;
	char *p;

	p = src;
	num = 0;

	/* check input*/
	if (!src || !*src || !fromc || !toc)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	while (*p != '\0')
	{
		if (*p == fromc)
		{
			*p = toc;
			num++;
		}

		p++;
	}

	return (size_t)(num);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t replace_chars_with_char(
							   char *strbuf, 
							   char *chrstorm, 
							   char chartorlcwith)
{
	char *offset;
	size_t replace_chars;

	/* check input*/
	if (!strbuf || !*strbuf || !chrstorm || !*chrstorm)
		return 0;

	offset = (char *)NULL;
	replace_chars = 0;

	while (*strbuf)
	{
		offset = strpbrk (strbuf, chrstorm);
		if (offset)
		{
			*(offset) = chartorlcwith;
			replace_chars++;
		}
		else
		{
			break;
		}
	}

	return replace_chars;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 * ini_str - 
 * @: 
 * @: 
 *
 * danger func
 */
size_t ini_str(char *src, const char __c, const  size_t src_size)
{
	/* check input*/
	if (!src || !*src)
		return 0;
	memset(src+src_size-1, 0, 1);
	memset(src, __c, src_size-1);
	return (size_t)src_size;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 * split_str - 
 * @: 
 * @: 
 *
 * find func based strstr, it find from left
 */
int split_str(
    const char * const src,
    char *left,
    char *right,
    const char * const spliter,
    size_t src_len)
{
	char *p;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !left || !right)
	{
		return -1000;
	}

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	strncpy(right, src, src_len);
	right[src_len] = '\0';

	/* get right pos */
	p = strstr(right, spliter);
	/* not found spliter */
	if (!p)
	{
#if 0
		// ret error here
		*left = '\0';
		*right = '\0';
		return -1;
#else
		// we load src to lefp
		// ret ok here
		strcpy(left, src);
		*right = '\0';
		return 0;
#endif

	}
	/* left */
	strncpy(left, right, p-right);
	left[p-right] = '\0';
	/* right */
	strcpy(right, p+strlen(spliter));

	return 0;
}


/**
 * func_name - 
 * @: 
 * @: 
 *
 * 分隔字符串， 取分隔符后面(右边)的值
 */
int r_split_str(
    const char * const src,
    char *right,
    const char * const spliter,
    size_t src_len)
{
	char *p;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !right)
		return -1000;
	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	strncpy(right, src, src_len);
	right[src_len] = '\0';

	/* get right pos */
	p = strstr(right, spliter);
	/* not found spliter */
	if (!p)
	{
		*right = '\0';
		return -1;
	}
	/* right */
	strcpy(right, p+strlen(spliter));

	return 0;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
// Return TotalLine
int dumpBuffer(FILE *fp,
               unsigned char *buf,
               const size_t len,
               size_t uiPrintMask)
{
	unsigned int iTotalLine;
	unsigned int iLiner;
	unsigned int iPostion;

	if (0 == uiPrintMask)
		return 0;

	if (0 == len)
		return 0;

	iTotalLine = len/16 + ((len%16) ? 1 : 0);

	for (iLiner=1; iLiner<=iTotalLine; iLiner++)
	{
		if (uiPrintMask & SHOW_LINER)
			fprintf(fp, "%04d: ", iLiner);

		if (uiPrintMask & SHOW_HEXAD)
			fprintf(fp, "0x%06x: ", 16*(iLiner-1));

		if (uiPrintMask & SHOW_BINAR)
			for (iPostion=16*(iLiner-1); (iPostion<16*(iLiner-1)+16); iPostion++)
			{
				if (iPostion<len)
					fprintf(fp, "%02x ", buf[iPostion]);
				else
					fprintf(fp, "   ");
			}

		if (uiPrintMask & SHOW_ASCII)
		{
			fprintf(fp, "; ");
			for (iPostion=16*(iLiner-1); (iPostion<len) && (iPostion<16*(iLiner-1)+16); iPostion++)
			{
				if ( isprint(buf[iPostion]) )
				{
					fprintf(fp, "%c", buf[iPostion]);
				}
				else
				{
					fprintf(fp, "%c", '.');
				}
			}
		}
		fprintf(fp, "\n");
	}

	return iTotalLine;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/* 
 * following use LITTLE_ENDIAN memory
 */
char * Ip2Str(const unsigned int iIp, char *pStr)
{
	unsigned char i;
	unsigned char cIp[4];

	for (i=0; i<4; i++)
		cIp[i] = (unsigned char)( (iIp>>(i*8)) & (0xff) );

	sprintf(pStr, "%d.%d.%d.%d", cIp[0], cIp[1], cIp[2], cIp[3]);
	return pStr;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 *  - 
 * @ : 
 * @ :
 * Desc
 *	j >= 0 used when BIG_ENDIAN
 *	do not comment it
 */
unsigned int Str2Ip(const char *pStr)
{
	signed char j = 0;
	unsigned char cIp;
	unsigned int uiRet = 0;
	const char *p = pStr;

	cIp = (unsigned char)atoi(p);
	uiRet |= ( ((unsigned int)(cIp)) << (j*8) );
	j++;

	while ( (*p != 0)
	        && j >= 0)
	{
		if (*p != '.')
		{
			p++;
			continue;
		}

		cIp = (unsigned char)atoi(++p);
		uiRet |= ( ((unsigned int)(cIp)) << (j*8) );
		j++;

	}

	return uiRet;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

int isCommentLine(char *src)
{
	char *p = src;

	/* check input*/
	if (!src || !*src)
		return 1;


	// skip blank
	while ((*p != '\0')
		&& ( (*p == ' ')
		|| (*p == '\t')
		|| (*p == '\r')
		|| (*p == '\n') ) )
	{
		p++;
	}

	// ‘#’ is commit charactor
	if (*p == '#' || *p == '\0')
	{
		return 1;
	}

	return 0;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

int is_ascii_string(const char * const src, const size_t len)
{
	size_t ii;

	for (ii=0; ii<len; ii++)
	{
		if ('\0' == src[ii])
		{
			break;
		}

		if (!isascii(src[ii]))
		{
			return -1;
		}
	}

	return 0;
}



