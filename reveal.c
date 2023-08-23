#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PGR_NAME "reveal"
#define PGR_VER "v9.1.3"
#define PARSE_EXIT_CODE(e) (e != 0 ? EXIT_FAILURE : EXIT_SUCCESS)
#define PARSE_OPT(opt, tgt, act)\
	if (!strcmp("--" opt, tgt)) {\
		act;\
	}
#define PARSE_META_OPT(opt, act)\
	PARSE_OPT(opt, argv[i],\
		act;\
		exit(EXIT_SUCCESS);\
	)
#define PARSE_DT_OPT(opt, dt)\
	PARSE_OPT(opt, arg,\
		if (AW_ARG)\
			reveal(path);\
		DT = dt;\
		AW_ARG = true;\
		if (is_last)\
			reveal(path);\
		return 1;\
	)
#define PARSE_LNK_OPT(opt, f)\
	PARSE_OPT(opt, arg,\
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
reveal(char *path)
{
	printf("Path: %s\nDt: %i\nFLNK: %i\n", path, DT, F_LNK);
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
	PARSE_DT_OPT("byte-size", DT_BT_SIZE);
	PARSE_DT_OPT("permissions", DT_PERMS);
	PARSE_DT_OPT("octal-permissions", DT_OCT_PERMS);
	PARSE_DT_OPT("user", DT_USR);
	PARSE_DT_OPT("user-uid", DT_USR_ID);
	PARSE_DT_OPT("group", DT_GRP);
	PARSE_DT_OPT("group-gid", DT_GRP_ID);
	PARSE_DT_OPT("modified-date", DT_MOD_DATE);
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
			parse_lnk_opts(arg, path, is_last) || reveal(path))
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
