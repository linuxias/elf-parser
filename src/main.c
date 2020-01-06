#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

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
    pthread_t display_ths;

	if (argc != 2) {
		__print_usage();
		exit(-1);
	}

	tui_create();

    ret = pthread_create(&display_ths, NULL, tui_display_routine, NULL);
    if (ret != 0) {
        perror("pthread_create()");
        tui_destroy();
        exit(-1);
    }

    while (1) {
        ret = tui_get_selection();
        if (ret == TUI_SELECTION_NONE || ret == TUI_SELECTION_EXIT)
            break;
    }

    tui_destroy();

	return 0;
}
