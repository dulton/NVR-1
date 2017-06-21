#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "CryptAES.h"
//#include "../../public/TypedefPlatform.h"

#define ENABLE_NETDATA_AES      1

#define NP_HEADFLAG             (unsigned short)21577 //MAKEWORD('I', 'T')

#ifdef WIN32

    #pragma pack(1)

    #if ENABLE_NETDATA_AES

        typedef struct _netmsgaes
        {
            unsigned short wHeadFlag;         // MAKEWORD('I', 'T') -> 21577
            int iDataLen;           // 后续数据总长度(iSrcLen+buflen)
	        int iSrcLen;
	        char chAESBuf[1];
        }NETMSGAES;

    #else

        typedef struct _netmsgaes
        {
            unsigned short wHeadFlag;         // MAKEWORD('I', 'T') -> 21577
            int iDataLen;           // chBuf总长度
	        char chBuf[1];
        }NETMSGAES;

    #endif

    #pragma pack()

#else

    #if ENABLE_NETDATA_AES

        typedef struct _netmsgaes
        {
            unsigned short wHeadFlag;         // MAKEWORD('I', 'T') -> 21577
            int iDataLen;           // 后续数据总长度(iSrcLen+buflen)
	        int iSrcLen;
	        char chAESBuf[1];
        }__attribute__ ((packed)) NETMSGAES;

    #else

        typedef struct _netmsgaes
        {
            unsigned short wHeadFlag;         // MAKEWORD('I', 'T') -> 21577
            int iDataLen;           // chBuf总长度
	        char chBuf[1];
        }__attribute__ ((packed)) NETMSGAES;

    #endif

#endif

int AESSocketSend(int sockfd, char* pData, int iDataLen, int flags)
{
#if ENABLE_NETDATA_AES
    aes_ctx m_ectEnc;
    unsigned char key[] = "1234567890123456";
    NETMSGAES* pSendBuf     = NULL;
    char* pBuf              = NULL;
    int iBufPos             = 0;
	int iSendRet            = 0;
	int iSendLen            = 0;
    int i                   = 0;
    
	printf("AESSocketSend,iDataLen=%d...\n",iDataLen);
	
    char* pMalloc = (char *)malloc(iDataLen + 26);
    if (!pMalloc)
        return 0;
	
    pSendBuf    = (NETMSGAES*)pMalloc;
    pBuf        = &(pSendBuf->chAESBuf[0]);
	
    pSendBuf->wHeadFlag = NP_HEADFLAG;
	pSendBuf->iSrcLen   = iDataLen;	

	aes_enc_key(key, 16, &m_ectEnc);

	for (i = 0; i < iDataLen; i += 16)
	{
		aes_enc_blk((unsigned char *)pData+i, (unsigned char *)pBuf+i, &m_ectEnc);
	}

    pSendBuf->iDataLen  = i + 4;        // + sizeof(int);

    //////////////////////////////////////////////////////////////////////////
    
    iSendLen = pSendBuf->iDataLen + 6;  // + sizeof(int) + sizeof(WORD);
    
	printf("AESSocketSend,iSendLen=%d...\n", iSendLen);
	
	while (iSendLen > 0)
	{
		iSendRet = send(sockfd, pMalloc+iBufPos, iSendLen, flags);
		if (iSendRet < 0)
		{
			// 发送失败
            free(pMalloc);
			return iSendRet;
		}
		iSendLen    -= iSendRet;
		iBufPos     += iSendRet;
	}
	
	printf("AESSocketSend,send over\n");
	
    free(pMalloc);
	return iDataLen;

#else
    
    NETMSGAES* pSendBuf = NULL;
    int iSendLen        = 0;
    int iPos            = 0;
    int iSendRet        = 0;

    char* pMalloc = malloc(iDataLen + 6);
    if (!pMalloc)
        return 0;

    pSendBuf = (NETMSGAES*)pMalloc;

    pSendBuf->wHeadFlag     = NP_HEADFLAG;
    pSendBuf->iDataLen      = iDataLen;

    memcpy(pSendBuf->chBuf, pData, iDataLen);

    iSendLen = pSendBuf->iDataLen + 6; // + sizeof(int) + sizeof(WORD);

    while (iSendLen > 0)
    {
        iSendRet = send(sockfd, pMalloc+iPos, iSendLen, flags);
        if (iSendRet < 0)
        {
            free(pMalloc);
            return iSendRet;
        }
        iPos        += iSendRet;
        iSendLen    -= iSendRet;
    }

    free(pMalloc);
    return iDataLen;
    
#endif
}

