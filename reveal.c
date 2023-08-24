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
#define EXIT_CD(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define NULL_STR(s) (s ? s : "")
#define PRS_R_CASE(v, a) case v: return a;
#define PRS_CASE(v, a) case v: a; break;
#define PRS_PUTS_CASE(v, t) PRS_CASE(v, puts(t))
#define PRS_SZ_P_MUL(p, c) z = s->st_size / (p); if ((int)z) {\
	printf("%.1f%cB\n", z, c); return;}
#define PRS_PRM(p, c) putchar(s->st_mode & p ? c : '-')
#define PRS_OPT(o, t, a) if (!strcmp("-" o, t)) {a;}
#define PRS_MT_OPT(o, t) PRS_OPT(o, a[i], t; exit(EXIT_SUCCESS))
#define PRS_DT_OPT(o, d) PRS_OPT(o, g, if (AW_ARG) {rvl(p);} DT = d;\
	AW_ARG = true; if (l) {rvl(p);}; return 1)
#define PRS_LNK_OPT(o, f) PRS_OPT(o, g, if (l) {rvl(p);} FLW_LNK = f; return 1;)

enum dt {
	DT_CT,
	DT_TP,
	DT_SZ,
	DT_BT_SZ,
	DT_PRM,
	DT_OCT_PRM,
	DT_USR,
	DT_USR_ID,
	DT_GRP,
	DT_GRP_ID,
	DT_M_DATE
};

static enum dt DT = DT_CT;
static bool H_ERR = false, FLW_LNK = true, AW_ARG = false;

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
		"symlink (l),\n                socket (s), fifo (f), character "
		"device (c), block device (b)\n                or unknown "
		"(-).");
	puts("  -s            print its size in a convenient unit: gigabyte "
		"(GB),\n                megabyte (MB), kilobyte (kB) or byte "
		"(B).");
	puts("  -bs           print its size in bytes with no unit besides.");
	puts("  -p            print its read (r), write (w), execute (x) and "
		"lack (-)\n                permissions for user, group and "
		"others.");
	puts("  -op           print its permissions in octal base.");
	puts("  -u            print the user that owns it.");
	puts("  -ui           print the ID of the user that owns it.");
	puts("  -g            print the group that owns it.");
	puts("  -gi           print the ID of the group that owns it.");
	puts("  -md           print the date when its contents were last "
		"modified.\n");
	puts("All these options expect a path following them. If not provided, "
		"they will\nconsider the last valid one given or, else, the "
		"current directory.\n");
	puts("If none of these is provided, the one marked as default will be "
		"considered.\n");
	puts("SYMLINKS OPTIONS");
	puts("These options change how symlinks following them will be "
		"handled, possibly\nchanging the origin of the data "
		"retrieved.\n");
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
pr_err(char *d0, char *d1, char *d2, char *f)
{
	fprintf(stderr, "%s: %s%s%s\n%s%s", PGR_NAME, NULL_STR(d0),
		NULL_STR(d1), NULL_STR(d2), NULL_STR(f), f ? "\n" : "");
	H_ERR = true;
	return 1;
}

static int
die(char *d)
{
	pr_err(d, NULL, NULL, NULL);
	exit(EXIT_FAILURE);
}

static void
rvl_tp(struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		PRS_PUTS_CASE(S_IFREG, "r");
		PRS_PUTS_CASE(S_IFDIR, "d");
		PRS_PUTS_CASE(S_IFLNK, "l");
		PRS_PUTS_CASE(S_IFSOCK, "s");
		PRS_PUTS_CASE(S_IFIFO, "f");
		PRS_PUTS_CASE(S_IFCHR, "c");
		PRS_PUTS_CASE(S_IFBLK, "b");
	default:
		puts("-");
	}
}

static void
rvl_sz(struct stat *s)
{
	float z = 0;
	PRS_SZ_P_MUL(1e9, 'G');
	PRS_SZ_P_MUL(1e6, 'M');
	PRS_SZ_P_MUL(1e3, 'k');
	printf("%ldB\n", s->st_size);
}

static void
rvl_bt_sz(struct stat *s)
{
	printf("%ld\n", s->st_size);
}

static void
rvl_prm(struct stat *s)
{
	PRS_PRM(S_IRUSR, 'r');
	PRS_PRM(S_IWUSR, 'w');
	PRS_PRM(S_IXUSR, 'x');
	PRS_PRM(S_IRGRP, 'r');
	PRS_PRM(S_IWGRP, 'w');
	PRS_PRM(S_IXGRP, 'x');
	PRS_PRM(S_IROTH, 'r');
	PRS_PRM(S_IWOTH, 'w');
	PRS_PRM(S_IXOTH, 'x');
	putchar('\n');
}

