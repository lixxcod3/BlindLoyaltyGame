#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "raylib.h"
#include <stdbool.h>
#include "tilemap.h"

#define KEY_DRAW_SIZE 16.0f
#define KEY_HITBOX_SIZE 20.0f
#define KEY_SPAWN_DELAY 10.0f

typedef struct KeyItem {
    Texture2D texture;
    Vector2 pos;
    Rectangle hitbox;
    bool spawned;
    bool collected;
    float spawnTimer;
} KeyItem;

/*
 * Create a centered square hitbox for the objective.
 */
static inline Rectangle GetKeyHitbox(Vector2 pos) {
    Rectangle rect = {
        pos.x - KEY_HITBOX_SIZE * 0.5f,
        pos.y - KEY_HITBOX_SIZE * 0.5f,
        KEY_HITBOX_SIZE,
        KEY_HITBOX_SIZE
    };
    return rect;
}

/*
 * Check if a rectangle is fully inside the tilemap bounds.
 */
static inline bool IsObjectiveRectFullyInsideMap(const Tilemap *map, Rectangle rect) {
    float mapPixelWidth = (float)map->width * (float)map->tileWidth;
    float mapPixelHeight = (float)map->height * (float)map->tileHeight;

    return rect.x >= 0.0f &&
           rect.y >= 0.0f &&
           rect.x + rect.width <= mapPixelWidth &&
           rect.y + rect.height <= mapPixelHeight;
}

/*
 * Check if the objective can be placed at a position.
 */
static inline bool CanPlaceKeyAt(Vector2 pos, const Tilemap *map) {
    Rectangle rect = GetKeyHitbox(pos);
    if (!IsObjectiveRectFullyInsideMap(map, rect)) return false;
    if (CheckMapCollision(map, rect)) return false;
    return true;
}

/*
 * Find the furthest valid objective spawn from the player.
 */
static inline Vector2 FindFurthestKeySpawn(const Tilemap *map, Vector2 playerPos) {
    Vector2 bestPos = playerPos;
    float bestDistSq = -1.0f;

    for (int row = 0; row < map->height; row++) {
        for (int col = 0; col < map->width; col++) {
            Vector2 candidate = {
                col * map->tileWidth + map->tileWidth * 0.5f,
                row * map->tileHeight + map->tileHeight * 0.5f
            };

            if (!CanPlaceKeyAt(candidate, map)) continue;

            float dx = candidate.x - playerPos.x;
            float dy = candidate.y - playerPos.y;
            float distSq = dx * dx + dy * dy;

            if (distSq > bestDistSq) {
                bestDistSq = distSq;
                bestPos = candidate;
            }
        }
    }

    return bestPos;
}

/*
 * Reset the objective to its pre-spawn state.
 */
static inline void ResetKey(KeyItem *key) {
    key->pos = (Vector2){ 0.0f, 0.0f };
    key->hitbox = GetKeyHitbox(key->pos);
    key->spawned = false;
    key->collected = false;
    key->spawnTimer = KEY_SPAWN_DELAY;
}

/*
 * Update the objective spawn logic and win condition.
 */
static inline void UpdateKeyLogic(KeyItem *key, float dt, const Tilemap *map, Vector2 playerPos, bool *gameWon) {
    if (!key->spawned) {
        key->spawnTimer -= dt;

        if (key->spawnTimer <= 0.0f) {
            key->pos = FindFurthestKeySpawn(map, playerPos);
            key->hitbox = GetKeyHitbox(key->pos);
            key->spawned = true;
        }
    }

    if (key->spawned && !key->collected) {
        if (CheckCollisionRecs(GetPlayerHitbox(playerPos), key->hitbox)) {
            key->collected = true;
            *gameWon = true;
        }
    }
}

/*
 * Draw the current objective texture.
 * This works for both the key and the document.
 */
static inline void DrawKey(const KeyItem *key) {
    if (!key->spawned || key->collected) return;
    if (key->texture.id <= 0) return;

    Rectangle src = { 0, 0, (float)key->texture.width, (float)key->texture.height };
    Rectangle dst = { key->pos.x, key->pos.y, KEY_DRAW_SIZE, KEY_DRAW_SIZE };
    Vector2 origin = { KEY_DRAW_SIZE * 0.5f, KEY_DRAW_SIZE * 0.5f };

    DrawTexturePro(key->texture, src, dst, origin, 0.0f, WHITE);
}

#endif