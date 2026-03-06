#include "logic.h"

float xMin = -6.0f, xMax = 6.0f, yMin = -6.0f, yMax = 6.0f, step = 0.001f;

int readExpression(char *expression)
{
    int nextChar = GetCharPressed();
    if (nextChar == 0)
        return 0; // no input

    size_t len = strlen(expression);
    if (len >= EXPRESSION_BUFFER - 1)
        return 0;

    // only allows valid characters
    if ((nextChar >= '0' && nextChar <= '9') ||
        (nextChar >= 'a' && nextChar <= 'z') ||
        nextChar == '+' || nextChar == '-' ||
        nextChar == '*' || nextChar == '/' ||
        nextChar == '^' || nextChar == '|' ||
        nextChar == '(' || nextChar == ')' ||
        nextChar == 'A')
    {
        expression[len] = (char)nextChar;
        expression[len + 1] = '\0';
        return 1;
    }

    // backspace
    if (nextChar == 8) // KEY_BACKSPACE
    {
        if (len > 0)
            expression[len - 1] = '\0';
        return 1;
    }

    return 0;
}

/*
    Evaluates a Reverse Polish Notation expression
    Input: RPN expression string and value for variable x
    Output: Calculated result as double
*/
double evaluateRPN(const char *rpn, double xValue)
{
    double stack[MAX_TOKENS];
    int stackTop = -1; // Stack pointer
    char token[50];
    int outputPos = 0;

    // Parse the RPN expression token by token
    for (int i = 0;; i++)
    {
        char c = rpn[i];

        // Token delimiter (space or end of string)
        if (isspace((unsigned char)c) || c == '\0')
        {
            if (outputPos > 0)
            {
                token[outputPos] = '\0';
                outputPos = 0;

                // Handle numbers (including negative numbers)
                if (isdigit((unsigned char)token[0]) ||
                    (token[0] == '-' && isdigit((unsigned char)token[1])))
                {
                    stack[++stackTop] = atof(token);
                }

                // Handle variable 'x'
                else if (strcmp(token, "x") == 0)
                {
                    stack[++stackTop] = xValue;
                }

                // Handle abs function
                else if (token[0] == 'A' && token[1] == '\0')
                {
                    if (stackTop >= 0)
                    {
                        double a = stack[stackTop--];
                        stack[++stackTop] = fabs(a);
                    }
                }

                // Similarly for the other function markers:
                else if (token[0] == 'S' && token[1] == '\0')
                {
                    if (stackTop >= 0)
                    {
                        double a = stack[stackTop--];
                        stack[++stackTop] = sin(a);
                    }
                }

                else if (token[0] == 'r' && token[1] == '\0')
                {
                    if (stackTop >= 0)
                    {
                        double a = stack[stackTop--];
                        stack[++stackTop] = sqrt(a);
                    }
                }

                else if (token[0] == 'C' && token[1] == '\0')
                {
                    if (stackTop >= 0)
                    {
                        double a = stack[stackTop--];
                        stack[++stackTop] = cos(a);
                    }
                }

                else if (token[0] == 'T' && token[1] == '\0')
                {
                    if (stackTop >= 0)
                    {
                        double a = stack[stackTop--];
                        stack[++stackTop] = tan(a);
                    }
                }

                // Handle addition operator
                else if (strcmp(token, "+") == 0)
                {
                    if (stackTop >= 1)
                    {
                        double b = stack[stackTop--];
                        double a = stack[stackTop--];
                        stack[++stackTop] = a + b;
                    }
                }

                // Handle subtraction operator
                else if (strcmp(token, "-") == 0)
                {
                    if (stackTop >= 1)
                    {
                        double b = stack[stackTop--];
                        double a = stack[stackTop--];
                        stack[++stackTop] = a - b;
                    }
                }

                // Handle multiplication operator
                else if (strcmp(token, "*") == 0)
                {
                    if (stackTop >= 1)
                    {
                        double b = stack[stackTop--];
                        double a = stack[stackTop--];
                        stack[++stackTop] = a * b;
                    }
                }

                // Handle division operator
                else if (strcmp(token, "/") == 0)
                {
                    if (stackTop >= 1)
                    {
                        double b = stack[stackTop--];
                        double a = stack[stackTop--];
                        if (b != 0)
                            stack[++stackTop] = a / b;
                        else
                            stack[++stackTop] = 0; // Division by zero protection
                    }
                }

                // Handle power operator ^
                else if (strcmp(token, "^") == 0)
                {
                    if (stackTop >= 1)
                    {
                        double b = stack[stackTop--];
                        double a = stack[stackTop--];
                        stack[++stackTop] = pow(a, b);
                    }
                }
            }
            // Break if we reached the end of the string
            if (c == '\0')
                break;
        }
        else
        {
            // Build the current token character by character
            token[outputPos++] = c;
        }
    }

    // Return the final result (top of the stack)
    return (stackTop >= 0) ? stack[stackTop] : 0;
}

