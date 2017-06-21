/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*
 * md5.h - _explain_
 *
 * $Id: md5.h 18029 2008-07-01 05:53:13Z yu_xiangyun $
 *
 * add this file to encode the passwd string.
 * I add DH_Superencipherment() func to fit this requirement
 * you can use it nimbly, such as license etc. I know that 
 * some telcom software use it to manage their license, 
 * 
 *  Explain:
 *     -explain-
 *
 *  Update:
 */
/*
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *  
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *  
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *  
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *  
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/


#ifndef __MD5_H__
#define __MD5_H__

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* POINTER defines a generic pointer type */
typedef unsigned char* POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short  UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long  UIN;

/* MD5 context. */
typedef struct
{
	UIN state[4];        /* state (ABCD) */
	UIN count[2];        /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];        /* input buffer */
}
MD5_CTX;

void md5_calc(unsigned char *output, unsigned char *input, unsigned int inlen);
void MD5Init(MD5_CTX* md5_ctx);
void MD5Update(MD5_CTX* md5_ctx, unsigned char* uc, unsigned int ui);
void MD5Final(unsigned char uc[16], MD5_CTX* md5_ctx);
signed char *
DH_Superencipherment(signed char *strOutput, unsigned char *strInput);
signed char *
SuperPasswd(signed char *strOutput, int iOutLen, unsigned char *strInput);
signed char *
SuperPasswdDH01(signed char *psw, int len);


unsigned char *ez_md5_calc_hexstr(unsigned char *output, unsigned char *input, unsigned int inlen);

#endif //__MD5_H__