int AESSocketRecv(int sockfd, char* pData, int iDataLen, int flags)
{
#if ENABLE_NETDATA_AES	

    int iLen        = 0;
    int iRet        = 0;
    char* pMalloc   = 0;
    char* pBuf      = 0;
    char* pTemp     = 0;
    int iPos        = 0;
    int i           = 0;
    NETMSGAES stRecvHead;
    NETMSGAES* pRecvBuf = 0;
    struct timeval tv;
    int retval = 0;
    fd_set select_fds;
    aes_ctx m_ectDec;	// 解密密钥
    unsigned char key[]="1234567890123456";

    memset(&stRecvHead, 0, sizeof(NETMSGAES));

    // 6 -> sizeof(WORD)+sizeof(int)
	iRet = recv(sockfd, (char*)(&stRecvHead), 6, flags);
	if (iRet <= 0)
	{
		return iRet;
	}

    if (NP_HEADFLAG != stRecvHead.wHeadFlag || stRecvHead.iDataLen <= 0)
    {
        return 0;
    }

    iLen = stRecvHead.iDataLen;


    // 6 -> sizeof(WORD) + sizeof(int)
    if (iLen > iDataLen + 26 - 6)
    {
        return 0;
    }

    pMalloc = (char *)malloc(iDataLen + 26);
    if (!pMalloc)
        return 0;

    pRecvBuf    = (NETMSGAES*)pMalloc;
    pBuf        = (char*)(&pRecvBuf->iSrcLen);

    pRecvBuf->iDataLen = iLen;

    //////////////////////////////////////////////////////////////////////////
    
    FD_ZERO(&select_fds);
    FD_SET((unsigned int)sockfd, &select_fds);
    
    while (iLen > 0)
    {
        tv.tv_sec  = 5;
        tv.tv_usec = 0;
        
        retval = select(sockfd + 1, &select_fds, NULL, NULL, &tv);
        if (retval > 0)
        {
            iRet = recv(sockfd, pBuf+iPos, iLen, flags);
            if (iRet <= 0)
            {
                free(pMalloc);
                return iRet;
            }
            iPos        += iRet;
            iLen        -= iRet;
        }
        else
        {
            free(pMalloc);
            return retval;
        }
    }

    pBuf = &(pRecvBuf->chAESBuf[0]);

    if (pRecvBuf->iDataLen-4 >= 4160)
    {
        // 4160是sizeof(NETMSG)头+MAX_DATA_LEN(4096)算出加密后的长度
        // 当源数据为最大值时, 密文数据要比传入的buf大，导致aes_dec_blk溢出
        // 所以必须临时分配一块内容用于周旋
        pTemp = (char *)malloc(pRecvBuf->iDataLen);

        // 4 -> sizeof(int)
        aes_dec_key(key, 16, &m_ectDec);
        for (i = 0; i < pRecvBuf->iDataLen-4; i += 16)
        {
            aes_dec_blk((unsigned char *)pBuf+i, (unsigned char *)pTemp+i, &m_ectDec);
	    }

        memcpy(pData, pTemp, pRecvBuf->iSrcLen);

        free(pTemp);
    }
    else
    {
        // 4 -> sizeof(int)
        aes_dec_key(key, 16, &m_ectDec);
        for (i = 0; i < pRecvBuf->iDataLen-4; i += 16)
        {
            aes_dec_blk((unsigned char *)pBuf+i, (unsigned char *)pData+i, &m_ectDec);
	    }
    }
    
    iLen = pRecvBuf->iSrcLen;
    free(pMalloc);
	return iLen;

#else

    int iDataLen    = 0;
    int iRet        = 0;
    char* pMalloc   = 0;
    int iRecvLen    = 0; 
    int iPos        = 0;
    NETMSGAES stRecvHead;

    memset(&stRecvHead, 0, sizeof(NETMSGAES));

    // 6 -> sizeof(WORD)+sizeof(int)
	iRet = recv(sockfd, (char*)(&stRecvHead), 6, flags);
	if (iRet <= 0)
	{
		return iRet;
	}

    if (NP_HEADFLAG != stRecvHead.wHeadFlag || stRecvHead.iDataLen <= 0)
    {
        return 0;
    }

    iLen = stRecvHead.iDataLen;

    if (iLen > iDataLen)
    {
        return 0;
    }

    iRecvLen = iLen;

    while (iRecvLen > 0)
    {
        iRet = recv(sockfd, pData+iPos, iRecvLen, flags);
        if (iRet <= 0)
        {
            return iRet;
        }
        iPos        += iRet;
        iRecvLen    -= iRet;
    }

    return iLen;

#endif
}


int AESCryptTS(char* pData, int iDataLen)
{
    int i = 0;
    int j = iDataLen / 160;
    aes_ctx m_ectDec;	// 解密密钥
    unsigned char key[]="1234567890123456";
    if (!pData || iDataLen <= 0)
        return 0;   
		
	aes_dec_key(key, 16, &m_ectDec);

    for (i = 0; i < j; i++) 
    {
        aes_dec_blk((unsigned char *)pData+i*16, (unsigned char *)pData+i*16, &m_ectDec);
    }
    return 1;
}

