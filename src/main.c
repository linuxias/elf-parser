#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <libgen.h>

#include "elf_parser.h"
#include "elf_parser_private.h"
#include "tui.h"

static void __print_usage()
{
	/* usage */
	printf("Usage : elfparser {elf file}\n");
}

int main(int argc, char *argv[])
{
    int ret;
    char *file_name;

    if (argc != 2) {
		__print_usage();
		exit(-1);
	}

    file_name = argv[1];
    ret = access(file_name, F_OK);
    if (ret != 0) {
        perror("access");
        exit(-1);
    }

	tui_create();

    while (1) {
        ret = tui_get_selection();
        if (ret == TUI_SELECTION_NONE || ret == TUI_SELECTION_EXIT)
            break;
    }

    tui_destroy();

	return 0;
}
