#include "menu.h"

#define TITLE_SIZE 40
#define ITEM_SIZE 24
#define ITEM_SPACING 44
#define HINT_SIZE 16

// step the selection up (-1) or down (+1), wrapping around and skipping disabled items
static void MoveSelection(Menu *menu, int step)
{
	for (int i = 0; i < menu->count; i++)
	{
		menu->selected = (menu->selected + step + menu->count) % menu->count;
		if (!menu->items[menu->selected].disabled)
			return;
	}
}

int UpdateMenu(Menu *menu)
{
	// the selection may be sitting on an item that has since been disabled
	if (menu->items[menu->selected].disabled)
		MoveSelection(menu, 1);

	if (IsKeyPressed(KEY_W))
		MoveSelection(menu, -1);

	if (IsKeyPressed(KEY_S))
		MoveSelection(menu, 1);

	if (IsKeyPressed(KEY_ENTER) && !menu->items[menu->selected].disabled)
		return menu->selected;

	return -1;
}

static void DrawTextCentered(const char *text, int y, int size, Color color)
{
	DrawText(text, (GetScreenWidth() - MeasureText(text, size)) / 2, y, size, color);
}

void DrawMenu(const Menu *menu)
{
	int itemsHeight = menu->count * ITEM_SPACING;
	int y = (GetScreenHeight() - itemsHeight) / 2;

	DrawTextCentered(menu->title, y - TITLE_SIZE - 50, TITLE_SIZE, WHITE);

	for (int i = 0; i < menu->count; i++)
	{
		const MenuItem *item = &menu->items[i];

		if (item->disabled)
			DrawTextCentered(item->label, y, ITEM_SIZE, DARKGRAY);
		else if (i == menu->selected)
			DrawTextCentered(TextFormat("> %s <", item->label), y, ITEM_SIZE, YELLOW);
		else
			DrawTextCentered(item->label, y, ITEM_SIZE, LIGHTGRAY);

		y += ITEM_SPACING;
	}

	DrawTextCentered("W / S: move    Enter: select", GetScreenHeight() - 40, HINT_SIZE, GRAY);
}
