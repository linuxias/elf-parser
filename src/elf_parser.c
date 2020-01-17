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
#include "elf_parser_private.h"
#include "s_list.h"

#define BUF_LEN 256

static int is_elf_file(elf_h handle)
{
#define FIRST_MAGIC_STRING 0x7f
#define ELF_STRING "ELF"

	if (handle->mem[0] != FIRST_MAGIC_STRING && strcmp((char *)(&handle->mem[1]), ELF_STRING)) {
		fprintf(stderr, "Not an ELF");
		return -1;
	}

	return 0;
}

static const char *__get_elf_header_class(unsigned int class)
{
	static char buf[32];

	switch(class) {
		case ELFCLASSNONE:	return "none";
		case ELFCLASS32:	return "ELF32";
		case ELFCLASS64:	return "ELF64";
		default:
			snprintf(buf, sizeof(buf), "<unknown: %x>", class);
			return buf;
	}
}

static const char *__get_elf_header_data(unsigned int data)
{
	static char buf[32];

	switch(data) {
		case ELFDATANONE:	return "none";
		case ELFDATA2LSB:	return EHDR_DATA_LSB;
		case ELFDATA2MSB:	return EHDR_DATA_MSB;
		default:
			snprintf(buf, sizeof(buf), "<unknown: %x>", data);
			return buf;
	}
}

static const char *__get_elf_header_version(unsigned int version)
{
	static char buf[32];

	switch(version) {
		case EV_CURRENT:	return "current";
		default:
			snprintf(buf, sizeof(buf), "<unknown: %x>", version);
			return buf;
	}
}

static const char *__get_elf_header_osabi(unsigned int osabi)
{
	static char buf[32];

	switch(osabi) {
		case ELFOSABI_NONE:			return "UNIX - System V";
		case ELFOSABI_HPUX:			return "UNIX - HP-UX";
		case ELFOSABI_NETBSD:		return "UNIX - NetBSD";
		case ELFOSABI_LINUX:		return "UNIX - Linux";
		case ELFOSABI_HURD:			return "GNU/Hurd";
		case ELFOSABI_SOLARIS:		return "UNIX - Solaris";
		case ELFOSABI_AIX:			return "UNIX - AIX";
		case ELFOSABI_IRIX:			return "UNIX - IRIX";
		case ELFOSABI_FREEBSD:		return "UNIX - FreeBSD";
		case ELFOSABI_TRU64:		return "UNIX - TRU64";
		case ELFOSABI_MODESTO:		return "Novell - Modesto";
		case ELFOSABI_OPENBSD:		return "UNIX - OpenBSD";
		case ELFOSABI_OPENVMS:		return "VMS - OpenVMS";
		case ELFOSABI_NSK:			return "HP - Non-Stop Kernel";
		case ELFOSABI_AROS:			return "AROS";
		case ELFOSABI_STANDALONE:	return "Standalone App";
		case ELFOSABI_ARM:			return "ARM";
		default:
			snprintf (buf, sizeof (buf), "<unknown: %x>", osabi);
			return buf;
	}
}

static const char *__get_elf_header_type(unsigned int type)
{
	static char buf[32];

	switch(type) {
		case ET_NONE:	return "NONE (None)";
		case ET_REL:	return "REL (Relocatable file)";
		case ET_EXEC:	return "EXEC (Executable file)";
		case ET_DYN:	return "DYN (Shared object file)";
		case ET_CORE:	return "CORE (core file)";
		default:
			if (type >= ET_LOOS && type <= ET_HIOS)
				snprintf(buf, sizeof(buf), "OS-specific <%x>", type);
			else if (type >=ET_LOPROC && type <= ET_HIPROC)
				snprintf(buf, sizeof(buf), "Processor-specific <%x>", type);
			else
				snprintf(buf, sizeof(buf),  "unknown <%x>", type);
			return buf;
	}
}

