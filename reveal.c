#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.3"
#define program_copyright__ "Copyright (c) Sherman Rofeman. MIT license."
#define program_support_url__ "https://github.com/skippyr/reveal/issues"
#define program_help__\
	"Usage: reveal [FLAGS | PATHS]\n"\
	"Reveals information about entries in the file system.\n\n"\
	"METADATA FLAGS\n"\
	"The following flags allow you to get information about the program "\
	"itself.\n\n"\
	"   --copyright   prints its copyright notice.\n"\
	"   --version     prints its version.\n"\
	"   --help        prints these help instructions.\n\n"\
	"DATA TYPE FLAGS\n"\
	"The following flags allow you to change what type of data to get from "\
	"the\nentries following them.\n\n"\
	"If none is used, the one marked as default will be considered in "\
	"use.\n\n"\
	"   --contents (default)   prints its contents.\n"\
	"   --type                 prints its type: regular, directory, "\
	"symlink,\n                          socket, fifo, block, character or "\
	"unknown.\n"\
	"   --size                 prints its size using the most formidable "\
	"unit:\n                          gigabyte (GB), megabyte (MB), "\
	"kilobyte (kB) or\n                          byte (B).\n"\
	"   --byte-size            prints its size in bytes.\n"\
	"   --permissions          prints its read (r), write (w) and execute "\
	"(x)\n                          permissions for user, group and "\
	"others. If a\n                          permission is not set "\
	"the lack (-) character\n                          will be used.\n"\
	"   --octal-permissions    prints its permissions in octal base.\n"\
	"   --user                 prints the user that owns it.\n"\
	"   --user-uid             prints the UID of the user that owns it.\n"\
	"   --group                prints the group that owns it.\n"\
	"   --group-gid            prints the GID of the group that owns it.\n"\
	"   --modified-date        prints the date when its contents where "\
	"last\n                          modified.\n\n"\
	"These flags expect, at least, one path following them. If none is "\
	"provided,\nthey will consider the last valid one given or else, the "\
	"current directory.\n\n"\
	"SYMLINKS FLAGS\n"\
	"These flags allow you to change how symlinks following them will be "\
	"handled,\naffecting the origin of the data you reveal.\n\n"\
	"If none is used, the one marked as default will be considered in "\
	"use.\n\n"\
	"   --follow-symlinks (default)   symlinks will be followed.\n"\
	"   --unfollow-symlinks           symlinks will not be followed\n\n"\
	"EXIT CODES\n"\
	"The exit code 1 will be throw if an error happens during its "\
	"execution.\n\n"\
	"Errors will be reported through the standard error stream.\n\n"\
	"SUPPORT\n"\
	"Report issues, questions or suggestion at:\n" program_support_url__ "."

#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (\
	is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__\
)

#define is_last_argument__ (argument_index == total_of_arguments - 1)

#define Print_Long__(value) printf("%ld\n", value);
#define Print_Unsigned__(value) printf("%u\n", value);
#define Print_Octal_Permissions__\
	printf("0%o\n", metadata.st_mode & (\
		S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH |\
		S_IWOTH | S_IXOTH\
	));

#define Parse_Null_String__(text) (text ? text : "")

#define Parse_Function_Case__(value, action)\
	case value:\
		action;\
		break;
#define Parse_Puts_Case__(value, text) Parse_Function_Case__(value, puts(text))
#define Parse_Return_Case__(value, action)\
	case value:\
		return (action);

#define Parse_Size__(multiplier, multiplier_character)\
	size = metadata->st_size / (multiplier);\
	if ((int) size) {\
		printf("%.1f%cB\n", size, multiplier_character);\
		return;\
	}

#define Parse_Permission__(permission, permission_character)\
	putchar(\
		metadata->st_mode & (permission) ? permission_character : lack_character\
	);

#define Parse_Flag__(flag, action)\
	if (!strcmp("--" flag, arguments[argument_index])) {\
		action;\
	}
