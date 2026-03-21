#ifndef SCENE2_H
#define SCENE2_H

#include "raylib.h"
#include <string.h>

// Structure to hold our scene's assets and data
typedef struct {
    // Background Animation Data
    Image bgAnimImage;
    Texture2D bgTexture;
    int animFrames;
    int currentAnimFrame;
    float frameTimer;
    float frameDelay;
    
    // Scene Assets
    const char* name;
    const char* dialogue;

    // Typewriter Effect Data
    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
} Scene2;

// --- IMPLEMENTATIONS ---

static inline void InitScene2(Scene2* scene) {
    // Load the animated GIF background
    scene->bgAnimImage = LoadImageAnim("images/Background/Scene/Scene2.gif", &scene->animFrames);
    scene->bgTexture = LoadTextureFromImage(scene->bgAnimImage);
    scene->currentAnimFrame = 0;
    scene->frameTimer = 0.0f;
    scene->frameDelay = 1.0f / 12.0f; // Approx 12 FPS for the GIF
    
    // Set the speaker's name
    scene->name = "Narrator";
    
    // Dialogue with a line break (\n) to fit the chatbox
    scene->dialogue = "Your name is Reuben. You grew up in a city that was constantly afraid.\nNews reports spoke about threats, attacks, and instability.";

    // Typewriter Setup
    scene->dialogueLength = TextLength(scene->dialogue); // Raylib function to get string length
    scene->charsDrawn = 0;                               // Start with 0 characters visible
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;                            // Speed: 0.04 seconds per letter
}

static inline void UpdateScene2(Scene2* scene) {
    // --- 1. ANIMATE BACKGROUND ---
    scene->frameTimer += GetFrameTime();
    if (scene->frameTimer >= scene->frameDelay) {
        scene->currentAnimFrame++;
        if (scene->currentAnimFrame >= scene->animFrames) {
            scene->currentAnimFrame = 0; 
        }
        unsigned int nextFrameDataOffset = scene->bgAnimImage.width * scene->bgAnimImage.height * 4 * scene->currentAnimFrame;
        UpdateTexture(scene->bgTexture, ((unsigned char *)scene->bgAnimImage.data) + nextFrameDataOffset);
        scene->frameTimer = 0.0f;
    }

    // --- 2. UPDATE TYPEWRITER EFFECT ---
    // Only increment the timer if we haven't finished typing the whole string
    if (scene->charsDrawn < scene->dialogueLength) {
        scene->textTimer += GetFrameTime();
        
        // Once the timer passes our text speed, reveal the next character!
        if (scene->textTimer >= scene->textSpeed) {
            scene->charsDrawn++;
            scene->textTimer = 0.0f; // Reset timer for the next character
        }
    }
}

static inline void DrawScene2(Scene2* scene, int screenWidth, int screenHeight) {
    // 1. Draw Animated Background
    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    // 2. Draw Chatbox
    // --> CHANGED: Now starts at x=20 and spans the full screen width with a 20px margin <--
    Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
    DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
    DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

    // 3. Draw Name Box
    // --> CHANGED: Moved to x=20 to align with the new chatbox <--
    Rectangle nameBox = { 20, 480, 200, 40 };
    DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
    DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

    // 4. Draw Dialogue Text (Typewriter Effect)
    DrawText(TextSubtext(scene->dialogue, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
}

static inline void UnloadScene2(Scene2* scene) {
    UnloadTexture(scene->bgTexture);
    UnloadImage(scene->bgAnimImage); 
}

#endif // SCENE2_H