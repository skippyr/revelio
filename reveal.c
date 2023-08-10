#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.5"
#define program_help__                                                         \
    "Usage: " program_name__ " [OPTION]... [PATH]...\n"                        \
    "Reveals information about entries in the file system.\n\n"                \
    "METADATA OPTIONS\n"                                                       \
    "These options can be used to get information about the program.\n\n"      \
    "    --version    print its version.\n"                                    \
    "    --help       print these help instructions.\n\n"                      \
    "DATA TYPE OPTIONS\n"                                                      \
    "These options can be used to change what type of data to get from the "   \
    "entries\nfollowing them.\n\n"                                             \
    "If none is used, the one marked as default will be considered.\n\n"       \
    "    --contents (default)    print its contents.\n"                        \
    "    --type                  print its type: regular, directory, symlink, "\
    "fifo,\n                            socket, character, block or unknown.\n"\
    "    --size                  print its size using a readable unit: "       \
    "gigabyte (GB),\n                            megabyte (MB), kilobyte (kB) "\
    "or byte (B).\n"                                                           \
    "    --byte-size             print its size in bytes.\n"                   \
    "    --permissions           print its read (r), write (w) and execute (x)"\
    "\n                            permissions for user, group and others.\n"  \
    "    --octal-permissions     print its permissions in octal base.\n"       \
    "    --user                  print the user that owns its.\n"              \
    "    --user-uid              print the UID of the user that owns its.\n"   \
    "    --group                 print the group that owns its.\n"             \
    "    --group-gid             print the GID of the group that owns its.\n"  \
    "    --modified-date         print the date when its contents were last "  \
    "modified.\n\n"                                                            \
    "These options expects a path following them. If none is used, they will " \
    "consider\nthe last valid one given or, else, the current directory.\n\n"  \
    "SYMLINK OPTIONS\n"                                                        \
    "These options change how symlinks following them will be treated, "       \
    "changing the\norigin of what is revealed.\n\n"                            \
    "If none is used, the one marked as default will be considered.\n\n"       \
    "    --follow-symlinks (default)    follow symlinks.\n"                    \
    "    --unfollow-symlinks            unfollow symlinks.\n\n"                \
    "EXIT CODES\n"                                                             \
    "The exit code 1 will be throw if an error happens."
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__                                                   \
    (is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__)
#define is_last_argument__ (argument_index == total_of_arguments - 1)
#define Use_String_On_Suggestion__(text) (suggestion ? text : "")
#define Print_Long__(value) printf("%ld\n", value);
#define Print_Unsigned__(value) printf("%u\n", value);
#define Print_Octal_Permissions__                                              \
    printf("0%o\n", metadata.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |\
                                        S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |\
                                        S_IXOTH))
#define Parse_Null_String__(text) (text ? text : "")
#define Parse_Return_Case__(value, action)                                     \
    case value:                                                                \
        return (action);
#define Parse_Case__(value, action)                                            \
    case value:                                                                \
        action;                                                                \
        break;
#define Parse_Puts_Case__(value, text) Parse_Case__(value, puts(text))
#define Parse_Size__(multiplier, multiplier_character)                         \
    size = metadata->st_size / (multiplier);                                   \
    if ((int) size)                                                            \
    {                                                                          \
        printf("%.1f%cB\n", size, multiplier_character);                       \
        return;                                                                \
    }
#define Parse_Permission__(permission, permission_character)                   \
    putchar(metadata->st_mode & permission ? permission_character :            \
                                             lack_character);
#define Parse_Option__(option, action)                                         \
    if (!strcmp("--" option, arguments[argument_index]))                       \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(option, text)                                  \
    Parse_Option__(option, puts(text); return (0))
#define Parse_Data_Type_Option__(option, data_type) Parse_Option__(option,     \
    if (OPTIONS & is_expecting_path_bit__)                                     \
    {                                                                          \
        Reveal(last_path);                                                     \
    }                                                                          \
    OPTIONS = (data_type | is_expecting_path_bit__ | (OPTIONS &                \
                                                      non_data_type_bits__));  \
    if (is_last_argument__)                                                    \
    {                                                                          \
        Reveal(last_path);                                                     \
    }                                                                          \
    continue;                                                                  \
)
#define Parse_Non_Data_Type_Option__(option, action) Parse_Option__(option,    \
    if (is_last_argument__)                                                    \
    {                                                                          \
        Reveal(last_path);                                                     \
    }                                                                          \
    action;                                                                    \
    continue;                                                                  \
)

