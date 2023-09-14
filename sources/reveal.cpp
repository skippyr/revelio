#include <cstring>
#include <ctime>
#include <dirent.h>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <sys/stat.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_VERSION "v15.0.0"
#define PARSE_SIZE_PREFIX_MULTIPLIER(multiplier, prefix)                       \
    size = metadata.st_size / (multiplier);                                    \
    if ((int)size)                                                             \
    {                                                                          \
        printFloatSize(size, prefix);                                          \
        return;                                                                \
    }
#define PARSE_OPTION(option, argument, action)                                 \
    if (!std::strcmp("--" option, argument))                                   \
    {                                                                          \
        action;                                                                \
    }
#define PARSE_METADATA_OPTION(option, action)                                  \
    PARSE_OPTION(option, arguments[argumentIndex], action; exit(EXIT_SUCCESS);)
#define PARSE_DATA_TYPE_OPTION(option, dataType)                               \
    PARSE_OPTION(                                                              \
        option, argument,                                                      \
        if (IS_EXPECTING_PATH_ARGUMENT) { reveal(path); } DATA_TYPE =          \
            dataType;                                                          \
        IS_EXPECTING_PATH_ARGUMENT = true;                                     \
        if (isLastArgument) { reveal(path); } return ParseStatus::Parsed;)
#define PARSE_SYMLINK_OPTION(option, isFollowingSymlinks)                      \
    PARSE_OPTION(                                                              \
        option, argument,                                                      \
        if (isLastArgument) { reveal(path); } IS_FOLLOWING_SYMLINKS =          \
            isFollowingSymlinks;                                               \
        return ParseStatus::Parsed;)

enum DataType
{
    Contents,
    Type,
    Size,
    ByteSize,
    Permissions,
    OctalPermissions,
    User,
    UserId,
    Group,
    GroupId,
    ModificationDate
};

enum ParseStatus
{
    Parsed,
    NotParsed
};

enum ReturnStatus
{
    Success,
    Failure
};

static DataType DATA_TYPE = DataType::Contents;
static bool HAD_ERROR = false, IS_EXPECTING_PATH_ARGUMENT = false,
            IS_FOLLOWING_SYMLINKS = true;

static int parseExitCode(int exitCode)
{
    return exitCode ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void parsePermission(struct stat &metadata, int permission,
                            char character)
{
    std::cout << (metadata.st_mode & permission ? character : '-');
}

static ReturnStatus printError(std::string description,
                               std::string fixSuggestion)
{
    std::cerr << PROGRAM_NAME << ": " << description << std::endl;
    if (fixSuggestion != "")
    {
        std::cout << fixSuggestion << std::endl;
    }
    HAD_ERROR = 1;
    return ReturnStatus::Failure;
}

static void printFloatSize(float value, std::string prefix)
{
    std::cout << std::fixed;
    std::cout.precision(1);
    std::cout << value << prefix << "B" << std::endl;
}

static void printByteSize(size_t size, bool isToUseUnit)
{
    std::cout << size << (isToUseUnit ? "B" : "") << std::endl;
}

static void printHelp()
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

static void throwError(std::string description)
{
    printError(description, "");
    std::exit(EXIT_FAILURE);
}

static void allocateDirectoryEntries(DIR *stream, char *entries[])
{
    size_t entryIndex = 0;
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
            throwError("can't allocate memory.");
        }
        std::memcpy(allocation, entry->d_name, size);
        entries[entryIndex] = allocation;
        entryIndex++;
    }
}

static void sortDirectoryEntries(char *entries[], size_t totalOfEntries)
{
    for (size_t entryIndex = 0; entryIndex < totalOfEntries - 1; entryIndex++)
    {
        size_t swapIndex = entryIndex;
        for (size_t checkIndex = entryIndex + 1; checkIndex < totalOfEntries;
             checkIndex++)
        {
            if (std::strcmp(entries[checkIndex], entries[swapIndex]) < 0)
            {
                swapIndex = checkIndex;
            }
        }
        if (swapIndex == entryIndex)
        {
            continue;
        }
        char *swapEntry = entries[entryIndex];
        entries[entryIndex] = entries[swapIndex];
        entries[swapIndex] = swapEntry;
    }
}

