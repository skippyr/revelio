#include <sys/stat.h>

#include <dirent.h>
#include <grp.h>
#include <pwd.h>

#include <cstring>
#include <ctime>
#include <iostream>

#define PROGRAM_NAME "reveal"
#define PROGRAM_VERSION "v15.0.0"
#define PARSE_SIZE_PREFIX_MULTIPLIER(multiplier, prefix)                       \
    size = metadata.st_size / (multiplier);                                    \
    if ((int)size)                                                             \
    {                                                                          \
        Print_Float_Size(size, prefix);                                        \
        return;                                                                \
    }
#define PARSE_OPTION(option, argument, action)                                 \
    if (!std::strcmp("--" option, argument))                                   \
    {                                                                          \
        action;                                                                \
    }
#define PARSE_METADATA_OPTION(option, action)                                  \
    PARSE_OPTION(option, arguments[argument_index], action; exit(EXIT_SUCCESS);)
#define PARSE_DATA_TYPE_OPTION(option, data_type)                              \
    PARSE_OPTION(                                                              \
        option, argument,                                                      \
        if (IS_EXPECTING_PATH_ARGUMENT) { Reveal(path); } DATA_TYPE =          \
            data_type;                                                         \
        IS_EXPECTING_PATH_ARGUMENT = true;                                     \
        if (is_last_argument) { Reveal(path); } return Parse_Status::Parsed;)
#define PARSE_SYMLINK_OPTION(option, is_following_symlinks)                    \
    PARSE_OPTION(                                                              \
        option, argument,                                                      \
        if (is_last_argument) { Reveal(path); } IS_FOLLOWING_SYMLINKS =        \
            is_following_symlinks;                                             \
        return Parse_Status::Parsed;)

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
    Modification_Date
};

enum Parse_Status
{
    Parsed,
    Not_Parsed
};

enum Return_Status
{
    Success,
    Failure
};

static Data_Type DATA_TYPE = Data_Type::Contents;
static bool HAD_ERROR = false, IS_EXPECTING_PATH_ARGUMENT = false,
            IS_FOLLOWING_SYMLINKS = true;

