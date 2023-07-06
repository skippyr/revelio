#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define PROGRAM_VERSION "v5.2.0"

#define PARSE_METADATA_FLAG(flag, fct, arg)                                    \
    if (!strcmp(flag, arg))                                                    \
    {                                                                          \
        fct();                                                                 \
        exit(0);                                                               \
    }

uint8_t exitCode = EXIT_SUCCESS;

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

void PrintComposedError(const char *const descriptionStart,
                        const char *const descriptionMiddle,
                        const char *const descriptionEnd)
{
    fprintf(stderr, "%s: %s%s%s\n", PROGRAM_NAME, descriptionStart,
            descriptionMiddle, descriptionEnd);
    exitCode = EXIT_FAILURE;
}

void RevealType(const struct stat *const metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
    case S_IFBLK:
        puts("Block");
        break;
    case S_IFCHR:
        puts("Character");
        break;
    case S_IFDIR:
        puts("Directory");
        break;
    case S_IFIFO:
        puts("Fifo");
        break;
    case S_IFLNK:
        puts("Symlink");
        break;
    case S_IFREG:
        puts("File");
        break;
    case S_IFSOCK:
        puts("Socket");
        break;
    default:
        puts("Unknown");
    }
}

void RevealFile(const char *const path)
{
    FILE *const file = fopen(path, "r");
    if (!file)
    {
        PrintComposedError("could not open file \"", path, "\".");
        return;
    }
    char character;
    while ((character = fgetc(file)) != EOF)
        putchar(character);
    fclose(file);
}

void RevealDirectory(const char *const path)
{
    char absolutePath[PATH_MAX];
    if (!realpath(path, absolutePath))
    {
        PrintComposedError("could not resolve absolute path of \"", path,
                           "\".");
        return;
    }
    DIR *const directory = opendir(path);
    if (!directory)
    {
        PrintComposedError("could not open directory \"", path, "\".");
        return;
    }
    const char *const separator = !strcmp(absolutePath, "/") ? "" : "/";
    const struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        printf("%s%s%s\n", absolutePath, separator, entry->d_name);
    }
    closedir(directory);
}

void Reveal(const char *const path, const uint8_t dataType,
            const uint8_t isTranspassing)
{
    struct stat metadata;
    if (isTranspassing ? stat(path, &metadata) : lstat(path, &metadata))
    {
        PrintComposedError("the path \"", path,
                           "\" does not points to anything.");
        return;
    }
    switch (dataType)
    {
    case 1: // --type
        RevealType(&metadata);
        break;
    default: // --contents
        switch (metadata.st_mode & S_IFMT)
        {
        case S_IFREG:
            RevealFile(path);
            break;
        case S_IFDIR:
            RevealDirectory(path);
            break;
        default:
            PrintComposedError("can not reveal the contents of \"", path,
                               "\" type.");
        }
    }
}

int main(int quantityOfArguments, const char **arguments)
{
    for (int i = 1; i < quantityOfArguments; i++)
    {
        PARSE_METADATA_FLAG("--license", PrintLicense, arguments[i]);
        PARSE_METADATA_FLAG("--help", PrintHelp, arguments[i]);
        PARSE_METADATA_FLAG("--version", PrintVersion, arguments[i]);
    }
    const char *dataTypeFlags[] = {"--contents",    "--type",
                                   "--size",        "--human-size",
                                   "--user",        "--user-id",
                                   "--group",       "--group-id",
                                   "--permissions", "--human-permissions",
                                   "--inode",       "--modified-date"};
    uint8_t dataType = 0, isTranspassing = 0;
    for (int i = 1; i < quantityOfArguments; i++)
    {
        for (uint8_t j = 0; j < sizeof(dataTypeFlags) / sizeof(NULL); j++)
        {
            if (!strcmp(dataTypeFlags[j], arguments[i]))
            {
                dataType = j;
                goto end;
            }
        }
        if (!strcmp("--transpass", arguments[i]))
            isTranspassing = 1;
        else if (!strcmp("--untranspass", arguments[i]))
            isTranspassing = 0;
        else
            Reveal(arguments[i], dataType, isTranspassing);
    end:;
    }
    return exitCode;
}