#define Parse_Metadata_Flag__(flag, text) Parse_Flag__(flag,\
	puts(text);\
	return (0);\
)
#define Parse_Data_Type_Flag__(flag, data_type) Parse_Flag__(flag,\
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
#define Parse_Non_Data_Type_Flag__(flag, action) Parse_Flag__(flag,\
	if (is_last_argument__) {\
		Reveal(last_path);\
	}\
	action;\
	continue;\
)

typedef enum {
	Data_Type_Contents,
	Data_Type_Type,
	Data_Type_Size,
	Data_Type_Byte_Size,
	Data_Type_Permissions,
	Data_Type_Octal_Permissions,
	Data_Type_User,
	Data_Type_User_Uid,
	Data_Type_Group,
	Data_Type_Group_Gid,
	Data_Type_Modified_Date
} Data_Type;

uint8_t OPTIONS = is_following_symlinks_bit__;

uint8_t Write_Error(
	const char* const description_split_0, const char* const description_split_1,
	const char* const description_split_2, const char* const suggestion
)
{
	fprintf(
		stderr, "%s:\n   %s%s%s\n%s%s%s", program_name__,
		Parse_Null_String__(description_split_0),
		Parse_Null_String__(description_split_1),
		Parse_Null_String__(description_split_2), suggestion ? "   " : "",
		Parse_Null_String__(suggestion), suggestion ? "\n" : ""
	);
	OPTIONS |= had_error_bit__;
	return (1);
}

void Reveal_Type(const struct stat* const metadata)
{
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

void Reveal_Size(const struct stat* const metadata)
{
	float size;
	Parse_Size__(1e9, 'G');
	Parse_Size__(1e6, 'M');
	Parse_Size__(1e3, 'k');
	printf("%ldB\n", metadata->st_size);
}

void Reveal_Permissions(const struct stat* const metadata)
{
	const char
		read_character = 'r', write_character = 'w', execute_character = 'x',
		lack_character = '-';
	Parse_Permission__(S_IRUSR, read_character);
	Parse_Permission__(S_IWUSR, write_character);
	Parse_Permission__(S_IXUSR, execute_character);
	Parse_Permission__(S_IRGRP, read_character);
	Parse_Permission__(S_IWGRP, write_character);
	Parse_Permission__(S_IXGRP, execute_character);
	Parse_Permission__(S_IROTH, read_character);
	Parse_Permission__(S_IWOTH, write_character);
	Parse_Permission__(S_IXOTH, execute_character);
	putchar('\n');
}

uint8_t Reveal_User(const char* const path, const struct stat* const metadata)
{
	const struct passwd* const user = getpwuid(metadata->st_uid);
	if (!user) {
		return (Write_Error("Can not find user that owns \"", path, "\".", NULL));
	}
	puts(user->pw_name);
	return (0);
}

uint8_t Reveal_Group(const char* const path, const struct stat* const metadata)
{
	const struct group* const group = getgrgid(metadata->st_gid);
	if (!group) {
		return (Write_Error("Can not find user that owns \"", path, "\".", NULL));
	}
	puts(group->gr_name);
	return (0);
}

uint8_t Reveal_Modified_Date(const struct stat* const metadata)
{
	char modified_date[29];
	if (!strftime(
		modified_date, sizeof(modified_date), "%a %b %d %T %Z %Y",
		localtime(&metadata->st_mtime)
	)) {
		return (Write_Error(
			"Overflowed buffer meant to store modified date.", NULL, NULL, NULL
		));
	}
	puts(modified_date);
	return (0);
}

uint8_t Reveal_File(const char* const path)
{
	FILE* const file = fopen(path, "r");
	if (!file) {
		return (Write_Error(
			"Can not open file \"", path, "\".",
			"Ensure that you have enough permissions to read it."
		));
	}
	char character;
	while ((character = fgetc(file)) != EOF) {
		putchar(character);
	}
	fclose(file);
	return (0);
}

uint8_t Reveal_Directory(const char* const path)
{
	DIR* const directory = opendir(path);
	if (!directory) {
		return (Write_Error(
			"Can not open directory \"", path, "\".",
			"Ensure that you have enough permissions to read it."
		));
	}
	const struct dirent* entry;
	while ((entry = readdir(directory))) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
			continue;
		}
		puts(entry->d_name);
	}
	closedir(directory);
	return (0);
}

