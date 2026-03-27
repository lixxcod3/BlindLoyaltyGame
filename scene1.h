#ifndef SCENE1_H
#define SCENE1_H

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

#define SCENE1_DIALOGUE_COUNT 7
#define SCENE1_TEXT_LEAD_SECONDS 3.0f
#define SCENE1_MIN_TEXT_DURATION 0.15f

typedef enum {
    SCENE1_DIALOGUE,
    SCENE1_CHOICE,
    SCENE1_FADE_TO_BLACK,
    SCENE1_NARRATOR,
    SCENE1_DONE
} Scene1State;

typedef struct {
    const char* speakerName;
    const char* text;
    Texture2D* portrait;
} DialogueLine;

typedef struct {
    // Visuals
    Texture2D bgTexture;
    Texture2D commanderPortrait;
    Texture2D reubenPortrait;

    float commanderScale;
    float reubenScale;
    float commanderOffsetX;
    float reubenOffsetX;

    // Dialogue
    DialogueLine lines[SCENE1_DIALOGUE_COUNT];
    int totalLines;
    int currentLine;

    // Text animation
    int charsDrawn;
    int narratorCharsDrawn;
    float textTimer;
    float textSpeed;
    float defaultTextSpeed;

    // State
    int currentChoice;
    float fadeAlpha;
    Scene1State currentState;

    // Narrator
    const char* narratorText;

    // Voice
    Sound dialogueVoice[SCENE1_DIALOGUE_COUNT];
    float dialogueDuration[SCENE1_DIALOGUE_COUNT];

    Sound narratorVoice;
    float narratorDuration;

    float voiceVolume;

    // Runtime flags
    bool firstDialogueStarted;
    bool narratorStarted;

} Scene1;

/* =========================================================
   AUDIO + TIMING HELPERS
   ========================================================= */

static inline int Scene1CountTextUnits(const char* text) {
    if (text == NULL) return 1;

    int len = TextLength(text);
    return (len > 0) ? len : 1;
}

static inline float Scene1LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

