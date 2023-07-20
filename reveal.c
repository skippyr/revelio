#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define programname__ "reveal"
#define programversion__ "v8.0.0"
#define programcopyright__ "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programhelp__ "help"

#define istranspassingbit__ (1 << 6)
#define haderrorbit__ (1 << 7)
#define nontypebits__ (istranspassingbit__ | haderrorbit__)

#define readchar__ 'r'
#define writechar__ 'w'
#define executechar__ 'x'
#define lackchar__ '-'

#define Parse_Flag__(f, a) if (!strcmp("--" f, args[i])) {a}
#define Parse_Meta_Flag__(f, t) Parse_Flag__(f, puts(t); return (0);)
#define Parse_Opt_Flag__(f, a) Parse_Flag__(f, a continue;)
#define Parse_Type_Flag__(f, t)\
	Parse_Opt_Flag__(f, opts = t | opts & nontypebits__;)

#define Parse_Fct_Case__(v, f) case v: f; break;
#define Parse_Puts_Case__(v, t) Parse_Fct_Case__(v, puts(t))
#define Parse_Size__(m, u)\
	h = s->st_size / (m); if ((int) h) {printf("%.1f%cB\n", h, u); return;}
#define Parse_Permission__(p, c) putchar(s->st_mode & p ? c : lackchar__);

#define Puts_Long__(v) printf("%ld\n", v);
#define Puts_Unsigned__(v) printf("%u\n", v);
#define Puts_Unsigned_Long__(v) printf("%lu\n", v);
#define Puts_Permissions__(m)\
	printf("0%o\n", (m) & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |\
	       S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));

uint8_t opts = 0;

void
Print_Error(char *e0, char *e1, char *e2)
{
	fprintf(stderr, "%s: %s%s%s\n", programname__, e0, e1, e2);
	opts |= haderrorbit__;
	return;
}

void
Reveal_Type(struct stat *s)
{
	switch (s->st_mode & S_IFMT)
	{
		Parse_Puts_Case__(S_IFBLK, "block")
		Parse_Puts_Case__(S_IFCHR, "character")
		Parse_Puts_Case__(S_IFIFO, "fifo")
		Parse_Puts_Case__(S_IFLNK, "symlink")
		Parse_Puts_Case__(S_IFSOCK, "socket")
		Parse_Puts_Case__(S_IFREG, "regular")
		Parse_Puts_Case__(S_IFDIR, "directory")
		default:
			puts("unknown");
	}
	return;
}

void
Reveal_Human_Size(struct stat *s)
{
	float h;
	Parse_Size__(1e9, 'G')
	Parse_Size__(1e6, 'M')
	Parse_Size__(1e3, 'K')
	printf("%ldB\n", s->st_size);
	return;
}

void
Reveal_User(struct stat *s, char *p)
{
	struct passwd *u = getpwuid(s->st_uid);
	if (u)
	{
		puts(u->pw_name);
	}
	else
	{
		Print_Error("can not get user that owns \"", p, "\".");
	}
	return;
}

void
Reveal_Group(struct stat *s, char *p)
{
	struct group *g = getgrgid(s->st_gid);
	if (g)
	{
		puts(g->gr_name);
	}
	else
	{
		Print_Error("can not get group that owns \"", p, "\".");
	}
	return;
}

void
Reveal_Human_Permissions(struct stat *s)
{
	Parse_Permission__(S_IRUSR, readchar__)
	Parse_Permission__(S_IWUSR, writechar__)
	Parse_Permission__(S_IXUSR, executechar__)
	Parse_Permission__(S_IRGRP, readchar__)
	Parse_Permission__(S_IWGRP, writechar__)
	Parse_Permission__(S_IXGRP, executechar__)
	Parse_Permission__(S_IROTH, readchar__)
	Parse_Permission__(S_IWOTH, writechar__)
	Parse_Permission__(S_IXOTH, executechar__)
	putchar('\n');
	return;
}

