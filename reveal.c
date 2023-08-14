#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.5"
#define program_help__                                                         \
    "Usage: " program_name__ " [OPTION]... [PATH]...\n"                        \
    "Reveals information about entries in the file system.\n\n"                \
    "METADATA OPTIONS\n"                                                       \
    "These options retrive information about the program.\n\n"                 \
    "    --help       print these help instructions.\n"                        \
    "    --version    print its version.\n\n"                                  \
    "DATA TYPE OPTIONS\n"                                                      \
    "These options change the data type retrived from the entries following "  \
    "them.\n\n"                                                                \
    "    --contents (default)    print its contents.\n"                        \
    "    --type                  print its type: regular, directory, symlink, "\
    "socket,\n                            fifo, character, block or unknown.\n"\
    "    --size                  print its size using a convenient unit: "     \
    "gigabyte (GB),\n                            megabyte (MB), kilobyte (kB) "\
    "or byte (B).\n"                                                           \
    "    --byte-size             print its size in bytes.\n"                   \
    "    --permissions           print its read (r), write (w) and execute (x)"\
    "\n                            permissions for user, group and others.\n"  \
    "    --octal-permissions     print its permissions using octal base.\n"    \
    "    --user                  print the user that owns it.\n"               \
    "    --user-uid              print the UID of the user that owns it.\n"    \
    "    --group                 print the group that owns it.\n"              \
    "    --group-gid             print the GID of the group that owns it.\n"   \
    "    --modified-date         print the date when its contents were last "  \
    "modified.\n\n"                                                            \
    "If none is used, the one marked as default will be considered.\n\n"       \
    "Each one of them expects at least one path following it. If none is "     \
    "given, it will\nconsider the last valid one given or, else, the current " \
    "directory.\n\n"                                                           \
    "SYMLINK FLAGS\n"                                                          \
    "These options change how the symlinks following them are handled, "       \
    "changing the\norigin of the data revealed.\n\n"                           \
    "    --follow-symlinks (default)    symlinks will be followed.\n"          \
    "    --unfollow-symlinks            symlinks will not be followed.\n\n"    \
    "If none is used, the one marked as default will be considered.\n\n"       \
    "EXIT CODES\n"                                                             \
    "It will throw code 1 if an error happens and 0 otherwise.\n\n"            \
    "SUPPORT\n"                                                                \
    "Report issues, questions and suggestions through its issues page:\n"      \
    "<https://github.com/skippyr/reveal/issues>."
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (is_expecting_path_bit__ |                        \
                              is_following_symlinks_bit__ | had_error_bit__)
#define Parse_Option__(option, action)                                         \
    if (!strcmp("--" option, arguments[argument_index]))                       \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(option, text) Parse_Option__(                  \
    option, puts(text); return(0));

int main(const int total_of_arguments, const char** arguments)
{
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("version", program_version__);
        Parse_Metadata_Option__("help", program_help__);
    }
    return (0);
}
