#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <elf.h>
#include <sys/mman.h>

static int __is_validate_file(uint8_t *mem)
{
#define FIRST_MAGIC_STRING 0x7f
#define ELF_STRING "ELF"

	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)mem;

	if (mem[0] != FIRST_MAGIC_STRING && strcmp(&mem[1], ELF_STRING)) {
		fprintf(stderr, "Not an ELF");
		return -1;
	}

	if (ehdr->e_type != ET_EXEC) {
		fprintF(stderr, "Not executable");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int fd;
	uint8_t *mem;
	struct stat st;
	char *string_table;
	char *interp;
	int i;

	Elf32_Ehdr *ehdr = NULL;
	Elf32_Phdr *phdr = NULL;
	Elf32_Shdr *shdr = NULL;

	if (argc < 2) {
		printf("Usage: %s <executable>\n", argv[0]);
		exit(0);
	}

	errno = 0;
	fd = open(argv[1], O_RDONLY);
	if (errno < 0 || fd == -1) {
		perror("open");
		exit(-1);
	}

	errno = 0;
	ret = fstat(fd, &st);
	if (ret < 0) {
		perror("fstat");
		exit(-1);
	}

	mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	ehdr = (Elf32_Ehdr *)mem;
	phdr = (Elf32_Phdr *)&mem[ehdr->e_phoff];
	shdr = (Elf32_Shdr *)&mem[ehdr->e_shoff];

	if (__is_validate_file(mem) < 0) {
		printf("%s is invalide file", argv[1]);
		exit(-1);
	}

	printf("Program Entry point : 0x%x\n", ehdr->e_entry);

	printf("Section Header list:\n");
	string_table = &mem[shdr[ehdr->e_shstrndex].sh_offset];
	for (i = 0; i < ehdr->e_shnum; i++)
		printf("%s : 0x%x\n", &string_table[shdr[i].sh_name], shdr[i].sh_addr);
	printf("\n");

	printf("Program Header list:\n");
	for (i = 0; i < ehdr->e_phnum; i++) {
		switch(phdr[i].p_type) {
			case PT_LOAD :
				if (phdr[i].p_offset == 0)
					printf("Text segment: 0x%x\n", phdr[i].p_vaddr);
				else
					printf("Data segment: 0x%x\n", phdr[i].p_vaddr);
			break;
			case PT_INTERP :
				interp = strdup((char *)&mem[phdr[i].p_offset]);
				printf("Interpreter: %s\n", interp);
				free(interp);
			break;
			case PT_NOTE :
				printf("Note segment: 0x%x\n", phdr[i].p_vaddr);
			break;
			case PT_DYNAMIC:
				printf("Dynamic segment: 0x%x\n", phdr[i].p_vaddr);
			break;
			case PT_PHDR:
				printf("Phdr segment: 0x%x\n", phdr[i].p_vaddr);
			break;
		}
	}

	close(fd);
	munmap(mem, st.st_size);

	return 0;
}
