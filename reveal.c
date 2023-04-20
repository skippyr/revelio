#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void print_usage_instructions()
{
	printf("Usage Instructions\n");
	printf("\tSummary\n");
	printf("\t\tReveals directory entries.\n");
	printf("\tUsage\n");
	printf("\t\tUse it in the following syntax:\n");
	printf("\t\t\treveal <path>\n");
	printf("\t\tIf no path is provided, it will consider your current directory.\n");
	return;
}

void print_error(const char *message)
{
	fprintf(
		stderr,
		"Opsie! %s",
		message
	);
	return;
}

void reveal_directory(char directory_path[])
{
	printf(
		"Revealing directory: %s.\n",
		directory_path
	);
	DIR *directory_stream = opendir(directory_path);
	if (directory_stream == NULL)
	{
		print_error("the directory can not be read.\n");
		exit(1);
	}
	struct dirent *directory_entry;
	unsigned short int quantity_of_directory_entries = 0;
	while((directory_entry = readdir(directory_stream)) != NULL)
	{
		if (
			!strcmp(
				directory_entry->d_name,
				"."
			) ||
			!strcmp(
				directory_entry->d_name,
				".."
			)
		)
		{
			continue;
		}
		char directory_entry_path[PATH_MAX];
		snprintf(
			directory_entry_path,
			sizeof(directory_entry_path),
			"%s%s%s",
			directory_path,
			!strcmp(
				directory_path,
				"/"
			) ?
			"" :
			"/",
			directory_entry->d_name
		);
		struct stat directory_entry_status;
		stat(
			directory_entry_path,
			&directory_entry_status
		);
		struct passwd *user_entry = getpwuid(directory_entry_status.st_uid);
		if (user_entry == NULL)
		{
			print_error("could not identify user entry.\n");
			exit(1);
		}
		printf(
			"%5hu | ",
			quantity_of_directory_entries + 1
		);
		switch (directory_entry->d_type)
		{
			case DT_DIR:
				printf("Directory ");
				break;
			case DT_REG:
				printf("File      ");
				break;
			case DT_LNK:
				printf("Symlink   ");
				break;
			case DT_FIFO:
				printf("Fifo      ");
				break;
			case DT_SOCK:
				printf("Socket    ");
				break;
			case DT_CHR:
				printf("Character ");
				break;
			case DT_BLK:
				printf("Block     ");
				break;
			case DT_UNKNOWN:
				printf("Unknown   ");
				break;
			default:
				printf(
					"%d",
					directory_entry->d_type
				);
				break;
		}
		printf(
			"   %-15s   %s\n",
			user_entry->pw_name,
			directory_entry->d_name
		);
		++ quantity_of_directory_entries;
	}
	closedir(directory_stream);
	return;
}

int main(
	int quantity_of_arguments,
	char *arguments[]
)
{
	char *relative_path = ".";
	char absolute_path[PATH_MAX];
	if (quantity_of_arguments >= 2)
	{
		if (
			!strcmp(
				arguments[1],
				"-h"
			) ||
			!strcmp(
				arguments[1],
				"--help"
			)
		)
		{
			print_usage_instructions();
			return (0);
		}
		else
		{
			relative_path = arguments[1];
		}
	}
	realpath(
		relative_path,
		absolute_path
	);
	struct stat absolute_path_status;
	stat(
		absolute_path,
		&absolute_path_status
	);
	if (S_ISDIR(absolute_path_status.st_mode))
	{
		reveal_directory(absolute_path);
	}
	else
	{
		print_error("can not reveal given path type.\n");
		return (1);
	}
	return (0);
}

