#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef struct
{
	const char *label;
	bool disabled;		// disabled items are drawn dim and can't be selected
} MenuItem;

typedef struct
{
	const char *title;
	MenuItem *items;
	int count;
	int selected;
} Menu;

// Move the selection with W/S. Returns the index of the item chosen with Enter this frame, or -1.
int UpdateMenu(Menu *menu);

// Draw the menu centered on the screen
void DrawMenu(const Menu *menu);

#endif
