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
#define PGRVRS "v11.0.1"
#define PARSEEXITCD(e) (e ? EXIT_FAILURE : EXIT_SUCCESS)
#define PARSENULLSTR(s) (s ? s : "")
#define PARSERETCASE(v, a) case v: return a;
#define PARSECASE(v, a) case v: a; break;
#define PARSEPUTSCASE(v, t) PARSECASE(v, puts(t))
#define PARSESIZEPREFMUL(m, c) z = s->st_size / (m); if ((int)z) {\
printf("%.1f%cB\n", z, c); return;}
#define PARSEPERM(p, c) putchar(s->st_mode & p ? c : '-')
#define PARSEOPT(o, t, a) if (!strcmp("-" o, t)) {a;}
#define PARSEMETAOPT(o, a) PARSEOPT(o, args[i], a; exit(EXIT_SUCCESS))
#define PARSEDTOPT(o, d) PARSEOPT(o, arg, if (AWARG) {reveal(path);} DT = d;\
AWARG = true; if (islast) {reveal(path);} return 1;)
#define PARSELNKOPT(o, f) PARSEOPT(o, arg, if (islast) {reveal(path);}\
FLLNK = f; return 1;)

enum datatype {
	DT_CTTS,
	DT_TYPE,
	DT_SIZE,
	DT_BSIZE,
	DT_PERMS,
	DT_OPERMS,
	DT_USR,
	DT_UID,
	DT_GRP,
	DT_GID,
	DT_MDATE
};

static enum datatype DT = DT_CTTS;
static bool HERR = false, FLLNK = true, AWARG = false;

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
	printf("Code %i will be throw if an error happens and %i "
	"otherwise.\n\n", EXIT_FAILURE, EXIT_SUCCESS);
	puts("SOURCE CODE");
	puts("Its source code is available at:");
	puts("<https://github.com/skippyr/reveal>.\n");
	puts("SUPPORT");
	puts("Report issues, questions and suggestions through its issues "
	"page:");
	puts("<https://github.com/skippyr/reveal/issues>");
}

static int printerr(char *desc0, char *desc1, char *desc2, char *fix) {
	return (HERR = fprintf(stderr, "%s: %s%s%s\n%s%s", PGRNAME,
	PARSENULLSTR(desc0), PARSENULLSTR(desc1), PARSENULLSTR(desc2),
	PARSENULLSTR(fix), fix ? "\n" : ""));
}

static void throwerr(char *desc) {
	printerr(desc, NULL, NULL, NULL);
	exit(EXIT_FAILURE);
}

static void revealtype(struct stat *s) {
	switch (s->st_mode & S_IFMT) {
		PARSEPUTSCASE(S_IFREG, "r");
		PARSEPUTSCASE(S_IFDIR, "d");
		PARSEPUTSCASE(S_IFLNK, "l");
		PARSEPUTSCASE(S_IFSOCK, "s");
		PARSEPUTSCASE(S_IFIFO, "f");
		PARSEPUTSCASE(S_IFCHR, "c");
		PARSEPUTSCASE(S_IFBLK, "b");
	default:
		puts("-");
	}
}

static void revealsize(struct stat *s) {
	float z;
	PARSESIZEPREFMUL(1e9, 'G');
	PARSESIZEPREFMUL(1e6, 'M');
	PARSESIZEPREFMUL(1e3, 'k');
	printf("%ldB\n", s->st_size);
}

static void revealbsize(struct stat *s) {
	printf("%ld\n", s->st_size);
}

static void revealperms(struct stat *s) {
	PARSEPERM(S_IRUSR, 'r');
	PARSEPERM(S_IWUSR, 'w');
	PARSEPERM(S_IXUSR, 'x');
	PARSEPERM(S_IRGRP, 'r');
	PARSEPERM(S_IWGRP, 'w');
	PARSEPERM(S_IXGRP, 'x');
	PARSEPERM(S_IROTH, 'r');
	PARSEPERM(S_IWOTH, 'w');
	PARSEPERM(S_IXOTH, 'x');
	putchar('\n');
}

