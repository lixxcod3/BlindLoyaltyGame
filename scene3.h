#ifndef SCENE3_H
#define SCENE3_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

#define SCENE3_TEXT_LEAD_SECONDS 3.0f
#define SCENE3_SHORT_VOICE_THRESHOLD 5.0f
#define SCENE3_SHORT_VOICE_TEXT_SPEED 0.03f
#define SCENE3_MIN_TEXT_DURATION 0.15f

typedef enum {
    SCENE3_TEXT_1,
    SCENE3_FADE_IN,
    SCENE3_TEXT_2,
    SCENE3_DONE
} Scene3State;

typedef struct {
    Texture2D bgTexture;
    Scene3State currentState;

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
} Scene3;

static inline float Scene3LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

static inline float Scene3CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    if (voiceDuration < SCENE3_SHORT_VOICE_THRESHOLD) {
        return SCENE3_SHORT_VOICE_TEXT_SPEED;
    }

    float targetTextDuration = voiceDuration - SCENE3_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE3_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE3_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

static inline void Scene3RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
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

static inline void Scene3StartText1(Scene3* scene) {
    if (scene->text1Started) return;

    scene->textSpeed = Scene3CalcTextSpeed(scene->text1, scene->text1Duration, scene->defaultTextSpeed);
    scene->textTimer = 0.0f;
    scene->charsDrawn = 0;

    PlaySound(scene->text1Voice);
    scene->text1Started = true;
}

static inline void Scene3StartText2(Scene3* scene) {
    if (scene->text2Started) return;

    scene->textSpeed = Scene3CalcTextSpeed(scene->text2, scene->text2Duration, scene->defaultTextSpeed);
    scene->textTimer = 0.0f;
    scene->charsDrawn = 0;

    PlaySound(scene->text2Voice);
    scene->text2Started = true;
}

static inline void InitScene3(Scene3* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene3.jpg");
    scene->currentState = SCENE3_TEXT_1;
    scene->name = "Narrator";

    scene->text1 =
        "Your parents used to say that the only reason people could\n"
        "sleep peacefully at night was because someone, somewhere,\n"
        "was willing to make difficult decisions.";
    scene->text2 = "And that idea stayed with you...";

    scene->text1Length = TextLength(scene->text1);
    scene->text2Length = TextLength(scene->text2);

    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;
    scene->defaultTextSpeed = 0.04f;
    scene->fadeAlpha = 0.0f;

    scene->text1Duration = Scene3LoadVoiceWithDuration("audio/Voice/Scene 3/Narrator part 1.mp3", &scene->text1Voice);
    scene->text2Duration = Scene3LoadVoiceWithDuration("audio/Voice/Scene 3/Narrator part 2.mp3", &scene->text2Voice);
    scene->text1Started = false;
    scene->text2Started = false;
}

static inline void UpdateScene3(Scene3* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    if (scene->currentState == SCENE3_TEXT_1) {
        Scene3StartText1(scene);

        if (scene->charsDrawn < scene->text1Length) {
            Scene3RevealTextSync(&scene->charsDrawn, scene->text1Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text1Length;
                StopSound(scene->text1Voice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->text1Voice);
                scene->currentState = SCENE3_FADE_IN;
            }
        }
    }
    else if (scene->currentState == SCENE3_FADE_IN) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE3_TEXT_2;
            scene->charsDrawn = 0;
            scene->textTimer = 0.0f;
        }
    }
    else if (scene->currentState == SCENE3_TEXT_2) {
        Scene3StartText2(scene);

        if (scene->charsDrawn < scene->text2Length) {
            Scene3RevealTextSync(&scene->charsDrawn, scene->text2Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text2Length;
                StopSound(scene->text2Voice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->text2Voice);
                scene->currentState = SCENE3_DONE;
            }
        }
    }
}

static inline void DrawScene3(Scene3* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, screenWidth, screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE3_TEXT_1 || scene->currentState == SCENE3_FADE_IN) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 };
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->text1, 0, scene->charsDrawn),
                 (int)chatBox.x + 40, (int)chatBox.y + 35, 28, RAYWHITE);
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE3_TEXT_2) {
        const char* currentNarText = TextSubtext(scene->text2, 0, scene->charsDrawn);
        int textWidth = MeasureText(currentNarText, 28);

        DrawText(currentNarText, (screenWidth / 2) - (textWidth / 2), screenHeight / 2 - 30, 28, RAYWHITE);

        if (scene->charsDrawn >= scene->text2Length) {
            DrawText("PRESS ENTER TO CONTINUE", screenWidth / 2 - 150, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

static inline void UnloadScene3(Scene3* scene) {
    StopSound(scene->text1Voice);
    StopSound(scene->text2Voice);
    UnloadSound(scene->text1Voice);
    UnloadSound(scene->text2Voice);
    UnloadTexture(scene->bgTexture);
}

#endif // 