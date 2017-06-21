#include "bbfunc.h"

#include <stdio.h>
#include <mntent.h>

#if defined(CONFIG_FEATURE_MTAB_SUPPORT)
const char bb_path_mtab_file[] = "/etc/mtab";
#else
const char bb_path_mtab_file[] = "/proc/mounts";
#endif

char bb_common_bufsiz1[BUFSIZ+1];

const char * const bb_msg_perm_denied_are_you_root = "permission denied. (are you root?)";

const char * const bb_msg_memory_exhausted = "memory exhausted";
static const char msg_enomem[] = "memory exhausted";

const char *bb_opt_complementally = NULL;

/* Add data to the start of the linked list.  */
void llist_add_to(llist_t **old_head, void *data)
{
	llist_t *new_head = xmalloc(sizeof(llist_t));
	new_head->data = data;
	new_head->link = *old_head;
	*old_head = new_head;
}

void llist_add_to_end(llist_t **list_head, void *data)
{
	llist_t *new_item = xmalloc(sizeof(llist_t));
	new_item->data = data;
	new_item->link = NULL;
	
	if (!*list_head) *list_head = new_item;
	else {
		llist_t *tail = *list_head;
		while (tail->link) tail = tail->link;
		tail->link = new_item;
	}
}

void *llist_pop(llist_t **head)
{
	void *data;
	
	if(!*head) data = *head;
	else {
		void *next = (*head)->link;
		data = (*head)->data;
		free(*head);
		*head = next;
	}
	
	return data;
}

void llist_free(llist_t *elm, void (*freeit)(void *data))
{
	while (elm) {
		void *data = llist_pop(&elm);
		if (freeit) freeit(data);
	}
}

void bb_show_usage (void)
{
	
}

void bb_verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	vfprintf(stderr, s, p);
}

void bb_error_msg(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	bb_verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

void bb_vperror_msg(const char *s, va_list p)
{
	int err=errno;
	if(s == 0) s = "";
	bb_verror_msg(s, p);
	if (*s) s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

void bb_perror_msg(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	bb_vperror_msg(s, p);
	va_end(p);
}

void bb_error_msg_and_die(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	bb_verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

void bb_perror_msg_and_die(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	bb_vperror_msg(s, p);
	va_end(p);
}

char *bb_xasprintf(const char *format, ...)
{
	va_list p;
	int r;
	char *string_ptr;
	
#ifdef HAVE_GNU_EXTENSIONS
	va_start(p, format);
	r = vasprintf(&string_ptr, format, p);
	va_end(p);
#else
	va_start(p, format);
	r = vsnprintf(NULL, 0, format, p);
	va_end(p);
	string_ptr = xmalloc(r+1);
	va_start(p, format);
	r = vsnprintf(string_ptr, r+1, format, p);
	va_end(p);
#endif
	
	if (r < 0) {
		bb_perror_msg_and_die("bb_xasprintf");
	}
	return string_ptr;
}

void *xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL && size != 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);
	return ptr;
}

char *bb_xstrdup(const char *s)
{
	char *r = strdup(s);
	
	if(r == NULL)
		bb_error_d(msg_enomem);
	return r;
}

void bb_error_d(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	vfprintf(stderr, s, p);
	va_end(p);
	putc('\n', stderr);
	//exit(1);
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);
	return ptr;
}

/* Get line.  Remove trailing /n */
char *bb_get_chomped_line_from_file(FILE * file)
{
	int i;
	char *c = bb_get_chunk_from_file(file, &i);
	
	if (i && c[--i] == '\n')
		c[i] = 0;
	
	return c;
}

char *bb_get_chunk_from_file(FILE * file, int *end)
{
	int ch;
	int idx = 0;
	char *linebuf = NULL;
	int linebufsz = 0;
	
	while ((ch = getc(file)) != EOF) {
		/* grow the line buffer as necessary */
		if (idx > linebufsz - 2) {
			linebuf = xrealloc(linebuf, linebufsz += 80);
		}
		linebuf[idx++] = (char) ch;
		if (!ch || (end && ch == '\n'))
			break;
	}
	if (end)
		*end = idx;
	if (linebuf) {
		if (ferror(file)) {
			free(linebuf);
			return NULL;
		}
		linebuf[idx] = 0;
	}
	return linebuf;
}