static size_t getTotalOfDirectoryEntries(DIR *stream)
{
    size_t totalOfEntries = 0;
    for (; readdir(stream); totalOfEntries++)
        ;
    return totalOfEntries - 2;
}

static void revealType(struct stat &metadata)
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

static void revealSize(struct stat &metadata)
{
    float size;
    PARSE_SIZE_PREFIX_MULTIPLIER(1e9, "G");
    PARSE_SIZE_PREFIX_MULTIPLIER(1e6, "M");
    PARSE_SIZE_PREFIX_MULTIPLIER(1e3, "k");
    printByteSize(metadata.st_size, true);
}

static void revealByteSize(struct stat &metadata)
{
    printByteSize(metadata.st_size, false);
}

static void revealPermissions(struct stat &metadata)
{
    char read = 'r', write = 'w', execute = 'x';
    parsePermission(metadata, S_IRUSR, read);
    parsePermission(metadata, S_IWUSR, write);
    parsePermission(metadata, S_IXUSR, execute);
    parsePermission(metadata, S_IRGRP, read);
    parsePermission(metadata, S_IWGRP, write);
    parsePermission(metadata, S_IXGRP, execute);
    parsePermission(metadata, S_IROTH, read);
    parsePermission(metadata, S_IWOTH, write);
    parsePermission(metadata, S_IXOTH, execute);
    std::cout << std::endl;
}

static void revealOctalPermissions(struct stat &metadata)
{
    std::cout << std::oct
              << (metadata.st_mode &
                  (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                   S_IROTH | S_IWOTH | S_IXOTH))
              << std::dec << std::endl;
}

static ReturnStatus revealUser(const char *path, struct stat &metadata)
{
    char buffer[255];
    struct passwd user, *result;
    if (getpwuid_r(metadata.st_uid, &user, buffer, sizeof(buffer), &result) ||
        !result)
    {
        return printError("can't discover user that owns \"" +
                              std::string(path) + "\".",
                          "Check if that is a dangling symlink.");
    }
    std::cout << user.pw_name << std::endl;
    return ReturnStatus::Success;
}

static ReturnStatus revealGroup(const char *path, struct stat &metadata)
{
    char buffer[255];
    struct group group, *result;
    if (getgrgid_r(metadata.st_gid, &group, buffer, sizeof(buffer), &result) ||
        !result)
    {
        return printError("can't discover group that owns \"" +
                              std::string(path) + "\".",
                          "Check if that is a dangling symlink.");
    }
    std::cout << group.gr_name << std::endl;
    return ReturnStatus::Success;
}

static void revealModificationDate(struct stat &metadata)
{
    char modificationDate[29];
    std::strftime(modificationDate, sizeof(modificationDate),
                  "%a %b %d %T %Z %Y", std::localtime(&metadata.st_mtime));
    std::cout << modificationDate << std::endl;
}

static ReturnStatus revealFile(const char *path)
{
    std::FILE *stream = std::fopen(path, "r");
    if (!stream)
    {
        return printError("can't open file \"" + std::string(path) + "\".",
                          "Check if you have permission to read it.");
    }
    for (char character; (character = std::fgetc(stream)) != EOF;
         std::cout << character)
        ;
    std::fclose(stream);
    return ReturnStatus::Success;
}

static ReturnStatus revealDirectory(const char *path)
{
    DIR *stream = opendir(path);
    if (!stream)
    {
        return printError("can't open directory \"" + std::string(path) + "\".",
                          "Check if you have permission to read it.");
    }
    size_t totalOfEntries = getTotalOfDirectoryEntries(stream);
    if (!totalOfEntries)
    {
        closedir(stream);
        return ReturnStatus::Success;
    }
    char *entries[totalOfEntries];
    rewinddir(stream);
    allocateDirectoryEntries(stream, entries);
    sortDirectoryEntries(entries, totalOfEntries);
    for (auto &entry : entries)
    {
        std::cout << entry << std::endl;
        delete[] entry;
    }
    closedir(stream);
    return ReturnStatus::Success;
}

static ReturnStatus revealContents(const char *path, struct stat &metadata)
{
    switch (metadata.st_mode & S_IFMT)
    {
    case S_IFREG:
        return revealFile(path);
    case S_IFDIR:
        return revealDirectory(path);
    case S_IFLNK:
        return printError("can't reveal contents of symlink \"" +
                              std::string(path) + "\".",
                          "Experiment to use the \"--follow-symlinks\" option "
                          "right before it.");
    default:
        return printError("can't reveal contents of \"" + std::string(path) +
                              "\" due to its unreadable nature.",
                          "");
    }
}

