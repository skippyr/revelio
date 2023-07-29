#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define program_name__ "reveal"
#define program_version__ "v9.0.0"
#define program_copyright__ "Copyright (c) Sherman Rofeman. MIT license."
#define program_support__ "https://github.com/skippyr/reveal/issues"
#define program_help__                                                         \
   "Usage: reveal [FLAGS | PATHS]\n"                                           \
   "Reveals information about entries in the file system.\n\n"                 \
   "METADATA FLAGS\n"                                                          \
   "The following flags allow you to get information about the program "       \
   "itself.\n\n"                                                               \
   "    --copyright    prints its copyright notice.\n"                         \
   "    --version      prints its version.\n"                                  \
   "    --help         prints these help instructions.\n\n"                    \
   "DATA TYPE FLAGS\n"                                                         \
   "The following flags allow you to change what type of data to get from "    \
   "the\nentries following them.\n\n"                                          \
   "If none is used, the one marked as default will be considered in "         \
   "use.\n\n"                                                                  \
   "    --contents (default)    prints its contents.\n"                        \
   "    --type                  prints its type: regular, directory, "         \
   "symlink,\n                            socket, fifo, block, character or "  \
   "unknown.\n"                                                                \
   "    --size                  prints its size using the most formidable "    \
   "unit:\n                            gigabyte (GB), megabyte (MB), "         \
   "kilobyte (KB) "                                                            \
   "or\n                            byte (B).\n"                               \
   "    --byte-size             prints its size in bytes.\n"                   \
   "    --permissions           prints its read (r), write (w) and execute "   \
   "(x)\n                            permissions for user, group and "         \
   "others. If a\n                            permission is not set "          \
   "the lack (-) character\n                            will be used.\n"       \
   "    --octal-permissions     prints its permissions in octal base.\n"       \
   "    --user                  prints the user that owns it.\n"               \
   "    --user-uid              prints the UID of the user that owns it.\n"    \
   "    --group                 prints the group that owns it.\n"              \
   "    --group-gid             prints the GID of the group that owns it.\n"   \
   "    --modified-date         prints the date when its contents where "      \
   "last\n                            "                                        \
   "modified.\n\n"                                                             \
   "These flags expects, at least, one path following them. If none is "       \
   "provided,\nthey will consider the last one given or else, the current "    \
   "directory.\n\n"                                                            \
   "SYMLINKS FLAGS\n"                                                          \
   "These flags allow you to change how symlinks following them will be\n"     \
   "handled, affecting the origin of the data you reveal.\n\n"                 \
   "If none is used, the one marked as default will be considered in "         \
   "use.\n\n"                                                                  \
   "    --follow-symlinks  (default)    symlinks will be followed.\n"          \
   "    --unfollow-symlinks             symlinks will not be followed\n\n"     \
   "EXIT CODES\n"                                                              \
   "The exit code 1 will be throw if an error happens during its "             \
   "execution.\n\n"                                                            \
   "Errors will be reported through the standard error stream.\n\n"            \
   "SUPPORT\n"                                                                 \
   "Report issues, questions or suggestion at:\n" program_support__ "."
#define is_expecting_path_bit__ (1 << 5)
#define is_following_symlinks_bit__ (1 << 6)
#define had_error_bit__ (1 << 7)
#define non_data_type_bits__                                                   \
   (is_expecting_path_bit__ | is_following_symlinks_bit__ | had_error_bit__)
#define Is_Last_Argument__ (arguments_index == total_of_arguments - 1)
#define Reveal_Last_Path__                                                     \
   Reveal(last_path_index == -1 ? "." : arguments[last_path_index])
#define Parse_Flag__(flag, action)                                             \
   if (!strcmp("--" flag, arguments[arguments_index])) {                       \
      action;                                                                  \
   }
#define Parse_Metadata_Flag__(flag, text)                                      \
   Parse_Flag__(flag, puts(text); return (0))
#define Parse_Data_Type_Flag__(flag, data_type)                                \
   Parse_Flag__(                                                               \
       flag,                                                                   \
       if (global_options & is_expecting_path_bit__) {                         \
          Reveal_Last_Path__;                                                  \
       } global_options = (data_type | is_expecting_path_bit__ |               \
                           (global_options & non_data_type_bits__));           \
       if (Is_Last_Argument__) { Reveal_Last_Path__; } continue)
#define Parse_Non_Data_Type_Flag__(flag, action)                               \
   Parse_Flag__(                                                               \
       flag, if (Is_Last_Argument__) { Reveal_Last_Path__; } action; continue)
#define Parse_Function_Case__(value, action)                                   \
   case (value):                                                               \
      action;                                                                  \
      break;
#define Parse_Puts_Case__(value, text) Parse_Function_Case__(value, puts(text))
#define Parse_Null_String__(text) (text ? text : "")

typedef enum {
   Data_Type_Contents,
   Data_Type_Type,
   Data_Type_Size,
   Data_Type_Byte_Size,
   Data_Type_Permissions,
   Data_Type_Octal_Permission,
   Data_Type_User,
   Data_Type_User_Uid,
   Data_Type_Group,
   Data_Type_Group_Gid,
   Data_Type_Modified_Date
} Data_Type;

