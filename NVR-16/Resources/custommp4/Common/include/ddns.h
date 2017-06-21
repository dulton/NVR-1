#ifndef DDNS_H_
#define DDNS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_OK (0)
#define ERR_SOCKET (-1)
#define ERR_SOCKET_TIMEOUT (-2)

int  SHX_GetLastError(void); //内部错误描述并不全面，不要调用这个函数
int  SHX_Init(void); // 0代表失败, 1代表成功
int  SHX_ReqDomain(const char *userid, int userlen,const char *password ,int passlen,char * serialnumber,int seriallen,char * key,int keylen); // 0代表失败, 1代表成功
void SHX_ReqStartClient(const char *userid, int userlen,const char *password ,int passlen);
void SHX_ReqStopClient(const char *userid, int userlen,const char *password ,int passlen);
void SHX_UnInit(void);

/*
只申请域名的函数调用顺序为 1)SHX_Init(初始化) 2)SHX_ReqDomain(申请) 3)SHX_UnInit(反初始化)
域名已经申请，只需要汇报ip的函数调用顺序为 1)SHX_Init(初始化) 2)SHX_ReqStartClient(开始解析) 3)SHX_ReqStopClient(停止解析) 4)SHX_UnInit(反初始化)
申请域名并立即解析 1)SHX_Init 2)SHX_ReqDomain 3)SHX_ReqStartClient 4)SHX_ReqStopClient 5)SHX_UnInit
*/
#ifdef __cplusplus
}
#endif

#endif /*DDNS_H_*/
