#include <sys/stat.h>

#include <dirent.h>
#include <grp.h>
#include <pwd.h>

#include <cstring>
#include <iostream>

#define PROGRAM_NAME "reveal"
#define PROGRAM_VERSION "v14.0.0"
#define PARSE_NULL_STRING(string) (string ? string : "")
#define PARSE_SIZE_PREFIX_MULTIPLIER(multiplier, prefix)                       \
    size = metadata.st_size / (multiplier);                                    \
    if ((int)size)                                                             \
    {                                                                          \
        Print_Float_Size(size, prefix);                                        \
        return;                                                                \
    }
#define PARSE_PERMISSION(permission, character)                                \
    std::cout << (metadata.st_mode & permission ? character : "-")
#define PARSE_OPTION(option, argument, action)                                 \
    if (!strcmp("--" option, argument))                                        \
    {                                                                          \
        action;                                                                \
    }
#define PARSE_METADATA_OPTION(option, action)                                  \
    PARSE_OPTION(option, arguments[argument_index],                            \
        action;                                                                \
        exit(EXIT_SUCCESS);                                                    \
    )
#define PARSE_DATA_TYPE_OPTION(option, data_type)                              \
    PARSE_OPTION(option, argument,                                             \
        if (IS_EXPECTING_PATH_ARGUMENT)                                        \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        DATA_TYPE = data_type;                                                 \
        IS_EXPECTING_PATH_ARGUMENT = true;                                     \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        return 1;                                                              \
    )
#define PARSE_SYMLINK_OPTION(option, is_following_symlinks)                    \
    PARSE_OPTION(option, argument,                                             \
        if (is_last_argument)                                                  \
        {                                                                      \
            Reveal(path);                                                      \
        }                                                                      \
        IS_FOLLOWING_SYMLINKS = is_following_symlinks;                         \
        return 1;                                                              \
    )
#define PARSE_EXIT_CODE(exit_code) (exit_code ? EXIT_FAILURE : EXIT_SUCCESS)

enum Data_Type
{
    Contents,
    Type,
    Size,
    Byte_Size,
    Permissions,
    Octal_Permissions,
    User,
    User_Id,
    Group,
    Group_Id,
    Modified_Date
};

static void Print_Help();
static void Print_Float_Size(float size, const char *prefix);
static void Print_Byte_Size(unsigned long size, bool is_to_use_unit);
static int Print_Error(const char *first_description,
                       const char *second_description,
                       const char *third_description,
                       const char *fix_suggestion);
static void Throw_Error(const char *description);
static void Allocate_Directory_Entries(DIR *stream, char *entries[]);
static void Sort_Directory_Entries(char *entries[], size_t total_of_entries);
static size_t Get_Total_Of_Directory_Entries(DIR *stream);
static int Reveal(const char *path);
static int Reveal_Contents(const char *path, struct stat &metadata);
static int Reveal_File(const char *path);
static int Reveal_Directory(const char *path);
static void Reveal_Type(struct stat &metadata);
static void Reveal_Size(struct stat &metadata);
static void Reveal_Byte_Size(struct stat &metadata);
static int Reveal_User(const char *path, struct stat &metadata);
static int Reveal_Group(const char *path, struct stat &metadata);
static void Reveal_Permissions(struct stat &metadata);
static void Reveal_Octal_Permissions(struct stat &metadata);
static void Reveal_Modified_Date(struct stat &metadata);
static int Parse_Data_Type_Options(const char *path, const char *argument,
                                   bool is_last_argument);
static int Parse_Symlink_Options(const char *path, const char *argument,
                                 bool is_last_argument);
static void Parse_Metadata_Options(int total_of_arguments,
                                   const char *arguments[]);
static void Parse_Non_Metadata_Options(int total_of_arguments,
                                       const char *arguments[]);

static Data_Type DATA_TYPE = Data_Type::Contents;
static bool HAD_ERROR = false, IS_EXPECTING_PATH_ARGUMENT = false,
            IS_FOLLOWING_SYMLINKS = true;

