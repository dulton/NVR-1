#ifndef _CRYPTAES_H_
#define _CRYPTAES_H_

#include "aesopt.h"

int AESSocketSend(int sockfd, char* pData, int iDataLen, int flags);
int AESSocketRecv(int sockfd, char* pData, int iDataLen, int flags);

int AESCryptTS(char* pData, int iDataLen);

#endif
