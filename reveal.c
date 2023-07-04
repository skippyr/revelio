#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define PROGRAM_VERSION "v5.0.2"

#define GIGA 1e9
#define MEGA 1e6
#define KILO 1e3

#define READ_CHAR 'r'
#define WRITE_CHAR 'w'
#define EXEC_CHAR 'x'
#define LACK_CHAR '-'

uint8_t exitCode = 0;

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
    printf("Reveals information about entries in the file system.\n\n");
    printf("METADATA FLAGS\n");
    printf("These flags show metadata about the program.\n\n");
    printf("  --help     prints these help instructions.\n");
    printf("  --version  prints its version.\n");
    printf("  --license  prints its license.\n\n");
    printf("MODE FLAGS\n");
    printf("These flags change the mode the program will use when revealing an "
           "entry.\n\n");
    printf("  --contents (default)  prints its contents.\n");
    printf("  --size                prints its size in bytes.\n");
    printf("  --human-size          prints its size using the most readable "
           "unit.\n");
    printf("  --user                prints the user that owns it.\n");
    printf("  --user-id             prints the ID of the user that owns it.\n");
    printf("  --group               prints the group that owns it.\n");
    printf(
        "  --group-id            prints the ID of the group that owns it.\n");
    printf("  --permissions         prints its permissions in octal base.\n");
    printf("  --human-permissions   prints its permissions for user, group "
           "and others,\n");
    printf(
        "                        respectively, using three set of characters "
        "each:\n");
    printf("                        read (r), write (w), execute (x) and lack "
           "(-).\n");
    printf("  --inode               prints its serial number.\n");
    printf("  --modified-date       prints the date where its contents were "
           "last modified.\n\n");
    printf("TRANSPASSING FLAGS\n");
    printf("These flags changes the way the metadata of symlinks are "
           "treated.\n\n");
    printf("  --untranspass (default)  does not resolve symlinks.\n");
    printf("  --transpass              resolves all levels of symlinks.\n\n");
    printf("EXIT CODES\n");
    printf("It will throw exit code 1 in the end of its execution if an "
           "error happened.\n\n");
    printf("However, while still able to continue, it will keep revealing the "
           "remaining\n");
    printf("arguments.\n\n");
    printf("SOURCE CODE\n");
    printf("Its source code is available at:\n");
    printf("  https://github.com/skippyr/reveal\n\n");
    printf("ISSUES\n");
    printf("Report issues found in this program at:\n");
    printf("  https://github.com/skippyr/reveal/issues\n");
}

void printErr(const char *const start, const char *const middle,
              const char *const end)
{
    fprintf(stderr, "%s: %s%s%s\n", PROGRAM_NAME, start, middle, end);
    exitCode = 1;
}

void revealFile(const char *const path)
{
    FILE *const file = fopen(path, "r");
    if (!file)
    {
        printErr("could not open file \"", path, "\".");
        return;
    }
    char c;
    while ((c = fgetc(file)) != EOF)
    {
        putchar(c);
    }
    fclose(file);
}

void revealDirectory(const char *const path)
{
    char absPath[PATH_MAX];
    if (!realpath(path, absPath))
    {
        printErr("could not resolve absolute path of \"", path, "\".");
        return;
    }
    DIR *const directory = opendir(path);
    if (!directory)
    {
        printErr("could not open directory \"", path, "\".");
        return;
    }
    const char *const separator = !strcmp(absPath, "/") ? "" : "/";
    const struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        printf("%s%s%s\n", absPath, separator, entry->d_name);
    }
    closedir(directory);
}

void printFloatSize(const float value, const char *const separator)
{
    printf("%.1f%s\n", value, separator);
}

