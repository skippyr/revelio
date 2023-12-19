/* See LICENSE for copyright and license details. */
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PARSEITFLAG(flag, itval) PARSEFLAG(flag, it = itval; continue);
#define PARSEFLAG(flag, act)\
	if (!strcmp("-" flag, argv[i])) {\
		act;\
	}
#define PARSELFLAG(flag, isflval) PARSEFLAG(flag, isfl = isflval; continue);
#define PARSEMETAFLAG(flag, act) PARSEFLAG(flag, act; return 0);

enum { ItCtts, ItType, ItSize, ItHSize, ItPerms, ItOPerms, ItUsr, ItUid, ItGrp,
       ItGid, ItMDate };

static int alphacmp(const void *str0, const void *str1);
static void *emalloc(size_t len);
static void die(char *fmt, ...);
static void reveal(char *path);
static void revealdir(char *path);
static void revealgrp(char *path, struct stat *s);
static void revealhsize(struct stat *s);
static void reveallnk(char *path);
static void revealmdate(struct stat *s);
static void revealperms(struct stat *s);
static void revealreg(char *path);
static void revealtype(struct stat *s);
static void revealusr(char *path, struct stat *s);

static int it = ItCtts;
static int isfl = 0;

static int
alphacmp(const void *str0, const void *str1)
{
	return strcmp(*(char **)str0, *(char **)str1);
}

static void *
emalloc(size_t len)
{
	void *p;
	if (!(p = malloc(len)))
		die("can't alloc memory.\n");
	return p;
}

static void
die(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "revelio: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

static void
reveal(char *path)
{
	struct stat s;
	if (isfl ? stat(path, &s) : lstat(path, &s))
		die("can't stat \"%s\".\n", path);
	else if (it == ItCtts && S_ISREG(s.st_mode))
		revealreg(path);
	else if (it == ItCtts && S_ISDIR(s.st_mode))
		revealdir(path);
	else if (it == ItCtts && S_ISLNK(s.st_mode))
		reveallnk(path);
	else if (it == ItCtts)
		die("can't reveal contents of \"%s\".\n", path);
	else if (it == ItType)
		revealtype(&s);
	else if (it == ItSize)
		printf("%ld\n", s.st_size);
	else if (it == ItHSize)
		revealhsize(&s);
	else if (it == ItPerms)
		revealperms(&s);
	else if (it == ItOPerms)
		printf("%o\n",
		       s.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				    S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
				    S_IXOTH));
	else if (it == ItUsr)
		revealusr(path, &s);
	else if (it == ItUid)
		printf("%u\n", s.st_uid);
	else if (it == ItGrp)
		revealgrp(path, &s);
	else if (it == ItGid)
		printf("%u\n", s.st_gid);
	else if (it == ItMDate)
		revealmdate(&s);
}

static void
revealdir(char *path)
{
	DIR *d = opendir(path);
	char **entnames;
	char *entname;
	int i;
	int z;
	struct dirent *e;
	if (!d)
		die("can't open directory \"%s\".\n", path);
	for (i = -2; readdir(d); i++);
	if (!i)
		goto close;
	entnames = emalloc(sizeof(NULL) * i);
	i = 0;
	rewinddir(d);
	while ((e = readdir(d))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		entname = emalloc(strlen(e->d_name) + 1);
		entnames[i] = strcpy(entname, e->d_name);
		i++;
	}
	qsort(entnames, i, sizeof(NULL), alphacmp);
	for (z = 0; z < i; z++) {
		printf("%s\n", entnames[z]);
		free(entnames[z]);
	}
	free(entnames);
close:
	closedir(d);
}

static void
revealgrp(char *path, struct stat *s)
{
	char buf[255];
	struct group *res;
	struct group grp;
	if (getgrgid_r(s->st_gid, &grp, buf, sizeof(buf), &res) || !res)
		die("can't find group that owns \"%s\".\n", path);
	printf("%s\n", grp.gr_name);
}

static void
revealhsize(struct stat *s)
{
	char pref[] = { 'G', 'M', 'k' };
	float mult[] = { 1e9, 1e6, 1e3 };
	float size;
	int i;
	for (i = 0; i < 3; i++) {
		if ((size = s->st_size / mult[i]) >= 1) {
			printf("%.1f%cB\n", size, pref[i]);
			return;
		}
	}
	printf("%ldB\n", s->st_size);
}

static void
reveallnk(char *path)
{
	char buf[100];
	buf[readlink(path, buf, sizeof(buf))] = 0;
	printf("%s\n", buf);
}

static void
revealmdate(struct stat *s)
{
	char buf[29];
	strftime(buf, sizeof(buf),"%a %b %d %T %Z %Y", localtime(&s->st_mtime));
	printf("%s\n", buf);
}

static void
revealperms(struct stat *s)
{
	char permchars[] = { 'r', 'w', 'x' };
	unsigned long permflags[] = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,
				      S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH,
				      S_IXOTH };
	int i;
	for (i = 0; i < 9; i++)
		putchar(s->st_mode & permflags[i] ?
			permchars[i < 3 ? i : (i - 3) % 3] : '-');
	putchar('\n');
}

static void
revealreg(char *path)
{
	FILE *f = fopen(path, "r");
	char c;
	if (!f)
		die("can't open file \"%s\".\n", path);
	for (; (c = fgetc(f)) != EOF; putchar(c));
	fclose(f);
}

static void
revealtype(struct stat *s)
{
	printf("%c\n", S_ISREG(s->st_mode) ? 'r' : S_ISDIR(s->st_mode) ? 'd' :
	       S_ISLNK(s->st_mode) ? 'l' : S_ISCHR(s->st_mode) ? 'c' :
	       S_ISBLK(s->st_mode) ? 'b' : S_ISFIFO(s->st_mode) ? 'f' : 's');
}

static void
revealusr(char *path, struct stat *s)
{
	char buf[255];
	struct passwd *res;
	struct passwd usr;
	if (getpwuid_r(s->st_uid, &usr, buf, sizeof(buf), &res) || !res)
		die("can't find user that owns \"%s\".\n", path);
	printf("%s\n", usr.pw_name);
}

int
main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++) {
		PARSEITFLAG("c", ItCtts);
		PARSEITFLAG("t", ItType);
		PARSEITFLAG("s", ItSize);
		PARSEITFLAG("hs", ItHSize);
		PARSEITFLAG("p", ItPerms);
		PARSEITFLAG("op", ItOPerms);
		PARSEITFLAG("u", ItUsr);
		PARSEITFLAG("ui", ItUid);
		PARSEITFLAG("g", ItGrp);
		PARSEITFLAG("gi", ItGid);
		PARSEITFLAG("md", ItMDate);
		PARSELFLAG("ul", 0);
		PARSELFLAG("fl", 1);
		reveal(argv[i]);
	}
	return 0;
}
