#ifndef SCENE4_H
#define SCENE4_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

#define SCENE4_TEXT_LEAD_SECONDS 2.0f
#define SCENE4_MIN_TEXT_DURATION 0.15f

typedef enum {
    SCENE4_FADE_IN,
    SCENE4_TEXT,
    SCENE4_FADE_OUT,
    SCENE4_DONE
} Scene4State;

typedef struct {
    Texture2D bgTexture;
    Scene4State currentState;

    const char* name;
    const char* dialogue;

    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
    float narratorDuration;
    bool narratorPlayed;
    Sound narratorVoice;
} Scene4;

static inline float Scene4LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

static inline float Scene4CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    float targetTextDuration = voiceDuration - SCENE4_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE4_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE4_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

static inline void Scene4RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
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

static inline void Scene4PlayNarratorIfNeeded(Scene4* scene) {
    if (!scene->narratorPlayed) {
        PlaySound(scene->narratorVoice);
        scene->narratorPlayed = true;
    }
}

static inline void InitScene4(Scene4* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene4.jpg");

    scene->currentState = SCENE4_FADE_IN;
    scene->name = "Narrator";

    scene->dialogue =
        "You were not the strongest student, but you were disciplined. Focused.\n"
        "You believed rules existed for a reason. While others questioned authority,\n"
        "you believed in protecting people, even if it meant doing things that\n"
        "others wouldn't understand.";

    scene->dialogueLength = TextLength(scene->dialogue);
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;
    scene->fadeAlpha = 1.0f;

    scene->narratorDuration = Scene4LoadVoiceWithDuration("audio/Voice/Scene 4/Narrator.mp3", &scene->narratorVoice);
    scene->narratorPlayed = false;
    scene->textSpeed = Scene4CalcTextSpeed(scene->dialogue, scene->narratorDuration, scene->textSpeed);
}

static inline void UpdateScene4(Scene4* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    if (scene->currentState == SCENE4_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f;
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE4_TEXT;
            Scene4PlayNarratorIfNeeded(scene);
        }
    }
    else if (scene->currentState == SCENE4_TEXT) {
        Scene4PlayNarratorIfNeeded(scene);

        if (scene->charsDrawn < scene->dialogueLength) {
            Scene4RevealTextSync(&scene->charsDrawn, scene->dialogueLength, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->dialogueLength;
                StopSound(scene->narratorVoice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->narratorVoice);
                scene->currentState = SCENE4_FADE_OUT;
            }
        }
    }
    else if (scene->currentState == SCENE4_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            StopSound(scene->narratorVoice);
            scene->currentState = SCENE4_DONE;
        }
    }
}

static inline void DrawScene4(Scene4* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, screenWidth, screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE4_TEXT) {
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

static inline void UnloadScene4(Scene4* scene) {
    StopSound(scene->narratorVoice);
    UnloadSound(scene->narratorVoice);
    UnloadTexture(scene->bgTexture);
}

#endif // 