#ifndef _UNICODEDEF_H_
#define _UNICODEDEF_H_

//by cj@20100703

#include <stdio.h>

//#define USE_UNICODE

#ifndef USE_UNICODE
//MBCS
#define _T(x)   x
#define LPSTR   char *
#define LPCSTR  const char *
#define TCHAR	char
#define printf	printf
#define fopen	fopen
#define fputs	fputs
#define strlen	strlen
#define strncpy	strncpy
#define fgets	fgets
#define strstr	strstr
#define strcmp	strcmp
#define strchr	strchr
#define sprintf	sprintf
#define strtok	strtok
#define strcpy	strcpy
#define strcat	strcat
#define atoi	atoi

#else
//UNICODE
#define _T(x)   L ## x
#define LPSTR   wchar_t *
#define LPCSTR  const wchar_t *
#define TCHAR	wchar_t
#define	printf	wprintf
#define fopen	_wfopen
#define fputs	fputws
#define strlen	wcslen
#define	strncpy	wcsncpy
#define	fgets	fgetws
#define strstr	wcsstr
#define strcmp	wcscmp
#define strchr	wcschr
#define sprintf	swprintf
#define strtok	wcstok
#define strcpy	wcscpy
#define strcat	wcscat
#define atoi	_wtoi

#endif

#endif

