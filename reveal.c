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

#define Parse_Possible_Null_String__(string) (string ? string : "")
#define Parse_Case__(case_value, case_action)                                  \
    case case_value:                                                           \
        case_action;                                                           \
        break;
#define Parse_Return_Case__(case_value, case_action)                           \
    case case_value:                                                           \
        return (case_action);
#define Parse_Puts_Case__(case_value, case_text) Parse_Case__(case_value,      \
                                                              puts(case_text))
#define Parse_Size_Prefix_Multiplier__(prefix_multiplier_value,                \
                                       prefix_multiplier_character)            \
    size = entry_metadata->st_size / prefix_multiplier_value;                  \
    if ((int)size)                                                             \
    {                                                                          \
        printf("%.1f%cB\n", size, prefix_multiplier_character);                \
        return;                                                                \
    }
#define Parse_Permission_Bit__(permission_bit_value, permission_bit_character) \
    putchar(entry_metadata->st_mode & permission_bit_value ?                   \
            permission_bit_character : lack_permission_character);
#define Parse_Option__(option, action)                                         \
    if (!strcmp("--" option, arguments[argument_index]))                       \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(metadata_option, metadata_action)              \
    Parse_Option__(metadata_option, metadata_action; return (EXIT_SUCCESS));
#define Parse_Data_Type_Option__(data_type_option, data_type_value)            \
    Parse_Option__(                                                            \
        data_type_option,                                                      \
        if (is_expecting_entry_path_argument)                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path, data_type, is_following_symlinks);   \
        }                                                                      \
        data_type = data_type_value;                                           \
        is_expecting_entry_path_argument = true;                               \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path, data_type, is_following_symlinks);   \
        }                                                                      \
        continue;                                                              \
    )
#define Parse_Symlink_Option__(symlink_option, symlink_option_value)           \
    Parse_Option__(                                                            \
        symlink_option,                                                        \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal_Entry(last_entry_path, data_type, is_following_symlinks);   \
        }                                                                      \
        is_following_symlinks = symlink_option_value;                          \
        continue;                                                              \
    )

typedef const char* const String;
typedef String* const Array_Of_String;
typedef const struct stat* const Metadata;
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
typedef enum
{
    Return_Status__Success,
    Return_Status__Failure
}
Return_Status;

static String PROGRAM_NAME = "reveal", PROGRAM_VERSION = "v9.1.3";
static bool HAD_ERROR = false;

static void Print_Help_Instructions(void)
{
    printf("Usage: %s [OPTION | PATH]...\n", PROGRAM_NAME);
    puts("Reveals information about entries in the file system.\n");
    puts("METADATA OPTIONS");
    puts("These options retrieve information about the program.\n");
    puts("    --help       print these help instructions.");
    puts("    --version    print its version.\n");
    puts("DATA TYPE OPTIONS");
    puts("These options change the data type retrieved from the entries "
         "following them.\n");
    puts("    --contents (default)    print its contents.");
    puts("    --type                  print its type: regular, directory, "
         "symlink, socket,\n                            fifo, character, "
         "block or unknown.");
    puts("    --size                  print its size using a convenient unit:"
         "\n                            gigabyte(GB), megabyte (MB), "
         "kilobyte (kB) or\n                            byte (B).");
    puts("    --byte-size             print its size in bytes.");
    puts("    --permissions           print its read (r), write (w) and "
         "execute (x)\n                            permissions for user, group "
         "and others.");
    puts("    --octal-permissions     print its permissions using octal base.");
    puts("    --user                  print the user that owns it.");
    puts("    --user-uid              print the UID of the user that owns it.");
    puts("    --group                 print the group that owns it.");
    puts("    --group-gid             print the GID of the group that owns "
         "it.");
    puts("    --modified-date         print the date when its contents were "
         "last modified.\n");
    puts("If none is used, the one marked as default will be considered.\n");
    puts("Each one of them expects at least one path following it. If none is "
         "given, it\nwill consider the last valid one given or else the "
         "current directory.\n");
    puts("SYMLINK OPTIONS");
    puts("These options change how the symlinks following them are handled, "
         "changing the\norigin of the data revealed.\n");
    puts("    --follow-symlinks (default)    symlinks will be followed.");
    puts("    --unfollow-symlinks            symlinks will not be followed.\n");
    puts("If none is used, the one marked as default will be considered.\n");
    puts("EXIT CODES");
    puts("It will throw exit code 1 if an error happens and 0 otherwise.\n");
    puts("SUPPORT");
    puts("Report issues, questions and suggestions through its issues page:");
    puts("<https://github.com/skippyr/reveal/issues>.");
}

static Return_Status Print_Error(String error_first_description,
                                 String error_second_description,
                                 String error_third_description,
                                 String error_fix_suggestion)
{
    fprintf(stderr, "%s: %s%s%s\n%s%s", PROGRAM_NAME,
            Parse_Possible_Null_String__(error_first_description),
            Parse_Possible_Null_String__(error_second_description),
            Parse_Possible_Null_String__(error_third_description),
            Parse_Possible_Null_String__(error_fix_suggestion),
            error_fix_suggestion ? "\n" : "");
    HAD_ERROR = true;
    return (Return_Status__Failure);
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
    Parse_Size_Prefix_Multiplier__(gigabyte_in_bytes, 'G');
    Parse_Size_Prefix_Multiplier__(megabyte_in_bytes, 'M');
    Parse_Size_Prefix_Multiplier__(kilobyte_in_bytes, 'k');
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
               execute_permission_character = 'x',
               lack_permission_character = '-';
    Parse_Permission_Bit__(S_IRUSR, read_permission_character);
    Parse_Permission_Bit__(S_IWUSR, write_permission_character);
    Parse_Permission_Bit__(S_IXUSR, execute_permission_character);
    Parse_Permission_Bit__(S_IRGRP, read_permission_character);
    Parse_Permission_Bit__(S_IWGRP, write_permission_character);
    Parse_Permission_Bit__(S_IXGRP, execute_permission_character);
    Parse_Permission_Bit__(S_IROTH, read_permission_character);
    Parse_Permission_Bit__(S_IWOTH, write_permission_character);
    Parse_Permission_Bit__(S_IXOTH, execute_permission_character);
    putchar('\n');
}

