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
#define PGR_VRS "v11.0.2"
#define PARSE_EXIT_CD(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define PARSE_NULL_STR(s) (s ? s : "")
#define PARSE_RET_CASE(v, a) case v: return a;
#define PARSE_CASE(v, a) case v: a; break;
#define PARSE_PUTS_CASE(v, t) PARSE_CASE(v, puts(t))
#define PARSE_SIZE_PREF_MUL(m, c) z = s->st_size / (m); if ((int)z) {\
printf("%.1f%cB\n", z, c); return;}
#define PARSE_PERM(p, c) putchar(s->st_mode & p ? c : '-')
#define PARSE_OPT(o, t, a) if (!strcmp("-" o, t)) {a;}
#define PARSE_META_OPT(o, a) PARSE_OPT(o, args[i], a; exit(EXIT_SUCCESS))
#define PARSE_DT_OPT(o, d) PARSE_OPT(o, arg, if (awaits_arg) {reveal(path);}\
data_type = d; awaits_arg = true; if (is_last) {reveal(path);} return 1)
#define PARSE_LNK_OPT(o, f) PARSE_OPT(o, arg, if (is_last) {reveal(path);}\
follows_lnk = f; return 1)

enum data_type {
	DT_CTTS,
	DT_TYPE,
	DT_SIZE,
	DT_BT_SIZE,
	DT_PERMS,
	DT_OCT_PERMS,
	DT_USR,
	DT_UID,
	DT_GRP,
	DT_GID,
	DT_MOD_DATE
};

static enum data_type data_type = DT_CTTS;
static bool had_err = false, follows_lnk = true, awaits_arg = false;

static int
print_err(char *dsc0, char *dsc1, char *dsc2, char *fix)
{
	return (had_err = fprintf(stderr, "%s: %s%s%s\n%s%s", PGR_NAME,
	PARSE_NULL_STR(dsc0), PARSE_NULL_STR(dsc1), PARSE_NULL_STR(dsc2),
	PARSE_NULL_STR(fix), fix ? "\n" : ""));
}

static void
throw_err(char *dsc)
{
	print_err(dsc, NULL, NULL, NULL);
	exit(EXIT_FAILURE);
}

static void
help(void)
{
	printf("Usage: %s [OPTION | PATH]...\n", PGR_NAME);
	puts("Reveals info about entries in the file system.\n");
	puts("META OPTIONS");
	puts("These options retrieve info about the program.\n");
	puts("  -v  print its version.");
	puts("  -h  print this help.\n");
	puts("DATA TYPE OPTIONS");
	puts("These options change the data type to retrieve from the entries "
	"following them.\n");
	puts("  -c (default)  print its contents.");
	puts("  -t            print its type: regular (r), directory (d), "
	"symlink (l),\n                socket (s), fifo (f), character device "
	"(c), block device (b) or\n                unknown (-).");
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
	printf("Code %i will be throw if an error happens and %i "
	"otherwise.\n\n", EXIT_FAILURE, EXIT_SUCCESS);
	puts("SOURCE CODE");
	puts("Its source code is available at: "
	"<https://github.com/skippyr/reveal>.\n");
	puts("SUPPORT");
	puts("If you need any kind of support, for instance: help with "
	"troubleshooting, have\nquestions about it or want to give improvement "
	"suggestions, please report them\nby filing new issues in its issues "
	"page:");
	puts("<https://github.com/skippyr/reveal/issues>.");
}

static void
reveal_type(struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		PARSE_PUTS_CASE(S_IFREG, "r");
		PARSE_PUTS_CASE(S_IFDIR, "d");
		PARSE_PUTS_CASE(S_IFLNK, "l");
		PARSE_PUTS_CASE(S_IFSOCK, "s");
		PARSE_PUTS_CASE(S_IFIFO, "f");
		PARSE_PUTS_CASE(S_IFCHR, "c");
		PARSE_PUTS_CASE(S_IFBLK, "b");
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
	PARSE_PERM(S_IRUSR, 'r');
	PARSE_PERM(S_IWUSR, 'w');
	PARSE_PERM(S_IXUSR, 'x');
	PARSE_PERM(S_IRGRP, 'r');
	PARSE_PERM(S_IWGRP, 'w');
	PARSE_PERM(S_IXGRP, 'x');
	PARSE_PERM(S_IROTH, 'r');
	PARSE_PERM(S_IWOTH, 'w');
	PARSE_PERM(S_IXOTH, 'x');
	putchar('\n');
}