static const char *__get_elf_header_machine(unsigned int machine)
{
	static char buf[64]; /* XXX */

	switch (machine)
	{
		case EM_NONE:		return "None";
		case EM_M32:		return "WE32100";
		case EM_SPARC:		return "Sparc";
		case EM_386:		return "Intel 80386";
		case EM_68K:		return "MC68000";
		case EM_88K:		return "MC88000";
		case EM_860:		return "Intel 80860";
		case EM_MIPS:		return "MIPS R3000";
		case EM_S370:		return "IBM System/370";
		case EM_MIPS_RS3_LE:	return "MIPS R4000 big-endian";
		case EM_PARISC:		return "HPPA";
		case EM_SPARC32PLUS:	return "Sparc v8+" ;
		case EM_960:		return "Intel 90860";
		case EM_PPC:		return "PowerPC";
		case EM_PPC64:		return "PowerPC64";
		case EM_V800:		return "NEC V800";
		case EM_FR20:		return "Fujitsu FR20";
		case EM_RH32:		return "TRW RH32";
		case EM_ARM:		return "ARM";
		case EM_AARCH64:		return "AArch64";
		case EM_SH:			return "Renesas / SuperH SH";
		case EM_SPARCV9:		return "Sparc v9";
		case EM_TRICORE:		return "Siemens Tricore";
		case EM_ARC:		return "ARC";
		case EM_H8_300:		return "Renesas H8/300";
		case EM_H8_300H:		return "Renesas H8/300H";
		case EM_H8S:		return "Renesas H8S";
		case EM_H8_500:		return "Renesas H8/500";
		case EM_IA_64:		return "Intel IA-64";
		case EM_MIPS_X:		return "Stanford MIPS-X";
		case EM_COLDFIRE:		return "Motorola Coldfire";
		case EM_68HC12:		return "Motorola M68HC12";
		case EM_D10V:		return "d10v";
		case EM_D30V:		return "d30v";
		case EM_M32R:		return "Renesas M32R (formerly Mitsubishi M32r)";
		case EM_V850:		return "NEC v850";
		case EM_MN10300:		return "mn10300";
		case EM_MN10200:		return "mn10200";
		case EM_FR30:		return "Fujitsu FR30";
		case EM_PJ:			return "picoJava";
		case EM_MMA:		return "Fujitsu Multimedia Accelerator";
		case EM_PCP:		return "Siemens PCP";
		case EM_NCPU:		return "Sony nCPU embedded RISC processor";
		case EM_NDR1:		return "Denso NDR1 microprocesspr";
		case EM_STARCORE:		return "Motorola Star*Core processor";
		case EM_ME16:		return "Toyota ME16 processor";
		case EM_ST100:		return "STMicroelectronics ST100 processor";
		case EM_TINYJ:		return "Advanced Logic Corp. TinyJ embedded processor";
		case EM_FX66:		return "Siemens FX66 microcontroller";
		case EM_ST9PLUS:		return "STMicroelectronics ST9+ 8/16 bit microcontroller";
		case EM_ST7:		return "STMicroelectronics ST7 8-bit microcontroller";
		case EM_68HC16:		return "Motorola MC68HC16 Microcontroller";
		case EM_68HC11:		return "Motorola MC68HC11 Microcontroller";
		case EM_68HC08:		return "Motorola MC68HC08 Microcontroller";
		case EM_68HC05:		return "Motorola MC68HC05 Microcontroller";
		case EM_SVX:		return "Silicon Graphics SVx";
		case EM_ST19:		return "STMicroelectronics ST19 8-bit microcontroller";
		case EM_VAX:		return "Digital VAX";
		case EM_AVR:		return "Atmel AVR 8-bit microcontroller";
		case EM_CRIS:		return "Axis Communications 32-bit embedded processor";
		case EM_JAVELIN:		return "Infineon Technologies 32-bit embedded cpu";
		case EM_FIREPATH:		return "Element 14 64-bit DSP processor";
		case EM_ZSP:		return "LSI Logic's 16-bit DSP processor";
		case EM_MMIX:		return "Donald Knuth's educational 64-bit processor";
		case EM_HUANY:		return "Harvard Universitys's machine-independent object format";
		case EM_PRISM:		return "Vitesse Prism";
		case EM_X86_64:		return "Advanced Micro Devices X86-64";
		case EM_S390:		return "IBM S/390";
		case EM_OPENRISC:	return "OpenRISC";
		case EM_XTENSA:		return "Tensilica Xtensa Processor";
		case EM_ALTERA_NIOS2:	return "Altera Nios II";
		case EM_MICROBLAZE:		return "Xilinx MicroBlaze";
		default:
			snprintf (buf, sizeof (buf), "<unknown>: 0x%x", machine);
			return buf;
	}
}

