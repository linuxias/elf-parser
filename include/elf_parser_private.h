#ifndef __ELF_PARSER_PRIVATE_H__
#define __ELF_PARSER_PRIVATE_H__

#include <sys/user.h>
#include <sys/types.h>
#include <elf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct elf_info *elf_info_s;

struct elf_info {
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;
	uint8_t *mem;
	char *symname;
	struct user_regs_struct pt_reg;
	char *exec;
} elf_info;

struct elf_info_32 {
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	Elf32_Shdr *shdr;
	uint8_t *mem;
	char *symname;
	struct user_regs_struct pt_reg;
	char *exec;
}

#define EHDR_DATA_LSB "2's complement, little endian"
#define EHDR_DATA_MSB "2's complement, big endian"

/* Values for e_ident[EI_OSABI]. */
#define	ELFOSABI_NONE		0	/* UNIX System V ABI */
#define	ELFOSABI_HPUX		1	/* HP-UX operating system */
#define	ELFOSABI_NETBSD		2	/* NetBSD */
//#define	ELFOSABI_LINUX		3	/* GNU/Linux */
#define	ELFOSABI_HURD		4	/* GNU/Hurd */
#define	ELFOSABI_86OPEN		5	/* 86Open common IA32 ABI */
#define	ELFOSABI_SOLARIS	6	/* Solaris */
#define	ELFOSABI_AIX		7	/* AIX */
#define	ELFOSABI_IRIX		8	/* IRIX */
#define	ELFOSABI_FREEBSD	9	/* FreeBSD */
#define	ELFOSABI_TRU64		10	/* TRU64 UNIX */
#define	ELFOSABI_MODESTO	11	/* Novell Modesto */
#define	ELFOSABI_OPENBSD	12	/* OpenBSD */
#define	ELFOSABI_OPENVMS	13	/* Open VMS */
#define	ELFOSABI_NSK		14	/* HP Non-Stop Kernel */
#define	ELFOSABI_AROS		15	/* Amiga Research OS */
#define	ELFOSABI_FENIXOS	16	/* FenixOS */
#define	ELFOSABI_CLOUDABI	17	/* Nuxi CloudABI */
#define	ELFOSABI_ARM_AEABI	64	/* ARM EABI */
#define	ELFOSABI_ARM		97	/* ARM */
#define	ELFOSABI_STANDALONE	255	/* Standalone (embedded) application */

#ifdef __cplusplus
}
#endif
#endif /* __ELF_PARSER_PRIVATE_H__ */
