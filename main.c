#include "raylib.h"
#include <math.h>
#include "character.h"
#include "menu.h"
#include "tilemap.h"
#include "scene2.h" // <-- 1. Include the new Scene 2 file

// 2. Add SCREEN_SCENE2 to your enum
typedef enum { SCREEN_MENU, SCREEN_SCENE2, SCREEN_GAMEPLAY } GameScreen;

int main(void) {
    const int vWidth = 1280;
    const int vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_MENU;

    Menu menu = { 0 };
    menu.background = LoadTexture("images/Background/TitleBackground.PNG");

    Tilemap map;
    if (!LoadTilemap(&map, "maps/map1/map1.json")) {
        TraceLog(LOG_ERROR, "Failed to load map1");
        UnloadTexture(menu.background);
        CloseWindow();
        return 1;
    }

    Player player;
    InitPlayer(&player, FindWalkableSpawn(&map));

    // 3. Initialize Scene 2
    Scene2 scene2 = { 0 };
    InitScene2(&scene2);

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    camera.target = player.pos;
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        float scale = fminf((float)GetScreenWidth() / vWidth, (float)GetScreenHeight() / vHeight);
        Vector2 vMouse = {
            (mouse.x - (GetScreenWidth() - (vWidth * scale)) * 0.5f) / scale,
            (mouse.y - (GetScreenHeight() - (vHeight * scale)) * 0.5f) / scale
        };

        // --- UPDATE LOGIC ---
        if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            // 4. Action 1 now takes you to the cutscene instead of gameplay
            if (action == 1) currentScreen = SCREEN_SCENE2; 
            if (action == 2) break;
        }
        else if (currentScreen == SCREEN_SCENE2) {
            // 5. Update Scene 2 and wait for the player to press Enter
            UpdateScene2(&scene2);
            
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            UpdatePlayer(&player, &map);
            camera.target = player.pos;

            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
        }

        // --- DRAWING LOGIC ---
        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_MENU) {
                DrawMenu(&menu, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE2) {
                // 6. Draw Scene 2
                DrawScene2(&scene2, vWidth, vHeight);
                DrawText("PRESS ENTER TO CONTINUE", vWidth - 300, vHeight - 40, 20, LIGHTGRAY);
            }
            else {
                BeginMode2D(camera);
                    DrawTilemapAll(&map);
                    DrawPlayer(&player);
                EndMode2D();

                DrawText("ESC TO MENU | LSHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);
            }

            DrawCircleV(vMouse, 5, RED);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);

            DrawTexturePro(
                target.texture,
                (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){
                    (GetScreenWidth() - (vWidth * scale)) * 0.5f,
                    (GetScreenHeight() - (vHeight * scale)) * 0.5f,
                    vWidth * scale,
                    vHeight * scale
                },
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
            );
        EndDrawing();
    }

    // 7. Cleanup Scene 2 Memory
    UnloadScene2(&scene2);
    UnloadPlayer(&player);
    UnloadTilemap(&map);
    UnloadTexture(menu.background);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}