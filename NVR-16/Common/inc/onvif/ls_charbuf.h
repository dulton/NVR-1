#ifndef __LS_CHARBUF_H
#define __LS_CHARBUF_H 1
 
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _IN_
#define _IN_OUT_
#define _OUT_

typedef struct ls_charbuf
{
    char   *p;
    char    stack[512];
    char   *heap;
    size_t  size;
    size_t  used;
} ls_charbuf_t;

extern uint32_t ls_charbuf_init(_IN_ ls_charbuf_t *me);
extern uint32_t ls_charbuf_reset(_IN_ ls_charbuf_t *me);
extern uint32_t ls_charbuf_clean(_IN_ ls_charbuf_t *me);

extern uint32_t ls_charbuf_append_front_ap(_IN_ ls_charbuf_t *me, _IN_ const char *msg, _IN_ va_list ap);
extern uint32_t ls_charbuf_append_front(_IN_ ls_charbuf_t *me, _IN_ const char *msg, ...);
extern uint32_t ls_charbuf_append_back_ap(_IN_ ls_charbuf_t *me, _IN_ const char *msg, _IN_ va_list ap);
extern uint32_t ls_charbuf_append_back(_IN_ ls_charbuf_t *me, _IN_ const char *msg, ...);

extern uint32_t ls_charbuf_erase_last_char(_IN_ ls_charbuf_t *me, _IN_ char c);

extern uint32_t ls_charbuf_get_str(_IN_ ls_charbuf_t *me, _OUT_ char **str);
extern uint32_t ls_charbuf_get_strlen(_IN_ ls_charbuf_t *me, _OUT_ size_t *strlen);

#ifdef __cplusplus
}
#endif

#endif
