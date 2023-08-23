#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PGR_NAME "reveal"
#define PGR_VER "v10.0.0"
#define PARSE_EXIT_CODE(e) (e != 0 ? EXIT_FAILURE : EXIT_SUCCESS)
#define PARSE_NULL_STR(s) (s ? s : "")
#define PARSE_CASE(v, a)\
	case v:\
		a;\
		break;
#define PARSE_PUTS_CASE(v, t) PARSE_CASE(v, puts(t);)
#define PARSE_RET_CASE(v, a)\
	case v:\
		return a;
#define PARSE_OPT(o, t, a)\
	if (!strcmp("--" o, t)) {\
		a;\
	}
#define PARSE_META_OPT(o, a)\
	PARSE_OPT(o, argv[i],\
		a;\
		exit(EXIT_SUCCESS);\
	)
#define PARSE_DT_OPT(o, d)\
	PARSE_OPT(o, arg,\
		if (AW_ARG)\
			reveal(path);\
		DT = d;\
		AW_ARG = true;\
		if (is_last)\
			reveal(path);\
		return 1;\
	)
#define PARSE_LNK_OPT(o, f)\
	PARSE_OPT(o, arg,\
		if (is_last)\
			reveal(path);\
		F_LNK = f;\
		return 1;\
	)

typedef enum {
	DT_CTTS,
	DT_TYPE,
	DT_SIZE,
	DT_BT_SIZE,
	DT_PERMS,
	DT_OCT_PERMS,
	DT_USR,
	DT_USR_ID,
	DT_GRP,
	DT_GRP_ID,
	DT_MOD_DATE
} data_type_t;

static data_type_t DT = DT_CTTS;
static bool HAD_ERR = false, F_LNK = true, AW_ARG = false;

static void
help()
{
	printf("Usage: %s [OPTION | PATH]...\n", PGR_NAME);
	puts("Reveals information about entries in the file system.");
}

static int
print_err(char *desc0, char *desc1, char *desc2, char *fix)
{
	fprintf(stderr, "%s: %s%s%s\n%s%s", PGR_NAME, PARSE_NULL_STR(desc0),
		PARSE_NULL_STR(desc1), PARSE_NULL_STR(desc2),
		PARSE_NULL_STR(fix), fix ? "\n" : "");
	return 1;
}

static void
reveal_type(struct stat *s)
{
	switch (s->st_mode & S_IFMT) {
		PARSE_PUTS_CASE(S_IFREG, "regular");
		PARSE_PUTS_CASE(S_IFDIR, "directory");
		PARSE_PUTS_CASE(S_IFLNK, "symlink");
		PARSE_PUTS_CASE(S_IFSOCK, "socket");
		PARSE_PUTS_CASE(S_IFIFO, "fifo");
		PARSE_PUTS_CASE(S_IFCHR, "character");
		PARSE_PUTS_CASE(S_IFBLK, "block");
	default:
		puts("unknown");
	}
}

static int
reveal_file(char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return print_err("can't open file \"", path, "\"", "Check if "
			"you have permission to read it.");
	char c;
	while ((c = fgetc(f)) != EOF)
		putchar(c);
	fclose(f);
	return 0;
}

static int
reveal_dir(char *path)
{
	return 0;
}

static int
reveal_ctts(struct stat *s, char *path)
{
	switch (s->st_mode & S_IFMT) {
		PARSE_RET_CASE(S_IFREG, reveal_file(path));
		PARSE_RET_CASE(S_IFDIR, reveal_dir(path));
		PARSE_RET_CASE(S_IFLNK, print_err("can't read symlink \"", path,
			"\".", "Try to use the \"--follow-symlinks\" option "
			"before it."))
	default:
		return print_err("can't read contents of \"", path, "\".",
			"Its type is unreadable.");
	}
}

static int
reveal(char *path)
{
	struct stat s;
	if (F_LNK ? stat(path, &s) : lstat(path, &s) < 0)
		return print_err("can't find entry \"", path, "\".", "Check if "
			"you misspelled it.");
	switch (DT) {
		PARSE_CASE(DT_TYPE, reveal_type(&s));
	default:
		return reveal_ctts(&s, path);
	}
	return 0;
}

static int
parse_lnk_opts(char *arg, char *path, bool is_last)
{
	PARSE_LNK_OPT("follow-symlinks", true);
	PARSE_LNK_OPT("unfollow-symlinks", false);
	return 0;
}

static int
parse_dt_opts(char *arg, char *path, bool is_last)
{
	PARSE_DT_OPT("contents", DT_CTTS);
	PARSE_DT_OPT("type", DT_TYPE);
	PARSE_DT_OPT("size", DT_SIZE);
	PARSE_DT_OPT("bt-size", DT_BT_SIZE);
	PARSE_DT_OPT("perms", DT_PERMS);
	PARSE_DT_OPT("oct-perms", DT_OCT_PERMS);
	PARSE_DT_OPT("usr", DT_USR);
	PARSE_DT_OPT("usr-id", DT_USR_ID);
	PARSE_DT_OPT("grp", DT_GRP);
	PARSE_DT_OPT("grp-id", DT_GRP_ID);
	PARSE_DT_OPT("mod-date", DT_MOD_DATE);
	return 0;
}

static void
parse_non_meta_opts(int argc, char **argv)
{
	char *path = ".";
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		bool is_last = i == argc - 1;
		if (parse_dt_opts(arg, path, is_last) ||
			parse_lnk_opts(arg, path, is_last) || reveal(arg))
			continue;
		AW_ARG = false;
		path = arg;
	}
}

static void
parse_meta_opts(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		PARSE_META_OPT("version", puts(PGR_VER));
		PARSE_META_OPT("help", help());
	}
}

int
main(int argc, char **argv)
{
	if (argc == 1)
		return PARSE_EXIT_CODE(reveal("."));
	parse_meta_opts(argc, argv);
	parse_non_meta_opts(argc, argv);
	return PARSE_EXIT_CODE(HAD_ERR ? EXIT_FAILURE : EXIT_SUCCESS);
}
