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

#define NAME "rvl"
#define PARSEFLAG(f, a) if (!strcmp(argv[i], "-" f)) {a;}
#define PARSEMFLAG(f, a) PARSEFLAG(f, a; return EXIT_SUCCESS)
#define PARSENMFLAG(f, a) PARSEFLAG(f, a; continue)
#define PARSEDTFLAG(f, d) PARSENMFLAG(f, dt_g = d)
#define PARSELFLAG(f, l) PARSENMFLAG(f, fl_g = l)

enum {DTC, DTT, DTS, DTHS, DTP, DTOP, DTU, DTUI, DTG, DTGI, DTMD};

static void die(char *fmt, ...);
static void help(void);
static void rvl(char *p);
static void rvldir(char *p);
static void rvlg(char *p, struct stat *s);
static void rvlhs(struct stat *s);
static void rvllnk(char *p);
static void rvlmd(struct stat *s);
static void rvlp(struct stat *s);
static void rvlreg(char *p);
static void rvls(struct stat *s);
static void rvlt(struct stat *s);
static void rvlu(char *p, struct stat *s);

int dt_g = DTC, fl_g = 0;

static void
die(char *fmt, ...)
{
	va_list a;
	va_start(a, fmt);
	fprintf(stderr, NAME ": ");
	vfprintf(stderr, fmt, a);
	va_end(a);
	exit(EXIT_FAILURE);
}

static void
help(void)
{
	puts("Usage: " NAME " [OPTIONS | PATHS]...");
	puts("Reveals info about entries in the file system.\n");
	puts("META OPTIONS");
	puts("Use these options to retrieve info about the program.\n");
	puts("    -h  show these help instructions.");
	puts("    -v  show its version.\n");
	puts("DATA TYPE OPTIONS");
	puts("Use one of these options before paths to change the data type to "
	     "retrieve from");
	puts("them. The default will be used if none is provided.\n");
	puts("    -c (default)  reveals its contents.");
	puts("    -t            reveals its type: regular (r), directory (d), "
	     "symlink (s),");
	puts("                  character (c), block (b), socket (s) or fifo "
	     "(f).");
	puts("    -s            reveals its byte size.");
	puts("    -hs           reveals its size using a human-readable unit.");
	puts("    -p            reveals its read (r), write (w) and execution "
	     "(x) permissions");
	puts("                  for user, group and others.");
	puts("    -op           reveals its octal permissions.");
	puts("    -u            reveals the user that owns it.");
	puts("    -ui           reveals the id of the user that owns it.");
	puts("    -g            reveals the group that owns it.");
	puts("    -gi           reveals the id of the group that owns it.");
	puts("    -md           reveals its last modified date.\n");
	puts("SYMLINK OPTIONS");
	puts("Use one of these options before paths to change the way symlinks "
	     "are handled.");
	puts("The default will be used if none is provided.\n");
	puts("  -ul (default)  unfollows symlinks.");
	puts("  -fl            follows symlinks.\n");
	puts("EXIT CODES");
	printf("It returns %d on success, and %d otherwise.\n", EXIT_SUCCESS,
	       EXIT_FAILURE);
}

static void
rvl(char *p)
{
	struct stat s;
	if (fl_g ? stat(p, &s) : lstat(p, &s))
		die("can't stat \"%s\".\n", p);
	if (dt_g == DTC) {
		if (S_ISREG(s.st_mode))
			rvlreg(p);
		else if (S_ISDIR(s.st_mode))
			rvldir(p);
		else if (S_ISLNK(s.st_mode))
			rvllnk(p);
		else
			die("can't reveal contents of \"%s\".\n", p);
	} else if (dt_g == DTT)
		rvlt(&s);
	else if (dt_g == DTS)
		printf("%ld\n", s.st_size);
	else if (dt_g == DTHS)
		rvlhs(&s);
	else if (dt_g == DTP)
		rvlp(&s);
	else if (dt_g == DTOP)
		printf("%o\n",
		       s.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				    S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
				    S_IXOTH));
	else if (dt_g == DTU)
		rvlu(p, &s);
	else if (dt_g == DTUI)
		printf("%u\n", s.st_uid);
	else if (dt_g == DTG)
		rvlg(p, &s);
	else if (dt_g == DTGI)
		printf("%u\n", s.st_gid);
	else if (dt_g == DTMD)
		rvlmd(&s);
}

