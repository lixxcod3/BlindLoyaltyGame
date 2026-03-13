#ifndef GAMEPLAY_H
#define GAMEPLAY_H
#include "raylib.h"

static inline void DrawMap1(int mapWidth, int mapHeight) {
    // Gambar pola lantai/rumput agar tidak pusing
    for (int x = 0; x < mapWidth; x += 100) {
        for (int y = 0; y < mapHeight; y += 100) {
            // Menggunakan warna hijau tua dan hijau yang sedikit lebih terang
            if ((x + y) % 200 == 0) 
                DrawRectangle(x, y, 100, 100, (Color){ 34, 139, 34, 255 }); // Forest Green
            else 
                DrawRectangle(x, y, 100, 100, (Color){ 0, 100, 0, 255 });    // Dark Green
        }
    }

    // Tambahkan garis batas map agar kamu tahu di mana ujung dunia game kamu
    DrawRectangleLinesEx((Rectangle){ 0, 0, (float)mapWidth, (float)mapHeight }, 10, MAROON);

    DrawText("MAP TESTING MODE - USE ARROWS TO MOVE", 20, 20, 20, RAYWHITE);
}
#endif