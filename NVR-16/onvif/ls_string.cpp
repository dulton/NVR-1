
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include "ls_string.h"
#include "ls_error.h"
#include "ls_type.h"

uint32_t ls_str_tirm(char *str)
{
    size_t  len  = 0;
    char   *head = NULL;
    char   *tail = NULL;

    if(NULL == str)    return LS_ERRNO_INVAL;
    if('\0' == (*str)) return LS_ERRNO_OK;

    len  = strlen(str);
    head = str;
    tail = str + len - 1;

    /* find head */
    while(*head && isspace(*head))
        head++;
    if(!(*head))
    {
        *str = '\0';
        return LS_ERRNO_OK;
    }

    /* find tail */
    while(head != tail && isspace(*tail))
        tail--;

    /* get length */
    len = tail - head + 1;

    /* cut string */
    if(head == str)
        *(str + len) = '\0';
    else
    {
        memmove(str, head, len);
        *(str + len) = '\0';
    }

    return LS_ERRNO_OK;
}

uint32_t ls_str_split(const char *str, char c, char *str1, size_t len1, char *str2, size_t len2)
{
    size_t i = 0, str_len;

    if(NULL == str || NULL == str1 || 0 == len1 || NULL == str2 || 0 == len2) return LS_ERRNO_INVAL;

    str_len = strlen(str);
    if(str_len - 1 > len1 + len2 - 2) return LS_ERRNO_INVAL;

    /* find char c */
    while(str[i] && str[i] != c)
        i++;
    if(!(str[i]))
        return LS_ERRNO_NOTFND; /* not find */

    if(i > len1 - 1) return LS_ERRNO_NOBUF;
    memcpy(str1, str, i);
    str1[i] = '\0';

    if(str_len - i - 1> len2 - 1) return LS_ERRNO_NOBUF;
    memcpy(str2, str + i + 1, str_len - i - 1);
    str2[str_len - i - 1] = '\0';

    return LS_ERRNO_OK;
}

/*uint32_t ls_str_get_substr(const char *str, char *end, char *line, size_t line_len, char **next)
{
    const char *p = NULL;

    if(NULL == str || NULL == end || NULL == line || 0 == line_len) return LS_ERRNO_INVAL;

    line[0] = '\0';
    if(next) *next = NULL;
    
    if(NULL == (p = strstr(str, end))) return LS_ERRNO_NOTFND;

    if(p - str > line_len - 1) return LS_ERRNO_NOBUF;

    if(p - str > 0)
    {
        memcpy(line, str, p - str);
        line[p - str] = '\0';
    }

    if(next) *next = p + strlen(end);
    return LS_ERRNO_OK;
}
*/
uint32_t ls_str_case_start(char *str, const char *start, char **next)
{
    while(*start && toupper((int)*start) == toupper((int)*str))
    {
        start++;
        str++;
    }
    if(!*start && next)
        *next = str;

    return ('\0' == *start ? LS_ERRNO_NOTFND : LS_ERRNO_OK);
}

uint32_t ls_str_tolower(char *str)
{
    if(NULL == str) return LS_ERRNO_INVAL;

    while(*str)
    {
        *str = tolower(*str);
        str++;
    }

    return LS_ERRNO_OK;
}

uint32_t ls_str_toupper(char *str)
{
    if(NULL == str) return LS_ERRNO_INVAL;

    while(*str)
    {
        *str = toupper(*str);
        str++;
    }

    return LS_ERRNO_OK;
}

