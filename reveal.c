#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define PROGRAM_VERSION "v5.2.0"

#define PARSE_METADATA_FLAG(flag, fct, arg)                                    \
    if (!strcmp(flag, arg))                                                    \
    {                                                                          \
        fct();                                                                 \
        exit(0);                                                               \
    }

void PrintLicense()
{
    puts(PROGRAM_LICENSE);
}

void PrintVersion()
{
    puts(PROGRAM_VERSION);
}

void PrintHelp()
{
    printf("Usage: %s [FLAGS]... [PATHS]...\n", PROGRAM_NAME);
    puts("Reveals information about entries in the file system.");
    puts("");
    puts("METADATA FLAGS");
    puts("These flags show metadata about the program.");
    puts("");
    puts("  --help     prints these help instructions.");
    puts("  --version  prints its version.");
    puts("  --license  prints its license.");
    puts("");
    puts("DATA TYPE FLAGS");
    puts("These flags change what data type the program will reveal from the "
         "entries.");
    puts("");
    puts("  --contents (default)  prints its contents.");
    puts("  --type                prints its type.");
    puts("  --size                prints its size in bytes.");
    puts("  --human-size          prints its size using the most readable "
         "unit.");
    puts("  --user                prints the user that owns it.");
    puts("  --user-id             prints the ID of the user that owns it.");
    puts("  --group               prints the group that owns it.");
    puts("  --group-id            prints the ID of the group that owns it.");
    puts("  --permissions         prints its permissions in octal base.");
    puts("  --human-permissions   prints its permissions for user, group "
         "and others,");
    puts("                        respectively, using three set of characters "
         "each:");
    puts("                        read (r), write (w), execute (x) and lack "
         "(-).");
    puts("  --inode               prints its serial number.");
    puts("  --modified-date       prints the date where its contents were "
         "last modified.");
    puts("");
    puts("TRANSPASSING FLAGS");
    puts("These flags changes the way the symlinks must be handled.");
    puts("");
    puts("  --untranspass (default)  does not resolve symlinks.");
    puts("  --transpass              resolves all levels of symlinks.");
    puts("");
    puts("EXIT CODES");
    puts("It will throw exit code 1 in the end of its execution if an "
         "error happened.");
    puts("");
    puts("However, while still able to continue, it will keep revealing the "
         "remaining");
    puts("arguments.");
    puts("");
    puts("All the errors found will be reported through stderr.");
    puts("");
    puts("SOURCE CODE");
    puts("Its source code is available at:");
    puts("  https://github.com/skippyr/reveal");
    puts("");
    puts("ISSUES");
    puts("Report issues found in this program at:");
    puts("  https://github.com/skippyr/reveal/issues");
}

int main(int argc, const char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        PARSE_METADATA_FLAG("--license", PrintLicense, argv[i]);
        PARSE_METADATA_FLAG("--help", PrintHelp, argv[i]);
        PARSE_METADATA_FLAG("--version", PrintVersion, argv[i]);
    }
}
