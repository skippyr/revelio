#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define PGRNAME "reveal"
#define PGRVRS "v11.0.0"
#define EXITCD(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define NULLSTR(s) (s ? s : "")
#define PRSRCASE(v, a) case v: return a;
#define PRSCASE(v, a) case v: a; break;
#define PRSPUTSCASE(v, t) PRSCASE(v, puts(t))
#define PRSSPREFMUL(p, c) z = s->st_size / (p); if ((int)z) {\
printf("%.1f%cB\n", z, c); return;}
#define PRSP(p, c) putchar(s->st_mode & p ? c : '-')
#define PRSOPT(o, t, a) if (!strcmp("-" o, t)) {a;}
#define PRSMTOPT(o, t) PRSOPT(o, a[i], t; exit(EXIT_SUCCESS))
#define PRSDTOPT(o, d) PRSOPT(o, g, if (EARG) {rvl(p);} DT = d; EARG = true;\
if (l) {rvl(p);}; return 1)
#define PRSLNKOPT(o, f) PRSOPT(o, g, if (l) {rvl(p);} FLNK = f; return 1;)

enum dt {
	DT_C,
	DT_T,
	DT_S,
	DT_BS,
	DT_P,
	DT_OP,
	DT_U,
	DT_UI,
	DT_G,
	DT_GI,
	DT_MD
};

static enum dt DT = DT_C;
static bool HERR = false, FLNK = true, EARG = false;

