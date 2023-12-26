#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PARSE_INFO_TYPE_OPTION(option, infoType)                               \
  PARSE_OPTION(option, infoType_g = infoType; continue);
#define PARSE_OPTION(option, action)                                           \
  if (!strcmp("-" option, arguments[argumentIndex])) {                         \
    action;                                                                    \
  }
#define PARSE_SYMLINK_OPTION(option, isFollowingSymlinks)                      \
  PARSE_OPTION(option, isFollowingSymlinks_g = isFollowingSymlinks; continue);

enum InfoType {
  InfoType_Contents,
  InfoType_Type,
  InfoType_Size,
  InfoType_HumanSize,
  InfoType_Permissions,
  InfoType_OctalPermissions,
  InfoType_User,
  InfoType_UID,
  InfoType_Group,
  InfoType_GID,
  InfoType_ModifiedDate
};

static void *allocate(size_t bytes);
static void die(char *format, ...);
static void reveal(char *path);
static void revealDirectory(char *path);
static void revealFile(char *path);
static void revealGroup(char *path, struct stat *metadata);
static void revealHumanSize(struct stat *metadata);
static void revealModifiedDate(struct stat *metadata);
static void revealPermissions(struct stat *metadata);
static void revealSymlink(char *path);
static void revealType(struct stat *metadata);
static void revealUser(char *path, struct stat *metadata);
static int sortAlphabetically(const void *stringI, const void *stringII);

static int infoType_g = InfoType_Contents;
static int isFollowingSymlinks_g = 0;

static void *allocate(size_t bytes) {
  void *allocation;
  if (!(allocation = malloc(bytes))) {
    die("can't allocate memory.\n");
  }
  return allocation;
}

static void die(char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  fprintf(stderr, "revelio: ");
  vfprintf(stderr, format, arguments);
  va_end(arguments);
  exit(1);
}

static void reveal(char *path) {
  struct stat metadata;
  if (isFollowingSymlinks_g ? stat(path, &metadata) : lstat(path, &metadata)) {
    die("can't stat \"%s\".\n", path);
  } else if (infoType_g == InfoType_Contents && S_ISREG(metadata.st_mode)) {
    revealFile(path);
  } else if (infoType_g == InfoType_Contents && S_ISDIR(metadata.st_mode)) {
    revealDirectory(path);
  } else if (infoType_g == InfoType_Contents && S_ISLNK(metadata.st_mode)) {
    revealSymlink(path);
  } else if (infoType_g == InfoType_Contents) {
    die("can't reveal contents of \"%s\".\n", path);
  } else if (infoType_g == InfoType_Type) {
    revealType(&metadata);
  } else if (infoType_g == InfoType_Size) {
    printf("%ld\n", metadata.st_size);
  } else if (infoType_g == InfoType_HumanSize) {
    revealHumanSize(&metadata);
  } else if (infoType_g == InfoType_Permissions) {
    revealPermissions(&metadata);
  } else if (infoType_g == InfoType_OctalPermissions) {
    printf("%o\n",
           metadata.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |
                               S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH));
  } else if (infoType_g == InfoType_User) {
    revealUser(path, &metadata);
  } else if (infoType_g == InfoType_UID) {
    printf("%u\n", metadata.st_uid);
  } else if (infoType_g == InfoType_Group) {
    revealGroup(path, &metadata);
  } else if (infoType_g == InfoType_GID) {
    printf("%u\n", metadata.st_gid);
  } else if (infoType_g == InfoType_ModifiedDate) {
    revealModifiedDate(&metadata);
  }
}

static void revealDirectory(char *path) {
  DIR *stream = opendir(path);
  char **entryNames;
  char *entryName;
  int entryIndexI;
  int entryIndexII;
  struct dirent *entry;
  if (!stream) {
    die("can't open directory \"%s\".\n", path);
  }
  for (entryIndexI = -2; readdir(stream); entryIndexI++)
    ;
  if (!entryIndexI) {
    goto close;
  }
  entryNames = allocate(sizeof(NULL) * entryIndexI);
  entryIndexI = 0;
  rewinddir(stream);
  while ((entry = readdir(stream))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }
    entryName = allocate(strlen(entry->d_name) + 1);
    entryNames[entryIndexI] = strcpy(entryName, entry->d_name);
    entryIndexI++;
  }
  qsort(entryNames, entryIndexI, sizeof(NULL), sortAlphabetically);
  for (entryIndexII = 0; entryIndexII < entryIndexI; entryIndexII++) {
    printf("%s\n", entryNames[entryIndexII]);
    free(entryNames[entryIndexII]);
  }
  free(entryNames);