static void
reveal_oct_perms(struct stat *s)
{
	printf("%o\n", s->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
	S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
}

static int
reveal_usr(char *path, struct stat *s)
{
	struct passwd *u = getpwuid(s->st_uid);
	return u ? puts(u->pw_name) == EOF : print_err("can't find user that "
	"owns \"", path, "\".", "Ensure that it is not a dangling symlink.");
}

static int
reveal_grp(char *path, struct stat *s)
{
	struct group *g = getgrgid(s->st_gid);
	return g ? puts(g->gr_name) == EOF : print_err("can't find group that "
	"owns \"", path, "\".", "Ensure that it is not a dangling symlink.");
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
	return strftime(m, sizeof(m), "%a %b %d %T %Z %Y",
	localtime(&s->st_mtime)) ? puts(m) == EOF : print_err("overflowed "
	"modified date buffer.", NULL, NULL, NULL);
}

static int
reveal_file(char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return print_err("can't open file \"", path, "\"", "Check if "
		"you have permission to read it.");
	for (char c; (c = fgetc(f)) != EOF; putchar(c));
	return fclose(f);
}

static size_t
get_tot_dirents(DIR *d)
{
	size_t t = 0;
	for (; readdir(d); t++);
	return t -= 2;
}

static void
alloc_dir_ents(DIR *d, void **buf)
{
	size_t i = 0;
	for (struct dirent *e; (e = readdir(d));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		size_t l = strlen(e->d_name) + 1;
		void *a = malloc(l);
		if (!a)
			throw_err("can't allocate memory.");
		memcpy(a, e->d_name, l);
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
	size_t t = get_tot_dirents(d);
	if (!t)
		return closedir(d);
	void *e[t];
	rewinddir(d);
	alloc_dir_ents(d, e);
	sort_dir_ents(e, t);
	for (size_t i = 0; i < t; i++) {
		puts(e[i]);
		free(e[i]);
	}
	return closedir(d);
}

static int
reveal_ctts(char *path, struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		PARSE_RET_CASE(S_IFREG, reveal_file(path));
		PARSE_RET_CASE(S_IFDIR, reveal_dir(path));
		PARSE_RET_CASE(S_IFLNK, print_err("can't read symlink \"", path,
		"\".", "Try to use the \"-fl\" option right before it."));
	}
	return print_err("can't read contents of \"", path, "\".", "Its type "
	"is unreadable.");
}

static int
reveal(char *path)
{
	struct stat s;
	if (follows_lnk ? stat(path, &s) : lstat(path, &s))
		return print_err("can't find entry \"", path, "\".", "Check if "
		"you misspelled it.");
	switch (data_type) {
		PARSE_CASE(DT_TYPE, reveal_type(&s));
		PARSE_CASE(DT_SIZE, reveal_size(&s));
		PARSE_CASE(DT_BT_SIZE, reveal_bt_size(&s));
		PARSE_CASE(DT_PERMS, reveal_perms(&s));
		PARSE_CASE(DT_OCT_PERMS, reveal_oct_perms(&s));
		PARSE_RET_CASE(DT_USR, reveal_usr(path, &s));
		PARSE_CASE(DT_UID, reveal_own_id(s.st_uid));
		PARSE_RET_CASE(DT_GRP, reveal_grp(path, &s));
		PARSE_CASE(DT_GID, reveal_own_id(s.st_gid));
		PARSE_RET_CASE(DT_MOD_DATE, reveal_mod_date(&s));
	default:
		return reveal_ctts(path, &s);
	}
	return 0;
}

static int
parse_lnk_opts(char *path, char *arg, bool is_last)
{
	PARSE_LNK_OPT("fl", true);
	PARSE_LNK_OPT("dfl", false);
	return 0;
}

static int
parse_dt_opts(char *path, char *arg, bool is_last)
{
	PARSE_DT_OPT("c", DT_CTTS);
	PARSE_DT_OPT("t", DT_TYPE);
	PARSE_DT_OPT("s", DT_SIZE);
	PARSE_DT_OPT("bs", DT_BT_SIZE);
	PARSE_DT_OPT("p", DT_PERMS);
	PARSE_DT_OPT("op", DT_OCT_PERMS);
	PARSE_DT_OPT("u", DT_USR);
	PARSE_DT_OPT("ui", DT_UID);
	PARSE_DT_OPT("g", DT_GRP);
	PARSE_DT_OPT("gi", DT_GID);
	PARSE_DT_OPT("md", DT_MOD_DATE);
	return 0;
}

static void
parse_non_meta_opts(int tot_args, char **args)
{
	char *path = ".";
	for (int i = 1; i < tot_args; i++) {
		char *arg = args[i];
		bool is_last = i == tot_args - 1;
		if (parse_dt_opts(path, arg, is_last) ||
		parse_lnk_opts(path, arg, is_last) || reveal(arg))
			continue;
		awaits_arg = false;
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
		return PARSE_EXIT_CD(reveal("."));
	parse_meta_opts(tot_args, args);
	parse_non_meta_opts(tot_args, args);
	return PARSE_EXIT_CD(had_err);
}
