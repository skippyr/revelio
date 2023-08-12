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
#define Parse_Option__(option, action)\
	if (!strcmp("--" option, arguments[argument_index])) {\
		action;\
	}
#define Parse_Metadata_Option__(option, text) Parse_Option__(\
	option, puts(text); return (0);\
)

uint8_t OPTIONS = is_following_symlinks_bit__;

int main(const int total_of_arguments, const char** arguments) {
	for (
		int argument_index = 0; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Metadata_Option__("version", program_version__);
		Parse_Metadata_Option__("help", program_help__);
	}
	return !!(OPTIONS & had_error_bit__);
}
