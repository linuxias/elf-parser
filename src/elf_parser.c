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

static void __print_elf_header(Elf64_Ehdr *ehdr)
{
	printf("ELF Header:\n");
	printf("  Magic : ");
	for (int i = 0; i < EI_NIDENT; i++) {
		printf("%02x ", ehdr->e_ident[i]);
	}
	printf("\n");
	printf("  Class:                                  %s\n",
			__get_elf_header_class(ehdr->e_ident[EI_CLASS]));
	printf("  Data:                                   %s\n",
			__get_elf_header_data(ehdr->e_ident[EI_DATA]));
	printf("  Version:                                %d(%s)\n",
			ehdr->e_ident[EI_VERSION], __get_elf_header_version(ehdr->e_ident[EI_VERSION]));
	printf("  OS/ABI:                                 %s\n",
			__get_elf_header_osabi(ehdr->e_ident[EI_OSABI]));
	printf("  ABI Version:                            %d\n",
			ehdr->e_ident[EI_ABIVERSION]);
	printf("  Type:                                   %s\n",
			__get_elf_header_type(ehdr->e_type));
	printf("  Machine:                                %s\n",
			__get_elf_header_machine(ehdr->e_machine));
	printf("  Version:                                0x%d\n",
			ehdr->e_version);
	printf("  Entry point address:                    0x%lx\n",
			ehdr->e_entry);
	printf("  Start of program headers:               %lu (bytes into file)\n",
			ehdr->e_phoff);
	printf("  Start of section headers:               %lu (bytes into file)\n",
			ehdr->e_shoff);
	printf("  Flags:                                  0x%x\n",
			ehdr->e_flags);
	printf("  Size of this headers:                   %d (byte)\n",
			ehdr->e_ehsize);
	printf("  Size of program headers:                %d (byte)\n",
			ehdr->e_phentsize);
	printf("  Number of program headers:              %d\n",
			ehdr->e_phnum);
	printf("  Size of section headers:                %d (byte)\n",
			ehdr->e_shentsize);
	printf("  Number of section headers:              %d\n",
			ehdr->e_shnum);
	printf("  Section header string table index:      %d\n",
			ehdr->e_shstrndx);
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
		case ELF_PARSER_ELF_ALL:
		case ELF_PARSER_MAX:
			break;
		default:
			break;
	}
}
