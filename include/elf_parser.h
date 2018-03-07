#ifndef __ELF_PARSER_H__
#define __ELF_PARSER_H__

#include "elf_parser_private.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _elf_parser_header_type {
	ELF_PARSER_ELF_ALL = -1,
	ELF_PARSER_ELF_HEADER,
	ELF_PARSER_PROGRAM_HEADER,
	ELF_PARSER_SECTION_HEADER,
	ELF_PARSER_MAX
} elf_parser_header_type_e;

void elf_parser_print_header(elf_info_s info, elf_parser_header_type_e type);

elf_info_s init_elf_info(const char *elf_file);

void destroy_elf_info(elf_info_s info);

#ifdef __cplusplus
}
#endif
#endif /* __ELF_PARSER_H__ */
