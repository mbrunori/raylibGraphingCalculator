#include "gui.h"
#include "logic.h"

#include <string.h>
#include <stdio.h>

const char *scenesName[menuScenesNum] = {"1. Graphic", "2. Exit"};
const char *gSolveOptionsName[gSolveOptions] = {"1. Intersect", "2. Roots", "3. Y-Intercepts"};

// implements the general menuSample() function for the main menu
void menu(Scene *nextScene)
{
    static int selectedOption = 0; // persistent across frames

    menuSample(scenesName, menuScenesNum, &selectedOption);

    if (IsKeyPressed(KEY_ENTER))
    {
        *nextScene = (Scene)selectedOption + 1;
    }
}

// draws the buttons in menu, they can be hovered and changes colors when they are so
void drawMenuButton(const char *text, Vector2 position, bool isClicked)
{
    Color textBackGroundColor = isClicked ? SKYBLUE : BLANK;
    int textWidth = MeasureText(text, menuFontSize);
    DrawRectangle(position.x, position.y, textWidth + 2, menuFontSize, textBackGroundColor);
    DrawText(text, position.x, position.y, menuFontSize, BLACK);
}

// the whole scene in which graphics are drew, it's a variadic function. allowing virtually infinite functions as parameters
void graphicScene(Scene *nextScene, int functionsNumber, char **expressions)
{
    Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
    const int paletteSize = sizeof(palette) / sizeof(palette[0]);
    char parsed[EXPRESSION_BUFFER];

    drawAxes();

    for (int i = 0; i < functionsNumber; i++)
    {
        shuntingYard(expressions[i], parsed);
        if (parsed[0] != '\0')
        {
            drawFunction(parsed, palette[i % paletteSize]);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE))
        *nextScene = INPUT;
    if (IsKeyPressed(KEY_V))
        *nextScene = VWINDOW;
    if (IsKeyPressed(KEY_G))
        *nextScene = GSOLVE;
}

// it's called in graphicScene(), its purpose is to draw the graph of a function, sliding between each x value, calculating the corresponding y value
void drawFunction(const char *rpn, Color color)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    int steps = (int)((xMax - xMin) / step);

    double scaleX = width / (xMax - xMin);
    double scaleY = height / (yMax - yMin);

    float x0 = -xMin / (xMax - xMin) * width;
    float y0 = height - (-yMin / (yMax - yMin) * height);

    for (int i = 0; i <= steps; i++)
    {
        double x = xMin + i * step;
        double y = evaluateRPN(rpn, x);

        int screenX = (int)(x0 + x * scaleX);
        int screenY = (int)(y0 - y * scaleY);

        if (screenX >= 0 && screenX < width &&
            screenY >= 0 && screenY < height)
        {
            DrawPixel(screenX, screenY, color);
        }
    }
}

// draws axes in graphicScene(), it also draws the grid
void drawAxes()
{
    // Calculate window width and height in pixels (considering scale)
    float width = screenWidth * screenScale;
    float height = screenHeight * screenScale;

    // Calculate pixel coordinates of the mathematical origin (0,0)
    float x0 = -xMin / (xMax - xMin) * width;
    float y0 = height - (-yMin / (yMax - yMin) * height);

    // Compute scale factor (pixels per unit)
    float xScale = width / (xMax - xMin);
    float yScale = height / (yMax - yMin);

    // Compute “nice” step sizes for X and Y axes
    float stepX = 1;
    float stepY = 1;

    // Draw background grid (light gray, unobtrusive)
    // Vertical lines
    for (float x = 0; x <= xMax; x += stepX)
    {
        float px = (x - xMin) * xScale;
        DrawLine(px, 0, px, height, LIGHTGRAY);
    }
    for (float x = -stepX; x >= xMin; x -= stepX)
    {
        float px = (x - xMin) * xScale;
        DrawLine(px, 0, px, height, LIGHTGRAY);
    }

    // Horizontal lines
    for (float y = 0; y <= yMax; y += stepY)
    {
        float py = height - (y - yMin) * yScale;
        DrawLine(0, py, width, py, LIGHTGRAY);
    }
    for (float y = -stepY; y >= yMin; y -= stepY)
    {
        float py = height - (y - yMin) * yScale;
        DrawLine(0, py, width, py, LIGHTGRAY);
    }

    // --- Draw X and Y axes (darker gray for contrast) ---
    DrawLine(0, (int)y0, (int)width, (int)y0, GRAY);  // X-axis
    DrawLine((int)x0, 0, (int)x0, (int)height, GRAY); // Y-axis

    // --- Draw small arrowheads at the end of each axis ---
    DrawTriangle((Vector2){width - 10, y0 - 4}, (Vector2){width - 10, y0 + 4}, (Vector2){width, y0}, GRAY); // X arrow
    DrawTriangle((Vector2){x0 - 4, 10}, (Vector2){x0 + 4, 10}, (Vector2){x0, 0}, GRAY);                     // Y arrow

    // --- Axis labels ---
    DrawText("X", width - 15, y0 + 5, 10, GRAY);
    DrawText("Y", x0 + 5, 5, 10, GRAY);
}

