#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.3"
#define program_copyright__ "Copyright (c) Sherman Rofeman. MIT license."
#define program_help__ "help"

#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (\
	is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__\
)

#define is_last_argument__ (argument_index == total_of_arguments - 1)

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

void Reveal(const char *const path)
{
	return;
}

int main(const int total_of_arguments, const char **arguments)
{
	if (total_of_arguments == 1) {
		Reveal(".");
	}
	const char *last_path = "";
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
	}
	return !!(OPTIONS & had_error_bit__);
}