static inline float Scene1CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = Scene1CountTextUnits(text);

    if (units <= 0) return fallbackSpeed;

    // If voice is shorter than 5 seconds, use constant typing speed
    if (voiceDuration < 5.0f) {
        return 0.03f;
    }

    if (voiceDuration <= 0.0f) return fallbackSpeed;

    float targetTextDuration = voiceDuration - SCENE1_TEXT_LEAD_SECONDS;

    if (targetTextDuration < SCENE1_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE1_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

static inline void Scene1StopAllVoices(Scene1* scene) {
    for (int i = 0; i < SCENE1_DIALOGUE_COUNT; i++) {
        StopSound(scene->dialogueVoice[i]);
    }
    StopSound(scene->narratorVoice);
}

static inline void Scene1ApplyDialogueTiming(Scene1* scene, int lineIndex) {
    if (lineIndex < 0 || lineIndex >= scene->totalLines) {
        scene->textSpeed = scene->defaultTextSpeed;
        return;
    }

    scene->textSpeed = Scene1CalcTextSpeed(
        scene->lines[lineIndex].text,
        scene->dialogueDuration[lineIndex],
        scene->defaultTextSpeed
    );
}

static inline void Scene1ApplyNarratorTiming(Scene1* scene) {
    scene->textSpeed = Scene1CalcTextSpeed(
        scene->narratorText,
        scene->narratorDuration,
        scene->defaultTextSpeed
    );
}

static inline void Scene1RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
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

static inline void Scene1StartDialogueLine(Scene1* scene, int lineIndex) {
    if (lineIndex < 0 || lineIndex >= scene->totalLines) return;

    scene->currentLine = lineIndex;
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;

    Scene1ApplyDialogueTiming(scene, lineIndex);
    Scene1StopAllVoices(scene);
    PlaySound(scene->dialogueVoice[lineIndex]);
}

static inline void Scene1StartNarrator(Scene1* scene) {
    scene->narratorCharsDrawn = 0;
    scene->textTimer = 0.0f;

    Scene1ApplyNarratorTiming(scene);
    Scene1StopAllVoices(scene);
    PlaySound(scene->narratorVoice);
}

/* =========================================================
   INIT
   ========================================================= */

static inline void InitScene1(Scene1* scene) {
    // Load images
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene1.png");
    scene->commanderPortrait = LoadTexture("images/Character/Commander/CommanderChat.png");
    scene->reubenPortrait = LoadTexture("images/Character/Reuben/ReubenChat.png");

    // Portrait size / position
    scene->commanderScale = 0.30f;
    scene->reubenScale = 0.28f;
    scene->commanderOffsetX = 20.0f;
    scene->reubenOffsetX = 20.0f;

    // Scene state
    scene->currentState = SCENE1_DIALOGUE;
    scene->totalLines = SCENE1_DIALOGUE_COUNT;
    scene->currentLine = 0;

    // Text animation
    scene->charsDrawn = 0;
    scene->narratorCharsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->defaultTextSpeed = 0.03f;
    scene->textSpeed = scene->defaultTextSpeed;

    // Choice / transition
    scene->currentChoice = 0;
    scene->fadeAlpha = 0.0f;

    // Runtime flags
    scene->firstDialogueStarted = false;
    scene->narratorStarted = false;

    // Narrator
    scene->narratorText =
        "Before the world called you a hero,\n"
        "you were just a child who believed in order.";

    // Dialogue
    scene->lines[0] = (DialogueLine){
        "Commander",
        "You've got a new mission from the president!",
        &scene->commanderPortrait
    };

    scene->lines[1] = (DialogueLine){
        "Reuben",
        "What mission commander?",
        &scene->reubenPortrait
    };

    scene->lines[2] = (DialogueLine){
        "Commander",
        "As usual... You need to eliminate a monster for the government.",
        &scene->commanderPortrait
    };

    scene->lines[3] = (DialogueLine){
        "Reuben",
        "Could you give me more details about that?",
        &scene->reubenPortrait
    };

    scene->lines[4] = (DialogueLine){
        "Commander",
        "For the past 2 years, The Bandit Village tribes stole our\n"
        "army's food supplies. But, they move like a shadow.\n"
        "No one can ever touch them.",
        &scene->commanderPortrait
    };

    scene->lines[5] = (DialogueLine){
        "Reuben",
        "So what's your plan commander?",
        &scene->reubenPortrait
    };

    scene->lines[6] = (DialogueLine){
        "Commander",
        "You need to find their secret bunker in order to prove that\n"
        "they stole the military supplies. Then we will launch massive\n"
        "troops to capture them. And the media will record it to the world.\n"
        "Do you understand your task?",
        &scene->commanderPortrait
    };

    // Voice settings
    scene->voiceVolume = 1.0f;

    // Load voice files + measure duration
    scene->dialogueDuration[0] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Commander part 1.mp3", &scene->dialogueVoice[0]);
    scene->dialogueDuration[1] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Reuben part 1.mp3", &scene->dialogueVoice[1]);
    scene->dialogueDuration[2] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Commander part 2.mp3", &scene->dialogueVoice[2]);
    scene->dialogueDuration[3] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Reuben part 2.mp3", &scene->dialogueVoice[3]);
    scene->dialogueDuration[4] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Commander part 3.mp3", &scene->dialogueVoice[4]);
    scene->dialogueDuration[5] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Reuben part 3.mp3", &scene->dialogueVoice[5]);
    scene->dialogueDuration[6] = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Commander part 4.mp3", &scene->dialogueVoice[6]);

    scene->narratorDuration = Scene1LoadVoiceWithDuration("audio/Voice/Scene 1/Narrator.mp3", &scene->narratorVoice);

    for (int i = 0; i < SCENE1_DIALOGUE_COUNT; i++) {
        SetSoundVolume(scene->dialogueVoice[i], scene->voiceVolume);
    }
    SetSoundVolume(scene->narratorVoice, scene->voiceVolume);
}

/* =========================================================
   UPDATE
   ========================================================= */

static inline void UpdateScene1(Scene1* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    // Start first voice only when Scene 1 is actually active
    if (!scene->firstDialogueStarted &&
        scene->currentState == SCENE1_DIALOGUE &&
        scene->currentLine == 0) {
        Scene1StartDialogueLine(scene, 0);
        scene->firstDialogueStarted = true;
    }

    if (scene->currentState == SCENE1_DIALOGUE) {
        int currentTextLength = TextLength(scene->lines[scene->currentLine].text);

        if (scene->charsDrawn < currentTextLength) {
            Scene1RevealTextSync(&scene->charsDrawn, currentTextLength, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = currentTextLength;
                StopSound(scene->dialogueVoice[scene->currentLine]);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                int nextLine = scene->currentLine + 1;

                if (nextLine >= scene->totalLines) {
                    Scene1StopAllVoices(scene);
                    scene->currentState = SCENE1_CHOICE;
                } else {
                    Scene1StartDialogueLine(scene, nextLine);
                }
            }
        }
    }
    else if (scene->currentState == SCENE1_CHOICE) {
        Rectangle choiceBox1 = { screenWidth / 2.0f - 150, 300, 300, 50 };
        Rectangle choiceBox2 = { screenWidth / 2.0f - 150, 370, 300, 50 };

        if (CheckCollisionPointRec(mousePos, choiceBox1)) {
            scene->currentChoice = 0;
            if (mouseClicked) {
                Scene1StopAllVoices(scene);
                scene->currentState = SCENE1_FADE_TO_BLACK;
            }
        }
        else if (CheckCollisionPointRec(mousePos, choiceBox2)) {
            scene->currentChoice = 1;
            if (mouseClicked) {
                Scene1StopAllVoices(scene);
                scene->currentState = SCENE1_FADE_TO_BLACK;
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) scene->currentChoice = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) scene->currentChoice = 1;

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            Scene1StopAllVoices(scene);
            scene->currentState = SCENE1_FADE_TO_BLACK;
        }
    }
    else if (scene->currentState == SCENE1_FADE_TO_BLACK) {
        scene->fadeAlpha += GetFrameTime() * 0.5f;

        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE1_NARRATOR;
            scene->charsDrawn = 0;
            scene->narratorCharsDrawn = 0;
            scene->textTimer = 0.0f;
        }
    }
    else if (scene->currentState == SCENE1_NARRATOR) {
        if (!scene->narratorStarted) {
            Scene1StartNarrator(scene);
            scene->narratorStarted = true;
        }

        int narTextLen = TextLength(scene->narratorText);

        if (scene->narratorCharsDrawn < narTextLen) {
            Scene1RevealTextSync(&scene->narratorCharsDrawn, narTextLen, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->narratorCharsDrawn = narTextLen;
                StopSound(scene->narratorVoice);
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                Scene1StopAllVoices(scene);
                scene->currentState = SCENE1_DONE;
            }
        }
    }
}