uint8_t Reveal(const char* const path)
{
	struct stat metadata;
	if (
		OPTIONS & is_following_symlinks_bit__ ? stat(path, &metadata) :
		lstat(path, &metadata)
	) {
		return (Write_Error(
			"\"", path, "\" does not exists.",
			"Ensure that you did not misspelled it."
		));
	}
	switch (OPTIONS & ~non_data_type_bits__) {
		Parse_Function_Case__(Data_Type_Type, Reveal_Type(&metadata));
		Parse_Function_Case__(Data_Type_Size, Reveal_Size(&metadata));
		Parse_Function_Case__(Data_Type_Byte_Size, Print_Long__(metadata.st_size));
		Parse_Function_Case__(
			Data_Type_Permissions, Reveal_Permissions(&metadata)
		);
		Parse_Function_Case__(
			Data_Type_Octal_Permissions, Print_Octal_Permissions__
		);
		Parse_Return_Case__(Data_Type_User, Reveal_User(path, &metadata));
		Parse_Function_Case__(
			Data_Type_User_Uid, Print_Unsigned__(metadata.st_uid)
		);
		Parse_Return_Case__(Data_Type_Group, Reveal_Group(path, &metadata));
		Parse_Function_Case__(
			Data_Type_Group_Gid, Print_Unsigned__(metadata.st_gid)
		);
		Parse_Return_Case__(
			Data_Type_Modified_Date, Reveal_Modified_Date(&metadata)
		);
	default:
		switch (metadata.st_mode & S_IFMT) {
			Parse_Return_Case__(S_IFREG, Reveal_File(path));
			Parse_Return_Case__(S_IFDIR, Reveal_Directory(path));
			Parse_Return_Case__(S_IFLNK, Write_Error(
				"Can not reveal contents of symlink \"", path, "\".",
				"Did you mean to use the \"--follow-symlinks\" flag before it?"
			));
		default:
			return (Write_Error(
				"Can not reveal \"", path, "\" due to its type.", NULL
			));
		}
	}
	return (0);
}

int main(const int total_of_arguments, const char** arguments)
{
	if (total_of_arguments == 1) {
		Reveal(".");
	}
	const char* last_path = ".";
	for (
		int argument_index = 1; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Metadata_Flag__("version", program_version__);
		Parse_Metadata_Flag__("copyright", program_copyright__);
		Parse_Metadata_Flag__("help", program_help__);
	}
	for (
		int argument_index = 1; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Data_Type_Flag__("contents", Data_Type_Contents);
		Parse_Data_Type_Flag__("type", Data_Type_Type);
		Parse_Data_Type_Flag__("size", Data_Type_Size);
		Parse_Data_Type_Flag__("byte-size", Data_Type_Byte_Size);
		Parse_Data_Type_Flag__("permissions", Data_Type_Permissions);
		Parse_Data_Type_Flag__("octal-permissions", Data_Type_Octal_Permissions);
		Parse_Data_Type_Flag__("user", Data_Type_User);
		Parse_Data_Type_Flag__("user-uid", Data_Type_User_Uid);
		Parse_Data_Type_Flag__("group", Data_Type_Group);
		Parse_Data_Type_Flag__("group-gid", Data_Type_Group_Gid);
		Parse_Data_Type_Flag__("modified-date", Data_Type_Modified_Date);
		Parse_Non_Data_Type_Flag__(
			"follow-symlinks", OPTIONS |= is_following_symlinks_bit__
		);
		Parse_Non_Data_Type_Flag__(
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
