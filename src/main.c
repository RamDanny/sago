#include "raylib.h"

#include "menu.h"
#include "player.h"
#include "storage.h"
#include "world.h"

// 15x13 tiles, odd counts so the player's tile sits exactly in the middle
#define SCREEN_WIDTH (15 * TILE_SIZE)
#define SCREEN_HEIGHT (13 * TILE_SIZE)

typedef enum
{
	SCREEN_MAIN_MENU,
	SCREEN_SETTINGS,
	SCREEN_GAME,
	SCREEN_PAUSED
} Screen;

enum { MAIN_NEW_GAME, MAIN_CONTINUE, MAIN_SETTINGS, MAIN_QUIT, MAIN_COUNT };
enum { PAUSE_SAVE_AND_LEAVE, PAUSE_LEAVE, PAUSE_COUNT };
enum { HAT_RED, HAT_GREEN, HAT_BLUE, HAT_COUNT, SETTINGS_BACK = HAT_COUNT, SETTINGS_COUNT };

// the state a new game starts from
static Player NewPlayer(Color hatColor)
{
	Player player = { 0 };
	player.facing = DIR_DOWN;
	player.hatColor = hatColor;
	return player;
}

int main ()
{
	// Tell the window to use vsync
	SetConfigFlags(FLAG_VSYNC_HINT);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "sago");
	SetTargetFPS(60);

	// ESCAPE doesn't close the game, only the Quit menu option or the window's Close button do
	SetExitKey(KEY_NULL);

	InitStorage();
	const Color hatColors[HAT_COUNT] = { RED, GREEN, BLUE };
	Color hatColor = LoadHatColor(GREEN);

	MenuItem mainItems[MAIN_COUNT] = { { "New Game" }, { "Continue" }, { "Settings" }, { "Quit" } };
	Menu mainMenu = { "sago", mainItems, MAIN_COUNT, 0 };
	mainItems[MAIN_CONTINUE].disabled = !HasSavedGame();

	MenuItem settingsItems[SETTINGS_COUNT] = { { "Red" }, { "Green" }, { "Blue" }, { "Back" } };
	Menu settingsMenu = { "Hat colour", settingsItems, SETTINGS_COUNT, 0 };

	MenuItem pauseItems[PAUSE_COUNT] = { { "Save and Leave" }, { "Leave" } };
	Menu pauseMenu = { "Paused", pauseItems, PAUSE_COUNT, 0 };

	Screen screen = SCREEN_MAIN_MENU;
	Player player = NewPlayer(hatColor);

	// the camera follows the player, keeping them in the middle of the screen
	Camera2D camera = { 0 };
	camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
	camera.zoom = 1.0f;

	bool quit = false;

	// game loop
	while (!quit && !WindowShouldClose())		// run the loop until the user picks Quit or presses the Close button on the window
	{
		// update
		switch (screen)
		{
			case SCREEN_MAIN_MENU:
			{
				switch (UpdateMenu(&mainMenu))
				{
					case MAIN_NEW_GAME:
						player = NewPlayer(hatColor);
						CreateNewGame(&player);
						screen = SCREEN_GAME;
						break;

					case MAIN_CONTINUE:
						player = NewPlayer(hatColor);
						if (LoadGame(&player))
							screen = SCREEN_GAME;
						break;

					case MAIN_SETTINGS:
						settingsMenu.selected = 0;
						screen = SCREEN_SETTINGS;
						break;

					case MAIN_QUIT:
						quit = true;
						break;
				}
				break;
			}

			case SCREEN_SETTINGS:
			{
				int choice = UpdateMenu(&settingsMenu);

				if (choice == SETTINGS_BACK)
				{
					screen = SCREEN_MAIN_MENU;
				}
				else if (choice >= 0)
				{
					hatColor = hatColors[choice];
					SaveHatColor(hatColor);
				}
				break;
			}

			case SCREEN_GAME:
			{
				if (IsKeyPressed(KEY_P))
				{
					pauseMenu.selected = 0;
					screen = SCREEN_PAUSED;
				}
				else
				{
					UpdatePlayer(&player, GetFrameTime());
				}
				break;
			}

			case SCREEN_PAUSED:
			{
				if (IsKeyPressed(KEY_P))
				{
					screen = SCREEN_GAME;
					break;
				}

				int choice = UpdateMenu(&pauseMenu);

				// leaving without saving just drops what's in memory, Continue will read the last save again
				if (choice == PAUSE_SAVE_AND_LEAVE)
					SaveGame(&player);

				if (choice >= 0)
				{
					mainItems[MAIN_CONTINUE].disabled = !HasSavedGame();
					screen = SCREEN_MAIN_MENU;
				}
				break;
			}
		}

		// drawing
		BeginDrawing();
		ClearBackground(BLACK);

		if (screen == SCREEN_GAME || screen == SCREEN_PAUSED)
		{
			camera.target = GetPlayerPosition(&player);

			BeginMode2D(camera);
			DrawGround(camera);
			DrawPlayer(&player);
			EndMode2D();
		}

		switch (screen)
		{
			case SCREEN_MAIN_MENU:
				DrawMenu(&mainMenu);
				break;

			case SCREEN_SETTINGS:
				DrawMenu(&settingsMenu);

				// show the colour that is currently picked
				DrawText("Current", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 130, 20, LIGHTGRAY);
				DrawRectangle(SCREEN_WIDTH / 2 + 30, SCREEN_HEIGHT / 2 + 126, 28, 28, hatColor);
				break;

			case SCREEN_PAUSED:
				// dim the game behind the menu
				DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
				DrawMenu(&pauseMenu);
				DrawText("P: resume", 12, 12, 16, GRAY);
				break;

			case SCREEN_GAME:
				break;
		}

		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
