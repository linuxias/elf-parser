#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include "tui.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static char *_item_list[] = {
	"ELF header",
	"Program header",
	"Section header",
	"Exit",
	(char *)NULL
};

static ITEM **_items;
static ITEM *_cur_item;
static int _nr_item;
static MENU *_menu;
static WINDOW *_menu_win;

static void __print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;

	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

static void __create_menu(void)
{
	int i;

	_nr_item = ARRAY_SIZE(_item_list);
	_items = (ITEM **)calloc(_nr_item, sizeof(ITEM *));

	for (i =0 ; i < _nr_item - 1; i++)
		_items[i] = new_item(_item_list[i], "");

	_menu = new_menu((ITEM **)_items);

	_menu_win = newwin(LINES - 2, COLS/2, 0, 0);

	keypad(_menu_win, TRUE);

	set_menu_win(_menu, _menu_win);
	set_menu_sub(_menu, derwin(_menu_win, 6, 38, 3, 1));
	set_menu_format(_menu, _nr_item, 1);

	/* Set menu mark to the string " * " */
	set_menu_mark(_menu, "* ");

	box(_menu_win, 0, 0);
	__print_in_middle(_menu_win, 1, 0, COLS/2, "ELF Parser", COLOR_PAIR(1));
	mvwaddch(_menu_win, 2, 0, ACS_LTEE);
	mvwhline(_menu_win, 2, 1, ACS_HLINE, COLS/2 - 2);
	mvwaddch(_menu_win, 2, COLS/2, ACS_RTEE);
	refresh();

	post_menu(_menu);
	wrefresh(_menu_win);
}

int tui_create(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	__create_menu();

	int c;
	while ((c = getch()) != KEY_F(1)) {
		switch(c)
		{   case KEY_DOWN:
			menu_driver(_menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
			menu_driver(_menu, REQ_UP_ITEM);
			break;
			case 10:
			_cur_item = current_item(_menu);
			if (strcmp(item_name(_cur_item), "Exit") == 0)
				return 0;
			break;
		}
		wrefresh(_menu_win);
	}

	getch();
	return 0;
}

void tui_destroy(void)
{
	int i;

	if (_items)
		for (i = 0; i < _nr_item - 1; i++)
			free_item(_items[i]);

	if (_menu) {
		unpost_menu(_menu);
		free_menu(_menu);
	}

	endwin();
}