static void Print_Help()
{
    std::cout << "Usage: " << PROGRAM_NAME << " [OPTION | PATH]..."
              << std::endl
              << "Reveals information about entries in the file system."
              << std::endl << std::endl
              << "METADATA OPTIONS" << std::endl
              << "These options retrieve information about the program."
              << std::endl << std::endl
              << "    --version    print its version." << std::endl
              << "    --help       print its help." << std::endl << std::endl
              << "Once used, they will imediatelly stop its execution after "
                 "perform their actions." << std::endl << std::endl
              << "DATA TYPE OPTIONS" << std::endl
              << "These options change the data type to reveal from the "
                 "entries following them." << std::endl << std::endl
              << "    --contents (default)    reveal its contents." << std::endl
              << "    --type                  reveal its type: regular, "
                 "directory, symlink," << std::endl
              << "                            socket, fifo, character, block "
                 "or unknown." << std::endl
              << "    --size                  reveal its size automatically "
                 "using the most" << std::endl
              << "                            convenient unit for a human to "
                 "read: gigabyte (GB)," << std::endl
              << "                            megabyte (MB), kilobyte (kB) or "
                 "byte (B)." << std::endl
              << "    --byte-size             reveal its size in bytes without "
                 "a unit beside it." << std::endl
              << "    --permissions           reveal its read (r), write (w), "
                 "execute (x) and" << std::endl
              << "                            lack (-) permissions for user, "
                 "group and others." << std::endl
              << "    --octal-permissions     reveal its permissions using "
                 "octal base." << std::endl
              << "    --user                  reveal the user that owns it."
              << std::endl
              << "    --user-id               reveal the ID of the user that "
                 "owns it."
              << std::endl
              << "    --group                 reveal the group that owns it."
              << std::endl
              << "    --group-id              reveal the ID of the group that "
                 "owns it."
              << std::endl
              << "    --modified-date         reveal the date when its "
                 "contents were last" << std::endl
              << "                            modified." << std::endl
              << std::endl
              << "All these options expect a path following them. If not "
                 "provided, they will" << std::endl
              << "consider the last valid one given or, else, the current "
                 "directory." << std::endl << std::endl
              << "If none of these is provided, the one marked as default will "
                 "be considered." << std::endl << std::endl
              << "SYMLINK OPTIONS" << std::endl
              << "These options change how symlinks following them will be "
                 "handled, possibly" << std::endl
              << "changing the origin of the data revealed." << std::endl
              << std::endl
              << "    --follow-symlinks (default)    follow symlinks."
              << std::endl
              << "    --unfollow-symlinks            don't follow symlinks."
              << std::endl << std::endl
              << "If none of these is provided, the one marked as default will "
                 "be considered." << std::endl << std::endl
              << "EXIT CODES" << std::endl
              << "Code " << EXIT_FAILURE << " will be throw if an error "
                 "happens and " << EXIT_SUCCESS << ", otherwise." << std::endl
              << std::endl
              << "SOURCE CODE" << std::endl
              << "Its source code is available at: "
                 "<https://github.com/skippyr/reveal>." << std::endl
              << std::endl
              << "ISSUES AND CONTRIBUTIONS" << std::endl
              << "You can use its issues page to provide your feedback, to "
                 "report any issue you" << std::endl << "are facing or to ask "
                 "for help: <https://github.com/skippyr/reveal/issues>."
              << std::endl << std::endl
              << "You can also fork it and send pull requests to be reviewed."
              << std::endl << std::endl
              << "All kinds of contributions, from people of any skill level, "
                 "is always" << std::endl
              << "appreciated." << std::endl;
}

static void Print_Float_Size(float size, const char *prefix)
{
    std::cout << std::fixed;
    std::cout.precision(1);
    std::cout << size << prefix << "B" << std::endl;
}

static void Print_Byte_Size(unsigned long size, bool is_to_use_unit)
{
    std::cout << size << (is_to_use_unit ? "B" : "") << std::endl;
}

static int Print_Error(const char *first_description,
                       const char *second_description,
                       const char *third_description,
                       const char *fix_suggestion)
{
    std::cerr << PROGRAM_NAME << ": " << PARSE_NULL_STRING(first_description)
              << PARSE_NULL_STRING(second_description)
              << PARSE_NULL_STRING(third_description) << std::endl
              << PARSE_NULL_STRING(fix_suggestion) << (fix_suggestion ? "\n" :
                                                                        "");
    HAD_ERROR = 1;
    return 1;
}

static void Throw_Error(const char *description)
{
    Print_Error(description, 0, 0, 0);
    exit(EXIT_FAILURE);
}

static void Allocate_Directory_Entries(DIR *stream, char *entries[])
{
    size_t entry_index = 0;
    for (struct dirent *entry; (entry = readdir(stream));)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        size_t size = strlen(entry->d_name) + 1;
        char *allocation;
        try
        {
            allocation = new char[size];
        }
        catch(std::bad_alloc &)
        {
            Throw_Error("can't allocate enough memory.");
        }
        memcpy(allocation, entry->d_name, size);
        entries[entry_index] = allocation;
        entry_index++;
    }
}

