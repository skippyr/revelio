#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define program_name__    "reveal"
#define program_version__ "v9.0.7"
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
    "    --size                  print its size using a convenient unit:\n    "\
    "                        gigabyte (GB), megabyte (MB), kilobyte (kB) or\n "\
    "                           byte (B).\n"                                   \
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
    "given, it\nwill consider the last valid one given or, else, the current " \
    "directory.\n\n"                                                           \
    "SYMLINK FLAGS\n"                                                          \
    "These options change how the symlinks following them are handled, "       \
    "changing the\norigin of the data revealed.\n\n"                           \
    "    --follow-symlinks (default)    symlinks will be followed.\n"          \
    "    --unfollow-symlinks            symlinks will not be followed.\n\n"    \
    "If none is used, the one marked as default will be considered.\n\n"       \
    "EXIT CODES\n"                                                             \
    "It will throw exit code 1 if an error happens and 0 otherwise.\n\n"       \
    "SUPPORT\n"                                                                \
    "Report issues, questions and suggestions through its issues page:\n"      \
    "<https://github.com/skippyr/reveal/issues>."
#define is_expecting_entry_path_bit__ (1 << 5)
#define is_following_symlinks_bit__   (1 << 6)
#define had_error_bit__               (1 << 7)
#define non_data_type_bits__          (is_expecting_entry_path_bit__ |         \
                                       is_following_symlinks_bit__   |         \
                                       had_error_bit__)
#define Skip_Dot_Directory_Entries__                                           \
    if (!strcmp(directory_entry->d_name, ".") ||                               \
        !strcmp(directory_entry->d_name, ".."))                                \
    {                                                                          \
        continue;                                                              \
    }
#define Parse_Case__(value, action)                                            \
    case value:                                                                \
        action;                                                                \
        break;
#define Parse_Return_Case__(value, action)                                     \
    case value:                                                                \
        return (action);
#define Parse_Puts_Case__(value, text) Parse_Case__(value, puts(text))
#define Parse_Null_String__(text) (text ? text : "")
#define Parse_Size_Multiplier__(multiplier, multiplier_character)              \
    size = metadata->st_size / (multiplier);                                   \
    if ((int)size)                                                             \
    {                                                                          \
        printf("%.1f%cB\n", size, multiplier_character);                       \
        return;                                                                \
    }
#define Parse_Option__(option, action)                                         \
    if (!strcmp("--" option, arguments[argument_index]))                       \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(option, text)                                  \
    Parse_Option__(                                                            \
        option,                                                                \
        puts(text);                                                            \
        return (0);                                                            \
    )
#define Parse_Data_Type_Option__(option, data_type)                            \
    Parse_Option__(                                                            \
        option,                                                                \
        if (OPTIONS & is_expecting_entry_path_bit__)                           \
        {                                                                      \
            Reveal_Entry(last_entry_path);                                     \
        }                                                                      \
        OPTIONS = data_type | is_expecting_entry_path_bit__ |                  \
                  (OPTIONS & non_data_type_bits__);                            \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path);                                     \
        }                                                                      \
        continue;                                                              \
    )
#define Parse_Non_Data_Type_Option__(option, action)                           \
    Parse_Option__(                                                            \
        option,                                                                \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path);                                     \
        }                                                                      \
        action;                                                                \
        continue;                                                              \
    )

typedef const char* const String;
typedef String* const Array_Of_String;
typedef const struct stat* const Metadata;
typedef enum
{
    Return_Status__Success,
    Return_Status__Failure
}
Return_Status;
typedef enum
{
    Data_Type__Contents,
    Data_Type__Type,
    Data_Type__Size,
    Data_Type__Byte_Size,
    Data_Type__Permissions,
    Data_Type__Octal_Permissions,
    Data_Type__User,
    Data_Type__User_Uid,
    Data_Type__Group,
    Data_Type__Group_Gid,
    Data_Type__Modified_Date
}
Data_Type;

static Return_Status Write_Error(String description_split_0,
                                 String description_split_1,
                                 String description_split_2,
                                 String fix_suggestion);
