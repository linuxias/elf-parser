#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <errno.h>

#include "elf_parser.h"

typedef struct elf_info *elf_info_s;
struct elf_info {
	uint8_t *mem;
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;
} elf_info;

static elf_info_s info;

static void __destroy_elf_info()
{
	if (info)
		free(info);
}

static int __is_executable()
{
#define FIRST_MAGIC_STRING 0x7f
#define ELF_STRING "ELF"

	if (info->mem[0] != FIRST_MAGIC_STRING && strcmp((char *)(&info->mem[1]), ELF_STRING)) {
		fprintf(stderr, "Not an ELF");
		return -1;
	}

	if (info->ehdr->e_type != ET_EXEC) {
		fprintf(stderr, "Not executable");
		return -1;
	}

	return 0;
}

static void __init_elf_info(const char *elf_file)
{
	int fd;
	struct stat st;

	if (info != NULL)
		return;

	info = (elf_info_s)calloc(1, sizeof(struct elf_info));
	if (info == NULL) {
		fprintf(stderr, "OOM");
		exit(-1);
	}

	errno = 0;
	fd = open(elf_file, O_RDONLY);
	if (errno < 0 || fd == -1) {
		perror("open");
		exit(-1);
	}

	errno = 0;
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		exit(-1);
	}

	info->mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (info->mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	info->ehdr = (Elf64_Ehdr *)info->mem;
	info->phdr = (Elf64_Phdr *)&(info->mem[info->ehdr->e_phoff]);
	info->shdr = (Elf64_Shdr *)&(info->mem[info->ehdr->e_shoff]);

	if (__is_executable() < 0) {
		fprintf(stderr, "%s is not ELF file", elf_file);
		__destroy_elf_info();
		exit(-1);
	}

	close(fd);
}

static void __print_elf_header(Elf64_Ehdr *ehdr)
{
	printf("\n =====\tELF Header\t=====\n");
	printf("Magic : ");
	for (int i = 0; i < EI_NIDENT; i++) {
		printf("%02x ", ehdr->e_ident[i]);
	}
	printf("\n");
}

static void __print_section_header(Elf64_Shdr *shdr)
{
	int i;
	unsigned char *string_table;

	printf("\n =====\tSection Header\t===== \n");
	string_table = &info->mem[shdr[info->ehdr->e_shstrndx].sh_offset];
	for (i = 0; i < info->ehdr->e_shnum; i++)
		printf("%s : 0x%lx\n", &string_table[shdr[i].sh_name], shdr[i].sh_addr);
	printf("\n");
}

static void __print_program_header(Elf64_Phdr *phdr)
{
	char *interp;
	int i;

	printf("\n =====\tProgram Header\t===== \n");
	for (i = 0; i < info->ehdr->e_phnum; i++) {
		switch(phdr[i].p_type) {
			case PT_LOAD :
				if (phdr[i].p_offset == 0)
					printf("Text segment: 0x%lx\n", phdr[i].p_vaddr);
				else
					printf("Data segment: 0x%lx\n", phdr[i].p_vaddr);
			break;
			case PT_INTERP :
				interp = strdup((char *)(&info->mem[phdr[i].p_offset]));
				printf("Interpreter: %s\n", interp);
				free(interp);
			break;
			case PT_NOTE :
				printf("Note segment: 0x%lx\n", phdr[i].p_vaddr);
			break;
			case PT_DYNAMIC:
				printf("Dynamic segment: 0x%lx\n", phdr[i].p_vaddr);
			break;
			case PT_PHDR:
				printf("Phdr segment: 0x%lx\n", phdr[i].p_vaddr);
			break;
		}
	}
}

void elf_parser_print_header(const char *elf_file, elf_parser_header_type_e type)
{
	if (info == NULL)
		__init_elf_info(elf_file);

	switch(type) {
		case ELF_PARSER_ELF_HEADER:
			__print_elf_header(info->ehdr);
			break;
		case ELF_PARSER_PROGRAM_HEADER:
			__print_program_header(info->phdr);
			break;
		case ELF_PARSER_SECTION_HEADER:
			__print_section_header(info->shdr);
			break;
	}
}
