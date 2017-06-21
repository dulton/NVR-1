#ifndef _GEOMETRIC_H_
#define _GEOMETRIC_H_

#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    几何相关的结构体
*/

//矩形结构体
typedef struct
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;
} SRect, *SPRect;

//点结构体
typedef struct
{
    s32 x;  //点坐标x
    s32 y; //点坐标y
} SPoint;

#ifdef __cplusplus
}
#endif

#endif
