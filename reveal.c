#include <dirent.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define SOFTWARE_NAME "reveal"

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
		printf(
			"%5hu | ",
			quantity_of_directory_entries + 1
		);
		switch (directory_entry->d_type)
		{
			case 4:
				printf("Directory");
				break;
			case 8:
				printf("File     ");
				break;
			case 10:
				printf("Symlink  ");
				break;
			default:
				printf(
					"%d",
					directory_entry->d_type
				);
				break;
		}
		printf(
			"   %s\n",
			directory_entry->d_name
		);
		++ quantity_of_directory_entries;
	}
	closedir(directory_stream);
	printf(
		"Quantity of directory entries: %hu.\n",
		quantity_of_directory_entries
	);
	return;
}

int main(
	int quantity_of_arguments,
	char *arguments[]
)
{
	char *relative_path = ".";
	char absolute_path[PATH_MAX + 1];
	if (quantity_of_arguments >= 2)
	{
		relative_path = arguments[1];
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

