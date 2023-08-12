#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.6"
#define program_help__\
	"Usage: " program_name__ " [OPTION]... [PATH]...\n"\
	"Reveals information about entries in the file system.\n\n"\
	"METADATA OPTIONS\n"\
	"These options retrive information about the program.\n\n"\
	"    --help       print these help instructions.\n"\
	"    --version    print its version.\n\n"\
	"DATA TYPE OPTIONS\n"\
	"These options change the data type retrived from the entries following "\
	"them.\n\n"\
	"    --contents (default)    print its contents.\n"\
	"    --type                  print its type: regular, directory, symlink, "\
	"socket,\n                            fifo, character, block or unknown.\n"\
	"    --size                  print its size using a convenient unit: "\
	"gigabyte (GB),\n                            megabyte (MB), kilobyte (kB) "\
	"or byte (B).\n"\
	"    --byte-size             print its size in bytes.\n"\
	"    --permissions           print its read (r), write (w) and execute (x)"\
	"\n                            permissions for user, group and others.\n"\
	"    --octal-permissions     print its permissions using octal base.\n"\
	"    --user                  print the user that owns it.\n"\
	"    --user-uid              print the UID of the user that owns it.\n"\
	"    --group                 print the group that owns it.\n"\
	"    --group-gid             print the GID of the group that owns it.\n"\
	"    --modified-date         print the date when its contents were last "\
	"modified.\n\n"\
	"If none is used, the one marked as default will be considered.\n\n"\
	"Each one of them expects at least one path following it. If none is "\
	"given, it will\nconsider the last valid one given or, else, the current "\
	"directory.\n\n"\
	"SYMLINK FLAGS\n"\
	"These options change how the symlinks following them are handled, "\
	"changing the\norigin of the data revealed.\n\n"\
	"    --follow-symlinks (default)    symlinks will be followed.\n"\
	"    --unfollow-symlinks            symlinks will not be followed.\n\n"\
	"If none is used, the one marked as default will be considered.\n\n"\
	"EXIT CODES\n"\
	"It will throw code 1 if an error happens and 0 otherwise."
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (\
	is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__\
)
#define gigabyte_in_bytes__ 1e9
#define megabyte_in_bytes__ 1e6
#define kilobyte_in_bytes__ 1e3
#define read_permission_character__ 'r'
#define write_permission_character__ 'w'
#define execute_permission_character__ 'x'
#define lack_permission_character__ '-'
#define is_last_argument__ (argument_index == total_of_arguments - 1)
#define Parse_Null_String__(text) (text ? text : "")
#define Parse_Size_Multiplier__(multiplier, multiplier_character)\
	size = metadata->st_size / (multiplier);\
	if ((int)size) {\
		printf("%.1f%cB\n", size, multiplier_character);\
		return;\
	}
#define Parse_Case__(value, action)\
	case value:\
		action;\
		break;
#define Parse_Return_Case__(value, action)\
	case value:\
		return (action);
#define Parse_Puts_Case__(value, text) Parse_Case__(value, puts(text));
#define Parse_Permission_Bit__(permission_bit, permission_character)\
	putchar(\
		metadata->st_mode & permission_bit ? permission_character :\
		lack_permission_character__\
	);
#define Parse_Option__(option, action)\
	if (!strcmp("--" option, arguments[argument_index])) {\
		action;\
	}
#define Parse_Metadata_Option__(option, text) Parse_Option__(\
	option, puts(text); return (0);\
)
#define Parse_Data_Type_Option__(option, data_type) Parse_Option__(\
	option,\
	if (OPTIONS & is_expecting_path_bit__) {\
		Reveal(last_path);\
	}\
	OPTIONS = (\
		data_type | is_expecting_path_bit__ | (OPTIONS & non_data_type_bits__)\
	);\
	if (is_last_argument__) {\
		Reveal(last_path);\
	}\
	continue;\
)
#define Parse_Non_Data_Type_Option__(option, action) Parse_Option__(\
	option,\
	if (is_last_argument__) {\
		Reveal(last_path);\
	}\
	action;\
	continue;\
)