/* Finds intersection points between two functions defined by their RPN expressions
   intersections is an array of Vector2 to store intersection points
   Returns the number of intersection points found
*/
int findIntSects(char *func1, char *func2, Vector2 *intersections)
{
    float prevDiff, currDiff;
    float func1Y, func2Y;
    int count = 0;

    // Get initial difference
    float x = xMin;
    func1Y = evaluateRPN(func1, x);
    func2Y = evaluateRPN(func2, x);
    prevDiff = func1Y - func2Y;

    // Scan for sign changes
    for (x = xMin + step; x <= xMax && count < MAX_INTERSECTIONS; x += step)
    {
        func1Y = evaluateRPN(func1, x);
        func2Y = evaluateRPN(func2, x);
        currDiff = func1Y - func2Y;

        // Sign change detected - functions crossed
        if (prevDiff * currDiff < 0)
        {
            // Linear interpolation to find intersection point
            float xPrev = x - step;
            float t = fabsf(prevDiff) / (fabsf(prevDiff) + fabsf(currDiff));
            float xIntersect = xPrev + t * step;

            // Calculate Y at intersection point
            float yIntersect = evaluateRPN(func1, xIntersect);

            intersections[count] = (Vector2){xIntersect, yIntersect};
            count++;

            prevDiff = currDiff;
        }
    }
    return count;
}

//generally finds intersection with axis (unique function for both axis)
int findAxisIntersections(char *func, Vector2 roots[], bool isXAxis)
{
    float funcValue, prevValue;
    int count = 0;
    bool inZeroZone = false;
    bool prevWasNaN = false;

    //computes intersections with x axis
    //double algorithm: first looks for sign changes in the function values
    //if no result, it looks for tangents using a range
    if (isXAxis)
    {
        prevValue = evaluateRPN(func, xMin);
        prevWasNaN = isnan(prevValue);

        // Check if function starts at zero
        if (!prevWasNaN && fabs(prevValue) <= step * 10)
        {
            roots[count++] = (Vector2){xMin, 0};
            inZeroZone = true;
        }

        for (float funcX = xMin + step; funcX <= xMax && count < MAX_INTERSECTIONS; funcX += step)
        {
            funcValue = evaluateRPN(func, funcX);
            bool currIsNaN = isnan(funcValue);

            // Transitioning from NaN to valid (function boundary - likely a root)
            if (prevWasNaN && !currIsNaN && fabs(funcValue) < 0.1)
            {
                roots[count++] = (Vector2){funcX, 0};
                inZeroZone = true;
            }
            // Sign change detected
            else if (!prevWasNaN && !currIsNaN && prevValue * funcValue < 0 && !inZeroZone)
            {
                float prevX = funcX - step;
                float x_intercept = prevX - prevValue * (step / (funcValue - prevValue));
                roots[count++] = (Vector2){x_intercept, 0};
            }
            // Very close to zero
            else if (!currIsNaN && fabs(funcValue) < step * 5 && !inZeroZone)
            {
                roots[count++] = (Vector2){funcX, 0};
                inZeroZone = true;
            }
            // Leaving zero zone
            else if (!currIsNaN && fabs(funcValue) > step * 10 && inZeroZone)
            {
                inZeroZone = false;
            }

            prevValue = funcValue;
            prevWasNaN = currIsNaN;
        }

        return count;
    }

    //Computes intersections with Y axis
    //Calculates the value of f(0) and saves it in a Vector2: (0, f(0))
    else 
    {
        if (0.0 >= xMin && 0.0 <= xMax) //only if x=0 is in the view range
        {
            funcValue = evaluateRPN(func, 0.0); //f(0)

            if (!isnan(funcValue))
            {
                // The Y-intercept point is (0, f(0))
                roots[count++] = (Vector2){0.0, funcValue};
            }
        }

        return count;
    }
}