static void Sort_Directory_Entries(char *entries[], size_t total_of_entries)
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
        char *swap_allocation = entries[entry_index];
        entries[entry_index] = entries[swap_index];
        entries[swap_index] = swap_allocation;
    }
}

static size_t Get_Total_Of_Directory_Entries(DIR *stream)
{
    size_t total_of_entries = 0;
    for (; readdir(stream); total_of_entries++);
    return total_of_entries - 2;
}

static int Reveal(const char *path)
{
    struct stat metadata;
    if (IS_FOLLOWING_SYMLINKS ? stat(path, &metadata) : lstat(path, &metadata))
    {
        return Print_Error("can't find entry \"", path, "\".", "Check if you "
                           "did misspelled it.");
    }
    switch (DATA_TYPE)
    {
    case Data_Type::Type:
        Reveal_Type(metadata);
        break;
    case Data_Type::Size:
        Reveal_Size(metadata);
        break;
    case Data_Type::Byte_Size:
        Reveal_Byte_Size(metadata);
        break;
    case Data_Type::Permissions:
        Reveal_Permissions(metadata);
        break;
    case Data_Type::Octal_Permissions:
        Reveal_Octal_Permissions(metadata);
        break;
    case Data_Type::Modified_Date:
        Reveal_Modified_Date(metadata);
        break;
    case Data_Type::User:
        return Reveal_User(path, metadata);
    case Data_Type::User_Id:
        std::cout << metadata.st_uid << std::endl;
        break;
    case Data_Type::Group:
        return Reveal_Group(path, metadata);
    case Data_Type::Group_Id:
        std::cout << metadata.st_gid << std::endl;
        break;
    default:
        return Reveal_Contents(path, metadata);
    }
    return 0;
}

static int Reveal_Contents(const char *path, struct stat &metadata)
{
    switch (metadata.st_mode & S_IFMT)
    {
    case S_IFREG:
        return Reveal_File(path);
    case S_IFDIR:
        return Reveal_Directory(path);
    case S_IFLNK:
        return Print_Error("can't reveal contents of symlink \"", path, "\".",
                           "Experiment to use the \"--follow-symlinks\" option "
                           "right before it.");
    }
    return Print_Error("can't reveal contents of \"", path, "\" due to its "
                       "unreadable type.", 0);
}

static int Reveal_File(const char *path)
{
    FILE *stream = fopen(path, "r");
    if (!stream)
    {
        return Print_Error("can't open file \"", path, "\".", "Check if you "
                           "have permission to read it.");
    }
    for (char character; (character = fgetc(stream)) != EOF;
         std::cout << character);
    fclose(stream);
    return 0;
}

static int Reveal_Directory(const char *path)
{
    DIR *stream = opendir(path);
    if (!stream)
    {
        return Print_Error("can't open directory \"", path, "\".", "Check if "
                           "you have permission to read it.");
    }
    size_t total_of_entries = Get_Total_Of_Directory_Entries(stream);
    if (!total_of_entries)
    {
        closedir(stream);
        return 0;
    }
    char *entries[total_of_entries];
    rewinddir(stream);
    Allocate_Directory_Entries(stream, entries);
    Sort_Directory_Entries(entries, total_of_entries);
    for (auto &entry : entries)
    {
        std::cout << entry << std::endl;
        delete[] entry;
    }
    closedir(stream);
    return 0;
}

static void Reveal_Type(struct stat &metadata)
{
    switch (metadata.st_mode & S_IFMT)
    {
    case S_IFREG:
        std::cout << "regular";
        break;
    case S_IFDIR:
        std::cout << "directory";
        break;
    case S_IFLNK:
        std::cout << "symlink";
        break;
    case S_IFSOCK:
        std::cout << "socket";
        break;
    case S_IFIFO:
        std::cout << "fifo";
        break;
    case S_IFCHR:
        std::cout << "character";
        break;
    case S_IFBLK:
        std::cout << "block";
        break;
    default:
        std::cout << "unknown";
    }
    std::cout << std::endl;
}

static void Reveal_Size(struct stat &metadata)
{
    float size;
    PARSE_SIZE_PREFIX_MULTIPLIER(1e9, "G");
    PARSE_SIZE_PREFIX_MULTIPLIER(1e6, "M");
    PARSE_SIZE_PREFIX_MULTIPLIER(1e3, "k");
    Print_Byte_Size(metadata.st_size, true);
}

