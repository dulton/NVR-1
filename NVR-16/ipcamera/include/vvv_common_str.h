
#ifndef __VVV_COMMON_STR_H__
#define __VVV_COMMON_STR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
    
#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#else

#endif
/*从字符串中获取变量值
VARAAA=VALVVV
VARBBB:VALVVV
*/
int vvv_str_getvalue(char* str, char* var, char split,  char* val);
int vvv_str_getvalue_int(char* str, char* var, char split,  int* val);
/*从字符串中获取变量值
VARAAA=VALVVV
VARBBB:VALVVV
判断的是VARAAA=, VARBBB:*/
int vvv_str_getvalue2(char* str, char* var, char* val);

/*
var name=bbb;
var city=shenzhen;
*/

int vvv_str_getcgival(char *buf, int buflen, char *name, char *value);
char* vvv_str_getline(char* buf, char* line);

/*
AA:BB 分离为两个值, AA和BB
str 扫描时, 遇到 \r or \n or \0 停止扫描, 不过滤空格
*/
int vvv_str_split(char* str, char split, char* v1, char* v2);
    
#ifdef _WIN32
#define snprintf _snprintf
#else
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif



