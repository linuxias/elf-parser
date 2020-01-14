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
static WINDOW *_info_win;
static tui_info _tui_info;

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

static void __create_menu(int x, int y)
{
	int i;

	_nr_item = ARRAY_SIZE(_item_list);
	_items = (ITEM **)calloc(_nr_item, sizeof(ITEM *));

	for (i = 0; i < _nr_item - 1; i++)
		_items[i] = new_item(_item_list[i], "");

	_menu = new_menu((ITEM **)_items);

	_menu_win = newwin(y, x, 0, 0);

	keypad(_menu_win, TRUE);

	set_menu_win(_menu, _menu_win);
	set_menu_sub(_menu, derwin(_menu_win, 6, 38, 3, 1));
	set_menu_format(_menu, _nr_item, 1);

	/* Set menu mark to the string " * " */
	set_menu_mark(_menu, "* ");

	box(_menu_win, 0, 0);
	__print_in_middle(_menu_win, 1, 0, x, "ELF Parser", COLOR_PAIR(1));
	mvwaddch(_menu_win, 2, 0, ACS_LTEE);
	mvwhline(_menu_win, 2, 1, ACS_HLINE, _tui_info.max_x/3 - 2);
	mvwaddch(_menu_win, 2, x, ACS_RTEE);
	refresh();

	post_menu(_menu);
	wrefresh(_menu_win);
}

static void __create_win(int x, int y)
{
	_info_win = newwin(LINES, x - 1, 0, _tui_info.max_x - x);
	box(_info_win, 0, 0);
	wrefresh(_info_win);
}

static void __update_tui_info(void)
{
	getmaxyx(stdscr, _tui_info.max_y, _tui_info.max_x);
	_tui_info.menu_x = _tui_info.max_x / 3;
	_tui_info.menu_y = _tui_info.max_y;
	_tui_info.info_x = (_tui_info.max_x / 3) * 2;
	_tui_info.info_y = _tui_info.max_y;
}

int tui_create(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	__update_tui_info();

	__create_menu(_tui_info.menu_x, _tui_info.menu_y);
	__create_win(_tui_info.info_x, _tui_info.info_y);

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

int tui_get_selection(void)
{
	int c;
    int index;
	while ((c = getch()) != KEY_F(1)) {
		switch(c) {
			case KEY_DOWN:
				menu_driver(_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(_menu, REQ_UP_ITEM);
				break;
			case 10:
				_cur_item = current_item(_menu);
                index = item_index(_cur_item);
                return index;
		}
		wrefresh(_menu_win);
	}
    return TUI_SELECTION_NONE;
}

int tui_print_str_list(SList *list)
{
    int i = 2;

    if (list == NULL)
        return -1;

    wmove(_info_win, 1, 1);
    wclrtobot(_info_win);
	box(_info_win, 0, 0);
    while (list) {
	    mvwprintw(_info_win, i, 1, "%s", (char *)s_list_get_data(list));
        list = list->next;
        i++;
    }
    wrefresh(_info_win);

    return 0;
}
