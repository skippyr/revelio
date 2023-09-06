#include <sys/stat.h>

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define program_name__    "reveal"
#define program_version__ "v11.0.4"

#define Parse_Exit_Code__(exit_code)     (exit_code ? EXIT_FAILURE :           \
                                                      EXIT_SUCCESS)
#define Parse_Null_String__(string)      (string ? string : "")
#define Parse_Puts_Case__(value, string) Parse_Case__(value, puts(string))
#define Parse_Permission__(permission, character)                              \
    putchar(metadata->st_mode & (permission) ? character : '-');
#define Parse_Return_Case__(value, action)                                     \
    case (value):                                                              \
        return (action);
#define Parse_Case__(value, action)                                            \
    case (value):                                                              \
        action;                                                                \
        break;
#define Parse_Size_Prefix_Multiplier__(multiplier, character)                  \
    size = metadata->st_size / (multiplier);                                   \
    if ((int)size)                                                             \
    {                                                                          \
        printf("%.1f%cB\n", size, character);                                  \
        return;                                                                \
    }
#define Parse_Option__(option, argument, action)                               \
    if (!strcmp("-" option, argument))                                         \
    {                                                                          \
        action;                                                                \
    }
#define Parse_Metadata_Option__(option, action)                                \
    Parse_Option__(option, arguments[argument_index],                          \
        action;                                                                \
        exit(EXIT_SUCCESS);                                                    \
    )
#define Parse_Data_Type_Option__(option, data_type)                            \
    Parse_Option__(option, argument,                                           \
        if (IS_AWAITING_PATH_ARGUMENT)                                         \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        DATA_TYPE                 = data_type;                                 \
        IS_AWAITING_PATH_ARGUMENT = 1;                                         \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        return (1);                                                            \
    )
#define Parse_Symlink_Option__(option, is_following_symlinks)                  \
    Parse_Option__(option, argument,                                           \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        IS_FOLLOWING_SYMLINKS = is_following_symlinks;                         \
        return (1);                                                            \
    )

enum Data_Type
{
    Data_Type__Contents,
    Data_Type__Type,
    Data_Type__Size,
    Data_Type__Byte_Size,
    Data_Type__Permissions,
    Data_Type__Octal_Permissions,
    Data_Type__User,
    Data_Type__User_Id,
    Data_Type__Group,
    Data_Type__Group_Id,
    Data_Type__Modified_Date
};

static enum Data_Type DATA_TYPE                 = Data_Type__Contents;
static uint8_t        IS_AWAITING_PATH_ARGUMENT = 0,
                      IS_FOLLOWING_SYMLINKS     = 1,
                      HAD_ERROR                 = 0;

static uint8_t Print_Error(char *first_description, char *second_description,
                           char *third_description, char *fix_suggestion);
static void    Print_Help(void);
static void    Throw_Error(char *description);
static void    Allocate_Directory_Entries(DIR *stream, char **entries);
static void    Sort_Directory_Entries(char **entries, size_t total_of_entries);
static size_t  Get_Total_Of_Directory_Entries(DIR *stream);
static void    Reveal_Type(struct stat *metadata);
static void    Reveal_Size(struct stat *metadata);
static void    Reveal_Byte_Size(struct stat *metadata);
static void    Reveal_Permissions(struct stat *metadata);
static void    Reveal_Octal_Permissions(struct stat *metadata);
static uint8_t Reveal_User(char *path, struct stat *metadata);
static uint8_t Reveal_Group(char *path, struct stat *metadata);
static void    Reveal_Ownership_Id(unsigned id);
static void    Reveal_Modified_Date(struct stat *metadata);
static uint8_t Reveal_File(char *path);
static uint8_t Reveal_Directory(char *path);
static uint8_t Reveal_Contents(char *path, struct stat *metadata);
static uint8_t Reveal(char *path);
static void    Parse_Metadata_Options(int total_of_arguments, char **arguments);
static uint8_t Parse_Data_Type_Options(char *path, char *argument,
                                       uint8_t is_last_argument);
static uint8_t Parse_Symlink_Options(char *path, char *argument,
                                     uint8_t is_last_argument);
static void    Parse_Non_Metadata_Option(int total_of_arguments,
                                         char **arguments);