/* =========================================================
   DRAW
   ========================================================= */

static inline void DrawScene1(Scene1* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE1_DIALOGUE || scene->currentState == SCENE1_CHOICE) {
        int safeLineIndex = scene->currentLine;
        if (safeLineIndex >= scene->totalLines) {
            safeLineIndex = scene->totalLines - 1;
        }

        DialogueLine current = scene->lines[safeLineIndex];

        Rectangle chatBox = { 280, 520, (float)(screenWidth - 280 - 20), 160 };
        Rectangle nameBox = { 280, 480, 200, 40 };

        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(current.speakerName, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        if (current.portrait != NULL) {
            float activeScale = 1.0f;
            float activeOffsetX = 0.0f;

            if (current.portrait == &scene->commanderPortrait) {
                activeScale = scene->commanderScale;
                activeOffsetX = scene->commanderOffsetX;
            }
            else if (current.portrait == &scene->reubenPortrait) {
                activeScale = scene->reubenScale;
                activeOffsetX = scene->reubenOffsetX;
            }

            Vector2 portraitPos = {
                activeOffsetX,
                screenHeight - (current.portrait->height * activeScale) + 20
            };

            DrawTextureEx(*current.portrait, portraitPos, 0.0f, activeScale, WHITE);
        }

        if (scene->currentState == SCENE1_DIALOGUE) {
            DrawText(
                TextSubtext(current.text, 0, scene->charsDrawn),
                (int)chatBox.x + 40,
                (int)chatBox.y + 35,
                28,
                RAYWHITE
            );
        } else {
            DrawText(
                current.text,
                (int)chatBox.x + 40,
                (int)chatBox.y + 35,
                28,
                RAYWHITE
            );
        }
    }

    if (scene->currentState == SCENE1_CHOICE) {
        Rectangle choiceBox1 = { screenWidth / 2.0f - 150, 300, 300, 50 };
        Rectangle choiceBox2 = { screenWidth / 2.0f - 150, 370, 300, 50 };

        Color color1 = (scene->currentChoice == 0) ? LIGHTGRAY : DARKGRAY;
        Color color2 = (scene->currentChoice == 1) ? LIGHTGRAY : DARKGRAY;

        DrawRectangleRec(choiceBox1, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(choiceBox1, 2.0f, color1);
        DrawText("Yes Sir!", (int)choiceBox1.x + 100, (int)choiceBox1.y + 15, 24,
                 (scene->currentChoice == 0) ? WHITE : GRAY);

        DrawRectangleRec(choiceBox2, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(choiceBox2, 2.0f, color2);
        DrawText("Yes Sir!", (int)choiceBox2.x + 100, (int)choiceBox2.y + 15, 24,
                 (scene->currentChoice == 1) ? WHITE : GRAY);
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE1_NARRATOR) {
        const char* currentNarText = TextSubtext(scene->narratorText, 0, scene->narratorCharsDrawn);

        DrawText(currentNarText, screenWidth / 2 - 350, screenHeight / 2 - 30, 28, RAYWHITE);

        if (scene->narratorCharsDrawn >= TextLength(scene->narratorText)) {
            DrawText("PRESS ENTER TO CONTINUE", screenWidth / 2 - 150, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

/* =========================================================
   UNLOAD
   ========================================================= */

static inline void UnloadScene1(Scene1* scene) {
    Scene1StopAllVoices(scene);

    UnloadTexture(scene->bgTexture);
    UnloadTexture(scene->commanderPortrait);
    UnloadTexture(scene->reubenPortrait);

    for (int i = 0; i < SCENE1_DIALOGUE_COUNT; i++) {
        UnloadSound(scene->dialogueVoice[i]);
    }

    UnloadSound(scene->narratorVoice);
}

#endif