#ifndef OASIS_MEDIA_CEO_H
#define OASIS_MEDIA_CEO_H

#include "boss_bandit.h"

void InitOasisMediaCEO(Enemy *boss, Vector2 startPos);
void UpdateOasisMediaCEO(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

#ifdef OASIS_MEDIA_CEO_IMPLEMENTATION
#ifndef OASIS_MEDIA_CEO_IMPLEMENTATION_ONCE
#define OASIS_MEDIA_CEO_IMPLEMENTATION_ONCE

void InitOasisMediaCEO(Enemy *boss, Vector2 startPos) {
    InitBossBandit(boss, startPos);

    if (boss->tex.id > 0) {
        UnloadTexture(boss->tex);
    }

    boss->tex = LoadTexture("images/Character/Oasis Media CEO/OasisCEO_walk.png");
}

void UpdateOasisMediaCEO(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    UpdateBossBandit(boss, allEnemies, totalEnemyCount, player, map);
}

#endif
#endif