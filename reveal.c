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

#define Parse_Flag__(flag, action)\
	if (!strcmp("--" flag, arguments[argument_index])) {\
		action;\
	}
#define Parse_Metadata_Flag__(flag, text) Parse_Flag__(flag,\
	puts(text);\
	return (0);\
)

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
		int argument_index = 0; argument_index < total_of_arguments;
		argument_index++
	) {
		Parse_Metadata_Flag__("version", program_version__);
		Parse_Metadata_Flag__("copyright", program_copyright__);
		Parse_Metadata_Flag__("help", program_help__);
	}
	return !!(OPTIONS & had_error_bit__);
}
