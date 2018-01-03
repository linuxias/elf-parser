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

	if (mem[0] != FIRST_MAGIC_STRING && strcmp(&mem[1], ELF_STRING)) {
		fprintf(stderr, "Not an ELF");
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
		printf("%s is not ELF file", argv[1]);
		exit(-1);
	}

	close(fd);
	munmap(mem, st.st_size);

	return 0;
}
