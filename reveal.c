#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define programName "reveal"
#define programVersion "v7.0.0"
#define programLicense "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programHelp                                                            \
    "Usage: " programName " [FLAGS, ENTRIES...]\n"                             \
    "Reveals information about entries in the file system.\n\n"                \
    "METADATA FLAGS\n"                                                         \
    "These flags can be used to retrieve metadata about the program.\n\n"      \
    "  --help     prints these help instructions.\n"                           \
    "  --version  prints its version.\n"                                       \
    "  --license  prints its license notice.\n\n"                              \
    "If one of these flags is used, the program will stop its execution right "\
    "after\nperform its respective action.\n\n"                                \
    "DATA TYPE FLAGS\n"                                                        \
    "These flags can be used to retrieve different data types from the entries"\
    " given\nas arguments.\n\n"                                                \
    "  --contents (default)  prints its contents.\n"                           \
    "  --type                prints its type: block, character, directory, "   \
    "fifo,\n                        symlink, regular or unknown.\n"            \
    "  --size                prints its size in bytes.\n"                      \
    "  --human-size          prints its size using a human readable unit: GB, "\
    "KB, MB\n                        or B.\n"                                  \
    "  --blocks              prints the quantity of blocks it occupies.\n"     \
    "  --hard-links          prints the quantity of hard links it has.\n"      \
    "  --user                prints the user that owns it.\n"                  \
    "  --user-id             prints the ID of the user that owns it.\n"        \
    "  --group               prints the group that owns it.\n"                 \
    "  --group-id            prints the ID of the group that owns it.\n"       \
    "  --mode                prints a number that contains bits representing " \
    "its\n                        attributes.\n"                               \
    "  --permissions         prints its permissions in octal base.\n"          \
    "  --human-permissions   prints its permissions for user, group and "      \
    "others,\n                        respectively, using three set of "       \
    "characters each. Each \n                        character can mean a "    \
    "permission set: read (r),\n                        write (w) and execute "\
    "(x), or its lack (-).\n"                                                  \
    "  --inode               prints its serial number.\n"                      \
    "  --modified-date       prints the date where its contents were last "    \
    "modified.\n"                                                              \
    "  --changed-date        prints the date where its metadata were last "    \
    "changed.\n"                                                               \
    "  --accessed-date       prints the date where its contents were last "    \
    "accessed.\n\n"                                                            \
    "If one of these flags is used, all the entries following it will be "     \
    "affected\nuntil it reaches another flag of this type. Else, the one "     \
    "marked as default will\nbe considered in use."
#define readCharacter 'r'
#define writeCharacter 'w'
#define executeCharacter 'x'
#define lackCharacter '-'
#define isExpectingEntryBit (1 << 5)
#define isTranspassingBit (1 << 6)
#define exitCodeBit (1 << 7)
#define nonDataTypeBits (isTranspassingBit | exitCodeBit | isExpectingEntryBit)
#define ParseMetadataFlag(flag, text)                                          \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        puts(text);                                                            \
        return (0);                                                            \
    }
#define ParseDataTypeFlag(flag, dataType)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        if (globalOptions & isExpectingEntryBit)                                  \
            Reveal(entry);                                                     \
        globalOptions = dataType | (globalOptions & nonDataTypeBits) |         \
                        isExpectingEntryBit;                                      \
        if (i == quantityOfArguments - 1)                                      \
            Reveal(entry);                                                     \
        continue;                                                              \
    }
#define ParseFunctionCase(value, function)                                     \
    case value:                                                                \
        function;                                                              \
        break;
#define ParsePutsCase(value, text) ParseFunctionCase(value, puts(text))
#define ParseSize(multiplier, unit)                                            \
    size = metadata->st_size / (multiplier);                                   \
    if ((int) size)                                                            \
    {                                                                          \
        printf("%.1f%s\n", size, unit);                                        \
        return;                                                                \
    }
#define ParsePermission(permission, character)                                 \
    putchar(metadata->st_mode & (permission) ? character : lackCharacter);
