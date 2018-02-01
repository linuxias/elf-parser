#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "elf_parser.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <executable>\n", argv[0]);
		exit(0);
	}

	elf_parser_print_header(argv[1], ELF_PARSER_ELF_HEADER);
	elf_parser_print_header(argv[1], ELF_PARSER_PROGRAM_HEADER);
	elf_parser_print_header(argv[1], ELF_PARSER_SECTION_HEADER);

	return 0;
}
