#include "raylib.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui.h"
#include "logic.h"

#define maxLines 16
#define maxLineLenght 300

int main(int argc, char const *argv[])
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "graphing calculator");
    SetExitKey(0); //disable exit key

    Scene currentScene = MENU;

    char **expressions = NULL; //array of strings to store the expressions
    int count = 0; //number of expressions

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);

        switch (currentScene)
        {
        case MENU:
            menu(&currentScene);
            break;
        case EXIT:
            CloseWindow();
            return 0;
        case INPUT:
            inputScene(&currentScene, &expressions, &count);
            break;
        case GRAPHICS:
            graphicScene(&currentScene, count, expressions);
            break;
        case VWINDOW:
            vWindowScene(&currentScene);
            break;
        case GSOLVE:
            gSolveScene(&currentScene, &count, &expressions);
        }

        EndDrawing();
    }

    //empties and frees expressions
    count = sizeof(expressions) / sizeof(expressions[0]);
    if (expressions != NULL)
    {
        for (int i = 0; i < count; i++)
            free(expressions[i]);
        free(expressions);
    }
    CloseWindow();
}