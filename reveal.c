#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ONE_GIGABYTE_IN_BYTES 1e+9
#define ONE_MEGABYTE_IN_BYTES 1e+6
#define ONE_KILOBYTE_IN_BYTES 1e+3

void print_usage_instructions(void)
{
	printf("Usage Instructions\n");
	printf("\tStarting Point\n");
	printf("\t\tReveal reveals directory entries and file contents.\n");
	printf("\tUsage\n");
	printf("\t\tUse it in the following syntax:\n");
	printf("\t\t\treveal [flags] <path>\n");
	printf("\t\tThe flags it can accept are:\n");
	printf("\t\t\t-h or --help: show these usage instructions.\n");
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

double convert_bytes_to_gigabytes(long bytes)
{
	return (bytes / ONE_GIGABYTE_IN_BYTES);
}

double convert_bytes_to_megabytes(long bytes)
{
	return (bytes / ONE_MEGABYTE_IN_BYTES);
}

double convert_bytes_to_kilobytes(long bytes)
{
	return (bytes / ONE_KILOBYTE_IN_BYTES);
}

void print_size(
	struct dirent *path_entry,
	struct stat *path_status
)
{
	if (
		!S_ISREG(path_status->st_mode) ||
		path_entry->d_type == DT_LNK
	)
	{
		printf("      -");
		return;
	}
	double
		size_in_gigabytes = convert_bytes_to_gigabytes(path_status->st_size),
		size_in_megabytes = convert_bytes_to_megabytes(path_status->st_size),
		size_in_kilobytes = convert_bytes_to_kilobytes(path_status->st_size);
	if ((int) size_in_gigabytes > 0)
	{
		printf(
			"%5.1lfGB",
			size_in_gigabytes
		);
	}
	else if ((int) size_in_megabytes > 0)
	{
		printf(
			"%5.1lfMB",
			size_in_megabytes
		);
	}
	else if ((int) size_in_kilobytes > 0)
	{
		printf(
			"%5.1lfKB",
			size_in_kilobytes
		);
	}
	else
	{
		printf(
			" %5ldB",
			path_status->st_size
		);
	}
	return;
}

void print_permissions(struct stat *path_status)
{
	printf(
		"%s%s%s",
		path_status->st_mode & S_IRUSR ?
		"r" :
		"-",
		path_status->st_mode & S_IWUSR ?
		"w" :
		"-",
		path_status->st_mode & S_IXUSR ?
		"x" :
		"-"
	);
	unsigned short int chmod_value = 0;
	if (path_status->st_mode & S_IRUSR)
	{
		chmod_value += 400;
	}
	if (path_status->st_mode & S_IRGRP)
	{
		chmod_value += 40;
	}
	if (path_status->st_mode & S_IROTH)
	{
		chmod_value += 4;
	}
	if (path_status->st_mode & S_IWUSR)
	{
		chmod_value += 200;
	}
	if (path_status->st_mode & S_IWGRP)
	{
		chmod_value += 20;
	}
	if (path_status->st_mode & S_IWOTH)
	{
		chmod_value += 2;
	}
	if (path_status->st_mode & S_IXUSR)
	{
		chmod_value += 100;
	}
	if (path_status->st_mode & S_IXGRP)
	{
		chmod_value += 10;
	}
	if (path_status->st_mode & S_IXOTH)
	{
		chmod_value += 1;
	}
	printf(
		" (%hu)",
		chmod_value
	);
	return;
}

void reveal_directory(char directory_path[])
{
	DIR *directory_stream = opendir(directory_path);
	if (directory_stream == NULL)
	{
		print_error("The directory can not be read.\n");
		exit(1);
	}
	printf(
		"Revealing directory: %s.\n",
		directory_path
	);
	struct dirent *directory_entry;
	unsigned short int quantity_of_directory_entries = 0;
	printf("Index | Type            Size   Permissions   Owner             Name\n");
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
		if (stat(
			directory_entry_path,
			&directory_entry_status
		) != 0)
		{
			continue;
		}
		struct passwd *user_entry = getpwuid(directory_entry_status.st_uid);
		if (user_entry == NULL)
		{
			print_error("Could not identify user entry.\n");
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
		printf("   ");
		print_size(
			directory_entry,
			&directory_entry_status
		);
		printf("   ");
		print_permissions(&directory_entry_status);
		printf(
			"     %-15s   %s\n",
			user_entry->pw_name,
			directory_entry->d_name
		);
		++ quantity_of_directory_entries;
	}
	closedir(directory_stream);
	return;
}

void reveal_file(char file_path[])
{
	FILE *file_stream = fopen(
		file_path,
		"r"
	);
	if (file_stream == NULL)
	{
		print_error("The file can not be read.\n");
		exit(1);
	}
	printf(
		"Revealing file: %s.\n",
		file_path
	);
	char character;
	unsigned short int quantity_of_lines = 0;
	unsigned short int last_character_was_a_line_break = 0;
	while ((character = fgetc(file_stream)) != EOF)
	{
		if (
			quantity_of_lines == 0 ||
			last_character_was_a_line_break
		)
		{
			++ quantity_of_lines;
			printf(
				"%5hu | ",
				quantity_of_lines
			);
		}
		printf(
			"%c",
			character
		);
		last_character_was_a_line_break = character == '\n';
	}
	printf("\n");
	fclose(file_stream);
	return;
}

int main(
	int quantity_of_arguments,
	char *arguments[]
)
{
	char *relative_path = ".";
	char absolute_path[PATH_MAX];
	for (
		unsigned short int iterator = 0;
		iterator != quantity_of_arguments;
		++ iterator
	)
	{
		if (
			!strcmp(
				arguments[iterator],
				"-h"
			) ||
			!strcmp(
				arguments[iterator],
				"--help"
			)
		)
		{
			print_usage_instructions();
			return (0);
		}
		else if (iterator > 0)
		{
			relative_path = arguments[1];
		}
	}
	if (realpath(
		relative_path,
		absolute_path
	) == NULL)
	{
		print_error("Could not define absolute path of given path.\n");
		return (1);
	}
	struct stat absolute_path_status;
	if (stat(
		absolute_path,
		&absolute_path_status
	) != 0)
	{
		print_error("Could not stat given path.\n");
		return (1);
	}
	if (S_ISDIR(absolute_path_status.st_mode))
	{
		reveal_directory(absolute_path);
	}
	else if (S_ISREG(absolute_path_status.st_mode))
	{
		reveal_file(absolute_path);
	}
	else
	{
		print_error("Can not reveal given path type.\n");
		return (1);
	}
	return (0);
}

