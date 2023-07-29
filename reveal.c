#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define program_version__ "v9.0.0"
#define program_copyright__ "Copyright (c) Sherman Rofeman. MIT license."
#define program_support__ "https://github.com/skippyr/reveal/issues"
#define program_help__                                                         \
    "Usage: reveal [FLAGS | PATHS]\n"                                          \
    "Reveals information about entries in the file system.\n\n"                \
    "METADATA FLAGS\n"                                                         \
    "The following flags allow you to get information about the program "      \
    "itself.\n\n"                                                              \
    "    --copyright    prints its copyright notice.\n"                        \
    "    --version      prints its version.\n"                                 \
    "    --help         prints these help instructions.\n\n"                   \
    "DATA TYPE FLAGS\n"                                                        \
    "The following flags allow you to change what type of data to get from "   \
    "the\nentries following them.\n\n"                                         \
    "If none is used, the one marked as default will be considered in "        \
    "use.\n\n"                                                                 \
    "    --contents (default)    prints its contents.\n"                       \
    "    --size                  prints its size using the most formidable "   \
    "unit:\n                            gigabyte (GB), megabyte (MB), "        \
    "kilobyte (KB) "                                                           \
    "or\n                            byte (B).\n"                              \
    "    --byte-size             prints its size in bytes.\n"                  \
    "    --permissions           prints its read (r), write (w) and execute "  \
    "(x)\n                            permissions for user, group and "        \
    "others. If a\n                            permission is not set "         \
    "the lack (-) character\n                            will be used.\n"      \
    "    --octal-permissions     prints its permissions in octal base.\n"      \
    "    --user                  prints the user that owns it.\n"              \
    "    --user-uid              prints the UID of the user that owns it.\n"   \
    "    --group                 prints the group that owns it.\n"             \
    "    --group-gid             prints the GID of the group that owns it.\n"  \
    "    --modified-date         prints the date when its contents where "     \
    "last\n                            "                                       \
    "modified.\n\n"                                                            \
    "SYMLINKS FLAGS\n"                                                         \
    "These flags allow you to change how symlinks following them will be\n"    \
    "handled, affecting the origin of the data you reveal.\n\n"                \
    "If none is used, the one marked as default will be considered in "        \
    "use.\n\n"                                                                 \
    "    --follow-symlinks  (default)    symlinks will be followed.\n"         \
    "    --unfollow-symlinks             symlinks will not be followed\n\n"    \
    "EXIT CODES\n"                                                             \
    "The exit code 1 will be throw if an error happens during its "            \
    "execution.\n\n"                                                           \
    "Errors will be reported through standard error stream.\n\n"               \
    "SUPPORT\n"                                                                \
    "Report issues, questions or suggestion at:\n" program_support__ "."
#define Parse_Flag__(flag, action)                                             \
    if (!strcmp("--" flag, arguments[i]))                                      \
    {                                                                          \
        action                                                                 \
    }
#define Parse_Metadata_Flag__(flag, text)                                      \
    Parse_Flag__(flag, puts(text); return (0);)

int main(const int total_of_arguments, const char **arguments)
{
    for (int i = 1; i < total_of_arguments; i++)
    {
        Parse_Metadata_Flag__("copyright", program_copyright__);
        Parse_Metadata_Flag__("version", program_version__);
        Parse_Metadata_Flag__("help", program_help__);
    }
}