char *bb_simplify_path(const char *path)
{
	char *s, *start, *p;
	
	if (path[0] == '/')
		start = bb_xstrdup(path);
	else {
		s = xgetcwd(NULL);
		start = concat_path_file(s, path);
		free(s);
	}
	p = s = start;
	
	do {
		if (*p == '/') {
			if (*s == '/') {	/* skip duplicate (or initial) slash */
				continue;
			} else if (*s == '.') {
				if (s[1] == '/' || s[1] == 0) {	/* remove extra '.' */
					continue;
				} else if ((s[1] == '.') && (s[2] == '/' || s[2] == 0)) {
					++s;
					if (p > start) {
						while (*--p != '/');	/* omit previous dir */
					}
					continue;
				}
			}
		}
		*++p = *s;
	} while (*++s);
	
	if ((p == start) || (*p != '/')) {	/* not a trailing slash */
		++p;					/* so keep last character */
	}
	*p = 0;
	
	return start;
}

char *xgetcwd (char *cwd)
{
	char *ret;
	unsigned path_max;
	
	path_max = (unsigned) PATH_MAX;
	path_max += 2;                /* The getcwd docs say to do this. */
	
	if(cwd==0)
		cwd = xmalloc (path_max);
	
	while ((ret = getcwd (cwd, path_max)) == NULL && errno == ERANGE) {
		path_max += PATH_INCR;
		cwd = xrealloc (cwd, path_max);
	}
	
	if (ret == NULL) {
		free (cwd);
		bb_perror_msg("getcwd()");
		return NULL;
	}
	
	return cwd;
}

char *concat_path_file(const char *path, const char *filename)
{
	char *lc;
	
	if (!path)
		path = "";
	lc = last_char_is(path, '/');
	while (*filename == '/')
		filename++;
	return bb_xasprintf("%s%s%s", path, (lc==NULL ? "/" : ""), filename);
}

char * last_char_is(const char *s, int c)
{
	char *sret = (char *)s;
	if (sret) {
		sret = strrchr(sret, c);
		if(sret != NULL && *(sret+1) != 0)
			sret = NULL;
	}
	return sret;
}

