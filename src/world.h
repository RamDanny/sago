#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"

#define TILE_SIZE 48	// size of one map tile in pixels

// The kinds of tile art we can draw, all generated in code
typedef enum
{
	TILE_INNER_FLOOR_BUILDING	// floor inside a building
} TileType;

// Draw one tile of the given type at a tile position
void DrawTile(TileType type, int tx, int ty);

// Draw the ground tiles that are visible through the camera
void DrawGround(Camera2D camera);

#endif
