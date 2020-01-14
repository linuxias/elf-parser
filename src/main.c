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
    SList *list = NULL;
    elf_h info;

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

    info = init_elf_info(file_name);
    if (info == NULL)
        exit(-1);

    elf_parser_get_header_info(info, ELF_PARSER_ELF_HEADER);

	tui_create();
    while (1) {
        ret = tui_get_selection();
        if (ret == TUI_SELECTION_NONE || ret == TUI_SELECTION_EXIT)
            break;

        switch (ret) {
            case TUI_SELECTION_ELF:
                list = elf_parser_get_header_info(info, ELF_PARSER_ELF_HEADER);
                break;
            case TUI_SELECTION_PROGRAM:
                list = elf_parser_get_header_info(info, ELF_PARSER_PROGRAM_HEADER);
                break;
            case TUI_SELECTION_SECTION:
                list = elf_parser_get_header_info(info, ELF_PARSER_SECTION_HEADER);
                break;
            default:
                break;
        }

        tui_print_str_list(list);
        s_list_free_full(list, free);
        list = NULL;
    }

    tui_destroy();

	return 0;
}