static uint8_t
Print_Error(char *first_description, char *second_description,
            char *third_description, char *fix_suggestion)
{
    fprintf(stderr, "%s: %s%s%s\n%s%s", program_name__,
            Parse_Null_String__(first_description),
            Parse_Null_String__(second_description),
            Parse_Null_String__(third_description),
            Parse_Null_String__(fix_suggestion), fix_suggestion ? "\n" : "");
    HAD_ERROR = 1;
    return (1);
}

static void
Print_Help(void)
{
    printf("Usage: %s [OPTION | PATH]...\n", program_name__);
    puts("Reveals information about entries in the file system.");
    puts("");
    puts("METADATA OPTIONS");
    puts("These options retrieve information about the program.");
    puts("");
    puts("    -v    print its version.");
    puts("    -h    print this help.");
    puts("");
    puts("Once used, they will imediatelly stop its execution after perform "
         "its action.");
    puts("");
    puts("DATA TYPE OPTIONS");
    puts("These options change the data type to reveal from the entries "
         "following them.");
    puts("");
    puts("    -c (default)    reveal its contents.");
    puts("    -t              reveal its type: regular (r), directory (d), "
         "symlink (l),");
    puts("                    socket (s), fifo (f), character device (c), "
         "block device (b)");
    puts("                    or unknown (-).");
    puts("    -s              reveal its size using a convenient unit: "
         "gigabyte (GB),");
    puts("                    megabyte (MB), kilobyte (kB) or byte (B).");
    puts("    -bs             reveal its size in bytes with no unit beside.");
    puts("    -p              reveal its read (r), write (w), execute (x) and "
         "lack (-)");
    puts("                    permissions for user, group and others.");
    puts("    -op             reveal its permissions using octal base.");
    puts("    -u              reveal the user that owns it.");
    puts("    -ui             reveal the ID of the user that owns it.");
    puts("    -g              reveal the group that owns it.");
    puts("    -gi             reveal the ID of the group that owns it.");
    puts("    -md             reveal the date when its contents were last "
         "modified.");
    puts("");
    puts("All these options expect a path following them. If not provided, "
         "they will");
    puts("consider the last valid one given or, else, the current directory.");
    puts("");
    puts("If none of these is provided, the one marked as default will be "
         "considered.");
    puts("");
    puts("SYMLINKS OPTIONS");
    puts("These options change how symlinks following them will be handled, "
         "possibly");
    puts("changing the origin of the data revealed.");
    puts("");
    puts("    -fl (default)    follow symlinks.");
    puts("    -dfl             don't follow symlinks.");
    puts("");
    puts("If none of these is provided, the one marked as default will be "
         "considered.");
    puts("");
    puts("EXIT CODES");
    printf("Code %i will be throw if an error happens and %i, otherwise.\n",
           EXIT_FAILURE, EXIT_SUCCESS);
    puts("");
    puts("SOURCE CODE");
    puts("Its source code is available at: "
         "<https://github.com/skippyr/reveal>.");
    puts("");
    puts("SUPPORT");
    puts("If you need any kind of support, for instance: help with "
         "troubleshooting, have");
    puts("questions about it or want to give improvement suggestions, please "
         "report them");
    puts("by filing new issues in its issues page:");
    puts("<https://github.com/skippyr/reveal/issues>.");
}

static void
Throw_Error(char *description)
{
    Print_Error(description, 0, 0, 0);
    exit(EXIT_FAILURE);
}

static void
Allocate_Directory_Entries(DIR *stream, char **entries)
{
    size_t entry_index = 0;
    for (struct dirent *entry; (entry = readdir(stream));)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        size_t size       = strlen(entry->d_name) + 1;
        char  *allocation = malloc(size);
        if (!allocation)
        {
            Throw_Error("can't allocate enough memory.");
        }
        memcpy(allocation, entry->d_name, size);
        entries[entry_index] = allocation;
        entry_index++;
    }
}

static void
Sort_Directory_Entries(char **entries, size_t total_of_entries)
{
    for (size_t entry_index = 0; entry_index < total_of_entries - 1;
         entry_index++)
    {
        size_t swap_index = entry_index;
        for (size_t check_index = entry_index + 1;
             check_index < total_of_entries; check_index++)
        {
            if (strcmp(entries[check_index], entries[swap_index]) < 0)
            {
                swap_index = check_index;
            }
        }
        if (swap_index == entry_index)
        {
            continue;
        }
        char *swap_entry     = entries[entry_index];
        entries[entry_index] = entries[swap_index];
        entries[swap_index]  = swap_entry;
    }
}

