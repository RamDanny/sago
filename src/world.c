#include "world.h"

#include <math.h>

// A gray square with a circle inside it, both bordered black
static void DrawInnerFloorBuilding(int x, int y)
{
	int centerX = x + TILE_SIZE / 2;
	int centerY = y + TILE_SIZE / 2;
	float radius = TILE_SIZE * 0.3f;

	DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, GRAY);
	DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, TILE_SIZE, TILE_SIZE }, 1, BLACK);

	// black circle with a smaller one on top gives us a bordered circle
	DrawCircle(centerX, centerY, radius, BLACK);
	DrawCircle(centerX, centerY, radius - 2, LIGHTGRAY);
}

void DrawTile(TileType type, int tx, int ty)
{
	int x = tx * TILE_SIZE;
	int y = ty * TILE_SIZE;

	switch (type)
	{
		case TILE_INNER_FLOOR_BUILDING: DrawInnerFloorBuilding(x, y); break;
	}
}

void DrawGround(Camera2D camera)
{
	// world space edges of the screen
	float left = camera.target.x - camera.offset.x / camera.zoom;
	float top = camera.target.y - camera.offset.y / camera.zoom;
	float right = left + GetScreenWidth() / camera.zoom;
	float bottom = top + GetScreenHeight() / camera.zoom;

	int minX = (int)floorf(left / TILE_SIZE);
	int minY = (int)floorf(top / TILE_SIZE);
	int maxX = (int)floorf(right / TILE_SIZE);
	int maxY = (int)floorf(bottom / TILE_SIZE);

	for (int ty = minY; ty <= maxY; ty++)
	{
		for (int tx = minX; tx <= maxX; tx++)
			DrawTile(TILE_INNER_FLOOR_BUILDING, tx, ty);
	}
}
