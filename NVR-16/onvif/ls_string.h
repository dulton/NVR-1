#ifndef __lS_STR_H
#define __LS_STR_H 1

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _IN_
#define _IN_OUT_
#define _OUT_

extern uint32_t ls_str_tirm(_IN_ char *str);
extern uint32_t ls_str_split(_IN_ const char *str, _IN_ char c, _OUT_ char *str1, _IN_ size_t len1, 
                              _OUT_ char *str2, _IN_ size_t len2);
/*extern uint32_t ls_str_get_substr(_IN_ const char *str, _IN_ char *end, _OUT_ char *line, _IN_ size_t line_len,
                                   _OUT_ char **next);
*/extern uint32_t ls_str_case_start(_IN_ char *str, _IN_ const char *start, _OUT_ char **next);

extern uint32_t ls_str_tolower(_IN_ char *str);
extern uint32_t ls_str_toupper(_IN_ char *str);

extern uint32_t ls_str_to_int8(_IN_ const char *str, _OUT_ int8_t *i);
extern uint32_t ls_str_to_uint8(_IN_ const char *str, _OUT_ uint8_t *i);
extern uint32_t ls_str_to_int16(_IN_ const char *str, _OUT_ int16_t *i);
extern uint32_t ls_str_to_uint16(_IN_ const char *str, _OUT_ uint16_t *i);
extern uint32_t ls_str_to_int32(_IN_ const char *str, _OUT_ int32_t *i);
extern uint32_t ls_str_to_uint32(_IN_ const char *str, _OUT_ uint32_t *i);
extern uint32_t ls_str_to_int64(_IN_ const char *str, _OUT_ int64_t *i);
extern uint32_t ls_str_to_uint64(_IN_ const char *str, _OUT_ uint64_t *i);
extern uint32_t ls_str_to_char(_IN_ const char *str, _OUT_ char *i);
extern uint32_t ls_str_to_uchar(_IN_ const char *str, _OUT_ unsigned char *i);
extern uint32_t ls_str_to_short(_IN_ const char *str, _OUT_ short *i);
extern uint32_t ls_str_to_ushort(_IN_ const char *str, _OUT_ unsigned short *i);
extern uint32_t ls_str_to_int(_IN_ const char *str, _OUT_ int *i);
extern uint32_t ls_str_to_uint(_IN_ const char *str, _OUT_ unsigned int *i);
extern uint32_t ls_str_to_long(_IN_ const char *str, _OUT_ long *i);
extern uint32_t ls_str_to_ulong(_IN_ const char *str, _OUT_ unsigned long *i);
extern uint32_t ls_str_to_llong(_IN_ const char *str, _OUT_ long long *i);
extern uint32_t ls_str_to_ullong(_IN_ const char *str, _OUT_ unsigned long long *i);
extern uint32_t ls_str_to_size(_IN_ const char *str, _OUT_ size_t *i);
extern uint32_t ls_str_to_double(_IN_ const char *str, _OUT_ double *d);
extern uint32_t ls_str_to_ldouble(_IN_ const char *str, _OUT_ long double *ld);
extern uint32_t ls_str_to_float(_IN_ const char *str, _OUT_ float *f);

extern uint32_t ls_str_to_three_digit_one_comma(_IN_ const char *str, _OUT_ char *buf, _IN_ size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif
