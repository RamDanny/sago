#ifndef STORAGE_H
#define STORAGE_H

#include "raylib.h"

#include "player.h"

// Everything the game remembers between runs lives in SQLite files in the storage folder:
//   save.db      the game in progress, replaced by New Game
//   settings.db  options that are kept across new games

// Find the storage folder (or create it) and make sure the settings file exists
void InitStorage(void);

// True if there is a saved game that Continue can load
bool HasSavedGame(void);

// Replace any saved game with a fresh file holding the given starting state
bool CreateNewGame(const Player *player);

// Write the player's state to the saved game. A player that is mid step is saved on the tile they are leaving.
bool SaveGame(const Player *player);

// Read the saved state into the player. Leaves the player untouched and returns false if there is no usable save.
bool LoadGame(Player *player);

Color LoadHatColor(Color fallback);
void SaveHatColor(Color color);

#endif
