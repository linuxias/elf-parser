#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "elf_parser.h"
#include "elf_parser_private.h"
#include "ftrace_private.h"

const char *short_options = "heps";
const struct option long_options[] = {
	{"help", 0, NULL, 'h'},
	{"elf", 0, NULL, 'e'},
	{"program", 0, NULL, 'p'},
	{"section", 0, NULL, 's'},
};

static void __print_usage()
{
	/* usage */
	printf("usage : ftrace [-h]\n");
	printf("               [-e path] [-p path] [-s path]\n");

	printf("\n");

	/*Output format*/
	printf("Option args : \n");
	printf("-h, --help              print this message\n");
	printf("-e, --elf               print elf header\n");
	printf("-p, --program           print program header\n");
	printf("-s, --section           print section header\n");
}

static void __run_option(int argc, char *argv[])
{
	int opt = -1;
	int opt_idx = 0;
	elf_info_s info;

	info = init_elf_info(argv[2]);

	while (1) {
		opt = getopt_long(argc, argv, short_options, long_options, &opt_idx);
		if (opt == -1)
			break;

		switch(opt) {
			case FTRACE_OPT_HELP:
				__print_usage();
				break;
			case FTRACE_OPT_ELF:
				elf_parser_print_header(info, ELF_PARSER_ELF_HEADER);
				break;
			case FTRACE_OPT_PROGRAM:
				elf_parser_print_header(info, ELF_PARSER_PROGRAM_HEADER);
				break;
			case FTRACE_OPT_SECTION:
				elf_parser_print_header(info, ELF_PARSER_SECTION_HEADER);
				break;
			default:
				__print_usage();
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		__print_usage();
		exit(0);
	}

	__run_option(argc, argv);

	return 0;
}