void revealHumanSize(const struct stat *const metadata)
{
    const float gb = metadata->st_size / GIGA;
    if ((int)gb > 0)
    {
        printFloatSize(gb, "GB");
        return;
    }
    const float mb = metadata->st_size / MEGA;
    if ((int)mb > 0)
    {
        printFloatSize(mb, "MB");
        return;
    }
    const float kb = metadata->st_size / KILO;
    if ((int)kb > 0)
    {
        printFloatSize(kb, "kB");
        return;
    }
    printf("%ldB\n", metadata->st_size);
}

void revealUser(const struct stat *const metadata, const char *const path)
{
    const struct passwd *const owner = getpwuid(metadata->st_uid);
    if (!owner)
    {
        printErr("could not get owner of \"", path, "\".");
        return;
    }
    printf("%s\n", owner->pw_name);
}

void revealGroup(const struct stat *const metadata, const char *const path)
{
    const struct group *const group = getgrgid(metadata->st_gid);
    if (!group)
    {
        printErr("could not get group of \"", path, "\".");
        return;
    }
    printf("%s\n", group->gr_name);
}

void revealHumanPermissions(const struct stat *const metadata)
{
    unsigned permissions[9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                               S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    for (uint8_t i = 0; i < 9; i++)
    {
        char c;
        if (!(metadata->st_mode & permissions[i]))
            c = LACK_CHAR;
        else if (i == 0 || i == 3 || i == 6)
            c = READ_CHAR;
        else if (i == 1 || i == 4 || i == 7)
            c = WRITE_CHAR;
        else
            c = EXEC_CHAR;
        putchar(c);
    }
    printf("\n");
}

void revealModifiedDate(const struct stat *const metadata)
{
    char date[29];
    if (!strftime(date, sizeof(date), "%a %b %d %T %Z %Y",
                  localtime(&metadata->st_mtime)))
    {
        printErr("overflowed buffer to store date.", "", "");
        return;
    }
    printf("%s\n", date);
}

void reveal(const char *const path, const uint8_t mode,
            const uint8_t isTranspassing)
{
    struct stat metadata;
    if (isTranspassing ? stat(path, &metadata) : lstat(path, &metadata))
    {
        printErr("the path \"", path, "\" does not exists.");
        return;
    }
    switch (mode)
    {
    case 1: // size
        printf("%ld\n", metadata.st_size);
        break;
    case 2: // human-size
        revealHumanSize(&metadata);
        break;
    case 3: // user
        revealUser(&metadata, path);
        break;
    case 4: // user-uid
        printf("%u\n", metadata.st_uid);
        break;
    case 5: // group
        revealGroup(&metadata, path);
        break;
    case 6: // group-uid
        printf("%u\n", metadata.st_gid);
        break;
    case 7: // permissions
        printf("0%o\n", metadata.st_mode &
                            (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |
                             S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
        break;
    case 8: // human-permissions
        revealHumanPermissions(&metadata);
        break;
    case 9: // inode
        printf("%lu\n", metadata.st_ino);
        break;
    case 10: // modified-date
        revealModifiedDate(&metadata);
        break;
    default: // contents
        if (S_ISREG(metadata.st_mode))
            revealFile(path);
        else if (S_ISDIR(metadata.st_mode))
            revealDirectory(path);
        else
            printErr("can not reveal the contents of \"", path, "\" type.");
    }
}

int main(int argc, const char **argv)
{
    void *metadataFlags[][2] = {{"--version", printVersion},
                                {"--license", printLicense},
                                {"--help", printHelp}};
    for (int i = 1; i < argc; i++)
        for (uint8_t j = 0; j < sizeof(metadataFlags) / (sizeof(NULL) * 2); j++)
            if (!strcmp(metadataFlags[j][0], argv[i]))
            {
                ((void (*)())metadataFlags[j][1])();
                exit(0);
            }
    uint8_t mode = 0;
    uint8_t isTranspassing = 0;
    const char *modeFlags[] = {
        "--contents",          "--size",  "--human-size",   "--user",
        "--user-id",           "--group", "--group-id",     "--permissions",
        "--human-permissions", "--inode", "--modified-date"};
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
    return exitCode;
}