static size_t
Get_Total_Of_Directory_Entries(DIR *stream)
{
    size_t total_of_entries = 0;
    for (struct dirent *entry; (entry = readdir(stream)); total_of_entries++);
    return (total_of_entries -= 2);
}

static void
Reveal_Type(struct stat *metadata)
{
    /* Modified: instead of printing the first letter now it prints full type name, more clarity */
    switch (metadata->st_mode & S_IFMT)
    {
        Parse_Puts_Case__(S_IFREG, "Regular");
        Parse_Puts_Case__(S_IFDIR, "Directory");
        Parse_Puts_Case__(S_IFLNK, "Link");
        Parse_Puts_Case__(S_IFSOCK, "Socket");
        Parse_Puts_Case__(S_IFIFO, "Fifo");
        Parse_Puts_Case__(S_IFCHR, "Character Driver");
        Parse_Puts_Case__(S_IFBLK, "Block Driver");
    default:
        puts("-");
    }
}

/**
 * Reveal Size with ./reveal -s PATH is unclear or not working, it only shows size
 * in kB, can't specify MB or GB in any way.
*/
static void
Reveal_Size(struct stat *metadata)
{
    float size;
    Parse_Size_Prefix_Multiplier__(1e9, 'G');
    Parse_Size_Prefix_Multiplier__(1e6, 'M');
    Parse_Size_Prefix_Multiplier__(1e3, 'k');
    printf("%ldB\n", metadata->st_size);
}

static void
Reveal_Byte_Size(struct stat *metadata)
{
    printf("%ld\n", metadata->st_size);
}

static void
Reveal_Permissions(struct stat *metadata)
{
    Parse_Permission__(S_IRUSR, 'r');
    Parse_Permission__(S_IWUSR, 'w');
    Parse_Permission__(S_IXUSR, 'x');
    Parse_Permission__(S_IRGRP, 'r');
    Parse_Permission__(S_IWGRP, 'w');
    Parse_Permission__(S_IXGRP, 'x');
    Parse_Permission__(S_IROTH, 'r');
    Parse_Permission__(S_IWOTH, 'w');
    Parse_Permission__(S_IXOTH, 'x');
    putchar('\n');
}

static void
Reveal_Octal_Permissions(struct stat *metadata)
{
    printf("%o\n", metadata->st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                                        S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
                                        S_IXOTH));
}

static uint8_t
Reveal_User(char *path, struct stat *metadata)
{
    struct passwd *user = getpwuid(metadata->st_uid);
    if (!user)
    {
        return (Print_Error("can't discover user that owns \"", path, "\".",
                            "Check if that isn't a dangling symlink."));
    }
    puts(user->pw_name);
    return (0);
}

static uint8_t
Reveal_Group(char *path, struct stat *metadata)
{
    struct group *group = getgrgid(metadata->st_gid);
    if (!group)
    {
        return (Print_Error("can't discover group that owns \"", path, "\".",
                            "Check if that isn't a dangling symlink."));
    }
    puts(group->gr_name);
    return (0);
}

static void
Reveal_Ownership_Id(unsigned id)
{
    printf("%u\n", id);
}

static void
Reveal_Modified_Date(struct stat *metadata)
{
    char modified_date[29];
    strftime(modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
             localtime(&metadata->st_mtime));
    puts(modified_date);
}

static uint8_t
Reveal_File(char *path)
{
    FILE *stream = fopen(path, "r");
    if (!stream)
    {
        return (Print_Error("can't open file \"", path, "\".", "Check if you "
                            "have permission to read it."));
    }
    for (char character; (character = fgetc(stream)) != EOF;
         putchar(character));
    fclose(stream);
    return (0);
}

static uint8_t
Reveal_Directory(char *path)
{
    DIR *stream = opendir(path);
    if (!stream)
    {
        return (Print_Error("can't open directory \"", path, "\".", "Check if "
                            "you have permission to read it."));
    }
    size_t total_of_entries = Get_Total_Of_Directory_Entries(stream);
    if (!total_of_entries)
    {
        closedir(stream);
        return (0);
    }
    char *entries[total_of_entries];
    rewinddir(stream);
    Allocate_Directory_Entries(stream, entries);
    Sort_Directory_Entries(entries, total_of_entries);
    for (size_t entry_index = 0; entry_index < total_of_entries; entry_index++)
    {
        puts(entries[entry_index]);
        free(entries[entry_index]);
    }
    closedir(stream);
    return (0);
}