static void help(void) {
	printf("Usage: %s [OPTION | PATH]...\n", PGRNAME);
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

static int perr(char *d0, char *d1, char *d2, char *f) {
	fprintf(stderr, "%s: %s%s%s\n%s%s", PGRNAME, NULLSTR(d0),
	NULLSTR(d1), NULLSTR(d2), NULLSTR(f), f ? "\n" : "");
	HERR = true;
	return 1;
}

static int die(char *d) {
	perr(d, NULL, NULL, NULL);
	exit(EXIT_FAILURE);
}

static void rvlt(struct stat *s) {
	switch (s->st_mode & S_IFMT) {
		PRSPUTSCASE(S_IFREG, "r");
		PRSPUTSCASE(S_IFDIR, "d");
		PRSPUTSCASE(S_IFLNK, "l");
		PRSPUTSCASE(S_IFSOCK, "s");
		PRSPUTSCASE(S_IFIFO, "f");
		PRSPUTSCASE(S_IFCHR, "c");
		PRSPUTSCASE(S_IFBLK, "b");
	default:
		puts("-");
	}
}

static void rvls(struct stat *s) {
	float z = 0;
	PRSSPREFMUL(1e9, 'G');
	PRSSPREFMUL(1e6, 'M');
	PRSSPREFMUL(1e3, 'k');
	printf("%ldB\n", s->st_size);
}

static void rvlbs(struct stat *s) {
	printf("%ld\n", s->st_size);
}

static void rvlp(struct stat *s) {
	PRSP(S_IRUSR, 'r');
	PRSP(S_IWUSR, 'w');
	PRSP(S_IXUSR, 'x');
	PRSP(S_IRGRP, 'r');
	PRSP(S_IWGRP, 'w');
	PRSP(S_IXGRP, 'x');
	PRSP(S_IROTH, 'r');
	PRSP(S_IWOTH, 'w');
	PRSP(S_IXOTH, 'x');
	putchar('\n');
}

static void rvlop(struct stat *s) {
	printf("%o\n", s->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
	S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
}

static int rvlu(struct stat *s, char *p) {
	struct passwd *u = getpwuid(s->st_uid);
	if (!u)
		return perr("can't find user that owns \"", p, "\".", "Ensure "
		"that it is not a dangling symlink.");
	puts(u->pw_name);
	return 0;
}

static int rvlg(struct stat *s, char *p) {
	struct group *g = getgrgid(s->st_gid);
	if (!g)
		return perr("can't find group that owns \"", p, "\".", "Ensure "
		"that it is not a dangling symlink.");
	puts(g->gr_name);
	return 0;
}

static int rvlreg(char *p) {
	FILE *f = fopen(p, "r");
	if (!f)
		return perr("can't open file \"", p, "\"", "Check if you have "
		"permission to read it.");
	char c;
	while ((c = fgetc(f)) != EOF)
		putchar(c);
	fclose(f);
	return 0;
}

static void rvlid(unsigned i) {
	printf("%u\n", i);
}

static int rvlmd(struct stat *s) {
	char m[29];
	if (!strftime(m, sizeof(m), "%a %b %d %T %Z %Y",
	localtime(&s->st_mtime)))
		return perr("overflowed modified date buffer.", NULL, NULL,
		NULL);
	puts(m);
	return 0;
}

static int gdirs(DIR *d) {
	size_t s = 0;
	for (; readdir(d); s++);
	return s -= 2;
}

static void alcdire(DIR *d, void **e) {
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

static void strdire(void **e, size_t s) {
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

static int rvldir(char *p) {
	DIR *d = opendir(p);
	if (!d)
		return perr("can't open directory \"", p, "\"", "Check if you "
		"have permission to read it.");
	size_t s = gdirs(d);
	if (!s) {
		closedir(d);
		return 0;
	}
	void *e[s];
	alcdire(d, e);
	strdire(e, s);
	for (size_t i = 0; i < s; i++) {
		puts(e[i]);
		free(e[i]);
	}
	closedir(d);
	return 0;
}

static int rvlc(struct stat *s, char *p) {
	switch (s->st_mode & S_IFMT) {
		PRSRCASE(S_IFREG, rvlreg(p));
		PRSRCASE(S_IFDIR, rvldir(p));
		PRSRCASE(S_IFLNK, perr("can't read symlink \"", p, "\".", "Try "
		"to use the \"-fl\" option right before it."));
	default:
		return perr("can't read contents of \"", p, "\".", "Its type "
		"is unreadable.");
	}
}

static int rvl(char *p) {
	struct stat s;
	if (FLNK ? stat(p, &s) : lstat(p, &s))
		return perr("can't find entry \"", p, "\".", "Check if you "
		"misspelled it.");
	switch (DT) {
		PRSCASE(DT_T, rvlt(&s));
		PRSCASE(DT_S, rvls(&s));
		PRSCASE(DT_BS, rvlbs(&s));
		PRSCASE(DT_P, rvlp(&s));
		PRSCASE(DT_OP, rvlop(&s));
		PRSRCASE(DT_U, rvlu(&s, p));
		PRSCASE(DT_UI, rvlid(s.st_uid));
		PRSRCASE(DT_G, rvlg(&s, p));
		PRSCASE(DT_GI, rvlid(s.st_gid));
		PRSRCASE(DT_MD, rvlmd(&s));
	default:
		return rvlc(&s, p);
	}
	return 0;
}

static int prslnkopts(char *p, char *g, bool l) {
	PRSLNKOPT("fl", true);
	PRSLNKOPT("dfl", false);
	return 0;
}

static int prsdtopts(char *p, char *g, bool l) {
	PRSDTOPT("c", DT_C);
	PRSDTOPT("t", DT_T);
	PRSDTOPT("s", DT_S);
	PRSDTOPT("bs", DT_BS);
	PRSDTOPT("p", DT_P);
	PRSDTOPT("op", DT_OP);
	PRSDTOPT("u", DT_U);
	PRSDTOPT("ui", DT_UI);
	PRSDTOPT("g", DT_G);
	PRSDTOPT("gi", DT_GI);
	PRSDTOPT("md", DT_MD);
	return 0;
}

static void prsnmtopts(int c, char **a) {
	char *p = ".";
	for (int i = 1; i < c; i++) {
		char *g = a[i];
		bool l = i == c - 1;
		if (prsdtopts(p, g, l) || prslnkopts(p, g, l) || rvl(g))
			continue;
		EARG = false;
		p = g;
	}
}

static void prsmtopts(int c, char **a) {
	for (int i = 1; i < c; i++) {
		PRSMTOPT("v", puts(PGRVRS));
		PRSMTOPT("h", help());
	}
}

int main(int c, char **a) {
	if (c == 1)
		return EXITCD(rvl("."));
	prsmtopts(c, a);
	prsnmtopts(c, a);
	return EXITCD(HERR);
}
