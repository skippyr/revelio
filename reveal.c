#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define programName "reveal"
#define programVersion "v7.0.0"
#define programLicense "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programHelp\
    "Usage: " programName " [FLAGS, PATHS...]"
#define isTranspassingBit (1 << 6)
#define exitCodeBit (1 << 7)
#define nonDataTypeBits (isTranspassingBit | exitCodeBit)
#define ParseMetadataFlag(flag, text)                                          \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        puts(text);                                                            \
        return (0);                                                            \
    }
#define ParseDataTypeFlag(flag, dataType)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        globalOptions = dataType | (globalOptions & nonDataTypeBits);          \
        continue;                                                              \
    }
#define ParseFunctionCase(value, function)                                     \
    case value:                                                                \
        function;                                                              \
        break;
#define ParsePutsCase(value, text) ParseFunctionCase(value, puts(text))

static void Reveal(const char *const path);
static void RevealFile(const char *const path);
static void RevealDirectory(const char *const path);
static void RevealType(const struct stat *const metadata);
static void PrintSplittedError(const char *const descriptionStart,
                               const char *const descriptionMiddle,
                               const char *const descriptionEnd);

static uint8_t globalOptions = 0;

int
main(int argumentsCount, const char **arguments)
{
    for (int i = 1; i < argumentsCount; i++)
    {
        ParseMetadataFlag("version", programVersion)
        ParseMetadataFlag("license", programLicense)
        ParseMetadataFlag("help", programHelp)
    }
    for (int i = 1; i < argumentsCount; i++)
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
        else
            Reveal(arguments[i]);
    }
    return !!(globalOptions & exitCodeBit);
}

static void
Reveal(const char *const path)
{
    struct stat metadata;
    if (globalOptions & isTranspassingBit ? stat(path, &metadata) :
        lstat(path, &metadata))
    {
        PrintSplittedError("the path \"", path, "\" does not points to "
                           "anything. Did you not mispelled it?");
        return;
    }
    switch (globalOptions & ~nonDataTypeBits)
    {
    ParseFunctionCase(1, RevealType(&metadata))
    // ParseFunctionCase(2, PrintLongValue(metadata.st_size))
    // ParseFunctionCase(3, RevealHumanSize(&metadata))
    // ParseFunctionCase(4, PrintLongValue(metadata.st_blocks))
    // ParseFunctionCase(5, PrintUnsignedLongValue(metadata.st_nlink))
    // ParseFunctionCase(6, RevealUser(&metadata, path))
    // ParseFunctionCase(7, PrintUnsignedValue(metadata.st_uid))
    // ParseFunctionCase(8, RevealGroup(&metadata, path))
    // ParseFunctionCase(9, PrintUnsignedValue(metadata.st_gid))
    // ParseFunctionCase(10, PrintUnsignedValue(metadata.st_mode))
    // ParseFunctionCase(11, printf("0%o\n", metadata.st_mode &
    //                              (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
    //                               S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
    //                               S_IXOTH)))
    // ParseFunctionCase(12, RevealHumanPermissions(&metadata))
    // ParseFunctionCase(13, PrintUnsignedLongValue(metadata.st_ino))
    // ParseFunctionCase(14, RevealDate(&metadata.st_mtime))
    // ParseFunctionCase(15, RevealDate(&metadata.st_ctime))
    // ParseFunctionCase(16, RevealDate(&metadata.st_atime))
    default:
        switch (metadata.st_mode & S_IFMT)
        {
            ParseFunctionCase(S_IFREG, RevealFile(path))
            ParseFunctionCase(S_IFDIR, RevealDirectory(path))
        default:
            PrintSplittedError("can not reveal the contents of \"", path,
                               "\" type.");
        }
    }
    return;
}

static void
RevealFile(const char *const path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        PrintSplittedError("could not open file \"", path, "\". Do you have "
                           "enough permissions?");
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
        PrintSplittedError("could not resolve absolute path of \"", path,
                           "\".");
        return;
    }
    DIR *directory = opendir(path);
    if (!directory)
    {
        PrintSplittedError("could not open directory \"", path, "\". Do you "
                           "have enough permissions?");
        return;
    }
    struct dirent *entry;
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
PrintSplittedError(const char *const descriptionStart,
                   const char *const descriptionMiddle,
                   const char *const descriptionEnd)
{
    fprintf(stderr, "%s: %s%s%s\n", programName, descriptionStart,
            descriptionMiddle, descriptionEnd);
    globalOptions |= exitCodeBit;
    return;
}
