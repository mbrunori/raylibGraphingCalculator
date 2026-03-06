#ifndef GUI_H
#define GUI_H

#include "logic.h"
#include "../lib/shuntingYard/shuntingYard.h"

#include <string.h>
#include <stdio.h>

#define menuFontSize 28
#define graphicFontSize 16
#define MAX_FUNCTIONS 6

#define PARAM_COUNT 4
#define BUFFER_SIZE 32

#define gSolveOptions 3
#define menuScenesNum 2

//enum representing the possible scenes in the applications
typedef enum
{
    MENU,
    INPUT,
    EXIT,
    GRAPHICS,
    VWINDOW,
    GSOLVE
}Scene;

extern const char *scenesName[menuScenesNum];

void menu(Scene *nextScene);
void menuSample(const char** optionsName, int optionsNum, int *selected);
void drawMenuButton(const char *text, Vector2 position, bool isClicked);

void graphicScene(Scene *nextScene, int functionsNumber, char **expressions);
void drawFunction(const char* rpn, Color color);
void drawAxes();

void inputScene(Scene *nextScene, char ***expressions, int *count);

void vWindowScene(Scene* nextScene);

void gSolveScene(Scene *nextScene, int *count, char ***expressions);

#endif