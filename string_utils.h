#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings.h"

char *alloc_dir(char *arg);
void parse_show(FILE *show, int *f_type);
void format_tordir(char *tor_dir_clean, int i, int last);
char *build_command(char *tordir, const char *torname, int vf);
