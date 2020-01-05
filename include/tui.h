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

int tui_create(void);

void tui_destroy(void);

#endif /* __TUI_H__ */
