/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*
 * des.h - _explain_
 *
 * Copyright (C) 2007 dahua Technologies, All Rights Reserved.
 *
 * $Id: des.h 2023 2007-5-24 15:00:09Z wujj $
 *
 *  Explain:
 *     -explain-
 *
 *  Update:
 *     2007-5-24 15:00:09 WuJunjie 10221 Create
 */
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
#ifndef _DES_H
#define _DES_H

#ifdef __cplusplus
extern "C"
{
#endif

	/*! 加密接口
	  * \param  pResult 加密后的缓存位置 
			缓存的长度 >= ((iOrigLen+7)/8)*8 
			即比iOrigLen大，且是8的倍数的最少整数
			pResult可以为pOrig，但是会覆盖原数据
	  * \param  pOrig 待加密的缓存位置
	  * \param  iOrigLen 待加密缓存长度
	  * \param  pKey 密钥 超过16字节后只取前16字节
	  * \param  iKeylen 密钥长度
	  * \return : <0失败，加密后数据长度(Byte)
	  */
	int DesEncrypt(char *pResult,
	               char *pOrig,
	               long iOrigLen,
	               const char *pKey,
	               int iKeylen);

	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

	/*! 脱密接口
	  * \param  pResult 脱密后的缓存位置
	  * \param  pOrig 待脱密的缓存位置
	  * \param  iOrigLen 待脱密缓存长度
	  * \param  pKey 密钥
	  * \param  iKeylen 密钥长度
	  * \return : <0失败
	  */
	int DesDecrypt(char *pResult,
	               char *pOrig,
	               long iOrigLen,
	               const char *pKey,
	               int iKeylen);

	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
	/*! 加密接口
	  * \param  pResult 加密后的缓存位置 
			缓存的长度 >= ((iOrigLen+7)/8)*8 
			即比iOrigLen大，且是8的倍数的最少整数
			pResult可以为pOrig，但是会覆盖原数据
	  * \param  pOrig 待加密的缓存位置
	  * \param  iOrigLen 待加密缓存长度
	  * \param  pKey 密钥 超过8字节后只取前8字节
	  * \param  iKeylen 密钥长度
	  * \return : <0失败，加密后数据长度(Byte)
	  */
	int ez_des_ecb_encrypt(unsigned char *pResult,
	                    unsigned char *pOrig,
	                    long iOrigLen,
	                    const char *pKey,
	                    int iKeylen);

	int ez_des_ecb_decrypt(unsigned char *pResult,
	                    unsigned char *pOrig,
	                    long iOrigLen,
	                    const char *pKey,
	                    int iKeylen);

#ifdef __cplusplus
}
#endif

#endif //_DES_H
