#include <dirent.h>
#include <grp.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int exit_code = 0;

enum Mode
{
    Unknown,
    OwnerUid,
    Owner,
    GroupUid,
    Group,
    Size,
    HumanSize,
    Permissions,
    HumanPermissions,
    ModifiedDate,
    Inode,
    Contents,
};

void print_help()
{
    std::cout
        << "Usage: reveal [FLAGS]... [PATHS]..." << std::endl
        << "Reveals information about entries in the file system." << std::endl
        << std::endl
        << "META FLAGS" << std::endl
        << "These flags show metadata about the program." << std::endl
        << "  --help     prints these help instructions." << std::endl
        << "  --version  prints the version of the program." << std::endl
        << "  --license  prints the license of the program." << std::endl
        << std::endl
        << "MODE FLAGS" << std::endl
        << "These flags change the mode the program will use when "
           "revealing an entry. All"
        << std::endl
        << "arguments placed after that flag will be affected by it."
        << std::endl
        << std::endl
        << "  --contents (default)  reveals the contents of the entry."
        << std::endl
        << "  --owner               reveals the user that owns the entry."
        << std::endl
        << "  --owner-uid           reveals the UID of the user that owns "
           "the "
           "entry."
        << std::endl
        << "  --group               reveals the group that owns the entry."
        << std::endl
        << "  --group-uid           reveals the UID of the group that owns "
           "the "
           "entry."
        << std::endl
        << "  --permissions         reveals the permissions of the entry "
           "using "
           "octal base."
        << std::endl
        << "  --human-permissions   reveals the permissions of the entry "
           "for: "
           "user, group"
        << std::endl
        << "                        and others, respectively, using a "
           "triple "
           "of characters:"
        << std::endl
        << "                        read (r), write (w) and execute (x)."
        << std::endl
        << "  --modified-date       reveals the date when the contents of "
           "the "
           "entry were "
        << std::endl
        << "                        last modified." << std::endl
        << "  --size                reveals the size in bytes of the entry."
        << std::endl
        << "  --human-size          reveals the size of the entry using "
           "the "
           "most convenient"
        << std::endl
        << "                        unit for a human to read." << std::endl
        << "  --inode               reveals the serial number that "
           "identifies "
           "the entry."
        << std::endl
        << std::endl
        << "TRANSPASSING FLAGS" << std::endl
        << "These flags change how metadata is gattered for symlink entries."
        << std::endl
        << "  --untranspass (default)  does not follow symlinks." << std::endl
        << "  --transpass              follows symlinks." << std::endl
        << std::endl
        << "EXIT CODES" << std::endl
        << "Reveal will throw exit code 1 in the end of its execution if an "
           "error happens."
        << std::endl
        << std::endl
        << "However, while still able to continue, Reveal will not stop "
           "revealing the"
        << std::endl
        << "remaining arguments." << std::endl
        << std::endl
        << "All the errors encountered will be printed to stderr." << std::endl
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
    std::cout << "4.0.0" << std::endl;
    exit(0);
}

void print_license()
{
    std::cout << "Copyright (c) 2023, Sherman Rofeman. MIT license."
              << std::endl;
    exit(0);
}

void print_error(std::string description)
{
    std::cerr << "reveal: " << description << std::endl;
    exit_code = 1;
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
    unsigned permissions =
        stats.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |
                         S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    std::cout << std::oct << 0 << permissions << std::dec << std::endl;
}

