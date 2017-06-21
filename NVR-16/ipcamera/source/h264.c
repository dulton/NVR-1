#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>

#include <errno.h>

/* error handling */
#if EDOM > 0
#define AVERROR(e)		(-(e))///< Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e)	(-(e))///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define AVERROR(e)		(e)
#define AVUNERROR(e)	(e)
#endif

#define ALT_BITSTREAM_READER
//#define ALT_BITSTREAM_READER_LE

#define av_unused
#define av_const
#define av_always_inline inline

void av_free(void *ptr)
{
	if(ptr)
	{
		free(ptr);
		ptr = NULL;
	}
}

//typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

#define FF_ARRAY_ELEMS(a)	(sizeof(a) / sizeof((a)[0]))

#define MAX_PICTURE_COUNT	32

#define EXTENDED_SAR		255

const uint8_t ff_golomb_vlc_len[512]=
{
	19,17,15,15,13,13,13,13,11,11,11,11,11,11,11,11,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

const uint8_t ff_ue_golomb_vlc_code[512]=
{
	32,32,32,32,32,32,32,32,31,32,32,32,32,32,32,32,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
	7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int8_t ff_se_golomb_vlc_code[512]=
{
	17, 17, 17, 17, 17, 17, 17, 17, 16, 17, 17, 17, 17, 17, 17, 17,  8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,
	4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

const uint8_t ff_ue_golomb_len[256]=
{
	 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,11,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,
};

const uint8_t ff_interleaved_golomb_vlc_len[256]=
{
	9,9,7,7,9,9,7,7,5,5,5,5,5,5,5,5,
	9,9,7,7,9,9,7,7,5,5,5,5,5,5,5,5,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	9,9,7,7,9,9,7,7,5,5,5,5,5,5,5,5,
	9,9,7,7,9,9,7,7,5,5,5,5,5,5,5,5,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

const uint8_t ff_interleaved_ue_golomb_vlc_code[256]=
{
	15,16,7, 7, 17,18,8, 8, 3, 3, 3, 3, 3, 3, 3, 3,
	19,20,9, 9, 21,22,10,10,4, 4, 4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	23,24,11,11,25,26,12,12,5, 5, 5, 5, 5, 5, 5, 5,
	27,28,13,13,29,30,14,14,6, 6, 6, 6, 6, 6, 6, 6,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const int8_t ff_interleaved_se_golomb_vlc_code[256]=
{
	8, -8,  4,  4,  9, -9, -4, -4,  2,  2,  2,  2,  2,  2,  2,  2,
	10,-10,  5,  5, 11,-11, -5, -5, -2, -2, -2, -2, -2, -2, -2, -2,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	12,-12,  6,  6, 13,-13, -6, -6,  3,  3,  3,  3,  3,  3,  3,  3,
	14,-14,  7,  7, 15,-15, -7, -7, -3, -3, -3, -3, -3, -3, -3, -3,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

const uint8_t ff_interleaved_dirac_golomb_vlc_code[256]=
{
	0, 1, 0, 0, 2, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 5, 2, 2, 6, 7, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 9, 4, 4, 10,11,5, 5, 2, 2, 2, 2, 2, 2, 2, 2,
	12,13,6, 6, 14,15,7, 7, 3, 3, 3, 3, 3, 3, 3, 3,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const uint8_t ff_log2_tab[256]=
{
	0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static const uint8_t default_scaling4[2][16]=
{
	{
		6,13,20,28,
		13,20,28,32,
		20,28,32,37,
		28,32,37,42
	},
	{
		10,14,20,24,
		14,20,24,27,
		20,24,27,30,
		24,27,30,34
	}
};

static const uint8_t default_scaling8[2][64]=
{
	{
		 6,10,13,16,18,23,25,27,
		10,11,16,18,23,25,27,29,
		13,16,18,23,25,27,29,31,
		16,18,23,25,27,29,31,33,
		18,23,25,27,29,31,33,36,
		23,25,27,29,31,33,36,38,
		25,27,29,31,33,36,38,40,
		27,29,31,33,36,38,40,42
	},
	{
		 9,13,15,17,19,21,22,24,
		13,13,17,19,21,22,24,25,
		15,17,19,21,22,24,25,27,
		17,19,21,22,24,25,27,28,
		19,21,22,24,25,27,28,30,
		21,22,24,25,27,28,30,32,
		22,24,25,27,28,30,32,33,
		24,25,27,28,30,32,33,35
	}
};

static const uint8_t zigzag_scan[16]=
{
	0+0*4, 1+0*4, 0+1*4, 0+2*4,
	1+1*4, 2+0*4, 3+0*4, 2+1*4,
	1+2*4, 0+3*4, 1+3*4, 2+2*4,
	3+1*4, 3+2*4, 2+3*4, 3+3*4,
};

const uint8_t ff_zigzag_direct[64]=
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

#ifndef AV_RL32
#define AV_RL32(x)\
	(((uint32_t)((const uint8_t*)(x))[3] << 24)|\
	(((const uint8_t*)(x))[2] << 16)|\
	(((const uint8_t*)(x))[1] <<  8)|\
	((const uint8_t*)(x))[0])
#endif

#ifndef AV_RB32
#define AV_RB32(x)\
	(((uint32_t)((const uint8_t*)(x))[0] << 24)|\
	(((const uint8_t*)(x))[1] << 16)|\
	(((const uint8_t*)(x))[2] <<  8)|\
	((const uint8_t*)(x))[3])
#endif

/**
 * The maximum number of slices supported by the decoder.
 * must be a power of 2
 */
#define MAX_SLICES					16

#define MAX_SPS_COUNT				32
#define MAX_PPS_COUNT				256

#define MAX_THREADS					16

#define QP_MAX_NUM					(51 + 4*6)// The maximum supported qp

#define CHROMA444					(sps->chroma_format_idc == 3)

#define FFMIN(a,b)					((a) > (b) ? (b) : (a))

/**
 * rational number numerator/denominator
 */
typedef struct AVRational
{
	int num; ///< numerator
	int den; ///< denominator
}AVRational;

enum AVChromaLocation
{
	AVCHROMA_LOC_UNSPECIFIED=0,
	AVCHROMA_LOC_LEFT       =1, ///< mpeg2/4, h264 default
	AVCHROMA_LOC_CENTER     =2, ///< mpeg1, jpeg, h263
	AVCHROMA_LOC_TOPLEFT    =3, ///< DV
	AVCHROMA_LOC_TOP        =4,
	AVCHROMA_LOC_BOTTOMLEFT =5,
	AVCHROMA_LOC_BOTTOM     =6,
	AVCHROMA_LOC_NB           , ///< Not part of ABI
};

enum AVColorPrimaries
{
	AVCOL_PRI_BT709      =1, ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
	AVCOL_PRI_UNSPECIFIED=2,
	AVCOL_PRI_BT470M     =4,
	AVCOL_PRI_BT470BG    =5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
	AVCOL_PRI_SMPTE170M  =6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	AVCOL_PRI_SMPTE240M  =7, ///< functionally identical to above
	AVCOL_PRI_FILM       =8,
	AVCOL_PRI_NB           , ///< Not part of ABI
};

enum AVColorTransferCharacteristic
{
	AVCOL_TRC_BT709      =1, ///< also ITU-R BT1361
	AVCOL_TRC_UNSPECIFIED=2,
	AVCOL_TRC_GAMMA22    =4, ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
	AVCOL_TRC_GAMMA28    =5, ///< also ITU-R BT470BG
	AVCOL_TRC_NB           , ///< Not part of ABI
};

enum AVColorSpace
{
	AVCOL_SPC_RGB        =0,
	AVCOL_SPC_BT709      =1, ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
	AVCOL_SPC_UNSPECIFIED=2,
	AVCOL_SPC_FCC        =4,
	AVCOL_SPC_BT470BG    =5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
	AVCOL_SPC_SMPTE170M  =6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
	AVCOL_SPC_SMPTE240M  =7,
	AVCOL_SPC_NB           , ///< Not part of ABI
};

/* NAL unit types */
enum
{
	NAL_SLICE=1,
	NAL_DPA,
	NAL_DPB,
	NAL_DPC,
	NAL_IDR_SLICE,
	NAL_SEI,
	NAL_SPS,
	NAL_PPS,
	NAL_AUD,
	NAL_END_SEQUENCE,
	NAL_END_STREAM,
	NAL_FILLER_DATA,
	NAL_SPS_EXT,
	NAL_AUXILIARY_SLICE=19
};

/**
 * Sequence parameter set
 */
typedef struct SPS
{
	int profile_idc;
	int level_idc;
	int chroma_format_idc;
	int transform_bypass;              ///< qpprime_y_zero_transform_bypass_flag
	int log2_max_frame_num;            ///< log2_max_frame_num_minus4 + 4
	int poc_type;                      ///< pic_order_cnt_type
	int log2_max_poc_lsb;              ///< log2_max_pic_order_cnt_lsb_minus4
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int poc_cycle_length;              ///< num_ref_frames_in_pic_order_cnt_cycle
	int ref_frame_count;               ///< num_ref_frames
	int gaps_in_frame_num_allowed_flag;
	int mb_width;                      ///< pic_width_in_mbs_minus1 + 1
	int mb_height;                     ///< pic_height_in_map_units_minus1 + 1
	int frame_mbs_only_flag;
	int mb_aff;                        ///<mb_adaptive_frame_field_flag
	int direct_8x8_inference_flag;
	int crop;                   ///< frame_cropping_flag
	unsigned int crop_left;            ///< frame_cropping_rect_left_offset
	unsigned int crop_right;           ///< frame_cropping_rect_right_offset
	unsigned int crop_top;             ///< frame_cropping_rect_top_offset
	unsigned int crop_bottom;          ///< frame_cropping_rect_bottom_offset
	int vui_parameters_present_flag;
	AVRational sar;
	int video_signal_type_present_flag;
	int full_range;
	int colour_description_present_flag;
	enum AVColorPrimaries color_primaries;
	enum AVColorTransferCharacteristic color_trc;
	enum AVColorSpace colorspace;
	int timing_info_present_flag;
	uint32_t num_units_in_tick;
	uint32_t time_scale;
	int fixed_frame_rate_flag;
	short offset_for_ref_frame[256]; //FIXME dyn aloc?
	int bitstream_restriction_flag;
	int num_reorder_frames;
	int scaling_matrix_present;
	uint8_t scaling_matrix4[6][16];
	uint8_t scaling_matrix8[6][64];
	int nal_hrd_parameters_present_flag;
	int vcl_hrd_parameters_present_flag;
	int pic_struct_present_flag;
	int time_offset_length;
	int cpb_cnt;                       ///< See H.264 E.1.2
	int initial_cpb_removal_delay_length; ///< initial_cpb_removal_delay_length_minus1 +1
	int cpb_removal_delay_length;      ///< cpb_removal_delay_length_minus1 + 1
	int dpb_output_delay_length;       ///< dpb_output_delay_length_minus1 + 1
	int bit_depth_luma;                ///< bit_depth_luma_minus8 + 8
	int bit_depth_chroma;              ///< bit_depth_chroma_minus8 + 8
	int residual_color_transform_flag; ///< residual_colour_transform_flag
	int constraint_set_flags;          ///< constraint_set[0-3]_flag
}SPS;

/**
 * Picture parameter set
 */
typedef struct PPS
{
	unsigned int sps_id;
	int cabac;                  ///< entropy_coding_mode_flag
	int pic_order_present;      ///< pic_order_present_flag
	int slice_group_count;      ///< num_slice_groups_minus1 + 1
	int mb_slice_group_map_type;
	unsigned int ref_count[2];  ///< num_ref_idx_l0/1_active_minus1 + 1
	int weighted_pred;          ///< weighted_pred_flag
	int weighted_bipred_idc;
	int init_qp;                ///< pic_init_qp_minus26 + 26
	int init_qs;                ///< pic_init_qs_minus26 + 26
	int chroma_qp_index_offset[2];
	int deblocking_filter_parameters_present; ///< deblocking_filter_parameters_present_flag
	int constrained_intra_pred; ///< constrained_intra_pred_flag
	int redundant_pic_cnt_present; ///< redundant_pic_cnt_present_flag
	int transform_8x8_mode;     ///< transform_8x8_mode_flag
	uint8_t scaling_matrix4[6][16];
	uint8_t scaling_matrix8[6][64];
	uint8_t chroma_qp_table[2][QP_MAX_NUM+1];  ///< pre-scaled (with chroma_qp_index_offset) version of qp_table
	int chroma_qp_diff;
}PPS;

static const AVRational pixel_aspect[17]=
{
	{0, 1},
	{1, 1},
	{12, 11},
	{10, 11},
	{16, 11},
	{40, 33},
	{24, 11},
	{20, 11},
	{32, 11},
	{80, 33},
	{18, 11},
	{15, 11},
	{64, 33},
	{160,99},
	{4, 3},
	{3, 2},
	{2, 1},
};

static av_always_inline av_const int av_log2(unsigned int v)
{
	int n = 0;
	if(v & 0xffff0000)
	{
		v >>= 16;
		n += 16;
	}
	if(v & 0xff00)
	{
		v >>= 8;
		n += 8;
	}
	n += ff_log2_tab[v];
	
	return n;
}

static inline av_const int sign_extend(int val, unsigned bits)
{
	return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

static inline av_const unsigned zero_extend(unsigned val, unsigned bits)
{
	return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

#define NEG_SSR32(a,s) (((int32_t)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))

typedef struct GetBitContext
{
    const uint8_t *buffer;
    int index;
    int size_in_bits;
}GetBitContext;

#define MIN_CACHE_BITS	25

#define OPEN_READER(name, gb)\
	unsigned int name##_index = (gb)->index;\
	av_unused unsigned int name##_cache

#define CLOSE_READER(name, gb) (gb)->index = name##_index

#ifdef ALT_BITSTREAM_READER_LE
	#define UPDATE_CACHE(name, gb)\
		name##_cache = AV_RL32(((const uint8_t *)(gb)->buffer)+(name##_index>>3)) >> (name##_index&0x07)
	
	#define SKIP_CACHE(name, gb, num) name##_cache >>= (num)
#else
	#define UPDATE_CACHE(name, gb)\
		name##_cache = AV_RB32(((const uint8_t *)(gb)->buffer)+(name##_index>>3)) << (name##_index&0x07)
	
	#define SKIP_CACHE(name, gb, num) name##_cache <<= (num)
#endif

//FIXME name?
#define SKIP_COUNTER(name, gb, num) name##_index += (num)

#define SKIP_BITS(name, gb, num)\
	do{\
		SKIP_CACHE(name, gb, num);\
		SKIP_COUNTER(name, gb, num);\
	}while(0)

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#define LAST_SKIP_CACHE(name, gb, num)

#ifdef ALT_BITSTREAM_READER_LE
	#define SHOW_UBITS(name, gb, num) zero_extend(name##_cache, num)
	
	#define SHOW_SBITS(name, gb, num) sign_extend(name##_cache, num)
#else
	#define SHOW_UBITS(name, gb, num) NEG_USR32(name##_cache, num)
	
	#define SHOW_SBITS(name, gb, num) NEG_SSR32(name##_cache, num)
#endif

#define GET_CACHE(name, gb)	((uint32_t)name##_cache)

static inline unsigned int get_bits(GetBitContext *s, int n)
{
	register int tmp;
	OPEN_READER(re, s);
	UPDATE_CACHE(re, s);
	tmp = SHOW_UBITS(re, s, n);
	LAST_SKIP_BITS(re, s, n);
	CLOSE_READER(re, s);
	return tmp;
}

static inline unsigned int get_bits1(GetBitContext *s)
{
#ifdef ALT_BITSTREAM_READER
	unsigned int index = s->index;
	uint8_t result = s->buffer[index>>3];
#ifdef ALT_BITSTREAM_READER_LE
	result >>= index & 7;
	result &= 1;
#else
	result <<= index & 7;
	result >>= 8 - 1;
#endif
	index++;
	s->index = index;
	
	return result;
#else
	return get_bits(s, 1);
#endif
}

static inline unsigned int get_bits_long(GetBitContext *s, int n)
{
	if(n <= MIN_CACHE_BITS)
	{
		return get_bits(s, n);
	}
	else
	{
#ifdef ALT_BITSTREAM_READER_LE
		int ret = get_bits(s, 16);
		return ret | (get_bits(s, n-16) << 16);
#else
		int ret = get_bits(s, 16) << (n-16);
		return ret | get_bits(s, n-16);
#endif
	}
}

static inline unsigned int show_bits(GetBitContext *s, int n)
{
	register int tmp;
	OPEN_READER(re, s);
	UPDATE_CACHE(re, s);
	tmp = SHOW_UBITS(re, s, n);
	return tmp;
}

static inline unsigned int show_bits_long(GetBitContext *s, int n)
{
	if(n <= MIN_CACHE_BITS)
	{
		return show_bits(s, n);
	}
	else
	{
		GetBitContext b = *s;
		return get_bits_long(&b, n);
	}
}

static inline int get_bits_count(const GetBitContext *s)
{
	return s->index;
}

static inline void skip_bits_long(GetBitContext *s, int n)
{
	s->index += n;
}

static inline int get_ue_golomb(GetBitContext *gb)
{
	unsigned int buf;
	int log;
	
	OPEN_READER(re, gb);
	UPDATE_CACHE(re, gb);
	buf=GET_CACHE(re, gb);
	
	if(buf >= (1<<27))
	{
		buf >>= 32 - 9;
		LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
		CLOSE_READER(re, gb);
		
		return ff_ue_golomb_vlc_code[buf];
	}
	else
	{
		log = 2*av_log2(buf) - 31;
		buf >>= log;
		buf--;
		LAST_SKIP_BITS(re, gb, 32 - log);
		CLOSE_READER(re, gb);
		
		return buf;
	}
}

static inline unsigned get_ue_golomb_long(GetBitContext *gb)
{
	unsigned buf, log;
	
	buf = show_bits_long(gb, 32);
	log = 31 - av_log2(buf);
	skip_bits_long(gb, log);
	
	return get_bits_long(gb, log + 1) - 1;
}

static inline int get_ue_golomb_31(GetBitContext *gb)
{
	unsigned int buf;
	
	OPEN_READER(re, gb);
	UPDATE_CACHE(re, gb);
	buf=GET_CACHE(re, gb);
	
	buf >>= 32 - 9;
	LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
	CLOSE_READER(re, gb);
	
	return ff_ue_golomb_vlc_code[buf];
}

static inline int get_se_golomb(GetBitContext *gb)
{
	unsigned int buf;
	int log;
	
	OPEN_READER(re, gb);
	UPDATE_CACHE(re, gb);
	buf=GET_CACHE(re, gb);
	
	if(buf >= (1<<27))
	{
		buf >>= 32 - 9;
		LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
		CLOSE_READER(re, gb);
		
		return ff_se_golomb_vlc_code[buf];
	}
	else
	{
		log = 2*av_log2(buf) - 31;
		buf >>= log;
		
		LAST_SKIP_BITS(re, gb, 32 - log);
		CLOSE_READER(re, gb);
		
		if(buf&1)
		{
			buf = -(buf>>1);
		}
		else
		{
			buf = (buf>>1);
		}
		
		return buf;
	}
}

static void decode_scaling_list(GetBitContext *gb, uint8_t *factors, int size, const uint8_t *jvt_list, const uint8_t *fallback_list)
{
	int i, last = 8, next = 8;
	const uint8_t *scan = size == 16 ? zigzag_scan : ff_zigzag_direct;
	if(!get_bits1(gb))/* matrix not written, we use the predicted one */
	{
		memcpy(factors, fallback_list, size*sizeof(uint8_t));
	}
	else
	{
		for(i=0;i<size;i++)
		{
			if(next)
			{
				next = (last + get_se_golomb(gb)) & 0xff;
			}
			if(!i && !next)
			{
				/* matrix not written, we use the preset one */
				memcpy(factors, jvt_list, size*sizeof(uint8_t));
				break;
			}
			last = factors[scan[i]] = next ? next : last;
		}
	}
}

static void decode_scaling_matrices(GetBitContext *gb, SPS *sps, PPS *pps, int is_sps, uint8_t(*scaling_matrix4)[16], uint8_t(*scaling_matrix8)[64])
{
	int fallback_sps = !is_sps && sps->scaling_matrix_present;
	const uint8_t *fallback[4] = 
	{
		fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
		fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
		fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
		fallback_sps ? sps->scaling_matrix8[3] : default_scaling8[1]
	};
	if(get_bits1(gb))
	{
		sps->scaling_matrix_present |= is_sps;
		decode_scaling_list(gb,scaling_matrix4[0],16,default_scaling4[0],fallback[0]); // Intra, Y
		decode_scaling_list(gb,scaling_matrix4[1],16,default_scaling4[0],scaling_matrix4[0]); // Intra, Cr
		decode_scaling_list(gb,scaling_matrix4[2],16,default_scaling4[0],scaling_matrix4[1]); // Intra, Cb
		decode_scaling_list(gb,scaling_matrix4[3],16,default_scaling4[1],fallback[1]); // Inter, Y
		decode_scaling_list(gb,scaling_matrix4[4],16,default_scaling4[1],scaling_matrix4[3]); // Inter, Cr
		decode_scaling_list(gb,scaling_matrix4[5],16,default_scaling4[1],scaling_matrix4[4]); // Inter, Cb
		if(is_sps || pps->transform_8x8_mode)
		{
			decode_scaling_list(gb,scaling_matrix8[0],64,default_scaling8[0],fallback[2]);  // Intra, Y
			if(sps->chroma_format_idc == 3)
			{
				decode_scaling_list(gb,scaling_matrix8[1],64,default_scaling8[0],scaling_matrix8[0]);  // Intra, Cr
				decode_scaling_list(gb,scaling_matrix8[2],64,default_scaling8[0],scaling_matrix8[1]);  // Intra, Cb
			}
			decode_scaling_list(gb,scaling_matrix8[3],64,default_scaling8[1],fallback[3]);  // Inter, Y
			if(sps->chroma_format_idc == 3)
			{
				decode_scaling_list(gb,scaling_matrix8[4],64,default_scaling8[1],scaling_matrix8[3]);  // Inter, Cr
				decode_scaling_list(gb,scaling_matrix8[5],64,default_scaling8[1],scaling_matrix8[4]);  // Inter, Cb
			}
		}
	}
}

int av_image_check_size(unsigned int w, unsigned int h)
{
	if((int)w>0 && (int)h>0 && (w+128)*(uint64_t)(h+128) < INT_MAX/8)
	{
		return 0;
	}
	
	printf("Picture size %ux%u is invalid\n", w, h);
	return AVERROR(EINVAL);
}

static inline int decode_hrd_parameters(GetBitContext *gb, SPS *sps)
{
	int cpb_count, i;
	cpb_count = get_ue_golomb_31(gb) + 1;
	
	if(cpb_count > 32U)
	{
		printf("cpb_count %d invalid\n", cpb_count);
		return -1;
	}
	
	get_bits(gb, 4); /* bit_rate_scale */
	get_bits(gb, 4); /* cpb_size_scale */
	for(i=0; i<cpb_count; i++)
	{
		get_ue_golomb_long(gb);/* bit_rate_value_minus1 */
		get_ue_golomb_long(gb);/* cpb_size_value_minus1 */
		get_bits1(gb);/* cbr_flag */
	}
	sps->initial_cpb_removal_delay_length = get_bits(gb, 5) + 1;
	sps->cpb_removal_delay_length = get_bits(gb, 5) + 1;
	sps->dpb_output_delay_length = get_bits(gb, 5) + 1;
	sps->time_offset_length = get_bits(gb, 5);
	sps->cpb_cnt = cpb_count;
	return 0;
}

static inline int decode_vui_parameters(GetBitContext *gb, SPS *sps)
{
	int aspect_ratio_info_present_flag;
	unsigned int aspect_ratio_idc;
	
	aspect_ratio_info_present_flag = get_bits1(gb);
	
	if(aspect_ratio_info_present_flag)
	{
		aspect_ratio_idc = get_bits(gb, 8);
		if(aspect_ratio_idc == EXTENDED_SAR)
		{
			sps->sar.num = get_bits(gb, 16);
			sps->sar.den = get_bits(gb, 16);
		}
		else if(aspect_ratio_idc < FF_ARRAY_ELEMS(pixel_aspect))
		{
			sps->sar = pixel_aspect[aspect_ratio_idc];
		}
		else
		{
			printf("illegal aspect ratio\n");
			return -1;
		}
	}
	else
	{
		sps->sar.num = sps->sar.den = 0;
	}
	
	//s->avctx->aspect_ratio = sar_width*s->width / (float)(s->height*sar_height);
	
	if(get_bits1(gb))
	{
		/* overscan_info_present_flag */
		get_bits1(gb);/* overscan_appropriate_flag */
	}
	
	sps->video_signal_type_present_flag = get_bits1(gb);
	if(sps->video_signal_type_present_flag)
	{
		get_bits(gb, 3);/* video_format */
		sps->full_range = get_bits1(gb);/* video_full_range_flag */
		
		sps->colour_description_present_flag = get_bits1(gb);
		if(sps->colour_description_present_flag)
		{
			sps->color_primaries = get_bits(gb, 8);/* colour_primaries */
			sps->color_trc       = get_bits(gb, 8);/* transfer_characteristics */
			sps->colorspace      = get_bits(gb, 8);/* matrix_coefficients */
			if(sps->color_primaries >= AVCOL_PRI_NB)
			{
				sps->color_primaries = AVCOL_PRI_UNSPECIFIED;
			}
			if(sps->color_trc >= AVCOL_TRC_NB)
			{
				sps->color_trc = AVCOL_TRC_UNSPECIFIED;
			}
			if(sps->colorspace >= AVCOL_SPC_NB)
			{
				sps->colorspace  = AVCOL_SPC_UNSPECIFIED;
			}
		}
	}
	
	if(get_bits1(gb))
	{
		/* chroma_location_info_present_flag */
		enum AVChromaLocation chroma_sample_location = get_ue_golomb(gb)+1;/* chroma_sample_location_type_top_field */
		printf("chroma_sample_location=%d\n",chroma_sample_location);
		get_ue_golomb(gb);/* chroma_sample_location_type_bottom_field */
	}
	
	sps->timing_info_present_flag = get_bits1(gb);
	if(sps->timing_info_present_flag)
	{
		sps->num_units_in_tick = get_bits_long(gb, 32);
		sps->time_scale = get_bits_long(gb, 32);
		if(!sps->num_units_in_tick || !sps->time_scale)
		{
			printf("time_scale/num_units_in_tick invalid or unsupported (%d/%d)\n", sps->time_scale, sps->num_units_in_tick);
			return -1;
		}
		sps->fixed_frame_rate_flag = get_bits1(gb);
	}
	
	sps->nal_hrd_parameters_present_flag = get_bits1(gb);
	if(sps->nal_hrd_parameters_present_flag)
		if(decode_hrd_parameters(gb, sps) < 0)
			return -1;
	sps->vcl_hrd_parameters_present_flag = get_bits1(gb);
	if(sps->vcl_hrd_parameters_present_flag)
		if(decode_hrd_parameters(gb, sps) < 0)
			return -1;
	if(sps->nal_hrd_parameters_present_flag || sps->vcl_hrd_parameters_present_flag)
	{
		get_bits1(gb);/* low_delay_hrd_flag */
	}
	sps->pic_struct_present_flag = get_bits1(gb);
	
	sps->bitstream_restriction_flag = get_bits1(gb);
	if(sps->bitstream_restriction_flag)
	{
		get_bits1(gb);     /* motion_vectors_over_pic_boundaries_flag */
		get_ue_golomb(gb); /* max_bytes_per_pic_denom */
		get_ue_golomb(gb); /* max_bits_per_mb_denom */
		get_ue_golomb(gb); /* log2_max_mv_length_horizontal */
		get_ue_golomb(gb); /* log2_max_mv_length_vertical */
		sps->num_reorder_frames = get_ue_golomb(gb);
		get_ue_golomb(gb); /*max_dec_frame_buffering*/
		
		if(gb->size_in_bits < get_bits_count(gb))
		{
			printf("Overread VUI by %d bits\n", get_bits_count(gb) - gb->size_in_bits);
			sps->num_reorder_frames = 0;
			sps->bitstream_restriction_flag = 0;
		}
		
		if(sps->num_reorder_frames > 16U /*max_dec_frame_buffering || max_dec_frame_buffering > 16*/)
		{
			printf("illegal num_reorder_frames %d\n", sps->num_reorder_frames);
			return -1;
		}
	}
	
	return 0;
}

int ff_h264_decode_seq_parameter_set(unsigned char*in_buf,unsigned int buffer_size,int *h,int *w,int *num_ref_frames)
{
	int profile_idc, level_idc, constraint_set_flags = 0;
	unsigned int sps_id;
	int i;
	
	GetBitContext b;
	b.buffer = in_buf;
	b.size_in_bits = buffer_size*8;
	b.index = 0;
	
	GetBitContext *gb = &b;
	
	SPS *sps = NULL;
	
	*w = 0;
	*h = 0;
	*num_ref_frames = 0;
	
	profile_idc = get_bits(gb, 8);
	//printf("ff_h264_decode_seq_parameter_set:profile_idc=%d\n",profile_idc);
	
	constraint_set_flags |= get_bits1(gb) << 0;//constraint_set0_flag
	constraint_set_flags |= get_bits1(gb) << 1;//constraint_set1_flag
	constraint_set_flags |= get_bits1(gb) << 2;//constraint_set2_flag
	constraint_set_flags |= get_bits1(gb) << 3;//constraint_set3_flag
	
	get_bits(gb, 4);//reserved
	
	level_idc = get_bits(gb, 8);
	//printf("set=%d level_idc=%d\n",constraint_set_flags,level_idc);
	
	sps_id = get_ue_golomb_31(gb);
	//printf("sps_id=%d\n",sps_id);
	
	if(sps_id >= MAX_SPS_COUNT)
	{
		printf("sps_id (%d) out of range\n", sps_id);
		return -1;
	}
	
	sps = malloc(sizeof(SPS));
	if(sps == NULL)
	{
		return -1;
	}
	
	sps->time_offset_length = 24;
	sps->profile_idc = profile_idc;
	sps->constraint_set_flags = constraint_set_flags;
	sps->level_idc = level_idc;
	
	memset(sps->scaling_matrix4, 16, sizeof(sps->scaling_matrix4));
	memset(sps->scaling_matrix8, 16, sizeof(sps->scaling_matrix8));
	sps->scaling_matrix_present = 0;
	
	if(sps->profile_idc >= 100)
	{
		//high profile
		sps->chroma_format_idc = get_ue_golomb_31(gb);
		if(sps->chroma_format_idc > 3U)
		{
			printf("chroma_format_idc %d is illegal\n", sps->chroma_format_idc);
			goto fail;
		}
		if(sps->chroma_format_idc == 3)
		{
			sps->residual_color_transform_flag = get_bits1(gb);
		}
		sps->bit_depth_luma   = get_ue_golomb(gb) + 8;
		sps->bit_depth_chroma = get_ue_golomb(gb) + 8;
		if(sps->bit_depth_luma > 12U || sps->bit_depth_chroma > 12U)
		{
			printf("illegal bit depth value (%d, %d)\n",
			sps->bit_depth_luma, sps->bit_depth_chroma);
			goto fail;
		}
		sps->transform_bypass = get_bits1(gb);
		decode_scaling_matrices(gb, sps, NULL, 1, sps->scaling_matrix4, sps->scaling_matrix8);
	}
	else
	{
		sps->chroma_format_idc = 1;
		sps->bit_depth_luma    = 8;
		sps->bit_depth_chroma  = 8;
	}
	
	//printf("index=%d\n",gb->index);
	
	sps->log2_max_frame_num = get_ue_golomb(gb) + 4;
	//printf("log2_max_frame_num=%d\n",sps->log2_max_frame_num);
	
	sps->poc_type = get_ue_golomb_31(gb);
	//printf("poc_type=%d\n",sps->poc_type);
	
	if(sps->poc_type == 0)
	{
		//FIXME #define
		sps->log2_max_poc_lsb = get_ue_golomb(gb) + 4;
	}
	else if(sps->poc_type == 1)
	{
		//FIXME #define
		sps->delta_pic_order_always_zero_flag = get_bits1(gb);
		sps->offset_for_non_ref_pic = get_se_golomb(gb);
		sps->offset_for_top_to_bottom_field = get_se_golomb(gb);
		sps->poc_cycle_length = get_ue_golomb(gb);
		
		if((unsigned)sps->poc_cycle_length >= FF_ARRAY_ELEMS(sps->offset_for_ref_frame))
		{
			printf("poc_cycle_length overflow %u\n", sps->poc_cycle_length);
			goto fail;
		}
		
		for(i=0; i<sps->poc_cycle_length; i++)
		{
			sps->offset_for_ref_frame[i] = get_se_golomb(gb);
		}
    }
	else if(sps->poc_type != 2)
	{
		printf("illegal POC type %d\n", sps->poc_type);
		goto fail;
	}
	
	sps->ref_frame_count = get_ue_golomb_31(gb);
	if(sps->ref_frame_count > MAX_PICTURE_COUNT-2 || sps->ref_frame_count > 16U)
	{
		printf("too many reference frames\n");
		goto fail;
	}
	sps->gaps_in_frame_num_allowed_flag = get_bits1(gb);
	sps->mb_width  = get_ue_golomb(gb) + 1;
	sps->mb_height = get_ue_golomb(gb) + 1;
	if((unsigned)sps->mb_width >= INT_MAX/16 || (unsigned)sps->mb_height >= INT_MAX/16 || av_image_check_size(16*sps->mb_width, 16*sps->mb_height))
	{
		printf("mb_width/height overflow\n");
		goto fail;
	}
	
	sps->frame_mbs_only_flag = get_bits1(gb);
	if(!sps->frame_mbs_only_flag)
	{
		sps->mb_aff = get_bits1(gb);
	}
	else
	{
		sps->mb_aff = 0;
	}
	
	sps->direct_8x8_inference_flag = get_bits1(gb);
	if(!sps->frame_mbs_only_flag && !sps->direct_8x8_inference_flag)
	{
		printf("This stream was generated by a broken encoder, invalid 8x8 inference\n");
		goto fail;
	}
	
#ifndef ALLOW_INTERLACE
	if(sps->mb_aff)
	{
		printf("MBAFF support not included; enable it at compile-time.\n");
	}
#endif
	
	sps->crop = get_bits1(gb);
	if(sps->crop)
	{
		int crop_limit   = sps->chroma_format_idc == 3 ? 16 : 8;
		sps->crop_left   = get_ue_golomb(gb);
		sps->crop_right  = get_ue_golomb(gb);
		sps->crop_top    = get_ue_golomb(gb);
		sps->crop_bottom = get_ue_golomb(gb);
		if(sps->crop_left || sps->crop_top)
		{
			printf("insane cropping not completely supported, this could look slightly wrong ...\n");
		}
		if(sps->crop_right >= crop_limit || sps->crop_bottom >= crop_limit)
		{
			printf("brainfart cropping not supported, this could look slightly wrong ...\n");
		}
	}
	else
	{
		sps->crop_left = sps->crop_right = sps->crop_top = sps->crop_bottom = 0;
	}
	
	sps->vui_parameters_present_flag = get_bits1(gb);
	if(sps->vui_parameters_present_flag)
	{
		if(decode_vui_parameters(gb, sps) < 0)
		{
			goto fail;
		}
	}
	
	if(!sps->sar.den)
	{
		sps->sar.den = 1;
	}
	
	if(0)
	{
		printf("sps:%u profile:%d/%d poc:%d ref:%d %dx%d %s %s crop:%d/%d/%d/%d %s %s %d/%d b%d\n",
				sps_id, sps->profile_idc, sps->level_idc,
				sps->poc_type,
				sps->ref_frame_count,
				sps->mb_width, sps->mb_height,
				sps->frame_mbs_only_flag ? "FRM" : (sps->mb_aff ? "MB-AFF" : "PIC-AFF"),
				sps->direct_8x8_inference_flag ? "8B8" : "",
				sps->crop_left, sps->crop_right,
				sps->crop_top, sps->crop_bottom,
				sps->vui_parameters_present_flag ? "VUI" : "",
				((const char*[]){"Gray","420","422","444"})[sps->chroma_format_idc],
				sps->timing_info_present_flag ? sps->num_units_in_tick : 0,
				sps->timing_info_present_flag ? sps->time_scale : 0,
				sps->bit_depth_luma);
	}
	
	int width = 0;
	int height = 0;
	
	sps->mb_width  = sps->mb_width;
	sps->mb_height = sps->mb_height * (2 - sps->frame_mbs_only_flag);
	
	width = 16*sps->mb_width - (2>>CHROMA444)*FFMIN(sps->crop_right, (8<<CHROMA444)-1);
	if(sps->frame_mbs_only_flag)
	{
		height = 16*sps->mb_height - (2>>CHROMA444)*FFMIN(sps->crop_bottom, (8<<CHROMA444)-1);
	}
	else
	{
		height = 16*sps->mb_height - (4>>CHROMA444)*FFMIN(sps->crop_bottom, (8<<CHROMA444)-1);
	}
	
	*w = width;
	*h = height;
	*num_ref_frames = sps->ref_frame_count;
	
	//printf("ff_h264_decode_seq_parameter_set width=%d height=%d\n",*w,*h);
	
	av_free(sps);
	return 0;
fail:
	av_free(sps);
	return -1;
}

