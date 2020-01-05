#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	if (argc != 2) {
		__print_usage();
		exit(-1);
	}

	tui_create();
	tui_destroy();

	return 0;
}