close:
  closedir(stream);
}

static void revealFile(char *path) {
  FILE *stream = fopen(path, "r");
  int character;
  if (!stream) {
    die("can't open file \"%s\".\n", path);
  }
  for (; (character = fgetc(stream)) != EOF; putchar(character))
    ;
  fclose(stream);
}

static void revealGroup(char *path, struct stat *metadata) {
  char buffer[255];
  struct group *result;
  struct group group;
  if (getgrgid_r(metadata->st_gid, &group, buffer, sizeof(buffer), &result) ||
      !result) {
    die("can't find group that owns \"%s\".\n", path);
  }
  printf("%s\n", group.gr_name);
}

static void revealHumanSize(struct stat *metadata) {
  char prefixes[] = {'G', 'M', 'k'};
  float multipliers[] = {1e9, 1e6, 1e3};
  float size;
  int multiplierIndex;
  for (multiplierIndex = 0; multiplierIndex < 3; multiplierIndex++) {
    if ((size = metadata->st_size / multipliers[multiplierIndex]) >= 1) {
      printf("%.1f%cB\n", size, prefixes[multiplierIndex]);
      return;
    }
  }
  printf("%ldB\n", metadata->st_size);
}

static void revealModifiedDate(struct stat *metadata) {
  char buffer[29];
  strftime(buffer, sizeof(buffer), "%a %b %d %T %Z %Y",
           localtime(&metadata->st_mtime));
  printf("%s\n", buffer);
}

static void revealPermissions(struct stat *metadata) {
  char characters[] = {'r', 'w', 'x'};
  int flags[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                 S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
  int flagIndex;
  for (flagIndex = 0; flagIndex < 9; flagIndex++) {
    putchar(metadata->st_mode & flags[flagIndex]
                ? characters[flagIndex < 3 ? flagIndex : (flagIndex - 3) % 3]
                : '-');
  }
  putchar('\n');
}

static void revealSymlink(char *path) {
  char buffer[100];
  buffer[readlink(path, buffer, sizeof(buffer))] = 0;
  printf("%s\n", buffer);
}

static void revealType(struct stat *metadata) {
  printf("%c\n", S_ISREG(metadata->st_mode)    ? 'r'
                 : S_ISDIR(metadata->st_mode)  ? 'd'
                 : S_ISLNK(metadata->st_mode)  ? 'l'
                 : S_ISCHR(metadata->st_mode)  ? 'c'
                 : S_ISBLK(metadata->st_mode)  ? 'b'
                 : S_ISFIFO(metadata->st_mode) ? 'f'
                                               : 's');
}

static void revealUser(char *path, struct stat *metadata) {
  char buffer[255];
  struct passwd *result;
  struct passwd user;
  if (getpwuid_r(metadata->st_uid, &user, buffer, sizeof(buffer), &result) ||
      !result) {
    die("can't find user that owns \"%s\".\n", path);
  }
  printf("%s\n", user.pw_name);
}

static int sortAlphabetically(const void *stringI, const void *stringII) {
  return strcmp(*(char **)stringI, *(char **)stringII);
}

int main(int totalOfArguments, char **arguments) {
  int argumentIndex;
  for (argumentIndex = 1; argumentIndex < totalOfArguments; argumentIndex++) {
    PARSE_INFO_TYPE_OPTION("c", InfoType_Contents);
    PARSE_INFO_TYPE_OPTION("t", InfoType_Type);
    PARSE_INFO_TYPE_OPTION("s", InfoType_Size);
    PARSE_INFO_TYPE_OPTION("hs", InfoType_HumanSize);
    PARSE_INFO_TYPE_OPTION("p", InfoType_Permissions);
    PARSE_INFO_TYPE_OPTION("op", InfoType_OctalPermissions);
    PARSE_INFO_TYPE_OPTION("u", InfoType_User);
    PARSE_INFO_TYPE_OPTION("ui", InfoType_UID);
    PARSE_INFO_TYPE_OPTION("g", InfoType_Group);
    PARSE_INFO_TYPE_OPTION("gi", InfoType_GID);
    PARSE_INFO_TYPE_OPTION("md", InfoType_ModifiedDate);
    PARSE_SYMLINK_OPTION("ul", 0);
    PARSE_SYMLINK_OPTION("fl", 1);
    reveal(arguments[argumentIndex]);
  }
  return 0;
}
