#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef enum
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} Direction;

typedef struct
{
	int tileX;			// tile the player is standing on (or walking away from)
	int tileY;
	Direction facing;
	bool moving;
	float progress;		// 0..1 of the way to the next tile while moving
	Color hatColor;
} Player;

// Read WASD and walk the player tile by tile
void UpdatePlayer(Player *player, float dt);

// World space position of the center of the player, in pixels
Vector2 GetPlayerPosition(const Player *player);

void DrawPlayer(const Player *player);

#endif
