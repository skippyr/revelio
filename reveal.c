#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PGR_NAME "reveal"
#define PGR_VRS "v11.0.0"
#define EXIT_CD(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define NULL_STR(s) (s ? s : "")
#define PRS_R_CASE(v, a) case v: return a;
#define PRS_CASE(v, a) case v: a; break;
#define PRS_PUTS_CASE(v, t) PRS_CASE(v, puts(t))
#define PRS_OPT(o, t, a) if (!strcmp("--" o, t)) {a;}
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
	puts("Reveals information about entries in the file system.");
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
			"Try to use the \"--flw-lnk\" option right before "
			"it."));
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
	default:
		return rvl_ct(&s, p);
	}
	return 0;
}

static int
prs_lnk_opts(char *p, char *g, bool l)
{
	PRS_LNK_OPT("follow-symlinks", true);
	PRS_LNK_OPT("unfollow-symlinks", false);
	return 0;
}

static int
prs_dt_opts(char *p, char *g, bool l)
{
	PRS_DT_OPT("contents", DT_CT);
	PRS_DT_OPT("type", DT_TP);
	PRS_DT_OPT("size", DT_SZ);
	PRS_DT_OPT("byte-size", DT_BT_SZ);
	PRS_DT_OPT("permissions", DT_PRM);
	PRS_DT_OPT("oct-permissions", DT_OCT_PRM);
	PRS_DT_OPT("user", DT_USR);
	PRS_DT_OPT("user-id", DT_USR_ID);
	PRS_DT_OPT("group", DT_GRP);
	PRS_DT_OPT("group-gid", DT_GRP_ID);
	PRS_DT_OPT("modified-date", DT_M_DATE);
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
		PRS_MT_OPT("version", puts(PGR_VRS));
		PRS_MT_OPT("help", help());
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
