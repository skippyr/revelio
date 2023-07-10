#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define programName "reveal"
#define programLicense "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programVersion "v5.2.2"
#define programIssuesPage "https://github.com/skippyr/" programName "/issues"
#define programHelp "Usage: " programName " [FLAGS]... [PATHS]...\nReveals "   \
                    "information about entries in the file system.\n\nMETADATA"\
                    " FLAGS\nThese flags show metadata about the program.\n\n "\
                    " --help     prints these help instructions.\n  --version "\
                    " prints its version.\n  --license  prints its license.\n" \
                    "\nDATA TYPE FLAGS\nThese flags change what data type the "\
                    "program will reveal from the entries.\n\n  --contents "   \
                    "(default)  prints its contents.\n  --type                "\
                    "prints its type.\n  --size                prints its size"\
                    ".\n  --human-size          prints its size using a "      \
                    "formidable unit.\n  --blocks              prints the "    \
                    "quantity of blocks it takes.\n  --hard-links          "   \
                    "prints the quantity of hard links it has.\n  --user      "\
                    "          prints the user that owns it.\n  --user-id     "\
                    "        prints the ID of the user that owns it.\n  "      \
                    "--group               prints the group that owns it.\n  " \
                    "--group-id            prints the ID of the group that "   \
                    "owns it.\n  --mode                prints a bit set "      \
                    "containing its properties and\n                        "  \
                    "permissions.\n  --permissions         prints its "        \
                    "permissions in octal base.\n  --human-permissions   "     \
                    "prints its permissions for user, group, and others,\n    "\
                    "                    respectively, using three set of "    \
                    "characters each:\n                        read (r), write"\
                    " (w), execute (x) and lack (-).\n  --inode               "\
                    "prints its serial number.\n  --modified-date       prints"\
                    " the date where its contents were last modified.\n  "     \
                    "--changed-date        prints the date where its metadata "\
                    "in the file system's was\n                        last "  \
                    "changed.\n  --accessed-date       prints the date where " \
                    "its contents were last accessed.\n\nTRANSPASSING FLAGS\n" \
                    "These flags changes the way symlinks are handled.\n\n  "  \
                    "--untranspass (default)  does not resolve symlinks.\n  "  \
                    "--transpass              resolves all levels of symlinks."\
                    "\n\nEXIT CODES\nIt will throw exit code 1 in the end of " \
                    "its execution if an error happens.\n\nISSUES\nReport "    \
                    "issues found in this program at:\n" programIssuesPage "."
#define ParseMetadataFlag(flag, function)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        function;                                                              \
        exit(0);                                                               \
    }
#define ParseDataTypeFlag(flag, dataType)                                      \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        globalOptions = dataType | globalOptions & (1 << 6 | 1 << 7);          \
        continue;                                                              \
    }
#define ParseFunctionCase(value, function)                                     \
    case value:                                                                \
        function;                                                              \
        break;                                                                 \

uint8_t globalOptions = 0;

void PrintSplittedError(char *start, char *middle, char *end)
{
    fprintf(stderr, "%s: %s%s%s\n", programName, start, middle, end);
    globalOptions |= 1 << 7;
}

void Reveal(char *path)
{
    struct stat metadata;
    if (globalOptions & 1 << 6 ? stat(path, &metadata) : lstat(path, &metadata))
    {
        PrintSplittedError("the path \"", path, "\" does not points to "
                           "anything. Did you not mispelled it?\n");
        return;
    }
    switch (globalOptions & ~(1 << 6 | 1 << 7))
    {

    }
}

int main(int quantityOfArguments, char **arguments)
{
    for (int i = 0; i < quantityOfArguments; i++)
    {
        ParseMetadataFlag("license", puts(programLicense))
        ParseMetadataFlag("version", puts(programVersion))
        ParseMetadataFlag("help", puts(programHelp))
    }
    for (int i = 0; i < quantityOfArguments; i++)
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
            globalOptions |= 1 << 6;
        else if (!strcmp("--untrapass", arguments[i]))
            globalOptions &= ~(1 << 6);
        else
            Reveal(arguments[i]);
    }
    return !!(globalOptions & 1 << 7);
}
