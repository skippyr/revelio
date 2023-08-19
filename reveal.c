#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define program_metadata__name__ "reveal"
#define program_metadata__version__ "v9.1.2"
#define program_metadata__help__                                               \
    "Usage: " program_metadata__name__ " [OPTION | PATH]...\n"                 \
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
#define bits__is_expecting_entry_path__ (1 << 5)
#define bits__is_following_symlinks__ (1 << 6)
#define bits__had_error__ (1 << 7)
#define bits__non_data_type_collection__                                       \
    (bits__is_expecting_entry_path__ | bits__is_following_symlinks__ |         \
     bits__had_error__)
#define Parse_Null_String__(string) (string ? string : "")
#define Parse_Return_Case__(case_value, case_action)                           \
    case case_value:                                                           \
        return (case_action);
#define Parse_Case__(case_value, case_action)                                  \
    case case_value:                                                           \
        case_action;                                                           \
        break;
#define Parse_Puts_Case__(case_value, text)                                    \
    Parse_Case__(case_value, puts(text));
#define Parse_Size_Si_Prefix_Multiplier__(si_prefix_multiplier_value,          \
                                          si_prefix_multiplier_character)      \
    size = entry_metadata->st_size / si_prefix_multiplier_value;               \
    if ((int)size != 0)                                                        \
    {                                                                          \
        printf("%.1f%cB\n", size, si_prefix_multiplier_character);             \
        return;                                                                \
    }
#define Parse_Option__(option, action)                                         \
    if (!strcmp("--" option, arguments[argument_index]))                       \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(metadata_option, metadata_text)                \
    Parse_Option__(metadata_option, puts(metadata_text); return (EXIT_SUCCESS))
#define Parse_Data_Type_Option__(data_type_option, data_type_value)            \
    Parse_Option__(                                                            \
        data_type_option,                                                      \
        if (OPTIONS & bits__is_expecting_entry_path__)                         \
        {                                                                      \
            Reveal_Entry(last_entry_path);                                     \
        }                                                                      \
        OPTIONS = data_type_value | bits__is_expecting_entry_path__ |          \
                  (OPTIONS & bits__non_data_type_collection__);                \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path);                                     \
        }                                                                      \
        continue;                                                              \
    )
