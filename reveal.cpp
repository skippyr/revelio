#include <dirent.h>
#include <iostream>
#include <map>
#include <string.h>
#include <sys/stat.h>

enum ProcessingOption
{
    Unknown,
    Read,
    OwnerUid,
};

void print_error(std::string description)
{
    std::cerr << "reveal: " << description << std::endl;
}

void reveal_owner_uid(struct stat &stats)
{
    std::cout << "Revealing owner uid" << std::endl;
    std::cout << stats.st_uid << std::endl;
}

void reveal_file(const char *path)
{
    std::cout << "Revealing default" << std::endl;
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
        const char *separator = !strcmp(path, "/") ? "" : "/";
        std::cout << path << separator << entry->d_name << std::endl;
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
    switch (option)
    {
    case ProcessingOption::OwnerUid:
        reveal_owner_uid(stats);
        break;
    case ProcessingOption::Read:
        char abs_path[PATH_MAX + 1];
        if (!realpath(path, abs_path))
        {
            print_error("could not resolve absolute path of \"" +
                        std::string(path) + "\".");
            return;
        }
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
