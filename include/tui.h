#ifndef __TUI_H__
#define __TUI_H__

typedef struct {
	int max_x;
	int max_y;
	int menu_x;
	int menu_y;
	int info_x;
	int info_y;
} tui_info;

typedef enum _tui_selection {
    TUI_SELECTION_NONE = -1,
    TUI_SELECTION_ELF,
    TUI_SELECTION_PROGRAM,
    TUI_SELECTION_SECTION,
    TUI_SELECTION_EXIT
} tui_selection_e;

int tui_create(void);

void tui_destroy(void);

int tui_get_selection(void);

#endif /* __TUI_H__ */
