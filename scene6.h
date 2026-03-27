#ifndef SCENE6_H
#define SCENE6_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

#define SCENE6_TEXT_LEAD_SECONDS 3.0f
#define SCENE6_SHORT_VOICE_THRESHOLD 5.0f
#define SCENE6_SHORT_VOICE_TEXT_SPEED 0.03f
#define SCENE6_MIN_TEXT_DURATION 0.15f

typedef enum {
    SCENE6_FADE_IN,
    SCENE6_TEXT_1,
    SCENE6_FADE_TO_BLACK,
    SCENE6_TEXT_2,
    SCENE6_WAIT,
    SCENE6_DONE
} Scene6State;

typedef struct {
    Texture2D bgTexture;
    Scene6State currentState;

    const char* name;
    const char* text1;
    const char* text2;

    int text1Length;
    int text2Length;

    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
    float defaultTextSpeed;

    Sound text1Voice;
    Sound text2Voice;
    float text1Duration;
    float text2Duration;
    bool text1Started;
    bool text2Started;
} Scene6;

/* =========================================================
   HELPERS
   ========================================================= */

static inline float Scene6LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

static inline float Scene6CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    // Scene 1 rule:
    // if voice duration < 5 sec, use constant 0.03f
    if (voiceDuration < SCENE6_SHORT_VOICE_THRESHOLD) {
        return SCENE6_SHORT_VOICE_TEXT_SPEED;
    }

    // otherwise finish text 3 seconds earlier than the voice
    float targetTextDuration = voiceDuration - SCENE6_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE6_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE6_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

static inline void Scene6RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
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

static inline void Scene6StartText1(Scene6* scene) {
    if (scene->text1Started) return;

    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = Scene6CalcTextSpeed(scene->text1, scene->text1Duration, scene->defaultTextSpeed);

    PlaySound(scene->text1Voice);
    scene->text1Started = true;
}

static inline void Scene6StartText2(Scene6* scene) {
    if (scene->text2Started) return;

    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = Scene6CalcTextSpeed(scene->text2, scene->text2Duration, scene->defaultTextSpeed);

    PlaySound(scene->text2Voice);
    scene->text2Started = true;
}

/* =========================================================
   INIT
   ========================================================= */

static inline void InitScene6(Scene6* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene6.jpg");

    scene->currentState = SCENE6_FADE_IN;
    scene->name = "Narrator";

    scene->text1 =
        "Over the years, you became one of their trusted operatives.\n"
        "The missions were clean, efficient, and necessary.";

    scene->text2 = "At least, that's what you were told.";

    scene->text1Length = TextLength(scene->text1);
    scene->text2Length = TextLength(scene->text2);

    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->defaultTextSpeed = 0.03f;
    scene->textSpeed = scene->defaultTextSpeed;
    scene->fadeAlpha = 1.0f;

    scene->text1Duration = Scene6LoadVoiceWithDuration("audio/Voice/Scene 6/Narrator part 1.mp3", &scene->text1Voice);
    scene->text2Duration = Scene6LoadVoiceWithDuration("audio/Voice/Scene 6/Narrator part 2.mp3", &scene->text2Voice);

    scene->text1Started = false;
    scene->text2Started = false;
}

/* =========================================================
   UPDATE
   ========================================================= */

static inline void UpdateScene6(Scene6* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    if (scene->currentState == SCENE6_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f;
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE6_TEXT_1;
        }
    }
    else if (scene->currentState == SCENE6_TEXT_1) {
        Scene6StartText1(scene);

        if (scene->charsDrawn < scene->text1Length) {
            Scene6RevealTextSync(&scene->charsDrawn, scene->text1Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text1Length;
                StopSound(scene->text1Voice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->text1Voice);
                scene->currentState = SCENE6_FADE_TO_BLACK;
            }
        }
    }
    else if (scene->currentState == SCENE6_FADE_TO_BLACK) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE6_TEXT_2;
            scene->charsDrawn = 0;
            scene->textTimer = 0.0f;
        }
    }
    else if (scene->currentState == SCENE6_TEXT_2) {
        Scene6StartText2(scene);

        if (scene->charsDrawn < scene->text2Length) {
            Scene6RevealTextSync(&scene->charsDrawn, scene->text2Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text2Length;
                StopSound(scene->text2Voice);
            }
        }
        else {
            scene->currentState = SCENE6_WAIT;
        }
    }
    else if (scene->currentState == SCENE6_WAIT) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
            StopSound(scene->text2Voice);
            scene->currentState = SCENE6_DONE;
        }
    }
}

/* =========================================================
   DRAW
   ========================================================= */

static inline void DrawScene6(Scene6* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, screenWidth, screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE6_TEXT_1 ||
        scene->currentState == SCENE6_FADE_IN ||
        scene->currentState == SCENE6_FADE_TO_BLACK) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 };
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        DrawText(
            TextSubtext(scene->text1, 0, scene->charsDrawn),
            (int)chatBox.x + 40,
            (int)chatBox.y + 35,
            28,
            RAYWHITE
        );
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE6_TEXT_2 || scene->currentState == SCENE6_WAIT) {
        const char* currentNarText = TextSubtext(scene->text2, 0, scene->charsDrawn);
        int textWidth = MeasureText(currentNarText, 28);

        DrawText(currentNarText, (screenWidth / 2) - (textWidth / 2), screenHeight / 2 - 30, 28, RAYWHITE);

        if (scene->currentState == SCENE6_WAIT) {
            DrawText("PRESS ENTER TO START MISSION", screenWidth / 2 - 160, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

/* =========================================================
   UNLOAD
   ========================================================= */

static inline void UnloadScene6(Scene6* scene) {
    StopSound(scene->text1Voice);
    StopSound(scene->text2Voice);
    UnloadSound(scene->text1Voice);
    UnloadSound(scene->text2Voice);
    UnloadTexture(scene->bgTexture);
}

#endif 