#include "player.h"

#include "world.h"

#include <math.h>

#define WALK_SPEED 4.0f		// tiles per second

#define BODY_HALF 14.0f		// half size of the black "hair" square
#define CAP_HALF 7.0f		// half size of the green cap square
#define ARM_HALF_LENGTH 4.0f	// half length of the arm side that touches the body
#define ARM_HALF_REACH 2.0f		// half of how far the arms stick out from the body

static const int moveKeys[] = { KEY_W, KEY_S, KEY_A, KEY_D };	// same order as Direction

// unit vector on screen for a direction (y points down)
static Vector2 DirectionVector(Direction dir)
{
	switch (dir)
	{
		case DIR_UP: return (Vector2){ 0, -1 };
		case DIR_DOWN: return (Vector2){ 0, 1 };
		case DIR_LEFT: return (Vector2){ -1, 0 };
		default: return (Vector2){ 1, 0 };
	}
}

// Find the direction the player is asking for, the most recently pressed key wins.
// Returns false if no movement key is held.
static bool ReadMoveInput(Direction *dir)
{
	static int latest = 0;

	for (int i = 0; i < 4; i++)
	{
		if (IsKeyPressed(moveKeys[i]))
			latest = i;
	}

	if (!IsKeyDown(moveKeys[latest]))
	{
		// the latest key was let go, fall back to any other key still held
		int held = -1;
		for (int i = 0; i < 4; i++)
		{
			if (IsKeyDown(moveKeys[i]))
				held = i;
		}

		if (held < 0)
			return false;

		latest = held;
	}

	*dir = (Direction)latest;
	return true;
}

void UpdatePlayer(Player *player, float dt)
{
	Direction wanted = player->facing;
	bool hasInput = ReadMoveInput(&wanted);

	if (!player->moving)
	{
		if (!hasInput)
			return;

		player->facing = wanted;
		player->moving = true;
		player->progress = 0;
	}

	player->progress += WALK_SPEED * dt;

	if (player->progress >= 1.0f)
	{
		// arrived on the next tile
		Vector2 step = DirectionVector(player->facing);
		player->tileX += (int)step.x;
		player->tileY += (int)step.y;

		if (hasInput)
		{
			// keep walking, carry the leftover progress over so the walk stays smooth
			player->facing = wanted;
			player->progress -= 1.0f;
			if (player->progress > 1.0f)
				player->progress = 1.0f;
		}
		else
		{
			player->moving = false;
			player->progress = 0;
		}
	}
}

Vector2 GetPlayerPosition(const Player *player)
{
	Vector2 step = DirectionVector(player->facing);

	return (Vector2){
		(player->tileX + 0.5f + step.x * player->progress) * TILE_SIZE,
		(player->tileY + 0.5f + step.y * player->progress) * TILE_SIZE
	};
}

static void DrawSquare(Vector2 center, float half, Color color)
{
	DrawRectangleV((Vector2){ center.x - half, center.y - half }, (Vector2){ half * 2, half * 2 }, color);
}

// point at "forward" pixels ahead of and "side" pixels to the right of the center
static Vector2 Offset(Vector2 center, Vector2 forward, Vector2 right, float ahead, float side)
{
	return (Vector2){
		center.x + forward.x * ahead + right.x * side,
		center.y + forward.y * ahead + right.y * side
	};
}

void DrawPlayer(const Player *player)
{
	Vector2 center = GetPlayerPosition(player);
	Vector2 forward = DirectionVector(player->facing);
	Vector2 right = { -forward.y, forward.x };

	// arms, one rectangle on each side of the body
	// the facing is always along an axis, so the rectangles just swap width and height when turning sideways
	Vector2 armHalfSize = {
		fabsf(forward.x) * ARM_HALF_LENGTH + fabsf(right.x) * ARM_HALF_REACH,
		fabsf(forward.y) * ARM_HALF_LENGTH + fabsf(right.y) * ARM_HALF_REACH
	};
	Vector2 armSize = { armHalfSize.x * 2, armHalfSize.y * 2 };

	for (int side = -1; side <= 1; side += 2)
	{
		Vector2 arm = Offset(center, forward, right, 0, side * (BODY_HALF + ARM_HALF_REACH));
		DrawRectangleV((Vector2){ arm.x - armHalfSize.x, arm.y - armHalfSize.y }, armSize, BROWN);
	}

	// hair
	DrawSquare(center, BODY_HALF, BLACK);

	// cap, a square with a triangle on its front side pointing where the player faces
	DrawSquare(center, CAP_HALF, player->hatColor);
	DrawTriangle(
		Offset(center, forward, right, BODY_HALF, 0),
		Offset(center, forward, right, CAP_HALF, -CAP_HALF),
		Offset(center, forward, right, CAP_HALF, CAP_HALF),
		player->hatColor);
}
