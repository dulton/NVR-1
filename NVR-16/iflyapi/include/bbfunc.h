#ifndef _BB_FUNC_H_
#define _BB_FUNC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <unistd.h>
#include <limits.h>
#include <sys/param.h>

#define ENABLE_FEATURE_MTAB_SUPPORT 1
#define ENABLE_FEATURE_MOUNT_NFS	0
#define ENABLE_FEATURE_MOUNT_LOOP	0
#define ENABLE_FEATURE_CLEAN_UP		1
#define ENABLE_GETOPT_LONG			0
#define ENABLE_FEATURE_UMOUNT_ALL	1

#define BB_GETOPT_ERROR 0x80000000UL

typedef struct {
	int opt;
	int list_flg;
	unsigned long switch_on;
	unsigned long switch_off;
	unsigned long incongruously;
	unsigned long requires;
	void **optarg;               /* char **optarg or llist_t **optarg */
	int *counter;
} t_complementally;

#define PATH_INCR 32
#define MTAB_MAX_ENTRIES 40

extern const char * const bb_msg_perm_denied_are_you_root;
extern const char bb_path_mtab_file[];
extern char bb_common_bufsiz1[BUFSIZ+1];

typedef struct llist_s {
	char *data;
	struct llist_s *link;
} llist_t;
extern void llist_add_to(llist_t **old_head, void *data);
extern void llist_add_to_end(llist_t **list_head, void *data);
extern void *llist_pop(llist_t **elm);
extern void llist_free(llist_t *elm, void (*freeit)(void *data));

void bb_show_usage (void);

void bb_verror_msg(const char *s, va_list p);

void bb_error_msg(const char *s, ...);

void bb_vperror_msg(const char *s, va_list p);

void bb_perror_msg(const char *s, ...);

void bb_error_msg_and_die(const char *s, ...);

void bb_perror_msg_and_die(const char *s, ...);

char *bb_xasprintf(const char *format, ...);

void *xmalloc(size_t size);

char *bb_xstrdup(const char *s);

void bb_error_d(const char *s, ...);

void *xrealloc(void *ptr, size_t size);

char *bb_get_chomped_line_from_file(FILE * file);

char *bb_get_chunk_from_file(FILE * file, int *end);

char *bb_simplify_path(const char *path);

char *xgetcwd(char *cwd);

char *concat_path_file(const char *path, const char *filename);

char * last_char_is(const char *s, int c);

unsigned long bb_getopt_ulflags (int argc, char **argv, const char *applet_opts, ...);

void erase_mtab(const char *name);

char * safe_strncpy(char *dst, const char *src, size_t size);

FILE *bb_wfopen(const char *path, const char *mode);

#endif

