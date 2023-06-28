#include <dirent.h>
#include <grp.h>
#include <iostream>
#include <map>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_VERSION "3.0.0"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."

enum Mode
{
    Unknown,
    OwnerUid,
    Owner,
    GroupUid,
    Group,
    Size,
    Permissions,
    ModifiedDate,
    Contents,
};

void print_help()
{
    std::cout
        << "Usage: " << PROGRAM_NAME << " [FLAGS]... [PATHS]" << std::endl
        << "Reveals information about entries in the file system." << std::endl
        << std::endl
        << "META FLAGS" << std::endl
        << "  --help     prints these help instructions." << std::endl
        << "  --version  prints the version of the program." << std::endl
        << "  --license  prints the license of the program." << std::endl
        << std::endl
        << "MODE FLAGS" << std::endl
        << "These flags changes the mode the program will use when "
           "revealing an entry. All"
        << std::endl
        << "arguments placed after that flag will be affected by it."
        << std::endl
        << std::endl
        << "  --contents (default)  reveals the contents of the entry."
        << std::endl
        << "  --owner               reveals the user that owns the entry."
        << std::endl
        << "  --owner-uid           reveals the UID of the user that owns the "
           "entry."
        << std::endl
        << "  --group               reveals the group that owns the entry."
        << std::endl
        << "  --group-uid           reveals the UID of the group that owns the "
           "entry."
        << std::endl
        << "  --permissions         reveals the permissions octal of the entry."
        << std::endl
        << "  --modified-date       reveals the date of the last modification "
           "of the entry."
        << std::endl
        << std::endl
        << "ISSUES AND CONTRIBUTIONS" << std::endl
        << "Report issues found in the program at:" << std::endl
        << "  https://github.com/skippyr/reveal/issues" << std::endl
        << std::endl
        << "Learn how to contribute to this software by visiting its "
           "source code page at:"
        << std::endl
        << "  https://github.com/skippyr/reveal" << std::endl;
    exit(0);
}

void print_version()
{
    std::cout << PROGRAM_VERSION << std::endl;
    exit(0);
}

void print_license()
{
    std::cout << PROGRAM_LICENSE << std::endl;
    exit(0);
}

void print_error(std::string description)
{
    std::cerr << PROGRAM_NAME << ": " << description << std::endl;
}

void reveal_owner_uid(struct stat &stats)
{
    std::cout << stats.st_uid << std::endl;
}

void reveal_owner(const char *path, struct stat &stats)
{
    struct passwd *pw = getpwuid(stats.st_uid);
    if (!pw)
    {
        print_error("could not get owner of \"" + std::string(path) + "\".");
        return;
    }
    std::cout << pw->pw_name << std::endl;
}

void reveal_group_uid(struct stat &stats)
{
    std::cout << stats.st_gid << std::endl;
}

void reveal_group(const char *path, struct stat &stats)
{
    struct group *g = getgrgid(stats.st_gid);
    if (!g)
    {
        print_error("could not get group of \"" + std::string(path) + "\".");
        return;
    }
    std::cout << g->gr_name << std::endl;
}

void reveal_permissions(struct stat &stats)
{
    unsigned int permissions =
        stats.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | // User
                         S_IRGRP | S_IWGRP | S_IXGRP | // Group
                         S_IROTH | S_IWOTH | S_IXOTH   // Others
                        );
    std::cout << std::oct << permissions << std::endl;
}

void reveal_size(struct stat &stats)
{
    std::cout << stats.st_size << std::endl;
}

void reveal_modified_date(struct stat &stats)
{
    char d[29];
    if (strftime(d, sizeof(d), "%a %b %d %T %Z %Y",
                 localtime(&stats.st_mtime)) == 0)
    {
        print_error("overflowed buffer to store date.");
        return;
    }
    std::cout << d << std::endl;
}

void reveal_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        print_error("could not open file \"" + std::string(path) + "\".");
        return;
    }
    char c;
    while ((c = fgetc(file)) != EOF)
    {
        std::cout << c;
    }
    fclose(file);
}

void reveal_directory(const char *path)
{
    DIR *directory = opendir(path);
    if (!directory)
    {
        print_error("could not open directory \"" + std::string(path) + "\".");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        const char *s = !strcmp(path, "/") ? "" : "/";
        std::cout << path << s << entry->d_name << std::endl;
    }
    closedir(directory);
}

void reveal(const char *path, Mode &mode)
{
    struct stat stats;
    if (stat(path, &stats) != 0)
    {
        print_error("the path \"" + std::string(path) + "\" does not exists.");
        return;
    }
    char abs_path[PATH_MAX];
    if (!realpath(path, abs_path))
    {
        print_error("could not resolve absolute path of \"" +
                    std::string(path) + "\".");
        return;
    }
    switch (mode)
    {
    case Mode::OwnerUid:
        reveal_owner_uid(stats);
        break;
    case Mode::Owner:
        reveal_owner(abs_path, stats);
        break;
    case Mode::GroupUid:
        reveal_group_uid(stats);
        break;
    case Mode::Group:
        reveal_group(path, stats);
        break;
    case Mode::Size:
        reveal_size(stats);
        break;
    case Mode::Permissions:
        reveal_permissions(stats);
        break;
    case Mode::ModifiedDate:
        reveal_modified_date(stats);
        break;
    case Mode::Contents:
        if S_ISREG (stats.st_mode)
        {
            reveal_file(abs_path);
        }
        else if S_ISDIR (stats.st_mode)
        {
            reveal_directory(abs_path);
        }
        else
        {
            print_error("the path \"" + std::string(path) +
                        "\" contains a type that can not be read.");
            return;
        }
    default:;
    }
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        if (!strcmp(arg, "--help"))
        {
            print_help();
        }
        else if (!strcmp(arg, "--version"))
        {
            print_version();
        }
        else if (!strcmp(arg, "--license"))
        {
            print_license();
        }
    }

    Mode mode = Mode::Contents;
    std::map<std::string, Mode> flagModes;
    flagModes["--owner-uid"] = Mode::OwnerUid;
    flagModes["--owner"] = Mode::Owner;
    flagModes["--group-uid"] = Mode::GroupUid;
    flagModes["--group"] = Mode::Group;
    flagModes["--contents"] = Mode::Contents;
    flagModes["--size"] = Mode::Size;
    flagModes["--permissions"] = Mode::Permissions;
    flagModes["--modified-date"] = Mode::ModifiedDate;

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        Mode &flagMode = flagModes[arg];
        if (flagMode != Mode::Unknown)
        {
            mode = flagMode;
            continue;
        }
        reveal(arg, mode);
    }
}
