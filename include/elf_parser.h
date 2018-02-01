#ifndef __ELF_PARSER_H__
#define __ELF_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum elf_parser_header_type {
	ELF_PARSER_ELF_ALL = -1,
	ELF_PARSER_ELF_HEADER,
	ELF_PARSER_PROGRAM_HEADER,
	ELF_PARSER_SECTION_HEADER,
	ELF_PARSER_MAX
} elf_parser_header_type_e;

void elf_parser_print_header(const char *elf_file, elf_parser_header_type_e type);

#ifdef __cplusplus
}
#endif

#endif /* __ELF_PARSER_H__ */