uint32_t ls_str_to_int8(const char *str, int8_t *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > INT8_MAX || tmp < INT8_MIN)
        return LS_ERRNO_RANGE;

    *i = (int8_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uint8(const char *str, uint8_t *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > UINT8_MAX)
        return LS_ERRNO_RANGE;
    
    *i = (uint8_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_int16(const char *str, int16_t *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > INT16_MAX || tmp < INT16_MIN)
        return LS_ERRNO_RANGE;

    *i = (int16_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uint16(const char *str, uint16_t *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > UINT16_MAX)
        return LS_ERRNO_RANGE;

    *i = (uint16_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_int32(const char *str, int32_t *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > INT32_MAX || tmp < INT32_MIN)
        return LS_ERRNO_RANGE;

    *i = (int32_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uint32(const char *str, uint32_t *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > UINT32_MAX)
        return LS_ERRNO_RANGE;

    *i = (uint32_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_int64(const char *str, int64_t *i)
{
    long long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoll(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LLONG_MIN == tmp || LLONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > INT64_MAX || tmp < INT64_MIN)
        return LS_ERRNO_RANGE;

    *i = (int64_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uint64(const char *str, uint64_t *i)
{
    unsigned long long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoull(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULLONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > UINT64_MAX)
        return LS_ERRNO_RANGE;

    *i = (uint64_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_char(const char *str, char *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > SCHAR_MAX || tmp < SCHAR_MIN)
        return LS_ERRNO_RANGE;

    *i = (char)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uchar(const char *str, unsigned char *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > UCHAR_MAX)
        return LS_ERRNO_RANGE;

    *i = (unsigned char)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_short(const char *str, short *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > SHRT_MAX || tmp < SHRT_MIN)
        return LS_ERRNO_RANGE;

    *i = (short)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_ushort(const char *str, unsigned short *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > USHRT_MAX)
        return LS_ERRNO_RANGE;

    *i = (unsigned short)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_int(const char *str, int *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    if(tmp > INT_MAX || tmp < INT_MIN)
        return LS_ERRNO_RANGE;

    *i = (int)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_uint(const char *str, unsigned int *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;
    
    if(tmp > UINT_MAX)
        return LS_ERRNO_RANGE;

    *i = (unsigned int)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_long(const char *str, long *i)
{
    long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtol(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LONG_MIN == tmp || LONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    *i = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_ulong(const char *str, unsigned long *i)
{
    unsigned long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoul(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    *i = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_llong(const char *str, long long *i)
{
    long long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoll(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && (LLONG_MIN == tmp || LLONG_MAX == tmp)))
        return LS_ERRNO_SYSCALL;

    *i = (char)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_ullong(const char *str, unsigned long long *i)
{
    unsigned long long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoull(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULLONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    *i = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_size(const char *str, size_t *i)
{
    unsigned long long tmp;

    if(NULL == str || NULL == i) return LS_ERRNO_INVAL;

    tmp = strtoull(str, (char **)NULL, 10);

    if((EINVAL == errno) || (ERANGE == errno && ULLONG_MAX == tmp))
        return LS_ERRNO_SYSCALL;

    if(tmp > SIZE_MAX)
        return LS_ERRNO_RANGE;

    *i = (size_t)tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_double(const char *str, double *d)
{
    double tmp;

    if(NULL == str || NULL == d) return LS_ERRNO_INVAL;

    tmp = strtod(str, (char **)NULL);

    if((EINVAL == errno) || (ERANGE == errno && (-HUGE_VAL == tmp || HUGE_VAL == tmp)))
        return LS_ERRNO_SYSCALL;

    *d = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_ldouble(const char *str, long double *ld)
{
    long double tmp;

    if(NULL == str || NULL == ld) return LS_ERRNO_INVAL;

    tmp = strtold(str, (char **)NULL);

    if((EINVAL == errno) || (ERANGE == errno && (-HUGE_VALL == tmp || HUGE_VALL == tmp)))
        return LS_ERRNO_SYSCALL;

    *ld = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_float(const char *str, float *f)
{
    float tmp;

    if(NULL == str || NULL == f) return LS_ERRNO_INVAL;

    tmp = strtof(str, (char **)NULL);

    if((EINVAL == errno) || (ERANGE == errno && (-HUGE_VALF == tmp || HUGE_VALF == tmp)))
        return LS_ERRNO_SYSCALL;

    *f = tmp;
    return LS_ERRNO_OK;
}

uint32_t ls_str_to_three_digit_one_comma(const char *str, char *buf, size_t buf_len)
{
    size_t i = 0, j = 0, n;

    if(NULL == str || 0 == buf_len) return LS_ERRNO_INVAL;
    memset(buf, 0, buf_len);

    n = strlen(str);
    if(buf_len < n + n % 3 + 1) return LS_ERRNO_NOBUF;

    for(i = 0; i < n; i++)
    {
        sprintf(buf + (j++), "%c", str[i]);
        if(0 == ((n - i - 1) % 3) && i != n - 1 && '-' != str[i])
            sprintf(buf + (j++), ",");
    }

    return LS_ERRNO_OK;
}
