#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define program_name__ "reveal"
#define program_version__ "version"
#define program_help__ "help"
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__ (is_expecting_path_bit__ |                        \
                              is_following_symlinks_bit__ | had_error_bit__)
#define is_last_argument__ (argument_index == total_of_arguments - 1)
#define Parse_Flag(flag, action)\
    if (!strcmp("--" flag, arguments[argument_index])) {                       \
        action;                                                                \
    }
#define Parse_Metadata_Flag__(flag, text) Parse_Flag(flag, puts(text); return 0)
#define Parse_Data_Type_Flag__(flag, data_type) Parse_Flag(flag,               \
    if (OPTIONS & is_expecting_path_bit__) {                                   \
        Reveal(last_path);                                                     \
    }                                                                          \
    OPTIONS = (data_type | is_expecting_path_bit__ | (OPTIONS &                \
                                                      non_data_type_bits__));  \
    if (is_last_argument__) {                                                  \
        Reveal(last_path);                                                     \
    }                                                                          \
    continue;                                                                  \
)
#define Parse_Non_Data_Type_Flag__(flag, action) Parse_Flag(flag,              \
    if (is_last_argument__) {                                                  \
        Reveal(last_path);                                                     \
    }                                                                          \
    action;                                                                    \
    continue;                                                                  \
)

typedef enum
{
    Data_Type_Content,
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

uint8_t Reveal(const char* const path)
{
    printf("%s (mode: %i) (sym: %i)\n", path, OPTIONS & ~non_data_type_bits__,
           OPTIONS & is_following_symlinks_bit__);
    return 0;
}

int main(const int total_of_arguments, const char** arguments)
{
    if (total_of_arguments == 1) {
        Reveal(".");
    }
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++) {
        Parse_Metadata_Flag__("version", program_version__);
        Parse_Metadata_Flag__("help", program_help__);
    }
    const char* last_path = ".";
    for (int argument_index = 1; argument_index < total_of_arguments;
         argument_index++) {
        Parse_Data_Type_Flag__("contents", Data_Type_Content);
        Parse_Data_Type_Flag__("type", Data_Type_Type);
        Parse_Data_Type_Flag__("size", Data_Type_Size);
        Parse_Data_Type_Flag__("byte-size", Data_Type_Byte_Size);
        Parse_Data_Type_Flag__("permissions", Data_Type_Permissions);
        Parse_Data_Type_Flag__("octal-permissions", Data_Type_Permissions);
        Parse_Data_Type_Flag__("user", Data_Type_User);
        Parse_Data_Type_Flag__("user-uid", Data_Type_User_Uid);
        Parse_Data_Type_Flag__("group", Data_Type_Group);
        Parse_Data_Type_Flag__("group-gid", Data_Type_Group_Gid);
        Parse_Data_Type_Flag__("modified-date", Data_Type_Modified_Date);
        Parse_Non_Data_Type_Flag__("follow-symlinks",
                                   OPTIONS |= is_following_symlinks_bit__);
        Parse_Non_Data_Type_Flag__("unfollow-symlinks",
                                   OPTIONS &= ~is_following_symlinks_bit__);
        if (Reveal(arguments[argument_index])) {
            continue;
        }
        OPTIONS &= ~is_expecting_path_bit__;
        last_path = arguments[argument_index];
    }
    return !!(OPTIONS & had_error_bit__);
}
