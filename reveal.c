#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define PGR_NAME "reveal"
#define PGR_VRS "v11.0.0"
#define EXIT_CODE(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define NULL_STR(s) (s ? s : "")
#define RET_CASE(val, act)\
case val:\
	return act;
#define CASE(val, act)\
case val:\
	act;\
	break;
#define PUTS_CASE(val, txt) CASE(val, puts(txt))
#define PUT_PERM_CHR(perm, chr) putchar(s->st_mode & perm ? chr : '-')
#define PARSE_SIZE_PREF_MUL(mul, chr)\
z = s->st_size / (mul);\
if ((int)z) {\
	printf("%.1f%cB\n", z, chr);\
	return;\
}
#define PARSE_OPT(opt, tgt, act)\
if (!strcmp("-" opt, tgt)) {\
	act;\
}
#define PARSE_META_OPT(opt, act)\
PARSE_OPT(opt, args[i],\
act;\
exit(EXIT_SUCCESS))
#define PARSE_DT_OPT(opt, dt)\
PARSE_OPT(opt, arg,\
if (AW_ARG) {\
	reveal(path);\
}\
DT = dt;\
AW_ARG = true;\
if (is_last_arg) {\
	reveal(path);\
}\
return 1)
#define PARSE_LNK_OPT(opt, fl)\
PARSE_OPT(opt, arg,\
if (is_last_arg) {\
	reveal(path);\
}\
FL_LNK = fl;\
return 1)

enum data_type {
	DT_CTTS,
	DT_TYPE,
	DT_SIZE,
	DT_BT_SIZE,
	DT_PERMS,
	DT_OCT_PERMS,
	DT_USR,
	DT_USR_ID,
	DT_GRP,
	DT_GRP_ID,
	DT_MOD_DATE
};

static enum data_type DT = DT_CTTS;
static bool H_ERR = false, FL_LNK = true, AW_ARG = false;

static void
help(void)
{
	printf("Usage: %s [OPTION | PATH]...\n", PGR_NAME);
	puts("Reveals info about entries in the file system.\n");
	puts("META OPTIONS");
	puts("These options retrieve information about the program.\n");
	puts("  -v  print its version.");
	puts("  -h  print this help.\n");
	puts("DATA TYPE OPTIONS");
	puts("These options change the data type to retrieve from the entries "
	"following them.\n");
	puts("  -c (default)  print its contents.");
	puts("  -t            print its type: regular (r), directory (d), "
	"symlink (l),\n                socket (s), fifo (f), character device "
	"(c), block device (b)\n                or unknown (-).");
	puts("  -s            print its size in a convenient unit: gigabyte "
	"(GB),\n                megabyte (MB), kilobyte (kB) or byte (B).");
	puts("  -bs           print its size in bytes with no unit besides.");
	puts("  -p            print its read (r), write (w), execute (x) and "
	"lack (-)\n                permissions for user, group and others.");
	puts("  -op           print its permissions in octal base.");
	puts("  -u            print the user that owns it.");
	puts("  -ui           print the ID of the user that owns it.");
	puts("  -g            print the group that owns it.");
	puts("  -gi           print the ID of the group that owns it.");
	puts("  -md           print the date when its contents were last "
	"modified.\n");
	puts("All these options expect a path following them. If not provided, "
	"they will\nconsider the last valid one given or, else, the current "
	"directory.\n");
	puts("If none of these is provided, the one marked as default will be "
	"considered.\n");
	puts("SYMLINKS OPTIONS");
	puts("These options change how symlinks following them will be "
	"handled, possibly\nchanging the origin of the data retrieved.\n");
	puts("  -fl (default)  follow symlinks.");
	puts("  -dfl           don't follow symlinks.\n");
	puts("If none of these is provided, the one marked as default will be "
	"considered.\n");
	puts("EXIT CODES");
	puts("Code 1 will be throw if an error happens and 0 otherwise.\n");
	puts("SOURCE CODE");
	puts("Its source code is available at:");
	puts("<https://github.com/skippyr/reveal>.\n");
	puts("SUPPORT");
	puts("Report issues, questions and suggestions through its issues "
	"page:");
	puts("<https://github.com/skippyr/reveal/issues>");
}

