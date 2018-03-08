#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>

#include "elf_parser.h"
#include "ptrace_util.h"

#define INIT_PROCESS 1

ptrace_info_s ptrace_util_init_info(pid_t pid, void *addr, void *data)
{
	ptrace_info_s info = NULL;

	info = (ptrace_info_s)calloc(1, sizeof(struct ptrace_info));
	if (info == NULL)
		return NULL;

	info->pid = pid;
	info->addr = addr;
	info->data = data;

	return info;
}

void ptrace_util_destroy_info(ptrace_info_s info)
{
	if (info)
		free(info);
}

void *ptrace_util_get_addr(ptrace_info_s info)
{
	return info->addr;
}

void *ptrace_util_get_data(ptrace_info_s info)
{
	return info->data;
}

int ptrace_util_set_addr(ptrace_info_s info, void *addr)
{
	if (addr == NULL || info == NULL)
		return -1;

	info->addr = addr;
	return 0;
}

int ptrace_util_set_data(ptrace_info_s info, void *data)
{
	if (data == NULL || info == NULL)
		return -1;

	info->data = data;
	return 0;
}

int ptrace_util_attch(ptrace_info_s info)
{
	int ret;

	if (info->pid <= INIT_PROCESS) {
		printf("ERR");
		return -1;
	}

	ret = ptrace(PTRACE_ATTACH, info->pid, NULL, NULL);
	if (ret < 0) {
		perror("ptrace");
		return -1;
	}

	return 0;
}

Elf64_Addr lookup_symbol(elf_info_s info, const char *symname)
{
	int i, j;
	char *strtab = NULL;
	Elf64_Sym *symtab;

	for (i = 0; i < info->ehdr->e_shnum; i++) {
		if (info->shdr[i].sh_type == SHT_SYMTAB) {
			strtab = (char *)&info->mem[info->shdr[info->shdr[i].sh_link].sh_offset];
			symtab = (Elf64_Sym *)&info->mem[info->shdr[i].sh_offset];
			for (j = 0; j < info->shdr[i].sh_size/sizeof(Elf64_Sym); j++) {
				if (strcmp(&strtab[symtab->st_name], symname) == 0)
					return symtab->st_value;
				symtab++;
			}
		}
	}
	return 0;
}