static int Parse_Exit_Code(int exit_code)
{
    return exit_code ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void Parse_Permission(struct stat &metadata, int permission,
                             char character)
{
    std::cout << (metadata.st_mode & permission ? character : '-');
}

static Return_Status Print_Error(std::string description,
                                 std::string fix_suggestion)
{
    std::cerr << PROGRAM_NAME << ": " << description << std::endl;
    if (fix_suggestion != "")
    {
        std::cout << fix_suggestion << std::endl;
    }
    HAD_ERROR = 1;
    return Return_Status::Failure;
}

static void Print_Float_Size(float value, std::string prefix)
{
    std::cout << std::fixed;
    std::cout.precision(1);
    std::cout << value << prefix << "B" << std::endl;
}

static void Print_Byte_Size(size_t size, bool is_to_use_unit)
{
    std::cout << size << (is_to_use_unit ? "B" : "") << std::endl;
}

static void Print_Help()
{
    std::cout
        << "Usage: " << PROGRAM_NAME << " [OPTION | PATH]..." << std::endl
        << "Reveals information about entries in the file system." << std::endl
        << std::endl
        << "METADATA OPTIONS" << std::endl
        << "These options retrieve information about the program." << std::endl
        << std::endl
        << "    --version    print its version." << std::endl
        << "    --help       print this help." << std::endl
        << std::endl
        << "DATA TYPE OPTIONS" << std::endl
        << "These options change the data type to retrieve from the entries "
           "following them:"
        << std::endl
        << std::endl
        << "    --contents (default)    reveal its contents." << std::endl
        << "    --type                  reveal its type: regular, directory, "
           "symlink,"
        << std::endl
        << "                            socket, fifo, character, block or "
           "unknown."
        << std::endl
        << "    --size                  reveal its size automatically using "
           "the most"
        << std::endl
        << "                            convenient unit: gigabyte (GB), "
           "megabyte (MB),"
        << std::endl
        << "                            kilobyte (kB) or byte (B)." << std::endl
        << "    --byte-size             reveal its size in bytes, without an "
           "unit beside it."
        << std::endl
        << "    --permissions           reveal its read (r), write (w), "
           "execute (x) and"
        << std::endl
        << "                            lack (-) permissions for user, group "
           "and others"
        << std::endl
        << "    --octal-permissions     reveal its permissions using octal "
           "base."
        << std::endl
        << "    --user                  reveal the user that owns it."
        << std::endl
        << "    --user-id               reveal the ID of the user that owns"
        << std::endl
        << "    --group                 reveal the group that owns it."
        << std::endl
        << "    --group-id              reveal the ID of the group that owns "
           "it."
        << std::endl
        << "    --modification-date     reveal the date when its contents were "
           "last"
        << std::endl
        << "                            modified." << std::endl
        << std::endl
        << "These options expect, at least, one path following them. If not "
           "provided, they"
        << std::endl
        << "will consider the last valid one given or, else, the current "
           "directory."
        << std::endl
        << std::endl
        << "If none of these is provided, the one marked as default will be "
           "considered."
        << std::endl
        << std::endl
        << "SYMLINK OPTIONS" << std::endl
        << "These options change how the symlinks following them will be "
           "handled, possibly"
        << std::endl
        << "changing the origin of the data reveal." << std::endl
        << std::endl
        << "    --follow-symlinks (default)    follow symlinks." << std::endl
        << "    --unfollow-symlinks            don't follow symlinks."
        << std::endl
        << std::endl
        << "If none of these is provided, the one marked as default will be "
           "considered."
        << std::endl
        << std::endl
        << "EXIT CODES" << std::endl
        << "Code " << EXIT_FAILURE << " will be throw if an error happens and "
        << EXIT_SUCCESS << ", otherwise." << std::endl
        << std::endl
        << "ISSUES AND CONTRIBUTIONS" << std::endl
        << "You can use its issues page to provide your feedback, to report "
           "any issue you"
        << std::endl
        << "are facing or to ask for help: "
           "<https://github.com/skippyr/reveal/issues>."
        << std::endl
        << std::endl
        << "You can also fork it and send pull requests to be reviewed."
        << std::endl
        << std::endl
        << "All kinds of contributions, from people of any skill level, is "
           "always"
        << std::endl
        << "appreciated." << std::endl;
}

static void Throw_Error(std::string description)
{
    Print_Error(description, "");
    std::exit(EXIT_FAILURE);
}

static void Allocate_Directory_Entries(DIR *stream, char *entries[])
{
    size_t entry_index = 0;
    for (struct dirent *entry; (entry = readdir(stream));)
    {
        if (!std::strcmp(entry->d_name, ".") ||
            !std::strcmp(entry->d_name, ".."))
        {
            continue;
        }
        size_t size = std::strlen(entry->d_name) + 1;
        char *allocation;
        try
        {
            allocation = new char[size];
        }
        catch (std::bad_alloc &)
        {
            Throw_Error("can't allocate memory.");
        }
        std::memcpy(allocation, entry->d_name, size);
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
            if (std::strcmp(entries[check_index], entries[swap_index]) < 0)
            {
                swap_index = check_index;
            }
        }
        if (swap_index == entry_index)
        {
            continue;
        }
        char *swap_entry = entries[entry_index];
        entries[entry_index] = entries[swap_index];
        entries[swap_index] = swap_entry;
    }
}

static size_t Get_Total_Of_Directory_Entries(DIR *stream)
{
    size_t total_of_entries = 0;
    for (; readdir(stream); total_of_entries++)
        ;
    return total_of_entries - 2;
}

