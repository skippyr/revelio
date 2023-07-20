#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define programname__ "reveal"
#define programversion__ "v8.0.0"
#define programcopyright__ "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programhelp__ "help"

#define istranspassingbit (1 << 6)
#define haderrorbit (1 << 7)
#define nontypebits (istranspassingbit | haderrorbit)

#define Parse_Flag__(f, a) if (!strcmp("--" f, args[i])) {a}
#define Parse_Meta_Flag__(f, t) Parse_Flag__(f, puts(t); return (0);)
#define Parse_Opt_Flag__(f, a) Parse_Flag__(f, a continue;)
#define Parse_Type_Flag__(f, t)\
	Parse_Opt_Flag__(f, opts = t | opts & nontypebits;)

uint8_t opts = 0;

void
Reveal(char *p)
{
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
		Parse_Opt_Flag__("transpass", opts |= istranspassingbit;)
		Parse_Opt_Flag__("untranspass", opts &= ~istranspassingbit;)
		Reveal(args[i]);
	}
	return (0);
}