#define Parse_Non_Data_Type_Option__(non_data_type_option, action)             \
    Parse_Option__(                                                            \
        non_data_type_option,                                                  \
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

static Return_Status Print_Error(String error_description_split_0,
                                 String error_description_split_1,
                                 String error_description_split_2,
                                 String error_fix_suggestion);
static void Print_Unsigned_Value(unsigned value);
static void Parse_Permission_Bit(Metadata entry_metadata,
                                 const uint16_t permission_bit,
                                 const char permission_character);
static void Sort_Directory_Entries_Alphabetically(
    void** directory_entries, size_t total_of_directory_entries);
static void Reveal_Type(Metadata entry_metadata);
static void Reveal_Size(Metadata entry_metadata);
static void Reveal_Byte_Size(Metadata entry_metadata);
static void Reveal_Permissions(Metadata entry_metadata);
static void Reveal_Octal_Permissions(Metadata entry_metadata);
static Return_Status Reveal_User(Metadata entry_metadata, String entry_path);
static Return_Status Reveal_Group(Metadata entry_metadata, String entry_path);
static Return_Status Reveal_Modified_Date(Metadata entry_metadata);
static Return_Status Reveal_File(String file_path);
static Return_Status Reveal_Directory(String directory_path);
static Return_Status Reveal_Entry(String entry_path);

static uint8_t OPTIONS = bits__is_following_symlinks__;

static void Parse_Permission_Bit(Metadata entry_metadata,
                                 const uint16_t permission_bit,
                                 const char permission_character)
{
    const char lack_permission_character = '-';
    putchar(entry_metadata->st_mode & permission_bit ?
            permission_character : lack_permission_character);
}

static Return_Status Print_Error(String error_description_split_0,
                                 String error_description_split_1,
                                 String error_description_split_2,
                                 String error_fix_suggestion)
{
    fprintf(stderr, "%s: %s%s%s\n%s%s", program_metadata__name__,
            Parse_Null_String__(error_description_split_0),
            Parse_Null_String__(error_description_split_1),
            Parse_Null_String__(error_description_split_2),
            Parse_Null_String__(error_fix_suggestion),
            error_fix_suggestion ? "\n" : "");
    return (Return_Status__Failure);
}

static void Print_Unsigned_Value(unsigned value)
{
    printf("%u\n", value);
}

static void Sort_Directory_Entries_Alphabetically(
    void** directory_entries, size_t total_of_directory_entries)
{
    for (size_t directory_entry_index = 0;
         directory_entry_index < total_of_directory_entries - 1;
         directory_entry_index++)
    {
        size_t swap_directory_entry_index = directory_entry_index;
        for (size_t check_directory_entry_index = directory_entry_index + 1;
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
}

static void Reveal_Type(Metadata entry_metadata)
{
    switch (entry_metadata->st_mode & S_IFMT)
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

static void Reveal_Size(Metadata entry_metadata)
{
    float size;
    const float gigabyte_in_bytes = 1e9, megabyte_in_bytes = 1e6,
                kilobyte_in_bytes = 1e3;
    Parse_Size_Si_Prefix_Multiplier__(gigabyte_in_bytes, 'G');
    Parse_Size_Si_Prefix_Multiplier__(megabyte_in_bytes, 'M');
    Parse_Size_Si_Prefix_Multiplier__(kilobyte_in_bytes, 'k');
    printf("%ldB\n", entry_metadata->st_size);
}

static void Reveal_Byte_Size(Metadata entry_metadata)
{
    printf("%ld\n", entry_metadata->st_size);
}

static void Reveal_Permissions(Metadata entry_metadata)
{
    const char read_permission_character = 'r',
               write_permission_character = 'w',
               execute_permission_character = 'x';
    Parse_Permission_Bit(entry_metadata, S_IRUSR, read_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IWUSR, write_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IXUSR, execute_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IRGRP, read_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IWGRP, write_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IXGRP, execute_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IROTH, read_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IWOTH, write_permission_character);
    Parse_Permission_Bit(entry_metadata, S_IXOTH, execute_permission_character);
    putchar('\n');
}

static void Reveal_Octal_Permissions(Metadata entry_metadata)
{
    printf("0%o\n", entry_metadata->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR |
                                               S_IRGRP | S_IWGRP | S_IXGRP |
                                               S_IROTH | S_IWOTH | S_IXOTH));
}

static Return_Status Reveal_User(Metadata entry_metadata, String entry_path)
{
    const struct passwd* const user = getpwuid(entry_metadata->st_uid);
    if (!user)
    {
        return (Print_Error("can not discover the user that owns the entry \"",
                            entry_path, "\".", "Ensure that it is not "
                            "dangling."));
    }
    puts(user->pw_name);
    return (Return_Status__Success);
}

static Return_Status Reveal_Group(Metadata entry_metadata, String entry_path)
{
    const struct group* const group = getgrgid(entry_metadata->st_gid);
    if (!group)
    {
        return (Print_Error("can not discover the group that owns the entry \"",
                            entry_path, "\".", "Ensure that it is not "
                            "dangling."));
    }
    puts(group->gr_name);
    return (Return_Status__Success);
}

static Return_Status Reveal_Modified_Date(Metadata entry_metadata)
{
    char modified_date[29];
    if (strftime(modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
                 localtime(&entry_metadata->st_mtime)) == 0)
    {
        return (Print_Error("overflowed buffer intended to store the modified "
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
        return (Print_Error("can not open the file \"", file_path, "\".",
                            "Ensure that you have permissions to read it."));
    }
    char file_character;
    while ((file_character = fgetc(file_stream)) != EOF)
    {
        putchar(file_character);
    }
    fclose(file_stream);
    return (Return_Status__Success);
}

static Return_Status Reveal_Directory(String directory_path)
{
    DIR* const directory_stream = opendir(directory_path);
    if (!directory_stream)
    {
        return (Print_Error("can not open the directory \"", directory_path,
                            "\".", "Ensure that you have permissions to read "
                            "it."));
    }
    const struct dirent* directory_entry;
    size_t total_of_directory_entries = 0;
    while ((directory_entry = readdir(directory_stream)))
    {
        total_of_directory_entries++;
    }
    total_of_directory_entries -= 2;
    if (total_of_directory_entries == 0)
    {
        closedir(directory_stream);
        return (Return_Status__Success);
    }
    void* directory_entries[total_of_directory_entries];
    size_t directory_entry_index = 0;
    rewinddir(directory_stream);
    while ((directory_entry = readdir(directory_stream)))
    {
        if (!strcmp(directory_entry->d_name, ".") ||
            !strcmp(directory_entry->d_name, ".."))
        {
            continue;
        }
        size_t directory_entry_size = strlen(directory_entry->d_name) + 1;
        void* directory_entry_allocation = malloc(directory_entry_size);
        if (!directory_entry_allocation)
        {
            Print_Error("can not allocate memory to reveal the directory \"",
                        directory_path, "\".", NULL);
            exit(EXIT_FAILURE);
        }
        memcpy(directory_entry_allocation, directory_entry->d_name,
               directory_entry_size);
        directory_entries[directory_entry_index] = directory_entry_allocation;
        directory_entry_index++;
    }
    Sort_Directory_Entries_Alphabetically(directory_entries,
                                          total_of_directory_entries);
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

static Return_Status Reveal_Entry(String entry_path)
{
    struct stat entry_metadata;
    if (OPTIONS & bits__is_following_symlinks__ ?
        stat(entry_path, &entry_metadata) :
        lstat(entry_path, &entry_metadata) < 0)
    {
        return (Print_Error("can not find the entry \"", entry_path, "\".",
                            "Ensure that you did not misspelled its path."));
    }
    switch (OPTIONS & ~bits__non_data_type_collection__)
    {
        Parse_Case__(Data_Type__Type, Reveal_Type(&entry_metadata));
        Parse_Case__(Data_Type__Size, Reveal_Size(&entry_metadata));
        Parse_Case__(Data_Type__Byte_Size, Reveal_Byte_Size(&entry_metadata));
        Parse_Case__(Data_Type__Permissions,
                     Reveal_Permissions(&entry_metadata));
        Parse_Case__(Data_Type__Octal_Permissions,
                     Reveal_Octal_Permissions(&entry_metadata));
        Parse_Return_Case__(Data_Type__User, Reveal_User(&entry_metadata,
                                                         entry_path));
        Parse_Case__(Data_Type__User_Uid,
                     Print_Unsigned_Value(entry_metadata.st_uid));
        Parse_Return_Case__(Data_Type__Group, Reveal_Group(&entry_metadata,
                                                           entry_path));
        Parse_Case__(Data_Type__Group_Gid,
                     Print_Unsigned_Value(entry_metadata.st_gid));
        Parse_Return_Case__(Data_Type__Modified_Date,
                            Reveal_Modified_Date(&entry_metadata));
    default:
        switch (entry_metadata.st_mode & S_IFMT)
        {
            Parse_Return_Case__(S_IFREG, Reveal_File(entry_path));
            Parse_Return_Case__(S_IFDIR, Reveal_Directory(entry_path));
            Parse_Return_Case__(S_IFLNK,
                                Print_Error("can not reveal the contents of "
                                            "the symlink \"", entry_path, "\".",
                                            "Try to use the "
                                            "\"--follow-symlinks\" option "
                                            "before it."));
        default:
            return (Print_Error("can not reveal the contents of the entry \"",
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
        return (Reveal_Entry(".") == Return_Status__Success ? EXIT_SUCCESS :
                                                              EXIT_FAILURE);
    }
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("help", program_metadata__help__);
        Parse_Metadata_Option__("version", program_metadata__version__);
    }
    const char* last_entry_path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        const bool is_last_argument = argument_index == total_of_arguments - 1;
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
                                     OPTIONS |= bits__is_following_symlinks__);
        Parse_Non_Data_Type_Option__("unfollow-symlinks",
                                     OPTIONS &= ~bits__is_following_symlinks__);
        String entry_path = arguments[argument_index];
        if (Reveal_Entry(entry_path) == Return_Status__Failure)
        {
            continue;
        }
        OPTIONS &= ~bits__is_expecting_entry_path__;
        last_entry_path = entry_path;
    }
    return (OPTIONS & bits__had_error__ ? EXIT_FAILURE : EXIT_SUCCESS);
}