static int
print_err(char *desc0, char *desc1, char *desc2, char *fix)
{
	fprintf(stderr, "%s: %s%s%s\n%s%s", PGR_NAME, NULL_STR(desc0),
	NULL_STR(desc1), NULL_STR(desc2), NULL_STR(fix), fix ? "\n" : "");
	H_ERR = true;
	return 1;
}

static void
throw_err(char *desc)
{
	print_err(desc, NULL, NULL, NULL);
	exit(EXIT_FAILURE);
}

static void
reveal_type(struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		PUTS_CASE(S_IFREG, "r");
		PUTS_CASE(S_IFDIR, "d");
		PUTS_CASE(S_IFLNK, "l");
		PUTS_CASE(S_IFSOCK, "s");
		PUTS_CASE(S_IFIFO, "f");
		PUTS_CASE(S_IFCHR, "c");
		PUTS_CASE(S_IFBLK, "b");
	default:
		puts("-");
	}
}

static void
reveal_size(struct stat *s)
{
	float z;
	PARSE_SIZE_PREF_MUL(1e9, 'G');
	PARSE_SIZE_PREF_MUL(1e6, 'M');
	PARSE_SIZE_PREF_MUL(1e3, 'k');
	printf("%ldB\n", s->st_size);
}

static void
reveal_bt_size(struct stat *s)
{
	printf("%ld\n", s->st_size);
}

static void
reveal_perms(struct stat *s)
{
	PUT_PERM_CHR(S_IRUSR, 'r');
	PUT_PERM_CHR(S_IWUSR, 'w');
	PUT_PERM_CHR(S_IXUSR, 'x');
	PUT_PERM_CHR(S_IRGRP, 'r');
	PUT_PERM_CHR(S_IWGRP, 'w');
	PUT_PERM_CHR(S_IXGRP, 'x');
	PUT_PERM_CHR(S_IROTH, 'r');
	PUT_PERM_CHR(S_IWOTH, 'w');
	PUT_PERM_CHR(S_IXOTH, 'x');
	putchar('\n');
}

static void
reveal_oct_perms(struct stat *s)
{
	printf("%o\n", s->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
	S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
}

static int
reveal_usr(struct stat *s, char *path)
{
	struct passwd *u = getpwuid(s->st_uid);
	if (!u)
		return print_err("can't find user that owns \"", path, "\".",
		"Ensure that it is not a dangling symlink.");
	puts(u->pw_name);
	return 0;
}

static int
reveal_grp(struct stat *s, char *path)
{
	struct group *g = getgrgid(s->st_gid);
	if (!g)
		return print_err("can't find group that owns \"", path, "\".",
		"Ensure that it is not a dangling symlink.");
	puts(g->gr_name);
	return 0;
}

static void
reveal_own_id(unsigned i)
{
	printf("%u\n", i);
}

static int
reveal_mod_date(struct stat *s)
{
	char m[29];
	if (!strftime(m, sizeof(m), "%a %b %d %T %Z %Y",
	localtime(&s->st_mtime)))
		return print_err("overflowed modified date buffer.", NULL, NULL,
		NULL);
	puts(m);
	return 0;
}

static int
reveal_file(char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return print_err("can't open file \"", path, "\"", "Check if "
		"you have permission to read it.");
	char c;
	while ((c = fgetc(f)) != EOF)
		putchar(c);
	fclose(f);
	return 0;
}

static int
get_tot_dir_ents(DIR *d)
{
	size_t s = 0;
	for (; readdir(d); s++);
	return s -= 2;
}

static void
alloc_dir_ents(DIR *d, void **buf)
{
	struct dirent *e;
	size_t i = 0;
	while ((e = readdir(d))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		size_t s = strlen(e->d_name) + 1;
		void *a = malloc(s);
		if (!a)
			throw_err("can't allocate memory.");
		memcpy(a, e->d_name, s);
		buf[i] = a;
		i++;
	}
}

static void
sort_dir_ents(void **buf, size_t buf_len)
{
	for (size_t i = 0; i < buf_len - 1; i++) {
		size_t w = i;
		for (size_t j = i + 1; j < buf_len; j++)
			if (strcmp(buf[j], buf[w]) < 0)
				w = j;
		if (w == i)
			continue;
		void *t = buf[i];
		buf[i] = buf[w];
		buf[w] = t;
	}
}

static int
reveal_dir(char *path)
{
	DIR *d = opendir(path);
	if (!d)
		return print_err("can't open directory \"", path, "\"", "Check "
		"if you have permission to read it.");
	size_t s = get_tot_dir_ents(d);
	if (!s) {
		closedir(d);
		return 0;
	}
	void *e[s];
	rewinddir(d);
	alloc_dir_ents(d, e);
	sort_dir_ents(e, s);
	for (size_t i = 0; i < s; i++) {
		puts(e[i]);
		free(e[i]);
	}
	closedir(d);
	return 0;
}

static int
reveal_ctts(char *path, struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		RET_CASE(S_IFREG, reveal_file(path));
		RET_CASE(S_IFDIR, reveal_dir(path));
		RET_CASE(S_IFLNK, print_err("can't read symlink \"", path,
		"\".", "Try to use the \"-fl\" option right before it."));
	default:
		return print_err("can't read contents of \"", path, "\".",
		"Its type is unreadable.");
	}
}

