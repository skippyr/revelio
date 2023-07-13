#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define programName "reveal"
#define programVersion "v7.0.1"
#define programLicense "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define programUpstream "https://github.com/skippyr/" programName
#define programHelp                                                            \
  "Usage: " programName " [FLAGS, ENTRIES...]\n"                               \
  "Reveals information about entries in the file system.\n\n"                  \
  "METADATA FLAGS\n"                                                           \
  "These flags can be used to retrieve metadata about the program.\n\n"        \
  "  --help     prints these help instructions.\n"                             \
  "  --version  prints its version.\n"                                         \
  "  --license  prints its license notice.\n\n"                                \
  "If one of these flags is used, the program will stop its execution right "  \
  "after\nperform its respective action.\n\n"                                  \
  "DATA TYPE FLAGS\n"                                                          \
  "These flags can be used to retrieve different data types from the entries " \
  "given\nas arguments.\n\n"                                                   \
  "  --contents (default)  prints its contents.\n"                             \
  "  --type                prints its type: block, character, directory, fifo,"\
  "\n                        symlink, regular or unknown.\n"                   \
  "  --size                prints its size in bytes.\n"                        \
  "  --human-size          prints its size using a human readable unit: GB, "  \
  "KB, MB\n                        or B.\n"                                    \
  "  --blocks              prints the quantity of blocks it occupies.\n"       \
  "  --hard-links          prints the quantity of hard links it has.\n"        \
  "  --user                prints the user that owns it.\n"                    \
  "  --user-id             prints the ID of the user that owns it.\n"          \
  "  --group               prints the group that owns it.\n"                   \
  "  --group-id            prints the ID of the group that owns it.\n"         \
  "  --mode                prints a number that contains bits representing its"\
  "\n                        attributes.\n"                                    \
  "  --permissions         prints its permissions in octal base.\n"            \
  "  --human-permissions   prints its permissions for user, group and others," \
  "\n                        respectively, using three set of characters each."\
  " Each \n                        character can mean a permission set: read " \
  "(r),\n                        write (w) and execute (x), or its lack (-).\n"\
  "  --inode               prints its serial number.\n"                        \
  "  --modified-date       prints the date where its contents were last "      \
  "modified.\n"                                                                \
  "  --changed-date        prints the date where its metadata were last "      \
  "changed.\n"                                                                 \
  "  --accessed-date       prints the date where its contents were last "      \
  "accessed.\n\n"                                                              \
  "If one of these flags is used, all the entries following it will be "       \
  "affected\nuntil it reaches another flag of this type. Else, the one marked "\
  "as default will\nbe considered in use.\n\n"                                 \
  "If you do not use an entry following them, they will consider the last "    \
  "entry\ngiven, else, the current directory.\n\n"                             \
  "TRANSPASSING FLAGS\n"                                                       \
  "These flags change the way information of symlinks will be handled.\n\n"    \
  "  --untranspass (default)  consider the symlinks themself.\n"               \
  "  --transpass              consider what the symlinks resolve to.\n\n"      \
  "If one of these flags is used, all the entries following it will be "       \
  "affected\nuntil it reaches another flag of this type. Else, the one marked "\
  "as default will\nbe considered in use.\n\n"                                 \
  "EXIT CODES\n"                                                               \
  "It will throw exit code 1 in the end of its execution if an error happens." \
  "\n\n"                                                                       \
  "ISSUES, QUESTIONS AND SUGGESTIONS\n"                                        \
  "Report issues, questions and suggestions at:\n"                             \
  programUpstream "/issues."

