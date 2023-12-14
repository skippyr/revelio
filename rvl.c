/* See LICENSE file for copyright and license details. */
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

#define PARSEDTFLAG(flag, dtval) PARSEFLAG(flag, dt = dtval; continue);
#define PARSEFLAG(flag, act) if (!strcmp("-" flag, argv[i])) {act;}
#define PARSELFLAG(flag, isflval) PARSEFLAG(flag, isfl = isflval; continue);
#define PARSEMETAFLAG(flag, act) PARSEFLAG(flag, act; return 0);

enum {DT_CTTS, DT_TYPE, DT_SIZE, DT_HSIZE, DT_PERMS, DT_OPERMS, DT_USR, DT_UID,
      DT_GRP, DT_GID, DT_MDATE};

static int alphacmp(const void *str0, const void *str1);
static void *emalloc(size_t len);
static void die(char *fmt, ...);
static void rvl(char *path);
static void rvldir(char *path);
static void rvlgrp(char *path, struct stat *s);
static void rvlhsize(struct stat *s);
static void rvllnk(char *path);
static void rvlmdate(struct stat *s);
static void rvlperms(struct stat *s);
static void rvlreg(char *path);
static void rvltype(struct stat *s);
static void rvlusr(char *path, struct stat *s);

static int dt = DT_CTTS;
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
	fprintf(stderr, "rvl: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

static void
rvl(char *path)
{
	struct stat s;
	if (isfl ? stat(path, &s) : lstat(path, &s))
		die("can't stat \"%s\".\n", path);
	if (dt == DT_CTTS && S_ISREG(s.st_mode))
		rvlreg(path);
	else if (dt == DT_CTTS && S_ISDIR(s.st_mode))
		rvldir(path);
	else if (dt == DT_CTTS && S_ISLNK(s.st_mode))
		rvllnk(path);
	else if (dt == DT_CTTS)
		die("can't reveal contents of \"%s\".\n", path);
	else if (dt == DT_TYPE)
		rvltype(&s);
	else if (dt == DT_SIZE)
		printf("%ld\n", s.st_size);
	else if (dt == DT_HSIZE)
		rvlhsize(&s);
	else if (dt == DT_PERMS)
		rvlperms(&s);
	else if (dt == DT_OPERMS)
		printf("%o\n",
		       s.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				    S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
				    S_IXOTH));
	else if (dt == DT_USR)
		rvlusr(path, &s);
	else if (dt == DT_UID)
		printf("%u\n", s.st_uid);
	else if (dt == DT_GRP)
		rvlgrp(path, &s);
	else if (dt == DT_GID)
		printf("%u\n", s.st_gid);
	else if (dt == DT_MDATE)
		rvlmdate(&s);
}

static void
rvldir(char *path)
{
	DIR *d = opendir(path);
	char **entnames;
	char *entname;
	int i;
	int z;
	size_t entlen;
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
		entlen = strlen(e->d_name) + 1;
		entname = emalloc(entlen);
		strcpy(entname, e->d_name);
		entnames[i] = entname;
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
rvlgrp(char *path, struct stat *s)
{
	char buf[255];
	struct group *res;
	struct group grp;
	if (getgrgid_r(s->st_gid, &grp, buf, sizeof(buf), &res) || !res)
		die("can't find group that owns \"%s\".\n", path);
	printf("%s\n", grp.gr_name);
}

static void
rvlhsize(struct stat *s)
{
	char pref[] = {'G', 'M', 'k'};
	float mult[] = {1e9, 1e6, 1e3};
	float size;
	int i;
	for (i = 0; i < 3; i++)
		if ((size = s->st_size / mult[i]) >= 1) {
			printf("%.1f%cB\n", size, pref[i]);
			return;
		}
	printf("%ldB\n", s->st_size);
}

static void
rvllnk(char *path)
{
	char buf[100];
	buf[readlink(path, buf, sizeof(buf))] = 0;
	printf("%s\n", buf);
}

static void
rvlmdate(struct stat *s)
{
	char buf[29];
	strftime(buf, sizeof(buf),"%a %b %d %T %Z %Y", localtime(&s->st_mtime));
	printf("%s\n", buf);
}

static void
rvlperms(struct stat *s)
{
	char permchars[] = {'r', 'w', 'x'};
	unsigned long permflags[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,
				     S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH,
				     S_IXOTH};
	int i;
	for (i = 0; i < 9; i++)
		putchar(s->st_mode & permflags[i] ?
			permchars[i < 3 ? i : (i - 3) % 3] : '-');
	putchar('\n');
}

static void
rvlreg(char *path)
{
	FILE *f = fopen(path, "r");
	char c;
	if (!f)
		die("can't open file \"%s\".\n", path);
	for (; (c = fgetc(f)) != EOF; putchar(c));
	fclose(f);
}

static void
rvltype(struct stat *s)
{
	printf("%c\n", S_ISREG(s->st_mode) ? 'r' : S_ISDIR(s->st_mode) ? 'd' :
	       S_ISLNK(s->st_mode) ? 'l' : S_ISCHR(s->st_mode) ? 'c' :
	       S_ISBLK(s->st_mode) ? 'b' : S_ISFIFO(s->st_mode) ? 'f' : 's');
}

static void
rvlusr(char *path, struct stat *s)
{
	char buf[255];
	struct passwd *res;
	struct passwd usr;
	if (getpwuid_r(s->st_uid, &usr, buf, sizeof(buf), &res) || !res)
		die("can't find user that owns \"%s\".\n", path);
	printf("%s\n", usr.pw_name);
}

int
main(int argc, char *argv[])
{
	int i;
	for (i = 1; i < argc; i++) {
		PARSEDTFLAG("c", DT_CTTS);
		PARSEDTFLAG("t", DT_TYPE);
		PARSEDTFLAG("s", DT_SIZE);
		PARSEDTFLAG("hs", DT_HSIZE);
		PARSEDTFLAG("p", DT_PERMS);
		PARSEDTFLAG("op", DT_OPERMS);
		PARSEDTFLAG("u", DT_USR);
		PARSEDTFLAG("ui", DT_UID);
		PARSEDTFLAG("g", DT_GRP);
		PARSEDTFLAG("gi", DT_GID);
		PARSEDTFLAG("md", DT_MDATE);
		PARSELFLAG("ul", 0);
		PARSELFLAG("fl", 1);
		rvl(argv[i]);
	}
	return 0;
}
