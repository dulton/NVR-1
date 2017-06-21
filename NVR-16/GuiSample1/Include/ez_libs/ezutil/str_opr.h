
#ifndef _STR_OPR_H
#define _STR_OPR_H

#include <stdio.h>

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * l_count_c - 
	 * @ src : 
	 *		data buf
	 * @ __c : 
	 *		the obj char
	 *
	 * the '__c' count in src from left
	 */
	size_t l_count_c(char *src, const char __c);
	size_t r_count_c(char *src, const char __c);

	size_t l_trim_c(char *src, const char trimmer, size_t src_len);
	size_t r_trim_c(char *src, const char trimmer, size_t src_len);
	size_t trim_c(char *src, const char trimmer, size_t src_len);
	size_t trim_blank(char *src, size_t src_len);

	size_t replace_c(
		char *src, 
		const char fromc, 
		const char toc, 
		size_t src_len);
	size_t replace_chars_with_char(
		char *strbuf, 
		char *chrstorm,
		char chartorlcwith);

	/* danger func */
	size_t ini_str(char *src, const char __c, const  size_t src_size);

	int split_str(
		const char * const src,
		char *left,
		char *right,
		const char * const spliter,
		size_t src_len);
	int r_split_str(
	    const char * const src,
	    char *right,
	    const char * const spliter,
	    size_t src_len);

	#define SHOW_HEXAD 0x01
	#define SHOW_ASCII 0x02
	#define SHOW_BINAR 0x04
	#define SHOW_LINER 0x08

	int dumpBuffer(
	    FILE *fp,
        unsigned char *buf,
	    const size_t len,
	    size_t uiPrintMask);

	char * Ip2Str(const unsigned int iIp, char *pStr);
	unsigned int Str2Ip(const char *pStr);


	int isCommentLine(char *src);
	int is_ascii_string(const char * const src, const size_t len);
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

#ifdef __cplusplus
}
#endif

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

#endif //_STR_OPR_H
