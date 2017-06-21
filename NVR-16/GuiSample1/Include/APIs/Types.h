
#ifndef __XC_TYPES_H__
#define __XC_TYPES_H__

//////////////////////////////////////////////////////////////////////////
// base types definition
// compilers should be restricted to ensure the following equalities!
typedef signed char				schar;	///< sizeof(uchar) == sizeof(schar) == sizeof(char) == 1
typedef unsigned char			uchar;	
typedef unsigned int			uint;	///< sizeof(uint) == sizeof(int) == 4
typedef unsigned short			ushort;	///< sizeof(ushort) == sizeof(short) == 2
typedef unsigned long			ulong;	///< sizeof(ulong) == sizeof(long) == 4
#ifdef WIN32
typedef __int64					int64;	///< sizeof(int64) == sizeof(uint64) == 8
typedef unsigned __int64		uint64;
#elif defined(__GNUC__)
typedef long long				int64;
typedef unsigned long long		uint64;
#elif defined(__TCS__)
typedef signed   long long int	int64;
typedef unsigned long long int	uint64;
#endif
//typedef void *					VD_HANDLE;
//typedef int						VD_BOOL;

#endif// __DH_TYPES_H__