static SList* __print_elf64_header(elf_h handle)
{
    char buf[BUF_LEN];
	elf64_info_s info = handle->elf64;
	Elf64_Ehdr *ehdr = info->ehdr;
    SList *list = NULL;

//	printf("ELF Header:\n");
//	printf("  Magic : ");
	for (int i = 0; i < EI_NIDENT; i++) {
//		printf("%02x ", ehdr->e_ident[i]);
	}

	snprintf(buf, BUF_LEN, "  Class:                                  %s",
			__get_elf_header_class(ehdr->e_ident[EI_CLASS]));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Data:                                   %s",
			__get_elf_header_data(ehdr->e_ident[EI_DATA]));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Version:                                %d(%s)",
			ehdr->e_ident[EI_VERSION], __get_elf_header_version(ehdr->e_ident[EI_VERSION]));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  OS/ABI:                                 %s",
			__get_elf_header_osabi(ehdr->e_ident[EI_OSABI]));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  ABI Version:                            %d",
			ehdr->e_ident[EI_ABIVERSION]);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Type:                                   %s",
			__get_elf_header_type(ehdr->e_type));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Machine:                                %s",
			__get_elf_header_machine(ehdr->e_machine));
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Version:                                0x%d",
			ehdr->e_version);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Entry point address:                    0x%x",
			(unsigned int)ehdr->e_entry);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Start of program headers:               %u (bytes into file)",
			(unsigned int)ehdr->e_phoff);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Start of section headers:               %u (bytes into file)",
			(unsigned int)ehdr->e_shoff);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Flags:                                  0x%x",
			(unsigned int)ehdr->e_flags);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Size of this headers:                   %d (byte)",
			ehdr->e_ehsize);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Size of program headers:                %d (byte)",
			ehdr->e_phentsize);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Number of program headers:              %d",
			ehdr->e_phnum);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Size of section headers:                %d (byte)",
			ehdr->e_shentsize);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Number of section headers:              %d",
			ehdr->e_shnum);
    list = s_list_append(list, strdup(buf));

	snprintf(buf, BUF_LEN, "  Section header string table index:      %d",
			ehdr->e_shstrndx);
    list = s_list_append(list, strdup(buf));

    return list;
}

static SList* __print_elf32_header(elf_h handle)
{
    SList *list = NULL;
    char buf[BUF_LEN];
	elf32_info_s info = handle->elf32;
	Elf32_Ehdr *ehdr = info->ehdr;

	list = s_list_append(list, strdup("ELF Header:\n"));
	list = s_list_append(list, strdup("  Magic : "));
	for (int i = 0; i < EI_NIDENT; i++) {
		printf("%02x ", ehdr->e_ident[i]);
	}
	snprintf(buf, BUF_LEN, "  Class:                                  %s",
			__get_elf_header_class(ehdr->e_ident[EI_CLASS]));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Data:                                   %s",
			__get_elf_header_data(ehdr->e_ident[EI_DATA]));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Version:                                %d(%s)",
			ehdr->e_ident[EI_VERSION], __get_elf_header_version(ehdr->e_ident[EI_VERSION]));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  OS/ABI:                                 %s",
			__get_elf_header_osabi(ehdr->e_ident[EI_OSABI]));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  ABI Version:                            %d",
			ehdr->e_ident[EI_ABIVERSION]);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Type:                                   %s",
			__get_elf_header_type(ehdr->e_type));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Machine:                                %s",
			__get_elf_header_machine(ehdr->e_machine));
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Version:                                0x%d",
			ehdr->e_version);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Entry point address:                    0x%x",
			ehdr->e_entry);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Start of program headers:               %u (bytes into file)",
			ehdr->e_phoff);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Start of section headers:               %u (bytes into file)",
			ehdr->e_shoff);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Flags:                                  0x%x",
			ehdr->e_flags);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Size of this headers:                   %d (byte)",
			ehdr->e_ehsize);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Size of program headers:                %d (byte)",
			ehdr->e_phentsize);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Number of program headers:              %d\n",
			ehdr->e_phnum);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Size of section headers:                %d (byte)",
			ehdr->e_shentsize);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Number of section headers:              %d",
			ehdr->e_shnum);
    list = s_list_append(list, strdup(buf));
	snprintf(buf, BUF_LEN, "  Section header string table index:      %d",
			ehdr->e_shstrndx);
    list = s_list_append(list, strdup(buf));

    return list;
}

