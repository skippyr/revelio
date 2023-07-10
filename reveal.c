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
#define programLicense "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programVersion "v6.0.0"
#define programIssuesPage "https://github.com/skippyr/" programName "/issues"
#define programHelp "Usage: " programName " [FLAGS]... [PATHS]...\nReveals "   \
                    "information about entries in the file system.\n\nMETADATA"\
                    " FLAGS\nThese flags show metadata about the program.\n\n "\
                    " --help     prints these help instructions.\n  --version "\
                    " prints its version.\n  --license  prints its license.\n" \
                    "\nDATA TYPE FLAGS\nThese flags change what data type the "\
                    "program will reveal from the entries.\n\n  --contents "   \
                    "(default)  prints its contents.\n  --type                "\
                    "prints its type: Block, Character, Directory, Fifo,\n"    \
                    "                        Symlink, Regular or Unknown.\n  " \
                    "--size                prints its size in bytes.\n  "      \
                    "--human-size          prints its size using a formidable "\
                    "unit.\n  --blocks              prints the quantity of "   \
                    "blocks it takes.\n  --hard-links          prints the "    \
                    "quantity of hard links it has.\n  --user                " \
                    "prints the user that owns it.\n  --user-id             "  \
                    "prints the ID of the user that owns it.\n  --group       "\
                    "        prints the group that owns it.\n  --group-id     "\
                    "       prints the ID of the group that owns it.\n  --mode"\
                    "                prints a bit set containing its "         \
                    "properties and\n                        permissions.\n  " \
                    "--permissions         prints its permissions in octal "   \
                    "base.\n  --human-permissions   prints its permissions for"\
                    " user, group, and others,\n                        "      \
                    "respectively, using three set of characters each:\n      "\
                    "                  read (r), write (w), execute (x) and "  \
                    "lack (-).\n  --inode               prints its serial "    \
                    "number.\n  --modified-date       prints the date where "  \
                    "its contents were last modified.\n  --changed-date       "\
                    " prints the date where its metadata in the file system's" \
                    "\n                        inode table was last changed.\n"\
                    "  --accessed-date       prints the date where its "       \
                    "contents were last accessed.\n\nTRANSPASSING FLAGS\nThese"\
                    " flags changes the way symlinks are handled.\n\n  "       \
                    "--untranspass (default)  does not resolve symlinks.\n  "  \
                    "--transpass              resolves all levels of symlinks."\
                    "\n\nEXIT CODES\nIt will throw exit code 1 in the end of " \
                    "its execution if an error happens.\n\nISSUES\nReport "    \
                    "issues found in this program at:\n" programIssuesPage "."
#define isTranspassingBit (1 << 6)
#define exitCodeBit (1 << 7)
#define nonDataTypeBits (isTranspassingBit | exitCodeBit)
#define readCharacter 'r'
#define writeCharacter 'w'
#define executeCharacter 'x'
#define lackCharacter '-'
#define ParseMetadataFlag(flag, function)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        function;                                                              \
        exit(0);                                                               \
    }
#define ParseDataTypeFlag(flag, dataType)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        globalOptions = dataType | globalOptions & nonDataTypeBits;            \
        continue;                                                              \
    }
#define ParseFunctionCase(value, function)                                     \
    case value:                                                                \
        function;                                                              \
        break;
#define ParsePutsCase(value, text) ParseFunctionCase(value, puts(text))
#define ParseSize(multiplier, unit)                                            \
    size = metadata->st_size / multiplier;                                     \
    if ((int) size)                                                            \
    {                                                                          \
        printf("%.1f%s\n", size, unit);                                        \
        return;                                                                \
    }
#define ParsePermission(permission, character)                                 \
    putchar(metadata->st_mode & permission ? character : lackCharacter);
#define PrintUnsignedValue(value) printf("%u\n", value);
#define PrintLongValue(value) printf("%ld\n", value);
#define PrintUnsignedLongValue(value) printf("%lu\n", value);

uint8_t globalOptions = 0;

void PrintSplittedError(char *start, char *middle, char *end)
{
    fprintf(stderr, "%s: %s%s%s\n", programName, start, middle, end);
    globalOptions |= exitCodeBit;
}

void RevealType(struct stat *metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
    ParsePutsCase(S_IFBLK, "Block")
    ParsePutsCase(S_IFCHR, "Character")
    ParsePutsCase(S_IFDIR, "Directory")
    ParsePutsCase(S_IFIFO, "Fifo")
    ParsePutsCase(S_IFLNK, "Symlink")
    ParsePutsCase(S_IFREG, "Regular")
    ParsePutsCase(S_IFSOCK, "Socket");
    default:
        puts("Unknown");
    }
}

void RevealHumanSize(struct stat *metadata)
{
    float size;
    ParseSize(1e9, "GB");
    ParseSize(1e6, "MB");
    ParseSize(1e3, "KB");
    printf("%ldB\n", metadata->st_size);
}

void RevealUser(struct stat *metadata, char *path)
{
    const struct passwd *const owner = getpwuid(metadata->st_uid);
    if (owner)
        puts(owner->pw_name);
    else
        PrintSplittedError("could not get user that owns \"", path, "\".");
}

void RevealGroup(struct stat *metadata, char *path)
{
    const struct group *const group = getgrgid(metadata->st_gid);
    if (group)
        puts(group->gr_name);
    else
        PrintSplittedError("could not get group that owns \"", path, "\".");
}

void RevealHumanPermissions(struct stat *metadata)
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
}

void RevealDate(time_t *date)
{
    char buffer[29];
    if (!strftime(buffer, sizeof(buffer), "%a %b %d %T %Z %Y", localtime(date)))
    {
        PrintSplittedError("", "", "overflowed buffer to store date.");
        return;
    }
    puts(buffer);
}

void RevealFile(char *path)
{
    FILE *const file = fopen(path, "r");
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
}

void RevealDirectory(char *path)
{
    char absolutePath[PATH_MAX];
    if (!realpath(path, absolutePath))
    {
        PrintSplittedError("could not resolve absolute path of \"", path,
                           "\".");
        return;
    }
    DIR *const directory = opendir(path);
    if (!directory)
    {
        PrintSplittedError("could not open directory \"", path, "\". Do you "
                           "have enough permissions?");
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
}

void Reveal(char *path)
{
    struct stat metadata;
    if (globalOptions & isTranspassingBit ? stat(path, &metadata) : lstat(path, &metadata))
    {
        PrintSplittedError("the path \"", path, "\" does not points to "
                           "anything. Did you not mispelled it?");
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
    ParseFunctionCase(11, printf("0%o\n", metadata.st_mode &
                                 (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                                  S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
                                  S_IXOTH)))
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
        default:
            PrintSplittedError("can not reveal the contents of \"", path,
                               "\" type.");
        }
    }
}

int main(int quantityOfArguments, char **arguments)
{
    for (int i = 1; i < quantityOfArguments; i++)
    {
        ParseMetadataFlag("license", puts(programLicense))
        ParseMetadataFlag("version", puts(programVersion))
        ParseMetadataFlag("help", puts(programHelp))
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
        else if (!strcmp("--untrapass", arguments[i]))
            globalOptions &= ~isTranspassingBit;
        else
            Reveal(arguments[i]);
    }
    return !!(globalOptions & exitCodeBit);
}
