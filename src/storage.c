#include "storage.h"

#include "external/sqlite3/sqlite3.h"

#include <stdio.h>
#include <string.h>

#define STORAGE_FOLDER "storage"

static char savePath[1024];
static char settingsPath[1024];

static const char *facingNames[] = { "up", "down", "left", "right" };	// same order as Direction

// Run one or more SQL statements that return no rows
static bool Exec(sqlite3 *db, const char *sql)
{
	char *error = NULL;
	if (sqlite3_exec(db, sql, NULL, NULL, &error) != SQLITE_OK)
	{
		TraceLog(LOG_WARNING, "STORAGE: %s", error ? error : "unknown SQL error");
		sqlite3_free(error);
		return false;
	}
	return true;
}

static sqlite3 *OpenDatabase(const char *path, int flags)
{
	sqlite3 *db = NULL;
	if (sqlite3_open_v2(path, &db, flags, NULL) != SQLITE_OK)
	{
		TraceLog(LOG_WARNING, "STORAGE: Failed to open %s: %s", path, sqlite3_errmsg(db));
		sqlite3_close(db);
		return NULL;
	}
	return db;
}

void InitStorage(void)
{
	// Look for the storage folder next to the executable and a few folders up,
	// so a build in bin/Debug uses the one in the project folder
	const char *appDir = GetApplicationDirectory();
	const char *storageDir = NULL;
	const char *candidates[] = { "%s" STORAGE_FOLDER, "%s../" STORAGE_FOLDER, "%s../../" STORAGE_FOLDER };

	for (int i = 0; i < 3 && storageDir == NULL; i++)
	{
		const char *candidate = TextFormat(candidates[i], appDir);
		if (DirectoryExists(candidate))
			storageDir = candidate;
	}

	if (storageDir == NULL)
	{
		storageDir = TextFormat(candidates[0], appDir);
		MakeDirectory(storageDir);
	}

	snprintf(savePath, sizeof(savePath), "%s/save.db", storageDir);
	snprintf(settingsPath, sizeof(settingsPath), "%s/settings.db", storageDir);
	TraceLog(LOG_INFO, "STORAGE: Using folder %s", storageDir);

	sqlite3 *db = OpenDatabase(settingsPath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	if (db == NULL)
		return;

	Exec(db, "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value INTEGER NOT NULL);");
	sqlite3_close(db);
}

static bool WritePlayer(sqlite3 *db, const Player *player)
{
	return Exec(db, TextFormat(
		"BEGIN;"
		"INSERT OR REPLACE INTO player (id, tile_x, tile_y, facing) VALUES (1, %d, %d, '%s');"
		"UPDATE save_info SET saved_at = datetime('now') WHERE id = 1;"
		"COMMIT;",
		player->tileX, player->tileY, facingNames[player->facing]));
}

bool HasSavedGame(void)
{
	Player ignored = { 0 };
	return LoadGame(&ignored);
}

bool CreateNewGame(const Player *player)
{
	if (FileExists(savePath) && remove(savePath) != 0)
	{
		TraceLog(LOG_WARNING, "STORAGE: Failed to remove the old save %s", savePath);
		return false;
	}

	sqlite3 *db = OpenDatabase(savePath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	if (db == NULL)
		return false;

	bool ok = Exec(db,
		"CREATE TABLE player ("
		"	id INTEGER PRIMARY KEY CHECK (id = 1),"
		"	tile_x INTEGER NOT NULL,"
		"	tile_y INTEGER NOT NULL,"
		"	facing TEXT NOT NULL);"
		"CREATE TABLE save_info ("
		"	id INTEGER PRIMARY KEY CHECK (id = 1),"
		"	created_at TEXT NOT NULL,"
		"	saved_at TEXT NOT NULL);"
		"INSERT INTO save_info (id, created_at, saved_at) VALUES (1, datetime('now'), datetime('now'));");

	ok = ok && WritePlayer(db, player);

	sqlite3_close(db);
	return ok;
}

bool SaveGame(const Player *player)
{
	if (!FileExists(savePath))
		return CreateNewGame(player);

	sqlite3 *db = OpenDatabase(savePath, SQLITE_OPEN_READWRITE);
	if (db == NULL)
		return false;

	bool ok = WritePlayer(db, player);

	sqlite3_close(db);
	return ok;
}

bool LoadGame(Player *player)
{
	if (!FileExists(savePath))
		return false;

	sqlite3 *db = OpenDatabase(savePath, SQLITE_OPEN_READONLY);
	if (db == NULL)
		return false;

	bool ok = false;
	sqlite3_stmt *query = NULL;

	if (sqlite3_prepare_v2(db, "SELECT tile_x, tile_y, facing FROM player WHERE id = 1;", -1, &query, NULL) == SQLITE_OK
		&& sqlite3_step(query) == SQLITE_ROW)
	{
		const char *facing = (const char *)sqlite3_column_text(query, 2);

		for (int i = 0; i < 4 && !ok; i++)
		{
			if (facing != NULL && strcmp(facing, facingNames[i]) == 0)
			{
				player->tileX = sqlite3_column_int(query, 0);
				player->tileY = sqlite3_column_int(query, 1);
				player->facing = (Direction)i;
				player->moving = false;
				player->progress = 0;
				ok = true;
			}
		}
	}

	sqlite3_finalize(query);
	sqlite3_close(db);
	return ok;
}

// Read one number from the settings table, returns false if it isn't there
static bool ReadSetting(sqlite3 *db, const char *key, int *value)
{
	bool ok = false;
	sqlite3_stmt *query = NULL;

	if (sqlite3_prepare_v2(db, "SELECT value FROM settings WHERE key = ?;", -1, &query, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(query, 1, key, -1, SQLITE_STATIC);
		if (sqlite3_step(query) == SQLITE_ROW)
		{
			*value = sqlite3_column_int(query, 0);
			ok = true;
		}
	}

	sqlite3_finalize(query);
	return ok;
}

Color LoadHatColor(Color fallback)
{
	sqlite3 *db = OpenDatabase(settingsPath, SQLITE_OPEN_READONLY);
	if (db == NULL)
		return fallback;

	Color color = fallback;
	int r = 0;
	int g = 0;
	int b = 0;

	if (ReadSetting(db, "hat_r", &r) && ReadSetting(db, "hat_g", &g) && ReadSetting(db, "hat_b", &b))
		color = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };

	sqlite3_close(db);
	return color;
}

void SaveHatColor(Color color)
{
	sqlite3 *db = OpenDatabase(settingsPath, SQLITE_OPEN_READWRITE);
	if (db == NULL)
		return;

	Exec(db, TextFormat(
		"INSERT OR REPLACE INTO settings (key, value) VALUES ('hat_r', %d), ('hat_g', %d), ('hat_b', %d);",
		color.r, color.g, color.b));

	sqlite3_close(db);
}
