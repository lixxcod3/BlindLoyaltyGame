#ifndef SECURITY_GUARD_H
#define SECURITY_GUARD_H

#include "bandit.h"

void InitSecurityGuard(Enemy *e, Vector2 startPos);
void UpdateSecurityGuard(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

#ifdef SECURITY_GUARD_IMPLEMENTATION
#ifndef SECURITY_GUARD_IMPLEMENTATION_ONCE
#define SECURITY_GUARD_IMPLEMENTATION_ONCE

void InitSecurityGuard(Enemy *e, Vector2 startPos) {
    InitBandit(e, startPos);

    if (e->tex.id > 0) {
        UnloadTexture(e->tex);
    }

    e->tex = LoadTexture("images/Character/Security Guard/Securityguard_walk.png");
}

void UpdateSecurityGuard(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    UpdateBandit(e, allEnemies, totalEnemyCount, player, map);
}

#endif
#endif