/*
    the scene to read the keyboard input of the functions
    expressions in a pointer to pointer to char, so that it can modify the original array of strings using malloc
*/
void inputScene(Scene *nextScene, char ***expressions, int *count)
{
    static int selected = 0;                    // selected line index
    static bool editing = false;                // edit mode flag
    static char buffer[EXPRESSION_BUFFER] = ""; // temporary buffer

    Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
    const int paletteSize = sizeof(palette) / sizeof(palette[0]);

    // allocates the expressions if needed
    if (*expressions == NULL)
    {
        *expressions = calloc(MAX_FUNCTIONS, sizeof(char *));
        *count = MAX_FUNCTIONS;
        for (int i = 0; i < MAX_FUNCTIONS; i++)
        {
            (*expressions)[i] = calloc(EXPRESSION_BUFFER, sizeof(char));
            (*expressions)[i][0] = '\0';
        }
    }

    // ──────────────────────────────
    // editing
    // ──────────────────────────────
    if (editing)
    {
        readExpression(buffer);
        DrawText(buffer, 40, 10, 20, palette[selected % paletteSize]);

        if (IsKeyPressed(KEY_ENTER))
        {
            strcpy((*expressions)[selected], buffer);
            buffer[0] = '\0';
            editing = false;
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            buffer[0] = '\0';
            editing = false;
        }

        if (IsKeyPressed(KEY_BACKSPACE) && strlen(buffer) > 0)
        {
            buffer[strlen(buffer) - 1] = '\0';
        }

        return;
    }

    // ──────────────────────────────
    // navigation
    // ──────────────────────────────
    DrawText("Insert desired functions", 10, 8, 20, BLACK);
    for (int i = 0; i < *count; i++)
    {
        Color color = palette[i % paletteSize];
        bool isSelected = (i == selected);

        // highlights selected line
        if (isSelected)
        {
            DrawRectangle(15, 33 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
        }

        char label[16];
        snprintf(label, sizeof(label), "Y%d =", i + 1);

        DrawText(label, 20, 38 + 30 * i, 20, color);
        DrawText((*expressions)[i], 80, 38 + 30 * i, 20, color);
    }

    if (IsKeyPressed(KEY_DOWN))
        selected = (selected + 1) % *count;

    if (IsKeyPressed(KEY_UP))
        selected = (selected - 1 + *count) % *count;

    if (IsKeyPressed(KEY_ENTER))
    {
        strcpy(buffer, (*expressions)[selected]);
        editing = true;
    }

    if (IsKeyPressed(KEY_DELETE))
    {
        (*expressions)[selected][0] = '\0';
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        int nonEmptyCount = 0;
        for (int i = 0; i < *count; i++)
            if (strlen((*expressions)[i]) > 0)
                nonEmptyCount++;

        if (nonEmptyCount > 0)
            *nextScene = GRAPHICS;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        *expressions = NULL;
        *count = 0;

        *nextScene = MENU;
    }
}

// vWindow scene, you can set x and y max and min
void vWindowScene(Scene *nextScene)
{
    static int selected = 0;
    static bool editing = false;
    static char buffer[32] = "";

    const char *labels[4] = {"Xmin =", "Xmax =", "Ymin =", "Ymax ="};
    float *vars[4] = {&xMin, &xMax, &yMin, &yMax};

    // INPUT
    if (!editing)
    {
        if (IsKeyPressed(KEY_DOWN))
            selected = (selected + 1) % 4;
        if (IsKeyPressed(KEY_UP))
            selected = (selected - 1 + 4) % 4;

        if (IsKeyPressed(KEY_ENTER))
        {
            editing = true;
            snprintf(buffer, sizeof(buffer), "%.2f", *vars[selected]);
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            *nextScene = GRAPHICS;
        }
    }
    else
    {
        // reads entered chars
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126 && strlen(buffer) < sizeof(buffer) - 1)
            {
                size_t len = strlen(buffer);
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && strlen(buffer) > 0)
        {
            buffer[strlen(buffer) - 1] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            *vars[selected] = atof(buffer);
            buffer[0] = '\0';
            editing = false;
        }
    }

    // DRAW
    DrawText("edit graphic window", 10, 10, 24, BLACK);

    for (int i = 0; i < 4; i++)
    {
        bool isSelected = (i == selected);
        Color color = BLACK;

        char valText[32];

        snprintf(valText, sizeof(valText), "%.2f", *vars[i]);

        if (isSelected)
        {
            int textWidth = MeasureText(labels[i], menuFontSize) + MeasureText(valText, menuFontSize);
            DrawRectangle(5, 40 + 30 * i, textWidth + 2, menuFontSize, SKYBLUE);
        }
        DrawText(labels[i], 10, 40 + 30 * i, 20, color);

        if (isSelected && editing)
        {
            DrawText(buffer, 80, 40 + 30 * i, 20, BLACK);
            if ((int)GetTime() % 2 == 0)
            {
                DrawText("|", 80 + (MeasureText(buffer, menuFontSize)*0.8), 40 + 30 * i, 20, BLACK);
            }
        }

        else
            DrawText(valText, 80, 40 + 30 * i, 20, BLACK);
    }
}