void reveal_human_permissions(struct stat &stats)
{
    unsigned permissions[9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                               S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    for (int i = 0; i < 9; i++)
    {
        if ((permissions[i] & stats.st_mode) != 0)
        {
            if (i == 0 || i == 3 || i == 6)
            {
                std::cout << "r";
            }
            else if (i == 1 || i == 4 || i == 7)
            {
                std::cout << "w";
            }
            else
            {
                std::cout << "x";
            }
        }
        else
        {
            std::cout << "-";
        }
    }
    std::cout << std::endl;
}

void reveal_size(struct stat &stats)
{
    std::cout << stats.st_size << std::endl;
}

void print_float_size(float size, std::string unit)
{
    std::cout << std::fixed << std::setprecision(1) << size << unit
              << std::defaultfloat << std::endl;
}

void reveal_human_size(struct stat &stats)
{
    float gb = stats.st_size / 1e9;
    if ((int)gb > 0)
    {
        print_float_size(gb, "GB");
        return;
    }
    float mb = stats.st_size / 1e6;
    if ((int)mb > 0)
    {
        print_float_size(mb, "MB");
        return;
    }
    float kb = stats.st_size / 1e3;
    if ((int)kb > 0)
    {
        print_float_size(kb, "kB");
        return;
    }
    std::cout << stats.st_size << "B" << std::endl;
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

void reveal_directory(const char *directory_path, bool isTranspassing)
{
    DIR *directory = opendir(directory_path);
    if (!directory)
    {
        print_error("could not open directory \"" +
                    std::string(directory_path) + "\".");
        return;
    }
    const char *separator = !strcmp(directory_path, "/") ? "" : "/";
    struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        char entry_path[PATH_MAX];
        if (snprintf(entry_path, sizeof(entry_path), "%s%s%s", directory_path,
                     separator, entry->d_name) < 0)
        {
            print_error("could not get path of entry \"" +
                        std::string(entry->d_name) + "\".");
            continue;
        }
        if (isTranspassing)
        {
            char resolved_entry_path[PATH_MAX];
            if (!realpath(entry_path, resolved_entry_path))
            {
                std::cout << entry_path << std::endl;
            }
            else
            {
                std::cout << resolved_entry_path << std::endl;
            }
        }
        else
        {
            std::cout << entry_path << std::endl;
        }
    }
    closedir(directory);
}

void reveal_inode(struct stat &stats)
{
    std::cout << stats.st_ino << std::endl;
}

void reveal(const char *path, Mode &mode, bool isTranspassing)
{
    struct stat stats;
    if (isTranspassing ? stat(path, &stats) : lstat(path, &stats) != 0)
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
    case Mode::HumanSize:
        reveal_human_size(stats);
        break;
    case Mode::Permissions:
        reveal_permissions(stats);
        break;
    case Mode::HumanPermissions:
        reveal_human_permissions(stats);
        break;
    case Mode::ModifiedDate:
        reveal_modified_date(stats);
        break;
    case Mode::Inode:
        reveal_inode(stats);
        break;
    case Mode::Contents:
        if S_ISREG (stats.st_mode)
        {
            reveal_file(abs_path);
        }
        else if S_ISDIR (stats.st_mode)
        {
            reveal_directory(abs_path, isTranspassing);
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
        char *arg = *(argv + i);
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
    std::map<std::string, Mode> flag_modes;
    flag_modes["--owner-uid"] = Mode::OwnerUid;
    flag_modes["--owner"] = Mode::Owner;
    flag_modes["--group-uid"] = Mode::GroupUid;
    flag_modes["--group"] = Mode::Group;
    flag_modes["--contents"] = Mode::Contents;
    flag_modes["--size"] = Mode::Size;
    flag_modes["--human-size"] = Mode::HumanSize;
    flag_modes["--permissions"] = Mode::Permissions;
    flag_modes["--human-permissions"] = Mode::HumanPermissions;
    flag_modes["--inode"] = Mode::Inode;
    flag_modes["--modified-date"] = Mode::ModifiedDate;

    bool isTranspassing = false;

    for (int i = 1; i < argc; i++)
    {
        char *arg = *(argv + i);
        Mode &flag_mode = flag_modes[arg];
        if (flag_mode != Mode::Unknown)
        {
            mode = flag_mode;
        }
        else if (!strcmp(arg, "--transpass"))
        {
            isTranspassing = true;
        }
        else if (!strcmp(arg, "--untranspass"))
        {
            isTranspassing = false;
        }
        else
        {
            reveal(arg, mode, isTranspassing);
        }
    }

    return exit_code;
}
