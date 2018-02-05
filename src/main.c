#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "elf_parser.h"
#include "ftrace_private.h"

const char *short_options = "h";
const struct option long_options[] = {
	{"help", 0, NULL, 'h'},
};

static void __print_usage()
{
	/* usage */
	printf("usage : ftrace [-h]\n");

	printf("\n");

	/*Output format*/
	printf("Output format : \n");
	printf("-h, --help \t\t\t print this message\n");
}

static void __run_option(int opt)
{
	switch(opt) {
		case FTRACE_OPT_HELP:
			__print_usage();
		break;
		default:
		break;
	}
}

int main(int argc, char *argv[])
{
	int opt = -1;
	int opt_idx = 0;

	if (argc < 2) {
		printf("Usage: %s <executable>\n", argv[0]);
		exit(0);
	}

	opt = getopt_long(argc, argv, short_options, long_options, &opt_idx);
	if (opt == -1) {
		fprintf(stdout, "getopt_long");
		exit(-1);
	}

	__run_option(opt);


	/*
	elf_parser_print_header(argv[1], ELF_PARSER_ELF_HEADER);
	elf_parser_print_header(argv[1], ELF_PARSER_PROGRAM_HEADER);
	elf_parser_print_header(argv[1], ELF_PARSER_SECTION_HEADER);
	*/

	return 0;
}