typedef enum {
	Data_Type__Contents,
	Data_Type__Type,
	Data_Type__Size,
	Data_Type__Byte_Size,
	Data_Type__Permissions,
	Data_Type__Octal_Permissions,
	Data_Type__User,
	Data_Type__User_Uid,
	Data_Type__Group,
	Data_Type__Group_Gid,
	Data_Type__Modified_Date
} Data_Type;
typedef const char* const String;
typedef const struct stat* const Metadata;

uint8_t OPTIONS = is_following_symlinks_bit__;

void Print_Unsigned(unsigned value) {
	printf("%u\n", value);
}

uint8_t Throw_Error(
	String description_split_0, String description_split_1,
	String description_split_2, String fix_suggestion
) {
	fprintf(
		stderr, "%s: %s%s%s\n%s%s", program_name__,
		Parse_Null_String__(description_split_0),
		Parse_Null_String__(description_split_1),
		Parse_Null_String__(description_split_2),
		Parse_Null_String__(fix_suggestion), fix_suggestion ? "\n" : ""
	);
	OPTIONS |= had_error_bit__;
	return (1);
}

void Reveal_Type(Metadata metadata) {
	switch (metadata->st_mode & S_IFMT) {
		Parse_Puts_Case__(S_IFREG, "regular");
		Parse_Puts_Case__(S_IFDIR, "directory");
		Parse_Puts_Case__(S_IFLNK, "symlink");
		Parse_Puts_Case__(S_IFSOCK, "socket");
		Parse_Puts_Case__(S_IFIFO, "fifo");
		Parse_Puts_Case__(S_IFCHR, "character");
		Parse_Puts_Case__(S_IFBLK, "block");
	default:
		puts("unknown");
	}
}

void Reveal_Size(Metadata metadata) {
	float size;
	Parse_Size_Multiplier__(gigabyte_in_bytes__, 'G');
	Parse_Size_Multiplier__(megabyte_in_bytes__, 'M');
	Parse_Size_Multiplier__(kilobyte_in_bytes__, 'k');
	printf("%ldB\n", metadata->st_size);
}

void Reveal_Permissions(Metadata metadata) {
	Parse_Permission_Bit__(S_IRUSR, read_permission_character__);
	Parse_Permission_Bit__(S_IWUSR, write_permission_character__);
	Parse_Permission_Bit__(S_IXUSR, execute_permission_character__);
	Parse_Permission_Bit__(S_IRGRP, read_permission_character__);
	Parse_Permission_Bit__(S_IWGRP, write_permission_character__);
	Parse_Permission_Bit__(S_IXGRP, execute_permission_character__);
	Parse_Permission_Bit__(S_IROTH, read_permission_character__);
	Parse_Permission_Bit__(S_IWOTH, write_permission_character__);
	Parse_Permission_Bit__(S_IXOTH, execute_permission_character__);
	putchar('\n');
}

void Reveal_Octal_Permissions(Metadata metadata) {
	printf("0%o\n", metadata->st_mode & (
		S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH |
		S_IWOTH | S_IXOTH
	));
}

uint8_t Reveal_User(Metadata metadata, String path) {
	const struct passwd* const user = getpwuid(metadata->st_uid);
	if (!user) {
		return (Throw_Error(
			"can not discover user that owns \"", path, "\".", NULL
		));
	} else {
		puts(user->pw_name);
	}
	return (0);
}

uint8_t Reveal_Group(Metadata metadata, String path) {
	const struct group* const group = getgrgid(metadata->st_gid);
	if (!group) {
		return (Throw_Error(
			"can not discover group that owns \"", path, "\".", NULL
		));
	} else {
		puts(group->gr_name);
	}
	return (0);
}

uint8_t Reveal_Modified_Date(Metadata metadata) {
	char modified_date[29];
	if (!strftime(
		modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
		localtime(&metadata->st_mtime)
	)) {
		return (Throw_Error(
			"overflowed buffer intended to store modified date.", NULL, NULL,
			NULL
		));
	}
	puts(modified_date);
	return (0);
}

uint8_t Reveal_File(String path) {
	FILE* const file = fopen(path, "r");
	if (!file) {
		return (Throw_Error(
			"can not open file \"", path, "\".", "Ensure that you have "
			"permissions to read it."
		));
	}
	char character;
	while ((character = fgetc(file)) != EOF) {
		putchar(character);
	}
	fclose(file);
	return (0);
}