void gSolveScene(Scene *nextScene, int *count, char ***expressions)
{
    static int selectedOption = 0; // persistent across frames
    static bool isOptionSelected = false;

    if (!isOptionSelected) // if options is still not selected
        menuSample(gSolveOptionsName, gSolveOptions, &selectedOption);

    if (IsKeyPressed(KEY_ENTER))
        isOptionSelected = true;

    if (IsKeyPressed(KEY_ESCAPE) && !isOptionSelected) // differentiated from the esc in switch: case 0
    {
        *nextScene = GRAPHICS;
        return;
    }

    if (isOptionSelected)
    {
        static bool justEntered0 = true;

        Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
        const int paletteSize = sizeof(palette) / sizeof(palette[0]);

        static int hovered = 0;

        switch (selectedOption)
        {
        case 0: // IntSect code

            static int intSectFuncs[2] = {-1, -1};
            static int intSectStage = 0;

            static Vector2 intersections[MAX_INTERSECTIONS];
            static int intsectsNum = 0;

            static bool calcJustEntered = true;

            // Ignores first frame input
            if (justEntered0)
            {
                // resets the static variables if is the first frame
                hovered = 0;
                intSectFuncs[0] = -1;
                intSectFuncs[1] = -1;
                intSectStage = 0;

                justEntered0 = false;
                break; // exits right away
            }

            if (IsKeyPressed(KEY_DOWN))
            {
                hovered++;
            }

            if (IsKeyPressed(KEY_UP))
            {
                hovered--;
            }

            // comes back to gSolve menu
            if (IsKeyPressed(KEY_ESCAPE))
            {
                isOptionSelected = false;
                justEntered0 = true;
                calcJustEntered = true;
                break;
            }

            if (IsKeyPressed(KEY_ENTER) && intSectStage < 2)
            {
                // doesn't allow to pick a empty function, doesn't allow to pick the same expression twice
                if (intSectFuncs[0] == hovered || (*expressions)[hovered][0] == '\0')
                    break;
                intSectFuncs[intSectStage] = hovered;
                intSectStage++;
            }

            if (intSectStage == 2)
            {
                if (calcJustEntered)
                {
                    char func1RPN[MAX_TOKENS];
                    char func2RPN[MAX_TOKENS];
                    shuntingYard((*expressions)[intSectFuncs[0]], func1RPN);
                    shuntingYard((*expressions)[intSectFuncs[1]], func2RPN);

                    intsectsNum = findIntSects(func1RPN, func2RPN, intersections);
                    calcJustEntered = false;
                }

                for (int i = 0; i < intsectsNum; i++)
                {
                    char label[64];
                    snprintf(label, sizeof(label), "int %d = (%.2f, %.2f)",
                             i + 1, intersections[i].x, intersections[i].y);

                    DrawText(label, 20, 10 + 30 * i, 20, BLACK);
                }

                break;
            }

            for (int i = 0; i < *count; i++)
            {
                Color color = palette[i % paletteSize];

                // Highlights the hovered
                if (hovered == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
                }

                if (intSectFuncs[0] == i || intSectFuncs[1] == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(SKYBLUE, 0.4f));
                }

                char label[16];
                snprintf(label, sizeof(label), "Y%d =", i + 1);

                // highlights
                DrawText(label, 20, 10 + 30 * i, 20, color);
                DrawText((*expressions)[i], 80, 10 + 30 * i, 20, color);
            }

            break;

        case 1:
            static bool justEntered1 = true, computeRoots = true, funcRootSelected = false;
            static int rootCount;
            static Vector2 roots[MAX_INTERSECTIONS];

            // Ignores first frame input
            if (justEntered1)
            {
                // resets the static variables if is the first frame
                hovered = 0;
                rootCount = -1;
                intSectStage = 0;

                justEntered1 = false;
                break; // exits right away
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                isOptionSelected = false;
                justEntered1 = true;
                computeRoots = true;
                funcRootSelected = false;
                break;
            }

            if (IsKeyPressed(KEY_ENTER) || funcRootSelected)
            {
                if (computeRoots)
                {
                    char funcRootRPN[MAX_TOKENS];
                    shuntingYard((*expressions)[hovered], funcRootRPN);

                    rootCount = findAxisIntersections(funcRootRPN, roots, true);
                }

                for (int i = 0; i < rootCount; i++)
                {
                    char label[64];
                    snprintf(label, sizeof(label), "roots: (%.2f, %.2f)",
                             roots[i].x, roots[i].y);

                    DrawText(label, 20, 10 + 30 * i, 20, BLACK);
                }

                funcRootSelected = true;
                break;
            }

            if (IsKeyPressed(KEY_DOWN))
            {
                hovered++;
            }

            if (IsKeyPressed(KEY_UP))
            {
                hovered--;
            }

            for (int i = 0; i < *count; i++)
            {
                Color color = palette[i % paletteSize];

                // Highlights the hovered
                if (hovered == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
                }

                char label[16];
                snprintf(label, sizeof(label), "Y%d =", i + 1);

                // highlights
                DrawText(label, 20, 10 + 30 * i, 20, color);
                DrawText((*expressions)[i], 80, 10 + 30 * i, 20, color);
            }
            break;

        case 2:
            static bool justEntered2 = true, computeYSepts = true, funcYSeptSelected = false;
            static int YSeptCount;
            static Vector2 Ycepts[MAX_INTERSECTIONS];

            // Ignores first frame input
            if (justEntered2)
            {
                // resets the static variables if is the first frame
                hovered = 0;
                YSeptCount = -1;
                intSectStage = 0;

                justEntered2 = false;
                break; // exits right away
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                isOptionSelected = false;
                justEntered2 = true;
                computeYSepts = true;
                funcYSeptSelected = false;
                break;
            }

            if (IsKeyPressed(KEY_ENTER) || funcYSeptSelected)
            {
                if (computeYSepts)
                {
                    char funcYSeptRPN[MAX_TOKENS];
                    shuntingYard((*expressions)[hovered], funcYSeptRPN);

                    YSeptCount = findAxisIntersections(funcYSeptRPN, Ycepts, false);
                }

                if (YSeptCount == 1)
                {
                    char label[64];
                    snprintf(label, sizeof(label), "Y-intercept: (%.2f, %.2f)",
                             Ycepts[0].x, Ycepts[0].y);

                    DrawText(label, 20, 10, 20, BLACK);
                }
                else
                {
                    DrawText("No Y-intercept.", 20, 10, 20, BLACK);
                }

                funcYSeptSelected = true;
                break;
            }

            if (IsKeyPressed(KEY_DOWN))
            {
                hovered++;
            }

            if (IsKeyPressed(KEY_UP))
            {
                hovered--;
            }

            for (int i = 0; i < *count; i++)
            {
                Color color = palette[i % paletteSize];

                // Highlights the hovered
                if (hovered == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
                }

                char label[16];
                snprintf(label, sizeof(label), "Y%d =", i + 1);

                // highlights
                DrawText(label, 20, 10 + 30 * i, 20, color);
                DrawText((*expressions)[i], 80, 10 + 30 * i, 20, color);
            }
            break;

        default:
            break;
        }
    }
}

void menuSample(const char **optionsName, int optionsNum, int *selected)
{
    Vector2 startingPos = {20, 20};

    // Navigation
    if (IsKeyPressed(KEY_DOWN))
        *selected = (*selected + 1 >= optionsNum) ? 0 : *selected + 1;

    if (IsKeyPressed(KEY_UP))
        *selected = (*selected == 0) ? optionsNum - 1 : *selected - 1;

    // Draw menu
    for (int i = 0; i < optionsNum; i++)
    {
        Vector2 btnPos = {startingPos.x, startingPos.y + i * menuFontSize};
        drawMenuButton(optionsName[i], btnPos, *selected == i);
    }
}