static void Reveal_Type(struct stat &metadata)
{
    switch (metadata.st_mode & S_IFMT)
    {
    case S_IFREG:
        puts("regular");
        break;
    case S_IFDIR:
        puts("directory");
        break;
    case S_IFLNK:
        puts("symlink");
        break;
    case S_IFSOCK:
        puts("socket");
        break;
    case S_IFIFO:
        puts("fifo");
        break;
    case S_IFCHR:
        puts("character");
        break;
    case S_IFBLK:
        puts("block");
        break;
    default:
        puts("unknown");
    }
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

static void Reveal_Permissions(struct stat &metadata)
{
    char read = 'r', write = 'w', execute = 'x';
    Parse_Permission(metadata, S_IRUSR, read);
    Parse_Permission(metadata, S_IWUSR, write);
    Parse_Permission(metadata, S_IXUSR, execute);
    Parse_Permission(metadata, S_IRGRP, read);
    Parse_Permission(metadata, S_IWGRP, write);
    Parse_Permission(metadata, S_IXGRP, execute);
    Parse_Permission(metadata, S_IROTH, read);
    Parse_Permission(metadata, S_IWOTH, write);
    Parse_Permission(metadata, S_IXOTH, execute);
    std::cout << std::endl;
}

static void Reveal_Octal_Permissions(struct stat &metadata)
{
    std::cout << std::oct
              << (metadata.st_mode &
                  (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                   S_IROTH | S_IWOTH | S_IXOTH))
              << std::dec << std::endl;
}

static Return_Status Reveal_User(const char *path, struct stat &metadata)
{
    char buffer[255];
    struct passwd user, *result;
    if (getpwuid_r(metadata.st_uid, &user, buffer, sizeof(buffer), &result) ||
        !result)
    {
        return Print_Error("can't discover user that owns \"" +
                               std::string(path) + "\".",
                           "Check if that is a dangling symlink.");
    }
    std::cout << user.pw_name << std::endl;
    return Return_Status::Success;
}

static Return_Status Reveal_Group(const char *path, struct stat &metadata)
{
    char buffer[255];
    struct group group, *result;
    if (getgrgid_r(metadata.st_gid, &group, buffer, sizeof(buffer), &result) ||
        !result)
    {
        return Print_Error("can't discover group that owns \"" +
                               std::string(path) + "\".",
                           "Check if that is a dangling symlink.");
    }
    std::cout << group.gr_name << std::endl;
    return Return_Status::Success;
}

static void Reveal_Modification_Date(struct stat &metadata)
{
    char modification_date[29];
    std::strftime(modification_date, sizeof(modification_date),
                  "%a %b %d %T %Z %Y", localtime(&metadata.st_mtime));
    std::cout << modification_date << std::endl;
}

static Return_Status Reveal_File(const char *path)
{
    std::FILE *stream = std::fopen(path, "r");
    if (!stream)
    {
        return Print_Error("can't open file \"" + std::string(path) + "\".",
                           "Check if you have permission to read it.");
    }
    for (char character; (character = std::fgetc(stream)) != EOF;
         std::cout << character)
        ;
    std::fclose(stream);
    return Return_Status::Success;
}

static Return_Status Reveal_Directory(const char *path)
{
    DIR *stream = opendir(path);
    if (!stream)
    {
        return Print_Error("can't open directory \"" + std::string(path) +
                               "\".",
                           "Check if you have permission to read it.");
    }
    size_t total_of_entries = Get_Total_Of_Directory_Entries(stream);
    if (!total_of_entries)
    {
        closedir(stream);
        return Return_Status::Success;
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
    return Return_Status::Success;
}

static Return_Status Reveal_Contents(const char *path, struct stat &metadata)
{
    switch (metadata.st_mode & S_IFMT)
    {
    case S_IFREG:
        return Reveal_File(path);
    case S_IFDIR:
        return Reveal_Directory(path);
    case S_IFLNK:
        return Print_Error("can't reveal contents of symlink \"" +
                               std::string(path) + "\".",
                           "Experiment to use the \"--follow-symlinks\" option "
                           "right before it.");
    default:
        return Print_Error("can't reveal contents of \"" + std::string(path) +
                               "\" due to its unreadable nature.",
                           "");
    }
}

static Return_Status Reveal(const char *path)
{
    struct stat metadata;
    if (IS_FOLLOWING_SYMLINKS ? stat(path, &metadata) : lstat(path, &metadata))
    {
        return Print_Error("can't find entry \"" + std::string(path) + "\".",
                           "Check if you misspelled it.");
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
    case Data_Type::Modification_Date:
        Reveal_Modification_Date(metadata);
        break;
    default:
        return Reveal_Contents(path, metadata);
    }
    return Return_Status::Success;
}

static Parse_Status Parse_Data_Type_Options(const char *path,
                                            const char *argument,
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
    PARSE_DATA_TYPE_OPTION("modification-date", Data_Type::Modification_Date);
    return Parse_Status::Not_Parsed;
}

static Parse_Status Parse_Symlink_Options(const char *path,
                                          const char *argument,
                                          bool is_last_argument)
{
    PARSE_SYMLINK_OPTION("follow-symlinks", true);
    PARSE_SYMLINK_OPTION("unfollow-symlinks", false);
    return Parse_Status::Not_Parsed;
}

static void Parse_Metadata_Options(int total_of_arguments,
                                   const char *arguments[])
{
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++)
    {
        PARSE_METADATA_OPTION("help", Print_Help());
        PARSE_METADATA_OPTION("version",
                              std::cout << PROGRAM_VERSION << std::endl);
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
        if (Parse_Data_Type_Options(path, argument, is_last_argument) ==
                Parse_Status::Parsed ||
            Parse_Symlink_Options(path, argument, is_last_argument) ==
                Parse_Status::Parsed ||
            Reveal(argument) == Return_Status::Failure)
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
        return Parse_Exit_Code(Reveal("."));
    }
    Parse_Metadata_Options(total_of_arguments, arguments);
    Parse_Non_Metadata_Options(total_of_arguments, arguments);
    return Parse_Exit_Code(HAD_ERROR);
}