typedef enum
{
    Data_Type_Content,
    Data_Type_Type,
    Data_Type_Size,
    Data_Type_Byte_Size,
    Data_Type_Permissions,
    Data_Type_Octal_Permissions,
    Data_Type_User,
    Data_Type_User_Uid,
    Data_Type_Group,
    Data_Type_Group_Gid,
    Data_Type_Modified_Date
} Data_Type;

uint8_t OPTIONS = is_following_symlinks_bit__;

uint8_t Throw_Error(const char* const description_split_0,
                    const char* const description_split_1,
                    const char* const description_split_2,
                    const char* const suggestion)
{
    fprintf(stderr, "%s: %s%s%s\n%s%s%s", program_name__,
            Parse_Null_String__(description_split_0),
            Parse_Null_String__(description_split_1),
            Parse_Null_String__(description_split_2),
            Use_String_On_Suggestion__("        "),
            Parse_Null_String__(suggestion), Use_String_On_Suggestion__("\n"));
    OPTIONS |= had_error_bit__;
    return (1);
}

void Reveal_Type(const struct stat* const metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
        Parse_Puts_Case__(S_IFREG, "regular");
        Parse_Puts_Case__(S_IFDIR, "directory");
        Parse_Puts_Case__(S_IFLNK, "symlink");
        Parse_Puts_Case__(S_IFSOCK, "socket");
        Parse_Puts_Case__(S_IFIFO, "fifo");
        Parse_Puts_Case__(S_IFCHR, "character");
        Parse_Puts_Case__(S_IFBLK, "block");
    default:
        puts("unknown");
    }
}

void Reveal_Size(const struct stat* const metadata)
{
    float size;
    Parse_Size__(1e9, 'G');
    Parse_Size__(1e6, 'M');
    Parse_Size__(1e3, 'k');
    printf("%ldB\n", metadata->st_size);
}

void Reveal_Permissions(const struct stat* const metadata)
{
    const char read_character = 'r', write_character = 'w',
               execute_character = 'x', lack_character = '-';
    Parse_Permission__(S_IRUSR, read_character);
    Parse_Permission__(S_IWUSR, write_character);
    Parse_Permission__(S_IXUSR, execute_character);
    Parse_Permission__(S_IRGRP, read_character);
    Parse_Permission__(S_IWGRP, write_character);
    Parse_Permission__(S_IXGRP, execute_character);
    Parse_Permission__(S_IROTH, read_character);
    Parse_Permission__(S_IWOTH, write_character);
    Parse_Permission__(S_IXOTH, execute_character);
    putchar('\n');
}

uint8_t Reveal_User(const struct stat* const metadata, const char* const path)
{
    const struct passwd* const user = getpwuid(metadata->st_uid);
    if (!user)
    {
        return (Throw_Error("can not get user that owns \"", path, "\".",
                            NULL));
    }
    puts(user->pw_name);
    return (0);
}

uint8_t Reveal_Group(const struct stat* const metadata, const char* const path)
{
    const struct group* const group = getgrgid(metadata->st_gid);
    if (!group)
    {
        return (Throw_Error("can not get group that owns \"", path, "\".",
                            NULL));
    }
    puts(group->gr_name);
    return (0);
}

uint8_t Reveal_Modified_Date(const struct stat* const metadata)
{
    char modified_date[29];
    if (!strftime(modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
        localtime(&metadata->st_mtime)))
    {
        return (Throw_Error("overflowed buffer meant to store modified date.",
                            NULL, NULL, NULL));
    }
    puts(modified_date);
    return (0);
}

