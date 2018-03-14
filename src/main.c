#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "elf_parser.h"
#include "elf_parser_private.h"

const char *short_options = "hepsa";
const struct option long_options[] = {
	{"help", 0, NULL, 'h'},
	{"elf", 0, NULL, 'e'},
	{"program", 0, NULL, 'p'},
	{"section", 0, NULL, 's'},
	{"all, 0, NULL, 'a'"}
};

static void __print_usage()
{
	/* usage */
	printf("Usage : sreadelf [-h|--help]\n");
	printf("                 [-e|--elf path] [-p|--program path] [-s|--section path]\n");
	printf("                 [-a|--all path]\n");

	printf("\n");

	/*Output format*/
	printf("Option args : \n");
	printf("-h, --help              Print this message\n");
	printf("-e, --elf               Print elf header\n");
	printf("-p, --program           Print program header\n");
	printf("-s, --section           Print section header\n");
	printf("-a, --all               Print all header\n");
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
			case OPT_HELP:
				__print_usage();
				break;
			case OPT_ELF:
				elf_parser_print_header(info, ELF_PARSER_ELF_HEADER);
				break;
			case OPT_PROGRAM:
				elf_parser_print_header(info, ELF_PARSER_PROGRAM_HEADER);
				break;
			case OPT_SECTION:
				elf_parser_print_header(info, ELF_PARSER_SECTION_HEADER);
				break;
			case OPT_ALL:
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
