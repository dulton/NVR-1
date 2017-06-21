#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ls_charbuf.h"
#include "ls_error.h"

uint32_t ls_charbuf_init(ls_charbuf_t *me)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    me->stack[0] = '\0';
    me->heap     = NULL;
    me->size     = sizeof(me->stack);
    me->used     = 0;
    me->p        = me->stack;

    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_reset(ls_charbuf_t *me)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    if(me->heap) free(me->heap);
    ls_charbuf_init(me);

    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_append_front_ap(ls_charbuf_t *me, const char *msg, va_list ap)
{
    uint32_t  ret;
    size_t    old_used;
    size_t    append_len;
    char     *tmp;

    if(NULL == me || NULL == msg) return LS_ERRNO_INVAL;

    old_used = me->used;

    if(0 != (ret = ls_charbuf_append_back_ap(me, msg, ap))) return ret;

    append_len = me->used - old_used;
    if(NULL == (tmp = (char *)malloc(append_len)))
    {
        me->used = old_used;
        *(me->p + old_used) = '\0';
        return LS_ERRNO_NOMEM;
    }

    memcpy(tmp, me->p + old_used, append_len);
    memmove(me->p + append_len, me->p, old_used);
    memcpy(me->p, tmp, append_len);
    free(tmp);
    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_append_front(ls_charbuf_t *me, const char *msg, ...)
{
    va_list  ap;
    uint32_t ret;

    if(NULL == me || NULL == msg) return LS_ERRNO_INVAL;

    va_start(ap, msg);
    ret = ls_charbuf_append_front_ap(me, msg, ap);
    va_end(ap);

    return ret;
}

uint32_t ls_charbuf_append_back_ap(ls_charbuf_t *me, const char *msg, va_list ap)
{
    int     n;
    char   *newheap;
    size_t  multiple;

    if(NULL == me || NULL == msg) return LS_ERRNO_INVAL;

    for(;;)
    {
        n = vsnprintf(me->p + me->used, me->size - me->used, msg, ap);

        if(me->used + n < me->size)
        {
            /* append OK */
            me->used += n;
            return LS_ERRNO_OK;
        }
        else
        {
            /* expand the storage space */
            multiple = ((me->used + n) / me->size) + 1;
            newheap = (char *)realloc(me->heap, me->size * multiple);
            if(newheap)
            {
                me->heap = newheap;
                me->size = me->size * multiple;
                if(me->p == me->stack)
                {
                    /* first malloc */
                    me->p = me->heap;
                    if(me->used > 0) strcpy(me->heap, me->stack);
                }
                else
                    me->p = me->heap;
            }
            else /* oom */
                return LS_ERRNO_NOMEM;
        }
    }
}

uint32_t ls_charbuf_append_back(ls_charbuf_t *me, const char *msg, ...)
{
    va_list  ap;
    uint32_t ret;

    if(NULL == me || NULL == msg) return LS_ERRNO_INVAL;

    va_start(ap, msg);
    ret = ls_charbuf_append_back_ap(me, msg, ap);
    va_end(ap);
    
    return ret;
}

uint32_t ls_charbuf_erase_last_char(ls_charbuf_t *me, char c)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    if(me->used > 0 && *(me->p + me->used - 1) == c)
    {
        *(me->p + me->used - 1) = '\0';
        me->used--;
    }

    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_get_str(ls_charbuf_t *me, char **str)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    *str = me->p;

    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_get_strlen(ls_charbuf_t *me, size_t *strlen)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    *strlen = me->used;

    return LS_ERRNO_OK;
}

uint32_t ls_charbuf_clean(ls_charbuf_t *me)
{
    if(NULL == me) return LS_ERRNO_INVAL;

    if(me->heap) free(me->heap);
    me->p = me->heap = NULL;

    return LS_ERRNO_OK;
}