static void
rvl_oct_prm(struct stat *s)
{
	printf("%o\n", s->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
		S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
}

static int
rvl_usr(struct stat *s, char *p)
{
	struct passwd *u = getpwuid(s->st_uid);
	if (!u)
		return pr_err("can't find user that owns \"", p, "\".",
			"Ensure that it is not a dangling symlink.");
	puts(u->pw_name);
	return 0;
}

static int
rvl_grp(struct stat *s, char *p)
{
	struct group *g = getgrgid(s->st_gid);
	if (!g)
		return pr_err("can't find group that owns \"", p, "\".",
			"Ensure that it is not a dangling symlink.");
	puts(g->gr_name);
	return 0;
}

static int
rvl_reg(char *p)
{
	FILE *f = fopen(p, "r");
	if (!f)
		return pr_err("can't open file \"", p, "\"", "Check if you "
			"have permission to read it.");
	char c;
	while ((c = fgetc(f)) != EOF)
		putchar(c);
	fclose(f);
	return 0;
}

static void
rvl_own_id(unsigned i)
{
	printf("%u\n", i);
}

static int
rvl_m_date(struct stat *s)
{
	char m[29];
	if (!strftime(m, sizeof(m), "%a %b %d %T %Z %Y",
		localtime(&s->st_mtime)))
		return pr_err("overflowed modified date buffer.", NULL, NULL,
			NULL);
	puts(m);
	return 0;
}

static int
g_dir_sz(DIR *d)
{
	size_t s = 0;
	for (; readdir(d); s++);
	return s -= 2;
}

static void
alloc_dir_e(DIR *d, void **e)
{
	struct dirent *t;
	size_t i = 0;
	rewinddir(d);
	while ((t = readdir(d))) {
		if (!strcmp(t->d_name, ".") || !strcmp(t->d_name, ".."))
			continue;
		size_t s = strlen(t->d_name) + 1;
		void *a = malloc(s);
		if (!a)
			die("can't allocate memory.");
		memcpy(a, t->d_name, s);
		e[i] = a;
		i++;
	}
}

static void
srt_dir_e(void **e, size_t s)
{
	for (size_t i = 0; i < s - 1; i++) {
		size_t w = i;
		for (size_t c = i + 1; c < s; c++)
			if (strcmp(e[c], e[w]) < 0)
				w = c;
		if (w == i)
			continue;
		void *t = e[i];
		e[i] = e[w];
		e[w] = t;
	}
}

static int
rvl_dir(char *p)
{
	DIR *d = opendir(p);
	if (!d)
		return pr_err("can't open directory \"", p, "\"", "Check if "
			"you have permission to read it.");
	size_t s = g_dir_sz(d);
	if (!s) {
		closedir(d);
		return 0;
	}
	void *e[s];
	alloc_dir_e(d, e);
	srt_dir_e(e, s);
	for (size_t i = 0; i < s; i++) {
		puts(e[i]);
		free(e[i]);
	}
	closedir(d);
	return 0;
}

static int
rvl_ct(struct stat *s, char *p)
{
	switch (s->st_mode & S_IFMT) {
		PRS_R_CASE(S_IFREG, rvl_reg(p));
		PRS_R_CASE(S_IFDIR, rvl_dir(p));
		PRS_R_CASE(S_IFLNK, pr_err("can't read symlink \"", p, "\".",
			"Try to use the \"-fl\" option right before it."));
	default:
		return pr_err("can't read contents of \"", p, "\".",
			"Its type is unreadable.");
	}
}

static int
rvl(char *p)
{
	struct stat s;
	if (FLW_LNK ? stat(p, &s) : lstat(p, &s))
		return pr_err("can't find entry \"", p, "\".", "Check if you "
			"misspelled it.");
	switch (DT) {
		PRS_CASE(DT_TP, rvl_tp(&s));
		PRS_CASE(DT_SZ, rvl_sz(&s));
		PRS_CASE(DT_BT_SZ, rvl_bt_sz(&s));
		PRS_CASE(DT_PRM, rvl_prm(&s));
		PRS_CASE(DT_OCT_PRM, rvl_oct_prm(&s));
		PRS_R_CASE(DT_USR, rvl_usr(&s, p));
		PRS_CASE(DT_USR_ID, rvl_own_id(s.st_uid));
		PRS_R_CASE(DT_GRP, rvl_grp(&s, p));
		PRS_CASE(DT_GRP_ID, rvl_own_id(s.st_gid));
		PRS_R_CASE(DT_M_DATE, rvl_m_date(&s));
	default:
		return rvl_ct(&s, p);
	}
	return 0;
}

static int
prs_lnk_opts(char *p, char *g, bool l)
{
	PRS_LNK_OPT("fl", true);
	PRS_LNK_OPT("dfl", false);
	return 0;
}

static int
prs_dt_opts(char *p, char *g, bool l)
{
	PRS_DT_OPT("c", DT_CT);
	PRS_DT_OPT("t", DT_TP);
	PRS_DT_OPT("s", DT_SZ);
	PRS_DT_OPT("bs", DT_BT_SZ);
	PRS_DT_OPT("p", DT_PRM);
	PRS_DT_OPT("op", DT_OCT_PRM);
	PRS_DT_OPT("u", DT_USR);
	PRS_DT_OPT("ui", DT_USR_ID);
	PRS_DT_OPT("g", DT_GRP);
	PRS_DT_OPT("gi", DT_GRP_ID);
	PRS_DT_OPT("md", DT_M_DATE);
	return 0;
}

static void
prs_nmt_opts(int c, char **a)
{
	char *p = ".";
	for (int i = 1; i < c; i++) {
		char *g = a[i];
		bool l = i == c - 1;
		if (prs_dt_opts(p, g, l) || prs_lnk_opts(p, g, l) || rvl(g))
			continue;
		AW_ARG = false;
		p = g;
	}
}

static void
prs_mt_opts(int c, char **a)
{
	for (int i = 1; i < c; i++) {
		PRS_MT_OPT("v", puts(PGR_VRS));
		PRS_MT_OPT("h", help());
	}
}

int
main(int c, char **a)
{
	if (c == 1)
		return EXIT_CD(rvl("."));
	prs_mt_opts(c, a);
	prs_nmt_opts(c, a);
	return EXIT_CD(H_ERR);
}