void
Reveal_File(char *p)
{
	FILE *f = fopen(p, "r");
	if (!f)
	{
		Print_Error("can not open file \"", p, "\".");
		return;
	}
	char c;
	while ((c = fgetc(f)) != EOF)
	{
		putchar(c);
	}
	fclose(f);
	return;
}

void
Reveal_Directory(char *p)
{
	char a[PATH_MAX];
	if (!realpath(p, a))
	{
		Print_Error("can not resolve absolute path of \"", p, "\".");
		return;
	}
	DIR *d = opendir(p);
	if (!d)
	{
		Print_Error("can not open directory \"", p, "\".");
		return;
	}
	struct dirent *e;
	while ((e = readdir(d)))
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
		{
			continue;
		}
		printf("%s%s%s\n", a, strcmp(a, "/") ? "/" : "", e->d_name);
	}
	closedir(d);
	return;
}

void
Reveal(char *p)
{
	struct stat s;
	if (opts & istranspassingbit__ ? stat(p, &s) : lstat(p, &s))
	{
		Print_Error("the path \"", p, "\" does not exists.");
		return;
	}
	switch (opts & ~nontypebits__)
	{
		Parse_Fct_Case__(1, Reveal_Type(&s))
		Parse_Fct_Case__(2, Puts_Long__(s.st_size))
		Parse_Fct_Case__(3, Reveal_Human_Size(&s))
		Parse_Fct_Case__(4, Puts_Long__(s.st_blocks))
		Parse_Fct_Case__(5, Puts_Unsigned_Long__(s.st_nlink))
		Parse_Fct_Case__(6, Reveal_User(&s, p))
		Parse_Fct_Case__(7, Puts_Unsigned__(s.st_uid))
		Parse_Fct_Case__(8, Reveal_Group(&s, p))
		Parse_Fct_Case__(9, Puts_Unsigned__(s.st_gid))
		Parse_Fct_Case__(10, Puts_Unsigned__(s.st_mode))
		Parse_Fct_Case__(11, Puts_Permissions__(s.st_mode))
		Parse_Fct_Case__(12, Reveal_Human_Permissions(&s))
		default:
			switch (s.st_mode & S_IFMT)
			{
				Parse_Fct_Case__(S_IFREG, Reveal_File(p))
				Parse_Fct_Case__(S_IFDIR, Reveal_Directory(p))
				default:
					Print_Error("can not reveal contents of the type \"", p,
					            "\" is.");
			}
	}
	return;
}

int
main(int qargs, char **args)
{
	for (int i = 1; i < qargs; i++)
	{
		Parse_Meta_Flag__("version", programversion__)
		Parse_Meta_Flag__("copyright", programcopyright__)
		Parse_Meta_Flag__("help", programhelp__)
	}
	for (int i = 1; i < qargs; i++)
	{
		Parse_Type_Flag__("contents", 0)
		Parse_Type_Flag__("type", 1)
		Parse_Type_Flag__("size", 2)
		Parse_Type_Flag__("human-size", 3)
		Parse_Type_Flag__("blocks", 4)
		Parse_Type_Flag__("hard-links", 5)
		Parse_Type_Flag__("user", 6)
		Parse_Type_Flag__("user-id", 7)
		Parse_Type_Flag__("group", 8)
		Parse_Type_Flag__("group-id", 9)
		Parse_Type_Flag__("mode", 10)
		Parse_Type_Flag__("permissions", 11)
		Parse_Type_Flag__("human-permissions", 12)
		Parse_Type_Flag__("inode", 13)
		Parse_Type_Flag__("modified-date", 14)
		Parse_Type_Flag__("changed-date", 15)
		Parse_Type_Flag__("accessed-date", 16)
		Parse_Opt_Flag__("transpass", opts |= istranspassingbit__;)
		Parse_Opt_Flag__("untranspass", opts &= ~istranspassingbit__;)
		Reveal(args[i]);
	}
	return !!(opts & haderrorbit__);
}