static void revealoperms(struct stat *s) {
	printf("%o\n", s->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
	S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
}

static int revealusr(char *path, struct stat *s) {
	struct passwd *u = getpwuid(s->st_uid);
	return u ? puts(u->pw_name) == EOF : printerr("can't find user that "
	"owns \"", path, "\".", "Ensure that it is not a dangling symlink.");
}

static int revealgrp(char *path, struct stat *s) {
	struct group *g = getgrgid(s->st_gid);
	return g ? puts(g->gr_name) == EOF : printerr("can't find group that "
	"owns \"", path, "\".", "Ensure that it is not a dangling symlink.");
}

static void revealownid(unsigned i) {
	printf("%u\n", i);
}

static int revealfile(char *path) {
	FILE *f = fopen(path, "r");
	if (!f)
		return printerr("can't open file \"", path, "\"", "Check if "
		"you have permission to read it.");
	for (char c; (c = fgetc(f)) != EOF; putchar(c));
	return fclose(f);
}

static size_t gettotdirents(DIR *d) {
	size_t t = 0;
	for (; readdir(d); t++);
	return t -= 2;
}

static void allocdirents(DIR *d, void **buf) {
	size_t i = 0;
	for (struct dirent *e; (e = readdir(d));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		size_t l = strlen(e->d_name) + 1;
		void *a = malloc(l);
		if (!a)
			throwerr("can't allocate memory.");
		memcpy(a, e->d_name, l);
		buf[i] = a;
		i++;
	}
}

static void sortdirents(void **buf, size_t buflen) {
	for (size_t i = 0; i < buflen - 1; i++) {
		size_t w = i;
		for (size_t j = i + 1; j < buflen; j++)
			if (strcmp(buf[j], buf[w]) < 0)
				w = j;
		void *t = buf[i];
		buf[i] = buf[w];
		buf[w] = t;
	}
}

static int revealdir(char *path) {
	DIR *d = opendir(path);
	if (!d)
		return printerr("can't open directory \"", path, "\"", "Check "
		"if you have permission to read it.");
	size_t t = gettotdirents(d);
	if (!t)
		return closedir(d);
	void *e[t];
	rewinddir(d);
	allocdirents(d, e);
	sortdirents(e, t);
	for (size_t i = 0; i < t; i++) {
		puts(e[i]);
		free(e[i]);
	}
	return closedir(d);
}

static int revealmdate(struct stat *s) {
	char m[29];
	return strftime(m, sizeof(m), "%a %b %d %T %Z %Y",
	localtime(&s->st_mtime)) ? puts(m) == EOF : printerr("overflowed "
	"modified date buffer.", NULL, NULL, NULL);
}

static int revealctts(char *path, struct stat *s) {
	switch (s->st_mode & S_IFMT) {
		PARSERETCASE(S_IFREG, revealfile(path));
		PARSERETCASE(S_IFDIR, revealdir(path));
		PARSERETCASE(S_IFLNK, printerr("can't read symlink \"", path,
		"\".", "Try to use the \"-fl\" option right before it."));
	}
	return printerr("can't read contents of \"", path, "\".", "Its type is "
	"unreadable.");
}

static int reveal(char *path) {
	struct stat s;
	if (FLLNK ? stat(path, &s) : lstat(path, &s))
		return printerr("can't find entry \"", path, "\".", "Check if "
		"you misspelled it.");
	switch (DT) {
		PARSECASE(DT_TYPE, revealtype(&s));
		PARSECASE(DT_SIZE, revealsize(&s));
		PARSECASE(DT_BSIZE, revealbsize(&s));
		PARSECASE(DT_PERMS, revealperms(&s));
		PARSECASE(DT_OPERMS, revealoperms(&s));
		PARSERETCASE(DT_USR, revealusr(path, &s));
		PARSECASE(DT_UID, revealownid(s.st_uid));
		PARSERETCASE(DT_GRP, revealgrp(path, &s));
		PARSECASE(DT_GID, revealownid(s.st_gid));
		PARSERETCASE(DT_MDATE, revealmdate(&s));
	default:
		return revealctts(path, &s);
	}
	return 0;
}

static int parselnkopts(char *path, char *arg, bool islast) {
	PARSELNKOPT("fl", true);
	PARSELNKOPT("dfl", false);
	return 0;
}

static int parsedtopts(char *path, char *arg, bool islast) {
	PARSEDTOPT("c", DT_CTTS);
	PARSEDTOPT("t", DT_TYPE);
	PARSEDTOPT("s", DT_SIZE);
	PARSEDTOPT("bs", DT_BSIZE);
	PARSEDTOPT("p", DT_PERMS);
	PARSEDTOPT("op", DT_OPERMS);
	PARSEDTOPT("u", DT_USR);
	PARSEDTOPT("ui", DT_UID);
	PARSEDTOPT("g", DT_GRP);
	PARSEDTOPT("gi", DT_GID);
	PARSEDTOPT("md", DT_MDATE);
	return 0;
}

static void parsenonmetaopts(int totargs, char **args) {
	char *path = ".";
	for (int i = 1; i < totargs; i++) {
		char *arg = args[i];
		bool islast = i == totargs - 1;
		if (parsedtopts(path, arg, islast) ||
		parselnkopts(path, arg, islast) || reveal(arg))
			continue;
		AWARG = false;
		path = arg;
	}
}

static void parsemetaopts(int totargs, char **args) {
	for (int i = 1; i < totargs; i++) {
		PARSEMETAOPT("v", puts(PGRVRS));
		PARSEMETAOPT("h", help());
	}
}

int main(int totargs, char **args) {
	if (totargs == 1)
		return PARSEEXITCD(reveal("."));
	parsemetaopts(totargs, args);
	parsenonmetaopts(totargs, args);
	return PARSEEXITCD(HERR);
}