uint8_t Reveal_File(const char* const path)
{
    FILE* const file = fopen(path, "r");
    if (!file)
    {
        return (Throw_Error("can not open file \"", path, "\".", "Ensure that "
                            "you have permissions to read it."));
    }
    char character;
    while ((character = fgetc(file)) != EOF)
    {
        putchar(character);
    }
    fclose(file);
    return (0);
}

uint8_t Reveal_Directory(const char* const path)
{
    DIR* const directory = opendir(path);
    if (!directory)
    {
        return (Throw_Error("can not open directory \"", path, "\".", "Ensure "
                            "that you have permissions to read it."));
    }
    const struct dirent* entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        puts(entry->d_name);
    }
    closedir(directory);
    return (0);
}

uint8_t Reveal(const char* const path)
{
    struct stat metadata;
    if (OPTIONS & is_following_symlinks_bit__ ? stat(path, &metadata) :
                                                lstat(path, &metadata))
    {
        return (Throw_Error("\"", path, "\" does not exists.", "Ensure that you"
                            " did not misspelled it."));
    }
    switch (OPTIONS & ~non_data_type_bits__)
    {
        Parse_Case__(Data_Type_Type, Reveal_Type(&metadata));
        Parse_Case__(Data_Type_Size, Reveal_Size(&metadata));
        Parse_Case__(Data_Type_Byte_Size, Print_Long__(metadata.st_size));
        Parse_Case__(Data_Type_Permissions, Reveal_Permissions(&metadata));
        Parse_Case__(Data_Type_Octal_Permissions, Print_Octal_Permissions__);
        Parse_Return_Case__(Data_Type_User, Reveal_User(&metadata, path));
        Parse_Case__(Data_Type_User_Uid, Print_Unsigned__(metadata.st_uid));
        Parse_Return_Case__(Data_Type_Group, Reveal_Group(&metadata, path));
        Parse_Case__(Data_Type_Group_Gid, Print_Unsigned__(metadata.st_gid));
        Parse_Return_Case__(Data_Type_Modified_Date,
                            Reveal_Modified_Date(&metadata));
    default:
        switch (metadata.st_mode & S_IFMT)
        {
            Parse_Return_Case__(S_IFREG, Reveal_File(path));
            Parse_Return_Case__(S_IFDIR, Reveal_Directory(path));
            Parse_Return_Case__(__S_IFLNK, Throw_Error(
                "can not reveal contents of symlink \"", path, "\".", "Did you "
                "mean to use the option \"--follow-symlinks\" before it?"));
        default:
            return (Throw_Error("can not reveal \"", path, "\" due to its "
                                "unreadable type.", NULL));
        }
    }
    return (0);
}

int main(const int total_of_arguments, const char** arguments)
{
    if (total_of_arguments == 1)
    {
        Reveal(".");
    }
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("version", program_version__);
        Parse_Metadata_Option__("help", program_help__);
    }
    const char* last_path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Data_Type_Option__("contents", Data_Type_Content);
        Parse_Data_Type_Option__("type", Data_Type_Type);
        Parse_Data_Type_Option__("size", Data_Type_Size);
        Parse_Data_Type_Option__("byte-size", Data_Type_Byte_Size);
        Parse_Data_Type_Option__("permissions", Data_Type_Permissions);
        Parse_Data_Type_Option__("octal-permissions",
                                 Data_Type_Octal_Permissions);
        Parse_Data_Type_Option__("user", Data_Type_User);
        Parse_Data_Type_Option__("user-uid", Data_Type_User_Uid);
        Parse_Data_Type_Option__("group", Data_Type_Group);
        Parse_Data_Type_Option__("group-gid", Data_Type_Group_Gid);
        Parse_Data_Type_Option__("modified-date", Data_Type_Modified_Date);
        Parse_Non_Data_Type_Option__("follow-symlinks",
                                     OPTIONS |= is_following_symlinks_bit__);
        Parse_Non_Data_Type_Option__("unfollow-symlinks",
                                     OPTIONS &= ~is_following_symlinks_bit__);
        if (Reveal(arguments[argument_index]))
        {
            continue;
        }
        OPTIONS &= ~is_expecting_path_bit__;
        last_path = arguments[argument_index];
    }
    return !!(OPTIONS & had_error_bit__);
}