unsigned long bb_getopt_ulflags (int argc, char **argv, const char *applet_opts, ...)
{
	unsigned long flags = 0;
	unsigned long requires = 0;
	t_complementally complementally[sizeof(flags) * 8 + 1];
	int c;
	const unsigned char *s;
	t_complementally *on_off;
	va_list p;
#if ENABLE_GETOPT_LONG
	const struct option *l_o;
#endif
	unsigned long trigger;
#ifdef CONFIG_PS
	char **pargv = NULL;
#endif
	int min_arg = 0;
	int max_arg = -1;

#define SHOW_USAGE_IF_ERROR     1
#define ALL_ARGV_IS_OPTS        2
#define FIRST_ARGV_IS_OPT       4
#define FREE_FIRST_ARGV_IS_OPT  8
	int spec_flgs = 0;

	va_start (p, applet_opts);

	c = 0;
	on_off = complementally;
	memset(on_off, 0, sizeof(complementally));

	/* skip GNU extension */
	s = (const unsigned char *)applet_opts;
	if(*s == '+' || *s == '-')
		s++;
	for (; *s; s++) {
		if(c >= (int)(sizeof(flags)*8))
			break;
		on_off->opt = *s;
		on_off->switch_on = (1 << c);
		if (s[1] == ':') {
			on_off->optarg = va_arg (p, void **);
			do
				s++;
			while (s[1] == ':');
		}
		on_off++;
		c++;
	}

#if ENABLE_GETOPT_LONG
	for(l_o = bb_applet_long_options; l_o->name; l_o++) {
		if(l_o->flag)
			continue;
		for(on_off = complementally; on_off->opt != 0; on_off++)
			if(on_off->opt == l_o->val)
				break;
		if(on_off->opt == 0) {
			if(c >= (int)(sizeof(flags)*8))
				break;
			on_off->opt = l_o->val;
			on_off->switch_on = (1 << c);
			if(l_o->has_arg != no_argument)
				on_off->optarg = va_arg (p, void **);
			c++;
		}
	}
#endif /* ENABLE_GETOPT_LONG */
	for (s = (const unsigned char *)bb_opt_complementally; s && *s; s++) {
		t_complementally *pair;
		unsigned long *pair_switch;

		if (*s == ':')
			continue;
		c = s[1];
		if(*s == '?') {
			if(c < '0' || c > '9') {
				spec_flgs |= SHOW_USAGE_IF_ERROR;
			} else {
				max_arg = c - '0';
				s++;
			}
			continue;
		}
		if(*s == '-') {
			if(c < '0' || c > '9') {
				if(c == '-') {
					spec_flgs |= FIRST_ARGV_IS_OPT;
					s++;
				} else
					spec_flgs |= ALL_ARGV_IS_OPTS;
			} else {
				min_arg = c - '0';
				s++;
			}
			continue;
		}
		for (on_off = complementally; on_off->opt; on_off++)
			if (on_off->opt == *s)
				break;
		if(c == ':' && s[2] == ':') {
			on_off->list_flg++;
			continue;
		}
		if(c == ':' || c == '\0') {
			requires |= on_off->switch_on;
			continue;
		}
		if(c == '-' && (s[2] == ':' || s[2] == '\0')) {
			flags |= on_off->switch_on;
			on_off->incongruously |= on_off->switch_on;
			s++;
			continue;
		}
		if(c == *s) {
			on_off->counter = va_arg (p, int *);
			s++;
		}
		pair = on_off;
		pair_switch = &(pair->switch_on);
		for(s++; *s && *s != ':'; s++) {
			if(*s == '?') {
				pair_switch = &(pair->requires);
			} else if (*s == '-') {
				if(pair_switch == &(pair->switch_off))
					pair_switch = &(pair->incongruously);
				else
					pair_switch = &(pair->switch_off);
			} else {
			    for (on_off = complementally; on_off->opt; on_off++)
				if (on_off->opt == *s) {
				    *pair_switch |= on_off->switch_on;
				    break;
				}
			}
		}
		s--;
	}
	va_end (p);

#if defined(CONFIG_AR) || defined(CONFIG_TAR)
	if((spec_flgs & FIRST_ARGV_IS_OPT)) {
		if(argv[1] && argv[1][0] != '-' && argv[1][0] != '\0') {
			argv[1] = bb_xasprintf("-%s", argv[1]);
			if(ENABLE_FEATURE_CLEAN_UP)
				spec_flgs |= FREE_FIRST_ARGV_IS_OPT;
		}
	}
#endif
#if ENABLE_GETOPT_LONG
	while ((c = getopt_long (argc, argv, applet_opts,
				 bb_applet_long_options, NULL)) >= 0) {
#else
	while ((c = getopt (argc, argv, applet_opts)) >= 0) {
#endif /* ENABLE_GETOPT_LONG */
#ifdef CONFIG_PS
loop_arg_is_opt:
#endif
		for (on_off = complementally; on_off->opt != c; on_off++) {
			/* c==0 if long opt have non NULL flag */
			if(on_off->opt == 0 && c != 0)
				bb_show_usage ();
		}
		if(flags & on_off->incongruously) {
			if((spec_flgs & SHOW_USAGE_IF_ERROR))
				bb_show_usage ();
			flags |= BB_GETOPT_ERROR;
		}
		trigger = on_off->switch_on & on_off->switch_off;
		flags &= ~(on_off->switch_off ^ trigger);
		flags |= on_off->switch_on ^ trigger;
		flags ^= trigger;
		if(on_off->counter)
			(*(on_off->counter))++;
		if(on_off->list_flg) {
			llist_add_to((llist_t **)(on_off->optarg), optarg);
		} else if (on_off->optarg) {
			*(char **)(on_off->optarg) = optarg;
		}
#ifdef CONFIG_PS
		if(pargv != NULL)
			break;
#endif
	}

#ifdef CONFIG_PS
	if((spec_flgs & ALL_ARGV_IS_OPTS)) {
		/* process argv is option, for example "ps" applet */
		if(pargv == NULL)
			pargv = argv + optind;
		while(*pargv) {
			c = **pargv;
			if(c == '\0') {
				pargv++;
			} else {
				(*pargv)++;
				goto loop_arg_is_opt;
			}
		}
	}
#endif

#if (defined(CONFIG_AR) || defined(CONFIG_TAR)) && \
				defined(CONFIG_FEATURE_CLEAN_UP)
	if((spec_flgs & FREE_FIRST_ARGV_IS_OPT))
		free(argv[1]);
#endif
	/* check depending requires for given options */
	for (on_off = complementally; on_off->opt; on_off++) {
		if(on_off->requires && (flags & on_off->switch_on) &&
					(flags & on_off->requires) == 0)
			bb_show_usage ();
	}
	if(requires && (flags & requires) == 0)
		bb_show_usage ();
	argc -= optind;
	if(argc < min_arg || (max_arg >= 0 && argc > max_arg))
		bb_show_usage ();
	return flags;
}

//#ifdef CONFIG_FEATURE_MTAB_SUPPORT
void erase_mtab(const char *name)
{
	struct mntent entries[MTAB_MAX_ENTRIES];
	int count = 0;
	FILE *mountTable = setmntent(bb_path_mtab_file, "r");
	struct mntent *m;
	
	/* Check if reading the mtab file failed */
	if (mountTable == 0
		/* Bummer.  fall back on trying the /proc filesystem */
		&& (mountTable = setmntent("/proc/mounts", "r")) == 0) {
		bb_perror_msg(bb_path_mtab_file);
		return;
	}
	
	while (((m = getmntent(mountTable)) != 0) && (count < MTAB_MAX_ENTRIES))
	{
		entries[count].mnt_fsname = strdup(m->mnt_fsname);
		entries[count].mnt_dir = strdup(m->mnt_dir);
		entries[count].mnt_type = strdup(m->mnt_type);
		entries[count].mnt_opts = strdup(m->mnt_opts);
		entries[count].mnt_freq = m->mnt_freq;
		entries[count].mnt_passno = m->mnt_passno;
		count++;
	}
	endmntent(mountTable);
	if ((mountTable = setmntent(bb_path_mtab_file, "w"))) {
		int i;
		
		for (i = 0; i < count; i++) {
			int result = (strcmp(entries[i].mnt_fsname, name) == 0
				|| strcmp(entries[i].mnt_dir, name) == 0);
			
			if (result)
				continue;
			else
				addmntent(mountTable, &entries[i]);
		}
		endmntent(mountTable);
	} else if (errno != EROFS)
	bb_perror_msg(bb_path_mtab_file);
}
//#endif

/* Like strncpy but make sure the resulting string is always 0 terminated. */
char * safe_strncpy(char *dst, const char *src, size_t size)
{
	dst[size-1] = '\0';
	return strncpy(dst, src, size-1);
}

FILE *bb_wfopen(const char *path, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(path, mode)) == NULL) {
		bb_perror_msg("%s", path);
		errno = 0;
	}
	return fp;
}

/*#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>*/

void printlog(char *msg)
{
	/*int s = socket(AF_INET,SOCK_DGRAM,0);
	if(s<0) return;
	struct sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = inet_addr("192.168.1.30");
	target.sin_port = htons(6370);
	sendto(s,msg,strlen(msg)+1,0,(struct sockaddr *)&target,sizeof(target));
	close(s);*/
}