uint8_t global_options = is_following_symlinks_bit__;

void
Print_Error(const char *const description_split_0,
            const char *const description_split_1,
            const char *const description_split_2, const char *const suggestion)
{
   fprintf(stderr, "%s: %s%s%s\n%s%s%s", program_name__,
           Parse_Null_String__(description_split_0),
           Parse_Null_String__(description_split_1),
           Parse_Null_String__(description_split_2),
           suggestion ? "        " : "", Parse_Null_String__(suggestion),
           suggestion ? "\n" : "");
   return;
}

void
Reveal_Type(const struct stat *const metadata)
{
   switch (metadata->st_mode & S_IFMT) {
      Parse_Puts_Case__(S_IFREG, "regular");
      Parse_Puts_Case__(S_IFDIR, "directory");
      Parse_Puts_Case__(S_IFLNK, "symlink");
      Parse_Puts_Case__(S_IFSOCK, "socket");
      Parse_Puts_Case__(S_IFIFO, "fifo");
      Parse_Puts_Case__(S_IFBLK, "block");
      Parse_Puts_Case__(S_IFCHR, "character");
   default:
      puts("unknown");
   }
   return;
}

void
Reveal_File(const char *const path)
{
   FILE *const file = fopen(path, "r");
   if (!file) {
      Print_Error("can not open file \"", path, "\".",
                  "Ensure that you have enough permissions.");
      return;
   }
   char character;
   while ((character = fgetc(file)) != EOF) {
      putchar(character);
   }
   fclose(file);
   return;
}

void
Reveal_Directory(const char *const path)
{
   DIR *const directory = opendir(path);
   if (!directory) {
      Print_Error("can not open directory \"", path, "\".",
                  "Ensure that you have enough permissions.");
      return;
   }
   const struct dirent *entry;
   while ((entry = readdir(directory))) {
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
         continue;
      }
      printf("%s\n", entry->d_name);
   }
   closedir(directory);
   return;
}

void
Reveal(const char *const path)
{
   struct stat metadata;
   if (global_options & is_following_symlinks_bit__ ? stat(path, &metadata)
                                                    : lstat(path, &metadata)) {
      Print_Error("the path \"", path, "\" does not exists.",
                  "Ensure that you did not misspelled it.");
      return;
   }
   switch (global_options & ~non_data_type_bits__) {
      Parse_Function_Case__(Data_Type_Type, Reveal_Type(&metadata));
   default:
      switch (metadata.st_mode & S_IFMT) {
         Parse_Function_Case__(S_IFREG, Reveal_File(path));
         Parse_Function_Case__(S_IFDIR, Reveal_Directory(path));
         Parse_Function_Case__(
             S_IFLNK,
             Print_Error(
                 "can not reveal contents of symlink \"", path, "\".",
                 "Did you mean to use \"--follow-symlinks\" before it?"));
      default:
         Print_Error("can not reveal the contents of \"", path,
                     "\" due to its type.", NULL);
      }
   }
   return;
}

int
main(const int total_of_arguments, const char **arguments)
{
   if (total_of_arguments == 1) {
      Reveal(".");
   }
   int last_path_index = -1;
   for (int arguments_index = 1; arguments_index < total_of_arguments;
        arguments_index++) {
      Parse_Metadata_Flag__("copyright", program_copyright__);
      Parse_Metadata_Flag__("version", program_version__);
      Parse_Metadata_Flag__("help", program_help__);
   }
   for (int arguments_index = 1; arguments_index < total_of_arguments;
        arguments_index++) {
      Parse_Data_Type_Flag__("contents", Data_Type_Contents);
      Parse_Data_Type_Flag__("type", Data_Type_Type);
      Parse_Data_Type_Flag__("size", Data_Type_Size);
      Parse_Data_Type_Flag__("byte-size", Data_Type_Byte_Size);
      Parse_Data_Type_Flag__("permissions", Data_Type_Permissions);
      Parse_Data_Type_Flag__("octal-permissions", Data_Type_Octal_Permission);
      Parse_Data_Type_Flag__("user", Data_Type_User);
      Parse_Data_Type_Flag__("user-uid", Data_Type_User_Uid);
      Parse_Data_Type_Flag__("group", Data_Type_Group);
      Parse_Data_Type_Flag__("group-gid", Data_Type_Group_Gid);
      Parse_Data_Type_Flag__("modified-date", Data_Type_Modified_Date);
      Parse_Non_Data_Type_Flag__("follow-symlinks",
                                 global_options |= is_following_symlinks_bit__);
      Parse_Non_Data_Type_Flag__(
          "unfollow-symlinks", global_options &= ~is_following_symlinks_bit__);
      global_options &= ~is_expecting_path_bit__;
      last_path_index = arguments_index;
      Reveal_Last_Path__;
   }
   return !!(global_options & had_error_bit__);
}