static SList* __print_section_header_64_bit(elf_h handle)
{
	int i;
	unsigned char *string_table;
	elf64_info_s info = handle->elf64;
	Elf64_Ehdr *ehdr = info->ehdr;
	Elf64_Shdr *shdr = info->shdr;
    SList* list = NULL;
    char buf[BUF_LEN];

	string_table = &handle->mem[shdr[ehdr->e_shstrndx].sh_offset];
	snprintf(buf, BUF_LEN, "           [Name]            [Type]          [Address]          [Offset]          [Size]");
    list = s_list_append(list, strdup(buf));
	for (i = 0; i < ehdr->e_shnum; i++) {
		snprintf(buf, BUF_LEN, "  %20s  %016u  %016ld  %016ld  %8ld",
                &string_table[shdr[i].sh_name],
                shdr[i].sh_type,
                shdr[i].sh_addr,
                shdr[i].sh_offset,
                shdr[i].sh_size);
        list = s_list_append(list, strdup(buf));
    }

    return list;
}

static SList* __print_section_header_32_bit(elf_h handle)
{
	int i;
	unsigned char *string_table;
	elf32_info_s info = handle->elf32;
	Elf32_Ehdr *ehdr = info->ehdr;
	Elf32_Shdr *shdr = info->shdr;
    SList* list = NULL;
    char buf[BUF_LEN];

	string_table = &handle->mem[shdr[ehdr->e_shstrndx].sh_offset];
	snprintf(buf, BUF_LEN, "           [Name]            [Type]          [Address]          [Offset]          [Size]");
    list = s_list_append(list, strdup(buf));
	for (i = 0; i < ehdr->e_shnum; i++) {
		snprintf(buf, BUF_LEN, "  %20s  %016u  %016d  %016d  %8d",
                &string_table[shdr[i].sh_name],
                shdr[i].sh_type,
                shdr[i].sh_addr,
                shdr[i].sh_offset,
                shdr[i].sh_size);
        list = s_list_append(list, strdup(buf));
    }

    return list;
}