uint8_t Reveal_Directory(String path) {
	DIR * const directory = opendir(path);
	if (!directory) {
		return (Throw_Error(
			"can not open directory \"", path, "\".", "Ensure that you have "
			"permissions to read it."
		));
	}
	const struct dirent* entry;
	while ((entry = readdir(directory))) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			puts(entry->d_name);
		}
	}
	closedir(directory);
	return (0);
}

uint8_t Reveal(String path) {
	struct stat metadata;
	if (
		OPTIONS & is_following_symlinks_bit__ ? stat(path, &metadata) :
		lstat(path, &metadata)
	) {
		return (Throw_Error(
			"the entry \"", path, "\" does not exists.", "Ensure that you did "
			"not misspelled it."
		));
	}
	switch (OPTIONS & ~non_data_type_bits__) {
		Parse_Case__(Data_Type__Type, Reveal_Type(&metadata));
		Parse_Case__(Data_Type__Size, Reveal_Size(&metadata));
		Parse_Case__(Data_Type__Permissions, Reveal_Permissions(&metadata));
		Parse_Case__(
			Data_Type__Octal_Permissions, Reveal_Octal_Permissions(&metadata)
		);
		Parse_Return_Case__(Data_Type__User, Reveal_User(&metadata, path));
		Parse_Case__(Data_Type__User_Uid, Print_Unsigned(metadata.st_uid));
		Parse_Return_Case__(Data_Type__Group, Reveal_Group(&metadata, path));
		Parse_Case__(Data_Type__Group_Gid, Print_Unsigned(metadata.st_gid));
		Parse_Return_Case__(
			Data_Type__Modified_Date, Reveal_Modified_Date(&metadata)
		);
	default:
		switch (metadata.st_mode & S_IFMT) {
			Parse_Return_Case__(S_IFREG, Reveal_File(path));
			Parse_Return_Case__(S_IFDIR, Reveal_Directory(path));
			Parse_Return_Case__(
				S_IFLNK, Throw_Error(
					"can not reveal the contents of symlink \"", path, "\".",
					"Did you mean to use the \"--follow-symlinks\" option "
					"before it?"
				)
			);
		default:
			return (Throw_Error(
				"can not reveal contents of \"", path, "\" due to its "
				"unreadable nature.", NULL
			));
		}
	}
	return (0);
}

int main(const int total_of_arguments, const char** arguments) {
	if (total_of_arguments == 1) {
		return (Reveal("."));
	}
	for (
		int argument_index = 1; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Metadata_Option__("version", program_version__);
		Parse_Metadata_Option__("help", program_help__);
	}
	const char* last_path = ".";
	for (
		int argument_index = 1; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Data_Type_Option__("contents", Data_Type__Contents);
		Parse_Data_Type_Option__("type", Data_Type__Type);
		Parse_Data_Type_Option__("size", Data_Type__Size);
		Parse_Data_Type_Option__("byte-size", Data_Type__Byte_Size);
		Parse_Data_Type_Option__("permissions", Data_Type__Permissions);
		Parse_Data_Type_Option__(
			"octal-permissions", Data_Type__Octal_Permissions
		);
		Parse_Data_Type_Option__("user", Data_Type__User);
		Parse_Data_Type_Option__("user-uid", Data_Type__User_Uid);
		Parse_Data_Type_Option__("group", Data_Type__Group);
		Parse_Data_Type_Option__("group-gid", Data_Type__Group_Gid);
		Parse_Data_Type_Option__("modified-date", Data_Type__Modified_Date);
		Parse_Non_Data_Type_Option__(
			"follow-symlinks", OPTIONS |= is_following_symlinks_bit__
		);
		Parse_Non_Data_Type_Option__(
			"unfollow-symlinks", OPTIONS &= ~is_following_symlinks_bit__
		);
		if (Reveal(arguments[argument_index])) {
			continue;
		}
		OPTIONS &= ~is_expecting_path_bit__;
		last_path = arguments[argument_index];
	}
	return !!(OPTIONS & had_error_bit__);
}