static int
reveal(char *path)
{
	struct stat s;
	if (FL_LNK ? stat(path, &s) : lstat(path, &s))
		return print_err("can't find entry \"", path, "\".", "Check "
		"for misspellings.");
	switch (DT) {
		CASE(DT_TYPE, reveal_type(&s));
		CASE(DT_SIZE, reveal_size(&s));
		CASE(DT_BT_SIZE, reveal_bt_size(&s));
		CASE(DT_PERMS, reveal_perms(&s));
		CASE(DT_OCT_PERMS, reveal_oct_perms(&s));
		RET_CASE(DT_USR, reveal_usr(&s, path));
		CASE(DT_USR_ID, reveal_own_id(s.st_uid));
		RET_CASE(DT_GRP, reveal_grp(&s, path));
		CASE(DT_GRP_ID, reveal_own_id(s.st_gid));
		RET_CASE(DT_MOD_DATE, reveal_mod_date(&s));
	default:
		return reveal_ctts(path, &s);
	}
	return 0;
}

static int
parse_lnk_opts(char *path, char *arg, bool is_last_arg)
{
	PARSE_LNK_OPT("fl", true);
	PARSE_LNK_OPT("dfl", false);
	return 0;
}

static int
parse_dt_opts(char *path, char *arg, bool is_last_arg)
{
	PARSE_DT_OPT("c", DT_CTTS);
	PARSE_DT_OPT("t", DT_TYPE);
	PARSE_DT_OPT("s", DT_SIZE);
	PARSE_DT_OPT("bs", DT_BT_SIZE);
	PARSE_DT_OPT("p", DT_PERMS);
	PARSE_DT_OPT("op", DT_OCT_PERMS);
	PARSE_DT_OPT("u", DT_USR);
	PARSE_DT_OPT("ui", DT_USR_ID);
	PARSE_DT_OPT("g", DT_GRP);
	PARSE_DT_OPT("gi", DT_GRP_ID);
	PARSE_DT_OPT("md", DT_MOD_DATE);
	return 0;
}

static void
parse_non_meta_opts(int tot_args, char **args)
{
	char *path = ".";
	for (int i = 1; i < tot_args; i++) {
		char *arg = args[i];
		bool is_last_arg = i == tot_args - 1;
		if (parse_dt_opts(path, arg, is_last_arg) ||
		parse_lnk_opts(path, arg, is_last_arg) || reveal(arg))
			continue;
		AW_ARG = false;
		path = arg;
	}
}

static void
parse_meta_opts(int tot_args, char **args)
{
	for (int i = 1; i < tot_args; i++) {
		PARSE_META_OPT("v", puts(PGR_VRS));
		PARSE_META_OPT("h", help());
	}
}

int
main(int tot_args, char **args)
{
	if (tot_args == 1)
		return EXIT_CODE(reveal("."));
	parse_meta_opts(tot_args, args);
	parse_non_meta_opts(tot_args, args);
	return EXIT_CODE(H_ERR);
}
