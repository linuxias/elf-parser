#include <stdlib.h>

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
static int _nr_item;
static MENU *_menu;
//static WINDOW *_menu_window;

static void __create_menu(void)
{
	int i;

	_nr_item = ARRAY_SIZE(_item_list);
	_items = (ITEM **)calloc(_nr_item, sizeof(ITEM *));

	for (i =0 ; i < _nr_item - 1; i++)
		_items[i] = new_item(_item_list[i], "");

	_menu = new_menu((ITEM **)_items);
	post_menu(_menu);
	refresh();
}

int tui_create(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	__create_menu();

	getch();
	return 0;
}

void tui_destroy(void)
{
	int i;

	if (_items) {
		for (i = 0; i < _nr_item - 1; i++)
		free_item(_items[i]);
	}

	if (_menu) {
		free_menu(_menu);
	}

	endwin();
}