static void Reveal_Byte_Size(struct stat &metadata)
{
    Print_Byte_Size(metadata.st_size, false);
}

static int Reveal_User(const char *path, struct stat &metadata)
{
    struct passwd *user = getpwuid(metadata.st_uid);
    if (!user)
    {
        return Print_Error("can't discover user that owns \"", path, "\".",
                           "Check if that is not a dangling symlink.");
    }
    std::cout << user->pw_name << std::endl;
    return 0;
}

static int Reveal_Group(const char *path, struct stat &metadata)
{
    struct group *group = getgrgid(metadata.st_gid);
    if (!group)
    {
        return Print_Error("can't discover group that owns \"", path, "\".",
                           "Check if that is not a dangling symlink.");
    }
    std::cout << group->gr_name << std::endl;
    return 0;
}

static void Reveal_Permissions(struct stat &metadata)
{
    PARSE_PERMISSION(S_IRUSR, "r");
    PARSE_PERMISSION(S_IWUSR, "w");
    PARSE_PERMISSION(S_IXUSR, "x");
    PARSE_PERMISSION(S_IRGRP, "r");
    PARSE_PERMISSION(S_IWGRP, "w");
    PARSE_PERMISSION(S_IXGRP, "x");
    PARSE_PERMISSION(S_IROTH, "r");
    PARSE_PERMISSION(S_IWOTH, "w");
    PARSE_PERMISSION(S_IXOTH, "x");
    std::cout << std::endl;
}

static void Reveal_Octal_Permissions(struct stat &metadata)
{
    std::cout << std::oct
              << (metadata.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                                      S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
                                      S_IXOTH)) << std::dec << std::endl;
}

static void Reveal_Modified_Date(struct stat &metadata)
{
    char modified_date[29];
    strftime(modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
             localtime(&metadata.st_mtime));
    std::cout << modified_date << std::endl;
}

static int Parse_Data_Type_Options(const char *path, const char *argument,
                                   bool is_last_argument)
{
    PARSE_DATA_TYPE_OPTION("contents", Data_Type::Contents);
    PARSE_DATA_TYPE_OPTION("type", Data_Type::Type);
    PARSE_DATA_TYPE_OPTION("size", Data_Type::Size);
    PARSE_DATA_TYPE_OPTION("byte-size", Data_Type::Byte_Size);
    PARSE_DATA_TYPE_OPTION("permissions", Data_Type::Permissions);
    PARSE_DATA_TYPE_OPTION("octal-permissions", Data_Type::Octal_Permissions);
    PARSE_DATA_TYPE_OPTION("user", Data_Type::User);
    PARSE_DATA_TYPE_OPTION("user-id", Data_Type::User_Id);
    PARSE_DATA_TYPE_OPTION("group", Data_Type::Group);
    PARSE_DATA_TYPE_OPTION("group-id", Data_Type::Group_Id);
    PARSE_DATA_TYPE_OPTION("modified-date", Data_Type::Modified_Date);
    return 0;
}

static int Parse_Symlink_Options(const char *path, const char *argument,
                                 bool is_last_argument)
{
    PARSE_SYMLINK_OPTION("follow-symlinks", true);
    PARSE_SYMLINK_OPTION("unfollow-symlinks", false);
    return 0;
}

static void Parse_Metadata_Options(int total_of_arguments,
                                   const char *arguments[])
{
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        PARSE_METADATA_OPTION("help", Print_Help());
        PARSE_METADATA_OPTION("version", std::cout << PROGRAM_VERSION
                                                   << std::endl);
    }
}

static void Parse_Non_Metadata_Options(int total_of_arguments,
                                       const char *arguments[])
{
    const char *path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        const char *argument = arguments[argument_index];
        bool is_last_argument = argument_index == total_of_arguments - 1;
        if (Parse_Data_Type_Options(path, argument, is_last_argument) ||
            Parse_Symlink_Options(path, argument, is_last_argument) ||
            Reveal(argument))
        {
            continue;
        }
        IS_EXPECTING_PATH_ARGUMENT = false;
        path = argument;
    }
}

int main(int total_of_arguments, const char *arguments[])
{
    if (total_of_arguments == 1)
    {
        return PARSE_EXIT_CODE(Reveal("."));
    }
    Parse_Metadata_Options(total_of_arguments, arguments);
    Parse_Non_Metadata_Options(total_of_arguments, arguments);
    return PARSE_EXIT_CODE(HAD_ERROR);
}