static void Throw_Error(String description_split_0, String description_split_1,
                        String description_split_2);
static void Reveal_Type(Metadata metadata);
static void Reveal_Size(Metadata metadata);
static void Parse_Permission_Bit(Metadata metadata, uint16_t permission_bit,
                                 const char permission_bit_character);
static void Reveal_Permissions(Metadata metadata);
static void Reveal_Octal_Permissions(Metadata metadata);
static Return_Status Reveal_User(Metadata metadata, String entry_path);
static Return_Status Reveal_Group(Metadata metadata, String entry_path);
static Return_Status Reveal_Modified_Date(Metadata metadata);
static Return_Status Reveal_File(String file_path);
static Return_Status Reveal_Directory(String directory_path);
static void Print_Unsigned(unsigned value);
static Return_Status Reveal_Entry(String entry_path);

static uint8_t OPTIONS = is_following_symlinks_bit__;

static Return_Status Write_Error(String description_split_0,
                                 String description_split_1,
                                 String description_split_2,
                                 String fix_suggestion)
{
    fprintf(stderr, "%s: %s%s%s\n%s%s", program_name__,
            Parse_Null_String__(description_split_0),
            Parse_Null_String__(description_split_1),
            Parse_Null_String__(description_split_2),
            Parse_Null_String__(fix_suggestion), fix_suggestion ? "\n" : "");
    OPTIONS |= had_error_bit__;
    return (Return_Status__Failure);
}

static void Throw_Error(String description_split_0, String description_split_1,
                        String description_split_2)
{
    Write_Error(description_split_0, description_split_1, description_split_2,
                NULL);
    exit(EXIT_FAILURE);
}

static void Reveal_Type(Metadata metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
        Parse_Puts_Case__(S_IFREG, "regular");
        Parse_Puts_Case__(S_IFDIR, "directory");
        Parse_Puts_Case__(S_IFLNK, "symlink");
        Parse_Puts_Case__(S_IFSOCK, "socket");
        Parse_Puts_Case__(S_IFBLK, "block");
        Parse_Puts_Case__(S_IFCHR, "character");
    default:
        puts("unknown");
    }
}

static void Reveal_Size(Metadata metadata)
{
    float size;
    const float one_gigabyte_in_bytes = 1e9,
                one_megabyte_in_bytes = 1e6,
                one_kilobyte_in_bytes = 1e3;
    Parse_Size_Multiplier__(one_gigabyte_in_bytes, 'G');
    Parse_Size_Multiplier__(one_megabyte_in_bytes, 'M');
    Parse_Size_Multiplier__(one_kilobyte_in_bytes, 'k');
    printf("%ldB\n", metadata->st_size);
}

static void Parse_Permission_Bit(Metadata metadata, uint16_t permission_bit,
                                 const char permission_bit_character)
{
    const char lack_permission_character = '-';
    putchar(metadata->st_mode & permission_bit ? permission_bit_character :
                                                 lack_permission_character);
}

static void Reveal_Permissions(Metadata metadata)
{
    const char read_permission_character    = 'r',
               write_permission_character   = 'w',
               execute_permission_character = 'x';
    Parse_Permission_Bit(metadata, S_IRUSR, read_permission_character);
    Parse_Permission_Bit(metadata, S_IWUSR, write_permission_character);
    Parse_Permission_Bit(metadata, S_IXUSR, execute_permission_character);
    Parse_Permission_Bit(metadata, S_IRGRP, read_permission_character);
    Parse_Permission_Bit(metadata, S_IWGRP, write_permission_character);
    Parse_Permission_Bit(metadata, S_IXGRP, execute_permission_character);
    Parse_Permission_Bit(metadata, S_IROTH, read_permission_character);
    Parse_Permission_Bit(metadata, S_IWOTH, write_permission_character);
    Parse_Permission_Bit(metadata, S_IXOTH, execute_permission_character);
    putchar('\n');
}

static void Reveal_Octal_Permissions(Metadata metadata)
{
    printf("0%o\n", metadata->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                                         S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
                                         S_IXOTH));
}

