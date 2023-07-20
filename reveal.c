#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define programname__ "reveal"
#define programversion__ "v8.0.0"
#define programcopyright__ "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programhelp__ "help"

#define istranspassingbit__ (1 << 6)
#define haderrorbit__ (1 << 7)
#define nontypebits__ (istranspassingbit__ | haderrorbit__)

#define Parse_Flag__(f, a) if (!strcmp("--" f, args[i])) {a}
#define Parse_Meta_Flag__(f, t) Parse_Flag__(f, puts(t); return (0);)
#define Parse_Opt_Flag__(f, a) Parse_Flag__(f, a continue;)
#define Parse_Type_Flag__(f, t)\
	Parse_Opt_Flag__(f, opts = t | opts & nontypebits__;)

#define Parse_Fct_Case__(v, f) case v: f; break;

#define Puts_Long__(v) printf("%ld\n", v);
#define Puts_Unsigned__(v) printf("%u\n", v);
#define Puts_Unsigned_Long__(v) printf("%lu\n", v);

uint8_t opts = 0;

void
Print_Error(char *e0, char *e1, char *e2)
{
	fprintf(stderr, "%s: %s%s%s\n", programname__, e0, e1, e2);
	opts |= haderrorbit__;
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
		puts(e->d_name);
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
	default:
		switch (s.st_mode & S_IFMT)
		{
		Parse_Fct_Case__(S_IFREG, Reveal_File(p))
		Parse_Fct_Case__(S_IFDIR, Reveal_Directory(p))
		default:
			Print_Error("can not reveal contents of the type \"", p, "\" is.");
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