static void Reveal_Octal_Permissions(Metadata entry_metadata)
{
    printf("0%o\n", entry_metadata->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR |
                                               S_IRGRP | S_IWGRP | S_IXGRP |
                                               S_IROTH | S_IWOTH | S_IXOTH));
}

static void Reveal_Ownership_Id(const unsigned ownership_id)
{
    printf("%u\n", ownership_id);
}

static Return_Status Reveal_User(Metadata entry_metadata, String entry_path)
{
    const struct passwd* const user = getpwuid(entry_metadata->st_uid);
    if (!user)
    {
        return (Print_Error("can not find the user that owns the entry \"",
                            entry_path, "\".", NULL));
    }
    puts(user->pw_name);
    return (Return_Status__Success);
}

static Return_Status Reveal_Group(Metadata entry_metadata, String entry_path)
{
    const struct group* const group = getgrgid(entry_metadata->st_gid);
    if (!group)
    {
        return (Print_Error("can not find the group that owns the entry \"",
                            entry_path, "\".", NULL));
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
        return (Print_Error("the buffer intended to store the modified date was"
                            "overflowed.", NULL, NULL, NULL));
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
                            "Ensure that you have permission to read it."));
    }
    char file_character;
    while ((file_character = fgetc(file_stream)) != EOF)
    {
        putchar(file_character);
    }
    fclose(file_stream);
    return (Return_Status__Success);
}

static size_t Get_Total_Of_Directory_Entries(DIR* const directory_stream)
{
    size_t total_of_directory_entries = 0;
    while ((readdir(directory_stream)))
    {
        total_of_directory_entries++;
    }
    total_of_directory_entries -= 2;
    return (total_of_directory_entries);
}

static void Allocate_Directory_Entries(DIR* const directory_stream,
                                       void** directory_entries,
                                       String directory_path)
{
    rewinddir(directory_stream);
    const struct dirent* directory_entry;
    size_t directory_entry_index = 0;
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

static Return_Status Reveal_Directory(String directory_path)
{
    DIR* const directory_stream = opendir(directory_path);
    if (!directory_stream)
    {
        return (Print_Error("can not open the directory \"", directory_path,
                            "\".", "Ensure that you have permission to read "
                            "it."));
    }
    size_t total_of_directory_entries =
        Get_Total_Of_Directory_Entries(directory_stream);
    if (total_of_directory_entries == 0)
    {
        closedir(directory_stream);
        return (Return_Status__Success);
    }
    void* directory_entries[total_of_directory_entries];
    Allocate_Directory_Entries(directory_stream, directory_entries,
                               directory_path);
    Sort_Directory_Entries_Alphabetically(directory_entries,
                                          total_of_directory_entries);
    for (size_t directory_entry_index = 0;
         directory_entry_index < total_of_directory_entries;
         directory_entry_index++)
    {
        puts(directory_entries[directory_entry_index]);
        free(directory_entries[directory_entry_index]);
    }
    closedir(directory_stream);
    return (Return_Status__Success);
}

static Return_Status Reveal_Entry(String entry_path, Data_Type data_type,
                                  bool is_following_symlinks)
{
    struct stat entry_metadata;
    if (is_following_symlinks ? stat(entry_path, &entry_metadata) :
        lstat(entry_path, &entry_metadata) < 0)
    {
        return (Print_Error("can not find the entry \"", entry_path, "\".",
                            "Ensure that you did not misspelled its path."));
    }
    switch (data_type)
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
                     Reveal_Ownership_Id(entry_metadata.st_uid));
        Parse_Return_Case__(Data_Type__Group, Reveal_Group(&entry_metadata,
                                                           entry_path));
        Parse_Case__(Data_Type__Group_Gid,
                     Reveal_Ownership_Id(entry_metadata.st_gid));
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
    Data_Type data_type = Data_Type__Contents;
    bool is_following_symlinks = true;
    if (total_of_arguments == 1)
    {
        return (Reveal_Entry(".", data_type, is_following_symlinks) ==
                Return_Status__Success ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("help", Print_Help_Instructions());
        Parse_Metadata_Option__("version", puts(PROGRAM_VERSION));
    }
    const char* last_entry_path = ".";
    bool is_expecting_entry_path_argument = false;
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
        Parse_Symlink_Option__("follow-symlinks", true);
        Parse_Symlink_Option__("unfollow-symlinks", false);
        String entry_path = arguments[argument_index];
        if (Reveal_Entry(entry_path, data_type, is_following_symlinks) ==
            Return_Status__Failure)
        {
            continue;
        }
        is_expecting_entry_path_argument = false;
        last_entry_path = entry_path;
    }
    return (HAD_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}