#define PrintPermissions(mode)                                                 \
    printf("0%o\n", (mode) & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |\
           S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
#define PrintUnsignedValue(value) printf("%u\n", value);
#define PrintLongValue(value) printf("%ld\n", value);
#define PrintUnsignedLongValue(value) printf("%lu\n", value);

static void Reveal(const char *const path);
static void RevealFile(const char *const path);
static void RevealDirectory(const char *const path);
static void RevealType(const struct stat *const metadata);
static void RevealUser(const struct stat *const metadata,
                       const char *const path);
static void RevealGroup(const struct stat *const metadata,
                        const char *const path);
static void RevealDate(const time_t *const date);
static void RevealHumanSize(const struct stat *const metadata);
static void RevealHumanPermissions(const struct stat *const metadata);
static void PrintSplittedError(const char *const descriptionSplit0,
                               const char *const descriptionSplit1,
                               const char *const descriptionSplit2,
                               const char *const descriptionSplit3,
                               const char *const descriptionSplit4);

static uint8_t globalOptions = 0;

int
main(const int quantityOfArguments, const char **arguments)
{
    if (quantityOfArguments == 1)
    {
        Reveal(".");
        return (0);
    }
    const char *entry = ".";
    for (int i = 1; i < quantityOfArguments; i++)
    {
        ParseMetadataFlag("version", programVersion)
        ParseMetadataFlag("license", programLicense)
        ParseMetadataFlag("help", programHelp)
    }
    for (int i = 1; i < quantityOfArguments; i++)
    {
        ParseDataTypeFlag("contents", 0)
        ParseDataTypeFlag("type", 1)
        ParseDataTypeFlag("size", 2)
        ParseDataTypeFlag("human-size", 3)
        ParseDataTypeFlag("blocks", 4)
        ParseDataTypeFlag("hard-links", 5)
        ParseDataTypeFlag("user", 6)
        ParseDataTypeFlag("user-id", 7)
        ParseDataTypeFlag("group", 8)
        ParseDataTypeFlag("group-id", 9)
        ParseDataTypeFlag("mode", 10)
        ParseDataTypeFlag("permissions", 11)
        ParseDataTypeFlag("human-permissions", 12)
        ParseDataTypeFlag("inode", 13)
        ParseDataTypeFlag("modified-date", 14)
        ParseDataTypeFlag("changed-date", 15)
        ParseDataTypeFlag("accessed-date", 16)
        if (!strcmp("--transpass", arguments[i]))
            globalOptions |= isTranspassingBit;
        else if (!strcmp("--untranspass", arguments[i]))
            globalOptions &= ~isTranspassingBit;
        else if (strlen(arguments[i]) > 2 && arguments[i][0] == '-' &&
                 arguments[i][1] == '-')
            PrintSplittedError("the flag \"", arguments[i], "\" is "
                               "unrecognized.\n        Did you mean the entry "
                               "\"./", arguments[i], "\"?");
        else
        {
            globalOptions &= ~isExpectingEntryBit;
            entry = arguments[i];
            Reveal(entry);
        }
    }
    return (!!(globalOptions & exitCodeBit));
}

static void
Reveal(const char *const path)
{
    struct stat metadata;
    if (globalOptions & isTranspassingBit ? stat(path, &metadata) :
        lstat(path, &metadata))
    {
        PrintSplittedError("the entry \"", path, "\" does not points to "
                           "anything.\n        Did you not mispelled it?", "", "");
        return;
    }
    switch (globalOptions & ~nonDataTypeBits)
    {
    ParseFunctionCase(1, RevealType(&metadata))
    ParseFunctionCase(2, PrintLongValue(metadata.st_size))
    ParseFunctionCase(3, RevealHumanSize(&metadata))
    ParseFunctionCase(4, PrintLongValue(metadata.st_blocks))
    ParseFunctionCase(5, PrintUnsignedLongValue(metadata.st_nlink))
    ParseFunctionCase(6, RevealUser(&metadata, path))
    ParseFunctionCase(7, PrintUnsignedValue(metadata.st_uid))
    ParseFunctionCase(8, RevealGroup(&metadata, path))
    ParseFunctionCase(9, PrintUnsignedValue(metadata.st_gid))
    ParseFunctionCase(10, PrintUnsignedValue(metadata.st_mode))
    ParseFunctionCase(11, PrintPermissions(metadata.st_mode));
    ParseFunctionCase(12, RevealHumanPermissions(&metadata))
    ParseFunctionCase(13, PrintUnsignedLongValue(metadata.st_ino))
    ParseFunctionCase(14, RevealDate(&metadata.st_mtime))
    ParseFunctionCase(15, RevealDate(&metadata.st_ctime))
    ParseFunctionCase(16, RevealDate(&metadata.st_atime))
    default:
        switch (metadata.st_mode & S_IFMT)
        {
            ParseFunctionCase(S_IFREG, RevealFile(path))
            ParseFunctionCase(S_IFDIR, RevealDirectory(path))
            ParseFunctionCase(S_IFLNK, PrintSplittedError(
                "can not reveal the contents of symlink \"", path, "\".\n      "
                "  Did you mean to use the \"--transpass\" flag before it?", "",
                ""))
        default:
            PrintSplittedError("the entry \"", path, "\" contains a type that "
                               "can not be read.", "", "");
        }
    }
    return;
}

static void
RevealFile(const char *const path)
{
    FILE *const file = fopen(path, "r");
    if (!file)
    {
        PrintSplittedError("can not open file \"", path, "\".\n        Do you "
                           "have enough permissions?", "", "");
        return;
    }
    char character;
    while ((character = fgetc(file)) != EOF)
        putchar(character);
    fclose(file);
    return;
}

static void
RevealDirectory(const char *const path)
{
    char absolutePath[PATH_MAX];
    if (!realpath(path, absolutePath))
    {
        PrintSplittedError("could not resolve absolute path of entry\"", path,
                           "\".", "", "");
        return;
    }
    DIR *const directory = opendir(path);
    if (!directory)
    {
        PrintSplittedError("can not open directory \"", path, "\".\n        Do "
                           "you have enough permissions?", "", "");
        return;
    }
    const struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        printf("%s%s%s\n", absolutePath, !strcmp(absolutePath, "/") ? "" : "/",
               entry->d_name);
    }
    closedir(directory);
    return;
}

