#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define program_version__ "v9.0.5"
#define program_help__ "help"
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (\
	is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__\
)
#define is_last_argument__ (argument_index == total_of_arguments - 1)
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

uint8_t OPTIONS = is_following_symlinks_bit__;

uint8_t Reveal(const char* const path) {
	printf(
		"%s (mode: %i) (is following symlinsk: %i)\n", path,
		OPTIONS & ~non_data_type_bits__, OPTIONS & is_following_symlinks_bit__
	);
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