static void
rvldir(char *p)
{
	DIR *d = opendir(p);
	if (!d)
		die("can't open directory \"%s\".", p);
	for (struct dirent *e; (e = readdir(d));)
		puts(e->d_name);
	closedir(d);
}

static void
rvlreg(char *p)
{
	FILE *f = fopen(p, "r");
	if (!f)
		die("can't open file \"%s\".", p);
	for (char c; (c = fgetc(f)) != EOF; putchar(c));
	fclose(f);
}

static void
rvlg(char *p, struct stat *s)
{
	char b[255];
	struct group u, *r;
	if (getgrgid_r(s->st_gid, &u, b, sizeof(b), &r) || !r)
		die("can't find group that owns \"%s\".", p);
	puts(u.gr_name);
}

static void
rvlhs(struct stat *s)
{
	float z, m[] = {1e9, 1e6, 1e3};
	char p[] = {'G', 'M', 'k'};
	for (int i = 0; i < 4; i++)
		if ((int)(z = s->st_size / m[i])) {
			printf("%.1f%cB\n", z, p[i]);
			return;
		}
	printf("%ldB\n", s->st_size);
}

static void
rvllnk(char *p)
{
	char b[100];
	b[readlink(p, b, sizeof(b))] = '\0';
	puts(b);
}

static void
rvlmd(struct stat *s)
{
	char m[29];
	strftime(m, sizeof(m),"%a %b %d %T %Z %Y", localtime(&s->st_mtime));
	puts(m);
}

static void
rvlp(struct stat *s)
{
	unsigned long p[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
			     S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
	char c[] = {'r', 'w', 'x'};
	for (int i = 0; i < 9; i++)
		putchar(s->st_mode & p[i] ? c[i < 3 ? i : (i - 3) % 3] : '-');
	putchar('\n');
}

static void
rvlt(struct stat *s)
{
	if (S_ISREG(s->st_mode))
		putchar('r');
	else if (S_ISDIR(s->st_mode))
		putchar('d');
	else if (S_ISLNK(s->st_mode))
		putchar('l');
	else if (S_ISCHR(s->st_mode))
		putchar('c');
	else if (S_ISBLK(s->st_mode))
		putchar('b');
	else if (S_ISSOCK(s->st_mode))
		putchar('s');
	else if (S_ISFIFO(s->st_mode))
		putchar('f');
	putchar('\n');
}

static void
rvlu(char *p, struct stat *s)
{
	char b[255];
	struct passwd u, *r;
	if (getpwuid_r(s->st_uid, &u, b, sizeof(b), &r) || !r)
		die("can't find user that owns \"%s\".", p);
	puts(u.pw_name);
}

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		PARSEMFLAG("h", help());
		PARSEMFLAG("v", puts("v16.0.0"));
	}
	for (int i = 1; i < argc; i++) {
		PARSEDTFLAG("c", DTC);
		PARSEDTFLAG("t", DTT);
		PARSEDTFLAG("s", DTS);
		PARSEDTFLAG("hs", DTHS);
		PARSEDTFLAG("p", DTP);
		PARSEDTFLAG("op", DTOP);
		PARSEDTFLAG("u", DTU);
		PARSEDTFLAG("ui", DTUI);
		PARSEDTFLAG("g", DTG);
		PARSEDTFLAG("gi", DTGI);
		PARSEDTFLAG("md", DTMD);
		PARSELFLAG("fl", 1);
		PARSELFLAG("ul", 0);
		rvl(argv[i]);
	}
	return EXIT_SUCCESS;
}