static Return_Status Reveal_User(Metadata metadata, String entry_path)
{
    const struct passwd* const user = getpwuid(metadata->st_uid);
    if (!user)
    {
        return (Write_Error("can not discover user that owns the entry \"",
                            entry_path, "\".", "Ensure that the entry is not "
                            "dangling."));
    }
    puts(user->pw_name);
    return (Return_Status__Success);
}

static Return_Status Reveal_Group(Metadata metadata, String entry_path)
{
    const struct group* const group = getgrgid(metadata->st_gid);
    if (!group)
    {
        return (Write_Error("can not discover group that owns the entry \"",
                            entry_path, "\".", "Ensure that the entry is not "
                            "dangling."));
    }
    puts(group->gr_name);
    return (Return_Status__Success);
}

static Return_Status Reveal_Modified_Date(Metadata metadata)
{
    char modified_date[29];
    if (!strftime(modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
                  localtime(&metadata->st_mtime)))
    {
        return (Write_Error("overflowed buffer intended to store modified "
                            "date.", NULL, NULL, NULL));
    }
    puts(modified_date);
    return (Return_Status__Success);
}

static Return_Status Reveal_File(String file_path)
{
    String read_mode = "r";
    FILE* const file_stream = fopen(file_path, read_mode);
    if (!file_stream)
    {
        return (Write_Error("can not open the file \"", file_path, "\".",
                            "Ensure that you have permissions to read."));
    }
    char character;
    while ((character = fgetc(file_stream)) != EOF)
    {
        putchar(character);
    }
    fclose(file_stream);
    return (Return_Status__Success);
}

static Return_Status Reveal_Directory(String directory_path)
{
    DIR* const directory_stream = opendir(directory_path);
    if (!directory_stream)
    {
        return (Write_Error("can not open the directory \"", directory_path,
                            "\".", "Ensure that you have permissions to read "
                            "it."));
    }
    const struct dirent* directory_entry;
    size_t total_of_directory_entries = 0;
    while ((directory_entry = readdir(directory_stream)))
    {
        Skip_Dot_Directory_Entries__;
        total_of_directory_entries++;
    }
    void* directory_entries[total_of_directory_entries];
    size_t directory_entry_index = 0;
    rewinddir(directory_stream);
    while ((directory_entry = readdir(directory_stream)))
    {
        Skip_Dot_Directory_Entries__;
        void* directory_entry_allocation =
            malloc(sizeof(directory_entry->d_name));
        if (!directory_entry_allocation)
        {
            Throw_Error("can not allocate memory to reveal the contents of the "
                        "directory", directory_path, ".");
        }
        memcpy(directory_entry_allocation, directory_entry->d_name,
               sizeof(directory_entry->d_name));
        directory_entries[directory_entry_index] = directory_entry_allocation;
        directory_entry_index++;
    }
    for (directory_entry_index = 0;
         directory_entry_index < total_of_directory_entries - 1;
         directory_entry_index++)
    {
        size_t swap_directory_entry_index = directory_entry_index;
        for (size_t check_directory_entry_index = directory_entry_index;
             check_directory_entry_index < total_of_directory_entries;
             check_directory_entry_index++)
        {
            if (strcmp(directory_entries[check_directory_entry_index],
                       directory_entries[swap_directory_entry_index]) < 0)
            {
                swap_directory_entry_index = check_directory_entry_index;
            }
        }
        if (swap_directory_entry_index == directory_entry_index)
        {
            continue;
        }
        void* temporary_swap_allocation =
            directory_entries[directory_entry_index];
        directory_entries[directory_entry_index] =
            directory_entries[swap_directory_entry_index];
        directory_entries[swap_directory_entry_index] =
            temporary_swap_allocation;
    }
    for (directory_entry_index = 0;
         directory_entry_index < total_of_directory_entries;
         directory_entry_index++)
    {
        puts(directory_entries[directory_entry_index]);
        free(directory_entries[directory_entry_index]);
    }
    closedir(directory_stream);
    return (Return_Status__Success);
}

static void Print_Unsigned(unsigned value)
{
    printf("%u\n", value);
}

