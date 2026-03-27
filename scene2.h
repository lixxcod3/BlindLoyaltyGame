#ifndef SCENE2_H
#define SCENE2_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

#define SCENE2_TEXT_LEAD_SECONDS 2.0f
#define SCENE2_MIN_TEXT_DURATION 0.15f

typedef enum {
    SCENE2_FADE_IN,
    SCENE2_TEXT,
    SCENE2_FADE_OUT,
    SCENE2_DONE
} Scene2State;

typedef struct {
    Texture2D bgTexture;
    float bgScrollX;
    float bgScrollSpeed;

    Scene2State currentState;
    const char* name;
    const char* dialogue;

    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;

    Sound narratorVoice;
    bool narratorPlayed;
    float narratorDuration;
} Scene2;

static inline float Scene2LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

static inline float Scene2CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    float targetTextDuration = voiceDuration - SCENE2_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE2_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE2_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

static inline void Scene2RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
    if (*charsDrawn >= maxChars) return;

    *timer += GetFrameTime();

    if (charDelay <= 0.0f) {
        *charsDrawn = maxChars;
        *timer = 0.0f;
        return;
    }

    while (*timer >= charDelay && *charsDrawn < maxChars) {
        (*charsDrawn)++;
        *timer -= charDelay;
    }
}

static inline void Scene2PlayNarratorIfNeeded(Scene2* scene) {
    if (!scene->narratorPlayed) {
        PlaySound(scene->narratorVoice);
        scene->narratorPlayed = true;
    }
}

static inline void InitScene2(Scene2* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene2.jpg");

    scene->bgScrollX = 0.0f;
    scene->bgScrollSpeed = -15.0f;

    scene->currentState = SCENE2_FADE_IN;
    scene->fadeAlpha = 1.0f;

    scene->name = "Narrator";
    scene->dialogue =
        "Your name is Reuben. You grew up in a city that was constantly afraid.\n"
        "News reports spoke about threats, attacks, and instability.";

    scene->dialogueLength = TextLength(scene->dialogue);
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;

    scene->narratorDuration = Scene2LoadVoiceWithDuration("audio/Voice/Scene 2/Narrator.mp3", &scene->narratorVoice);
    scene->narratorPlayed = false;
    scene->textSpeed = Scene2CalcTextSpeed(scene->dialogue, scene->narratorDuration, scene->textSpeed);
}

static inline void UpdateScene2(Scene2* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    scene->bgScrollX += scene->bgScrollSpeed * GetFrameTime();

    if (scene->currentState == SCENE2_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f;
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE2_TEXT;
            Scene2PlayNarratorIfNeeded(scene);
        }
    }
    else if (scene->currentState == SCENE2_TEXT) {
        Scene2PlayNarratorIfNeeded(scene);

        if (scene->charsDrawn < scene->dialogueLength) {
            Scene2RevealTextSync(&scene->charsDrawn, scene->dialogueLength, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->dialogueLength;
                StopSound(scene->narratorVoice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->narratorVoice);
                scene->currentState = SCENE2_FADE_OUT;
            }
        }
    }
    else if (scene->currentState == SCENE2_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            StopSound(scene->narratorVoice);
            scene->currentState = SCENE2_DONE;
        }
    }
}

static inline void DrawScene2(Scene2* scene, int screenWidth, int screenHeight) {
    float bgScale = (float)screenHeight / scene->bgTexture.height;
    float scaledWidth = scene->bgTexture.width * bgScale;

    if (scene->bgScrollX <= -scaledWidth || scene->bgScrollX >= scaledWidth) {
        scene->bgScrollX = 0.0f;
    }

    DrawTextureEx(scene->bgTexture, (Vector2){ scene->bgScrollX, 0 }, 0.0f, bgScale, WHITE);

    float secondX = (scene->bgScrollSpeed < 0)
        ? (scene->bgScrollX + scaledWidth)
        : (scene->bgScrollX - scaledWidth);

    DrawTextureEx(scene->bgTexture, (Vector2){ secondX, 0 }, 0.0f, bgScale, WHITE);

    if (scene->currentState == SCENE2_TEXT) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 };
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->dialogue, 0, scene->charsDrawn),
                 (int)chatBox.x + 40, (int)chatBox.y + 35, 28, RAYWHITE);
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }
}

static inline void UnloadScene2(Scene2* scene) {
    StopSound(scene->narratorVoice);
    UnloadSound(scene->narratorVoice);
    UnloadTexture(scene->bgTexture);
}

#endif // 