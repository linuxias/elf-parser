#include <ncurses.h>

#include "tui.h"

int tui_create(void)
{
	initscr();

	return 0;
}

void tui_destroy(void)
{
	endwin();
}