static SList* __print_program_header_64_bit(elf_h handle)
{
    SList *list = NULL;
    char buf[BUF_LEN];
	char *interp;
	int i;
    elf64_info_s info = handle->elf64;
    Elf64_Ehdr *ehdr = info->ehdr;
    Elf64_Phdr *phdr = info->phdr;

    snprintf(buf, BUF_LEN, " Program Header :");
    list = s_list_append(list, strdup(buf));
    for (i = 0; i < ehdr->e_phnum; i++) {
        switch(phdr[i].p_type) {
            case PT_LOAD :
                if (phdr[i].p_offset == 0)
                    snprintf(buf, BUF_LEN, "  Text segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                else
                    snprintf(buf, BUF_LEN, "  Data segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_INTERP :
                interp = strdup((char *)(&handle->mem[phdr[i].p_offset]));
                snprintf(buf, BUF_LEN, "  Interpreter : %s", interp);
                list = s_list_append(list, strdup(buf));
                free(interp);
                break;
            case PT_NOTE :
                snprintf(buf, BUF_LEN, "  Note segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_DYNAMIC:
                snprintf(buf, BUF_LEN, "  Dynamic segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_PHDR:
                snprintf(buf, BUF_LEN, "  Phdr segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
        }
    }

    return list;
}

static SList* __print_program_header_32_bit(elf_h handle)
{
    SList *list = NULL;
    char buf[BUF_LEN];
	char *interp;
	int i;
	elf32_info_s info = handle->elf32;
	Elf32_Ehdr *ehdr = info->ehdr;
	Elf32_Phdr *phdr = info->phdr;

    snprintf(buf, BUF_LEN, " Program Header :");
    list = s_list_append(list, strdup(buf));
    for (i = 0; i < ehdr->e_phnum; i++) {
        switch(phdr[i].p_type) {
            case PT_LOAD :
                if (phdr[i].p_offset == 0)
                    snprintf(buf, BUF_LEN, "  Text segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                else
                    snprintf(buf, BUF_LEN, "  Data segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_INTERP :
                interp = strdup((char *)(&handle->mem[phdr[i].p_offset]));
                snprintf(buf, BUF_LEN, "  Interpreter : %s", interp);
                list = s_list_append(list, strdup(buf));
                free(interp);
                break;
            case PT_NOTE :
                snprintf(buf, BUF_LEN, "  Note segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_DYNAMIC:
                snprintf(buf, BUF_LEN, "  Dynamic segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
            case PT_PHDR:
                snprintf(buf, BUF_LEN, "  Phdr segment : 0x%x", (unsigned int)phdr[i].p_vaddr);
                list = s_list_append(list, strdup(buf));
                break;
        }
    }

    return list;
}

static uint8_t *__get_mapped_mem(int fd)
{
	struct stat st;
	uint8_t *mem;

	errno = 0;
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		exit(-1);
	}

	mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	return mem;
}

static void __set_elf_info(int fd, elf_h *handle)
{
	elf_h h = *handle;
	char buf[EI_NIDENT];
	elf64_info_s elf64;
	elf32_info_s elf32;
	ssize_t size;

	size = read(fd, buf, EI_NIDENT);
	if (size == 0 || size < EI_NIDENT) {
		perror("read");
		exit(-1);
	}

	if (buf[EI_CLASS] == ELF32) {
		elf32 = (void *)calloc(1, sizeof(struct elf32_info));
		if (elf32 == NULL) {
			fprintf(stderr, "OOM");
			exit(-1);
		}
		elf32->ehdr = (Elf32_Ehdr *)h->mem;
		elf32->phdr = (Elf32_Phdr *)&(h->mem[elf32->ehdr->e_phoff]);
		elf32->shdr = (Elf32_Shdr *)&(h->mem[elf32->ehdr->e_shoff]);
		h->elf32 = elf32;
	} else if (buf[EI_CLASS] == ELF64) {
		elf64 = (void *)calloc(1, sizeof(struct elf64_info));
		if (elf64 == NULL) {
			fprintf(stderr, "OOM");
			exit(-1);
		}
		elf64->ehdr = (Elf64_Ehdr *)h->mem;
		elf64->phdr = (Elf64_Phdr *)&(h->mem[elf64->ehdr->e_phoff]);
		elf64->shdr = (Elf64_Shdr *)&(h->mem[elf64->ehdr->e_shoff]);
		h->elf64 = elf64;
	} else {
		fprintf(stderr, "ELF Class NONE");
		exit(-1);
	}
	h->arch = buf[EI_CLASS];
}

SList* elf_parser_get_header_info(elf_h handle, elf_parser_header_type_e type)
{
    SList *list = NULL;
	int is_32bit = handle->arch == ELF32 ? 1 : 0;

	switch(type) {
		case ELF_PARSER_ELF_HEADER:
			if (is_32bit)
				list = __print_elf32_header(handle);
			else
				list = __print_elf64_header(handle);
			break;
		case ELF_PARSER_PROGRAM_HEADER:
			if (is_32bit)
				list = __print_program_header_32_bit(handle);
			else
				list = __print_program_header_64_bit(handle);
			break;
		case ELF_PARSER_SECTION_HEADER:
			if (is_32bit)
				list = __print_section_header_32_bit(handle);
			else
				list = __print_section_header_64_bit(handle);
			break;
		case ELF_PARSER_ELF_ALL:
		case ELF_PARSER_MAX:
			break;
		default:
			break;
	}

    return list;
}

void destroy_elf_info(elf_h handle)
{
	if (handle) {
		if (handle->filename)
			free(handle->filename);
		if (handle->elf64)
			free(handle->elf64);
		if (handle->elf32)
			free(handle->elf32);
		free(handle);
	}
}

elf_h init_elf_info(const char *elf_file)
{
	int fd;
	elf_h handle = NULL;

	handle = (elf_h)calloc(1, sizeof(struct elf_info));
	if (handle == NULL) {
		fprintf(stderr, "OOM");
		exit(-1);
	}

	errno = 0;
	fd = open(elf_file, O_RDONLY);
	if (errno < 0 || fd == -1) {
		perror("open");
		exit(-1);
	}

	handle->filename = strdup(elf_file);
	handle->mem = __get_mapped_mem(fd);
	__set_elf_info(fd, &handle);

	if (is_elf_file(handle) < 0) {
		fprintf(stderr, "%s is not ELF file", elf_file);
		destroy_elf_info(handle);
		exit(-1);
	}

	close(fd);

	return handle;
}
