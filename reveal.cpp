#include <dirent.h>
#include <iostream>
#include <map>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>

enum ProcessingOption
{
    Unknown,
    Read,
    OwnerUid,
    Owner,
};

void print_error(std::string description)
{
    std::cerr << "reveal: " << description << std::endl;
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

void reveal(const char *path, ProcessingOption &option)
{
    struct stat stats;
    if (stat(path, &stats) != 0)
    {
        print_error("the path \"" + std::string(path) + "\" does not exists.");
        return;
    }
    char abs_path[PATH_MAX + 1];
    if (!realpath(path, abs_path))
    {
        print_error("could not resolve absolute path of \"" +
                    std::string(path) + "\".");
        return;
    }
    switch (option)
    {
    case ProcessingOption::OwnerUid:
        reveal_owner_uid(stats);
        break;
    case ProcessingOption::Owner:
        reveal_owner(abs_path, stats);
        break;
    case ProcessingOption::Read:
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
    ProcessingOption option = Read;
    std::map<std::string, ProcessingOption> flagOptions;
    flagOptions["--owner-uid"] = ProcessingOption::OwnerUid;
    flagOptions["--read"] = ProcessingOption::Read;
    flagOptions["--owner"] = ProcessingOption::Owner;

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        ProcessingOption &flagOption = flagOptions[arg];
        if (flagOption != ProcessingOption::Unknown)
        {
            option = flagOption;
            continue;
        }
        reveal(arg, option);
    }
}