static uint8_t
Reveal_Contents(char *path, struct stat *metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
        Parse_Return_Case__(S_IFREG, Reveal_File(path));
        Parse_Return_Case__(S_IFDIR, Reveal_Directory(path));
        Parse_Return_Case__(S_IFLNK, Print_Error("can't reveal contents of "
                                                 "symlink \"", path, "\".",
                                                 "Experiment to use the "
                                                 "\"-fl\" option right before "
                                                 "it."));
    default:
        return (Print_Error("can't reveal contents of \"", path, "\" due to "
                            "its unreadable type.", 0));
    }
}

static uint8_t
Reveal(char *path)
{
    struct stat metadata;
    if (IS_FOLLOWING_SYMLINKS ? stat(path, &metadata) : lstat(path, &metadata))
    {
        return (Print_Error("can't find entry \"", path, "\".", "Check if you "
                            "misspelled it."));
    }
    switch (DATA_TYPE)
    {
        Parse_Case__(Data_Type__Type, Reveal_Type(&metadata));
        Parse_Case__(Data_Type__Size, Reveal_Size(&metadata));
        Parse_Case__(Data_Type__Byte_Size, Reveal_Byte_Size(&metadata));
        Parse_Case__(Data_Type__Permissions, Reveal_Permissions(&metadata));
        Parse_Case__(Data_Type__Octal_Permissions,
                     Reveal_Octal_Permissions(&metadata));
        Parse_Return_Case__(Data_Type__User, Reveal_User(path, &metadata));
        Parse_Case__(Data_Type__User_Id, Reveal_Ownership_Id(metadata.st_uid));
        Parse_Return_Case__(Data_Type__Group, Reveal_Group(path, &metadata));
        Parse_Case__(Data_Type__Group_Id, Reveal_Ownership_Id(metadata.st_gid));
        Parse_Case__(Data_Type__Modified_Date, Reveal_Modified_Date(&metadata));
    default:
        return (Reveal_Contents(path, &metadata));
    }
    return (0);
}

static void
Parse_Metadata_Options(int total_of_arguments, char **arguments)
{
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        Parse_Metadata_Option__("h", Print_Help());
        Parse_Metadata_Option__("v", puts(program_version__));
    }
}

static uint8_t
Parse_Data_Type_Options(char *path, char *argument, uint8_t is_last_argument)
{
    Parse_Data_Type_Option__("c", Data_Type__Contents);
    Parse_Data_Type_Option__("t", Data_Type__Type);
    Parse_Data_Type_Option__("s", Data_Type__Size);
    Parse_Data_Type_Option__("bs", Data_Type__Byte_Size);
    Parse_Data_Type_Option__("p", Data_Type__Permissions);
    Parse_Data_Type_Option__("op", Data_Type__Octal_Permissions);
    Parse_Data_Type_Option__("u", Data_Type__User);
    Parse_Data_Type_Option__("ui", Data_Type__User_Id);
    Parse_Data_Type_Option__("g", Data_Type__Group);
    Parse_Data_Type_Option__("gi", Data_Type__Group_Id);
    Parse_Data_Type_Option__("md", Data_Type__Modified_Date);
    return (0);
}

static uint8_t
Parse_Symlink_Options(char *path, char *argument, uint8_t is_last_argument)
{
    Parse_Symlink_Option__("fl", 1);
    Parse_Symlink_Option__("dfl", 0);
    return (0);
}

static void
Parse_Non_Metadata_Option(int total_of_arguments, char **arguments)
{
    char *path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        char   *argument         = arguments[argument_index];
        uint8_t is_last_argument = argument_index == total_of_arguments - 1;
        if (Parse_Data_Type_Options(path, argument, is_last_argument) ||
            Parse_Symlink_Options(path, argument, is_last_argument)   ||
            Reveal(argument))
        {
            continue;
        }
        IS_AWAITING_PATH_ARGUMENT = 0;
        path                      = argument;
    }
}

int
main(int total_of_arguments, char **arguments)
{
    if (total_of_arguments == 1)
    {
        return (Parse_Exit_Code__(Reveal(".")));
    }
    Parse_Metadata_Options(total_of_arguments, arguments);
    Parse_Non_Metadata_Option(total_of_arguments, arguments);
    return (Parse_Exit_Code__(HAD_ERROR));
}
