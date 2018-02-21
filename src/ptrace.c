#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

Elf64_Addr lookup_symbol(elf_info_s info, const char *symname)
{
	int i, j;
	char *strtab = NULL;
	Elf64_Sym *symtab;

	for (i = 0; i < info->ehdr->e_shnum; i++) {
		if (info->shdr[i].sh_type == SHT_SYMTAB) {
			strtab = (char *)&info->mem[info->shdr[info->shdr[i].sh_link].sh_offset];
			symtab = (Elf64_Sym *)&info->mem[h->shdr[i].sh_offset];
			for (j = 0; j < info->shdr[i].sh_size/sizeof(Elf64_Sym); j++) {
				if (strcmp(&strtab[symtab->st_name], symname) == 0)
					return symtab->st_value;
				symtab++;
			}
		}
	}
	return 0;
}