static Return_Status Reveal_Entry(String entry_path)
{
    struct stat metadata;
    if (OPTIONS & is_following_symlinks_bit__ ? stat(entry_path, &metadata) :
                                                lstat(entry_path, &metadata))
    {
        return (Write_Error("can not find the entry \"", entry_path, "\".",
                            "Ensure that you did not misspelled its path."));
    }
    switch (OPTIONS & ~non_data_type_bits__)
    {
        Parse_Case__(Data_Type__Type, Reveal_Type(&metadata));
        Parse_Case__(Data_Type__Size, Reveal_Size(&metadata));
        Parse_Case__(Data_Type__Byte_Size, printf("%ld\n", metadata.st_size));
        Parse_Case__(Data_Type__Permissions, Reveal_Permissions(&metadata));
        Parse_Case__(Data_Type__Octal_Permissions,
                     Reveal_Octal_Permissions(&metadata));
        Parse_Return_Case__(Data_Type__User, Reveal_User(&metadata,
                                                         entry_path));
        Parse_Case__(Data_Type__User_Uid, Print_Unsigned(metadata.st_uid));
        Parse_Return_Case__(Data_Type__Group, Reveal_Group(&metadata,
                                                           entry_path));
        Parse_Case__(Data_Type__Group_Gid, Print_Unsigned(metadata.st_gid));
        Parse_Return_Case__(Data_Type__Modified_Date,
                            Reveal_Modified_Date(&metadata))
    default:
        switch (metadata.st_mode & S_IFMT)
        {
            Parse_Return_Case__(S_IFREG, Reveal_File(entry_path));
            Parse_Return_Case__(S_IFDIR, Reveal_Directory(entry_path));
            Parse_Return_Case__(S_IFLNK,
                                Write_Error("can not reveal the contents of "
                                            "the symlink \"", entry_path, "\".",
                                            "Try to use the "
                                            "\"--follow-symlinks\" option "
                                            "before it."));
        default:
            return (Write_Error("can not reveal the contents of the entry \"",
                                entry_path, "\" due to its unreadable nature.",
                                NULL));
        }
    }
    return (Return_Status__Success);
}

int main(const int total_of_arguments, Array_Of_String arguments,
         Array_Of_String __environment_variables)
{
    if (total_of_arguments == 1)
    {
        return (Reveal_Entry("."));
    }
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("help", program_help__);
        Parse_Metadata_Option__("version", program_version__);
    }
    const char* last_entry_path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        const uint8_t is_last_argument = argument_index == total_of_arguments -
                                                           1;
        Parse_Data_Type_Option__("contents", Data_Type__Contents);
        Parse_Data_Type_Option__("type", Data_Type__Type);
        Parse_Data_Type_Option__("size", Data_Type__Size);
        Parse_Data_Type_Option__("byte-size", Data_Type__Byte_Size);
        Parse_Data_Type_Option__("permissions", Data_Type__Permissions);
        Parse_Data_Type_Option__("octal-permissions",
                                 Data_Type__Octal_Permissions);
        Parse_Data_Type_Option__("user", Data_Type__User);
        Parse_Data_Type_Option__("user-uid", Data_Type__User_Uid);
        Parse_Data_Type_Option__("group", Data_Type__Group);
        Parse_Data_Type_Option__("group-gid", Data_Type__Group_Gid);
        Parse_Data_Type_Option__("modified-date", Data_Type__Modified_Date);
        Parse_Non_Data_Type_Option__("follow-symlinks",
                                     OPTIONS |= is_following_symlinks_bit__);
        Parse_Non_Data_Type_Option__("unfollow-symlinks",
                                     OPTIONS &= ~is_following_symlinks_bit__);
        String entry_path = arguments[argument_index];
        if (Reveal_Entry(entry_path) == Return_Status__Failure)
        {
            continue;
        }
        OPTIONS &= ~is_expecting_entry_path_bit__;
        last_entry_path = entry_path;
    }
    return !!(OPTIONS & had_error_bit__ ? EXIT_FAILURE : EXIT_SUCCESS);
}
