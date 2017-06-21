/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*
 * crc.h - /home/wujj/src/libs/src_ezutil
 *
 * $Id: crc.h 5884 2008-7-16 11:33:42Z wu_junjie $
 *
 *  Explain:
 *     crc16/crc32
 *
 *  Update:
 *     2008-7-16 11:33:24 WuJunjie 10221 Create
 */
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

#ifndef _CRC_H
#define _CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * ez_crc16 - compute the CRC-16 for the data buffer
	 * \param crc:	previous CRC value
	 * \param buffer:	data pointer
	 * \param len:	number of bytes in the buffer
	 *
	 * \return: The updated CRC value.
	 */
	unsigned short ez_crc16(unsigned short crc, unsigned char const *buffer, unsigned int length);

	/**
	 * ez_crc32 - compute the CRC-32 for the data buffer
	 * \param buffer:	data pointer
	 * \param len:	number of bytes in the buffer
	 *
	 * \return: The updated CRC value.
	 */
	unsigned int ez_crc32 (unsigned char *buffer, unsigned int length);

#ifdef __cplusplus
}
#endif

#endif //_CRC_H


/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/* USAGE
	//--------------------------
	printf("orig:%s\n", pOrig);
	usret = ez_crc16(0x0, (unsigned char *)pOrig, strlen(pOrig));
	printf("ez_crc16:0x%0X\n", usret);

	//--------------------------
	printf("orig:%s\n", pOrig);
	uiret = ez_crc32((unsigned char *)pOrig, strlen(pOrig));
	printf("ez_crc32:0x%0X\n", uiret);
	printf("this crc32 alo is:%s\n", uiret==0x5CFCCFF6?"OK":"NOK"); 
*/
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
