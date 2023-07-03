#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define PROGRAM_VERSION "v4.0.1"

void printVersion()
{
    printf("%s\n", PROGRAM_VERSION);
}

void printLicense()
{
    printf("%s\n", PROGRAM_LICENSE);
}

void printHelp()
{
    printf("Usage: %s [FLAGS]... [PATHS]...\n", PROGRAM_NAME);
    printf("Reveals informations about entries in the file system.\n\n");
    printf("META FLAGS\n");
    printf("These flags show metadata about the program.\n\n");
    printf("  --help     prints theses help instructions.\n");
    printf("  --version  prints its version.\n");
    printf("  --license  prints its license.\n\n");
    printf("MODE FLAGS\n");
    printf("These flags change the mode the program will use when revealing an "
           "entry.\n\n");
    printf("  --contents (default)  prints its contents.\n");
    printf("  --size                prints its size in bytes.\n");
    printf("  --human-size          prints its size using the most formidable "
           "unit.\n");
    printf("  --owner               prints its owner.\n");
    printf("  --owner-uid           prints its owner's UID.\n");
    printf("  --group               prints the group that owns it.\n");
    printf(
        "  --group-uid           prints the UID of the group that owns it.\n");
    printf("  --permissions         prints its permissions in octal base.\n");
    printf("  --human-permissions   prints its permissions for owner, group "
           "and others,\n");
    printf(
        "                        respectively, using three set of characters "
        "each:\n");
    printf("                        read (r), write (w), execute (x) and not "
           "set (-).\n");
    printf("  --modified-date       prints the date where its contents were "
           "last modified.\n\n");
    printf("TRANSPASSING FLAGS\n");
    printf("These flags changes the way the metadata of symlinks are "
           "treated.\n\n");
    printf("  --untranspass (default)  do not follow symlinks.\n");
    printf("  --transpass              resolve all levels of symlinks.\n\n");
    printf("SOURCE CODE\n");
    printf("Its source code is available at:\n");
    printf("  https://github.com/skippyr/reveal\n\n");
    printf("ISSUES\n");
    printf("Report issues found in this program at:\n");
    printf("  https://github.com/skippyr/reveal/issues\n\n");
}

void reveal(const char *path, uint8_t mode, uint8_t isTranspassing)
{
}

int main(int argc, const char **argv)
{
    void *metaFlags[][2] = {{"--version", printVersion},
                            {"--license", printLicense},
                            {"--help", printHelp}};
    for (int i = 1; i < argc; i++)
        for (uint8_t j = 0; j < sizeof(metaFlags) / (sizeof(NULL) * 2); j++)
            if (!strcmp(metaFlags[j][0], argv[i]))
            {
                ((void (*)())metaFlags[j][1])();
                exit(0);
            }
    uint8_t mode = 0;
    uint8_t isTranspassing = 0;
    const char *modeFlags[] = {"--contents",          "--size",
                               "--human-size",        "--owner",
                               "--owner-uid",         "--group",
                               "--group-uid",         "--permissions",
                               "--human-permissions", "--modified-date"};
    for (int i = 1; i < argc; i++)
    {
        uint8_t changedMode = 0;
        for (uint8_t j = 0; j < sizeof(modeFlags) / sizeof(NULL); j++)
            if (!strcmp(modeFlags[j], argv[i]))
            {
                mode = j;
                changedMode = 1;
                break;
            }
        if (changedMode)
            continue;
        if (!strcmp("--transpass", argv[i]))
            isTranspassing = 1;
        else if (!strcmp("--untranspass", argv[i]))
            isTranspassing = 0;
        else
            reveal(argv[i], mode, isTranspassing);
    }
}
