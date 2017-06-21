#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "vlog.h"

#if defined(_WIN32) || defined(_WIN64)
#define vsnprintf _vsnprintf
#define vsprintf _vsprintf
#endif

#define MAX_PRINT_LEN	2048

//#define VLOG_FILE	"vlog.txt"

#ifdef VLOG_FILE
static FILE *g_log_file = NULL;
#endif

VLOG_Level_t debuglevel = VLOG_WARNING;
static int neednl;
static FILE *fmsg;

static const char *levels[] = 
{
    "CRIT", "ERROR", "WARNING", "INFO",
    "DEBUG", "DEBUG2"
};

static const int log_color[VLOG_ALL]= {34,31,33,35,32,36,};

static fVLOG_Callback vlog_default, *cb = vlog_default;

static void vlog_default(int level, const char *format, va_list vl)
{
    char str[MAX_PRINT_LEN] = "";
    vsnprintf(str, MAX_PRINT_LEN-1, format, vl);

    /* Filter out 'no-name' */
    if ( debuglevel<VLOG_ALL && strstr(str, "no-name" ) != NULL )
        return;
	
#ifdef VLOG_FILE
	if(g_log_file == NULL){
		g_log_file = fopen(VLOG_FILE,"wb+");
		if(g_log_file == NULL){
			printf("open file failed\n");
			return NULL;
		}
		printf("create file:%s success",VLOG_FILE);
	}
    if ( !fmsg ) fmsg = g_log_file;
#else
	if ( !fmsg ) fmsg = stderr;
#endif

    if ( level <= debuglevel ) {
        if (neednl) {
            putc('\n', fmsg);
            neednl = 0;
        }
		//fprintf(fmsg, "\033[1;%dmVLOG-%s:\033[0m %s\n", log_color[level],levels[level], str);
		fprintf(fmsg, "VLOG-%s: [%s]\n", levels[level], str);
#ifdef _DEBUG
        fflush(fmsg);
#endif
    }
}

void VLOG_SetOutput(FILE *file)
{
    fmsg = file;
}

void VLOG_SetLevel(VLOG_Level_t level)
{
    debuglevel = level;
}

void VLOG_SetCallback(fVLOG_Callback *cbp)
{
    cb = cbp;
}

VLOG_Level_t VLOG_GetLevel()
{
    return debuglevel;
}

void VLOG(int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    cb(level, format, args);
    va_end(args);
}

static const char hexdig[] = "0123456789abcdef";

void VLOG_Hex(int level, const unsigned char *data, unsigned long len)
{
    unsigned long i;
    char line[50], *ptr;

    if ( level > debuglevel )
        return;

    ptr = line;

    for (i=0; i<len; i++) {
        *ptr++ = hexdig[0x0f & (data[i] >> 4)];
        *ptr++ = hexdig[0x0f & data[i]];
        if ((i & 0x0f) == 0x0f) {
            *ptr = '\0';
            ptr = line;
            VLOG(level, "%s", line);
        } else {
            *ptr++ = ' ';
        }
    }
    if (i & 0x0f) {
        *ptr = '\0';
        VLOG(level, "%s", line);
    }
}

void VLOG_HexString(int level, const unsigned char *data, unsigned long len)
{
#define BP_OFFSET 9
#define BP_GRAPH 60
#define BP_LEN	80
    char	line[BP_LEN];
    unsigned long i;

    if ( !data || level > debuglevel )
        return;

    /* in case len is zero */
    line[0] = '\0';

    for ( i = 0 ; i < len ; i++ ) {
        int n = i % 16;
        unsigned off;

        if ( !n ) {
            if ( i ) VLOG( level, "%s", line );
            memset( line, ' ', sizeof(line)-2 );
            line[sizeof(line)-2] = '\0';

            off = i % 0x0ffffU;

            line[2] = hexdig[0x0f & (off >> 12)];
            line[3] = hexdig[0x0f & (off >>  8)];
            line[4] = hexdig[0x0f & (off >>  4)];
            line[5] = hexdig[0x0f & off];
            line[6] = ':';
        }

        off = BP_OFFSET + n*3 + ((n >= 8)?1:0);
        line[off] = hexdig[0x0f & ( data[i] >> 4 )];
        line[off+1] = hexdig[0x0f & data[i]];

        off = BP_GRAPH + n + ((n >= 8)?1:0);

        if ( isprint( data[i] )) {
            line[BP_GRAPH + n] = data[i];
        } else {
            line[BP_GRAPH + n] = '.';
        }
    }

    VLOG( level, "%s", line );
}

/* These should only be used by apps, never by the library itself */
void VLOG_Printf(const char *format, ...)
{
    char str[MAX_PRINT_LEN]="";
    int len;
    va_list args;
    va_start(args, format);
    len = vsnprintf(str, MAX_PRINT_LEN-1, format, args);
    va_end(args);

    if ( debuglevel==VLOG_CRIT )
        return;

    if ( !fmsg ) fmsg = stderr;

    if (neednl) {
        putc('\n', fmsg);
        neednl = 0;
    }

    if (len > MAX_PRINT_LEN-1)
        len = MAX_PRINT_LEN-1;
    fprintf(fmsg, "%s", str);
    if (str[len-1] == '\n')
        fflush(fmsg);
}

void VLOG_Status(const char *format, ...)
{
    char str[MAX_PRINT_LEN]="";
    va_list args;
    va_start(args, format);
    vsnprintf(str, MAX_PRINT_LEN-1, format, args);
    va_end(args);

    if ( debuglevel==VLOG_CRIT )
        return;

    if ( !fmsg ) fmsg = stderr;

    fprintf(fmsg, "%s", str);
    fflush(fmsg);
    neednl = 1;
}