static ReturnStatus reveal(const char *path)
{
    struct stat metadata;
    if (IS_FOLLOWING_SYMLINKS ? stat(path, &metadata) : lstat(path, &metadata))
    {
        return printError("can't find entry \"" + std::string(path) + "\".",
                          "Check if you misspelled it.");
    }
    switch (DATA_TYPE)
    {
    case DataType::Type:
        revealType(metadata);
        break;
    case DataType::Size:
        revealSize(metadata);
        break;
    case DataType::ByteSize:
        revealByteSize(metadata);
        break;
    case DataType::Permissions:
        revealPermissions(metadata);
        break;
    case DataType::OctalPermissions:
        revealOctalPermissions(metadata);
        break;
    case DataType::User:
        return revealUser(path, metadata);
    case DataType::UserId:
        std::cout << metadata.st_uid << std::endl;
        break;
    case DataType::Group:
        return revealGroup(path, metadata);
    case DataType::GroupId:
        std::cout << metadata.st_gid << std::endl;
        break;
    case DataType::ModificationDate:
        revealModificationDate(metadata);
        break;
    default:
        return revealContents(path, metadata);
    }
    return ReturnStatus::Success;
}

static ParseStatus parseDataTypeOptions(const char *path, const char *argument,
                                        bool isLastArgument)
{
    PARSE_DATA_TYPE_OPTION("contents", DataType::Contents);
    PARSE_DATA_TYPE_OPTION("type", DataType::Type);
    PARSE_DATA_TYPE_OPTION("size", DataType::Size);
    PARSE_DATA_TYPE_OPTION("byte-size", DataType::ByteSize);
    PARSE_DATA_TYPE_OPTION("permissions", DataType::Permissions);
    PARSE_DATA_TYPE_OPTION("octal-permissions", DataType::OctalPermissions);
    PARSE_DATA_TYPE_OPTION("user", DataType::User);
    PARSE_DATA_TYPE_OPTION("user-id", DataType::UserId);
    PARSE_DATA_TYPE_OPTION("group", DataType::Group);
    PARSE_DATA_TYPE_OPTION("group-id", DataType::GroupId);
    PARSE_DATA_TYPE_OPTION("modification-date", DataType::ModificationDate);
    return ParseStatus::NotParsed;
}

static ParseStatus parseSymlinkOptions(const char *path, const char *argument,
                                       bool isLastArgument)
{
    PARSE_SYMLINK_OPTION("follow-symlinks", true);
    PARSE_SYMLINK_OPTION("unfollow-symlinks", false);
    return ParseStatus::NotParsed;
}

static void parseMetadataOptions(int totalOfArguments, const char *arguments[])
{
    for (int argumentIndex = 1; argumentIndex < totalOfArguments;
         argumentIndex++)
    {
        PARSE_METADATA_OPTION("help", printHelp());
        PARSE_METADATA_OPTION("version",
                              std::cout << PROGRAM_VERSION << std::endl);
    }
}

static void parseNonMetadataOptions(int totalOfArguments,
                                    const char *arguments[])
{
    const char *path = ".";
    for (int argumentIndex = 1; argumentIndex < totalOfArguments;
         argumentIndex++)
    {
        const char *argument = arguments[argumentIndex];
        bool isLastArgument = argumentIndex == totalOfArguments - 1;
        if (parseDataTypeOptions(path, argument, isLastArgument) ==
                ParseStatus::Parsed ||
            parseSymlinkOptions(path, argument, isLastArgument) ==
                ParseStatus::Parsed ||
            reveal(argument) == ReturnStatus::Failure)
        {
            continue;
        }
        IS_EXPECTING_PATH_ARGUMENT = false;
        path = argument;
    }
}

int main(int totalOfArguments, const char *arguments[])
{
    if (totalOfArguments == 1)
    {
        return parseExitCode(reveal("."));
    }
    parseMetadataOptions(totalOfArguments, arguments);
    parseNonMetadataOptions(totalOfArguments, arguments);
    return parseExitCode(HAD_ERROR);
}