static void
RevealType(const struct stat *const metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
    ParsePutsCase(S_IFBLK, "block")
    ParsePutsCase(S_IFCHR, "character")
    ParsePutsCase(S_IFDIR, "directory")
    ParsePutsCase(S_IFIFO, "fifo")
    ParsePutsCase(S_IFLNK, "symlink")
    ParsePutsCase(S_IFREG, "regular")
    ParsePutsCase(S_IFSOCK, "socket");
    default:
        puts("unknown");
    }
    return;
}

static void
RevealHumanSize(const struct stat *const metadata)
{
    float size;
    ParseSize(1e9, "GB");
    ParseSize(1e6, "MB");
    ParseSize(1e3, "KB");
    printf("%ldB\n", metadata->st_size);
    return;
}

static void
RevealUser(const struct stat *const metadata, const char *const path)
{
    const struct passwd *const user = getpwuid(metadata->st_uid);
    if (user)
        puts(user->pw_name);
    else
        PrintSplittedError("could not get user that owns the entry \"", path,
                           "\".", "", "");
    return;
}

static void
RevealGroup(const struct stat *const metadata, const char *const path)
{
    const struct group *const group = getgrgid(metadata->st_gid);
    if (group)
        puts(group->gr_name);
    else
        PrintSplittedError("could not get group that owns the entry \"", path,
                           "\".", "", "");
    return;
}

static void
RevealDate(const time_t *const date)
{
    char formattedDate[29];
    if (strftime(formattedDate, sizeof(formattedDate), "%a %b %d %T %Z %Y",
        localtime(date)))
        puts(formattedDate);
    else
        PrintSplittedError("overflowed buffer to store date.", "", "", "", "");
    return;
}

static void
RevealHumanPermissions(const struct stat *const metadata)
{
    ParsePermission(S_IRUSR, readCharacter)
    ParsePermission(S_IWUSR, writeCharacter)
    ParsePermission(S_IXUSR, executeCharacter)
    ParsePermission(S_IRGRP, readCharacter)
    ParsePermission(S_IWGRP, writeCharacter)
    ParsePermission(S_IXGRP, executeCharacter)
    ParsePermission(S_IROTH, readCharacter)
    ParsePermission(S_IWOTH, writeCharacter)
    ParsePermission(S_IXOTH, executeCharacter)
    putchar('\n');
    return;
}

static void
PrintSplittedError(const char *const descriptionSplit0,
                   const char *const descriptionSplit1,
                   const char *const descriptionSplit2,
                   const char *const descriptionSplit3,
                   const char *const descriptionSplit4)
{
    fprintf(stderr, "%s: %s%s%s%s%s\n", programName, descriptionSplit0,
            descriptionSplit1, descriptionSplit2, descriptionSplit3,
            descriptionSplit4);
    globalOptions |= exitCodeBit;
    return;
}

