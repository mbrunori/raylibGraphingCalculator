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

// returns precedence of operators (sin, cos, tan, roots are not considered here since they need parenthesis)
int precedence(char op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    if (op == '^')
        return 3;
    return 0;
}

// Inserts explicit multiplication operator (*) where needed
void addExplicitMultiplication(char *s)
{
    int n = strlen(s);
    int extra = 0;

    // Calculate new length with extra '*' characters
    for (int i = 0; i < n - 1; i++)
    {
        char c = s[i];
        char next = s[i + 1];

        // not between function letters
        if (isalpha((unsigned char)c) && isalpha((unsigned char)next))
            continue;

        // not between function name and '('
        for (int len = 1; len <= 10; len++)
        {
            if (i - len + 1 < 0)
                break;
            char temp[11] = {0};
            strncpy(temp, s + i - len + 1, len);
            if (isFunction(temp) && next == '(')
            {
                goto skip_add; // avoids adding *
            }
        }

        // general cases
        if ((isdigit((unsigned char)c) && isalpha((unsigned char)next)) ||
            ((isdigit((unsigned char)c) || isalpha((unsigned char)c)) && next == '(') ||
            (c == ')' && (isdigit((unsigned char)next) || isalpha((unsigned char)next) || next == '(')))
        {
            extra++;
        }
    skip_add:;
    }

    int newLen = n + extra;
    s[newLen] = '\0';

    // write from the end to avoid overwriting
    for (int i = n - 1, j = newLen - 1; i >= 0; i--)
    {
        s[j--] = s[i];

        if (i == 0)
            break;

        char c = s[i - 1];
        char next = s[i];
        bool need = false;

        // not between function letters
        if (isalpha((unsigned char)c) && isalpha((unsigned char)next))
            continue;

        // not between function name and '('
        for (int len = 1; len <= 10; len++)
        {
            if (i - 1 - len + 1 < 0)
                break;
            char temp[11] = {0};
            strncpy(temp, s + i - 1 - len + 1, len);
            if (isFunction(temp) && next == '(')
            {
                goto skip_star; // avoid *
            }
        }

        if ((isdigit((unsigned char)c) && isalpha((unsigned char)next)) ||
            ((isdigit((unsigned char)c) || isalpha((unsigned char)c)) && next == '(') ||
            (c == ')' && (isdigit((unsigned char)next) || isalpha((unsigned char)next) || next == '(')))
        {
            need = true;
        }

        if (need)
            s[j--] = '*';
    skip_star:;
    }
}

// Checks if a character is a valid operator
bool isOperator(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

// Checks if string is a function name
bool isFunction(const char *str)
{
    return (strcmp(str, "sin") == 0 || strcmp(str, "cos") == 0 ||
            strcmp(str, "tan") == 0 || strcmp(str, "A") == 0 ||
            strcmp(str, "r") == 0);
}

/*
    Shunting Yard algorithm - converts infix notation to RPN
    Supports: ^, sin, cos, tan, abs (||)
    Input: Mathematical expression in infix notation (e.g., "sin(x)+3^2")
    Output: Expression in Reverse Polish Notation (e.g., "x sin 3 2 ^ +")
*/
void shuntingYard(char *input, char *output)
{
    char operatorStack[EXPRESSION_BUFFER];
    int stackTop = -1;
    int outputPos = 0;

    output[0] = '\0';

    char preprocessed[EXPRESSION_BUFFER];
    int prePos = 0;
    int absOpen = 0; // 0 = chiuso, 1 = aperto

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '|')
        {
            if (absOpen == 0) // Opening |
            {
                strcpy(preprocessed + prePos, "A(");
                prePos += 2;
                absOpen = 1;
            }
            else // Closing |
            {
                preprocessed[prePos++] = ')';
                absOpen = 0;
            }
        }
        else
        {
            preprocessed[prePos++] = input[i];
        }
    }

    addExplicitMultiplication(input);
    preprocessed[prePos] = '\0';

    // use preprocessed string
    input = preprocessed;
    int len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        char token = input[i];

        // Skip whitespace characters
        if (isspace((unsigned char)token))
            continue;

        // Handle numbers (including negative numbers and decimals)
        if (isdigit((unsigned char)token) ||
            (token == '-' && (i == 0 || input[i - 1] == '(' || isOperator(input[i - 1])) &&
             i + 1 < len && isdigit((unsigned char)input[i + 1])))
        {
            int start = i;
            if (token == '-')
                i++; // Include the minus sign
            // Continue reading digits and decimal points
            while (i < len && (isdigit((unsigned char)input[i]) || input[i] == '.'))
                i++;
            int numLen = i - start;
            // Copy the number to output
            strncpy(output + outputPos, input + start, numLen);
            outputPos += numLen;
            output[outputPos++] = ' ';
            output[outputPos] = '\0';
            i--; // Adjust for loop increment
        }

        // Handle functions (sin, cos, tan) and variable x
        else if (isalpha((unsigned char)token))
        {
            char func[10];
            int funcPos = 0;

            // Read the whole function name or variable
            while (i < len && isalpha((unsigned char)input[i]))
            {
                func[funcPos++] = input[i++];
            }
            func[funcPos] = '\0';
            i--; // Adjust for loop increment

            // Check if it's a function
            if (isFunction(func))
            {
                // Push a marker (like 'T' for tan, 'S' for sin, etc.)
                if (strcmp(func, "sin") == 0)
                    operatorStack[++stackTop] = 'S';
                else if (strcmp(func, "cos") == 0)
                    operatorStack[++stackTop] = 'C';
                else if (strcmp(func, "tan") == 0)
                    operatorStack[++stackTop] = 'T';
                else if (strcmp(func, "r") == 0)
                    operatorStack[++stackTop] = 'r';
                else if (strcmp(func, "A") == 0)
                    operatorStack[++stackTop] = 'A';
            }

            else // It's a variable (x)
            {
                strcpy(output + outputPos, func);
                outputPos += strlen(func);
                output[outputPos++] = ' ';
                output[outputPos] = '\0';
            }
        }

        // Handle opening parenthesis
        else if (token == '(')
        {
            operatorStack[++stackTop] = token;
        }

        // Handle closing parenthesis
        else if (token == ')')
        {
            // Pop operators until we find the matching opening parenthesis
            while (stackTop >= 0 && operatorStack[stackTop] != '(')
            {
                output[outputPos++] = operatorStack[stackTop--];
                output[outputPos++] = ' '; // <-- ADD SPACE after each operator
                output[outputPos] = '\0';
            }
            // Remove the opening parenthesis from stack
            if (stackTop >= 0 && operatorStack[stackTop] == '(')
                stackTop--;

            // Check if there's a function marker (A, S, C, T, r)
            if (stackTop >= 0 && (operatorStack[stackTop] == 'A' || operatorStack[stackTop] == 'S' ||
                                  operatorStack[stackTop] == 'C' || operatorStack[stackTop] == 'T' ||
                                  operatorStack[stackTop] == 'r'))
            {
                output[outputPos++] = operatorStack[stackTop--];
                output[outputPos++] = ' '; // <-- IMPORTANT: Add space after function marker
                output[outputPos] = '\0';
            }
        }

        // Handle operators (+, -, *, /, ^)
        else if (isOperator(token))
        {
            // ^ is right-associative, others are left-associative
            if (token == '^')
            {
                while (stackTop >= 0 && isOperator(operatorStack[stackTop]) &&
                       precedence(operatorStack[stackTop]) > precedence(token))
                {
                    output[outputPos++] = operatorStack[stackTop--];
                    output[outputPos++] = ' ';
                    output[outputPos] = '\0';
                }
            }
            else
            {
                while (stackTop >= 0 && isOperator(operatorStack[stackTop]) &&
                       precedence(operatorStack[stackTop]) >= precedence(token))
                {
                    output[outputPos++] = operatorStack[stackTop--];
                    output[outputPos++] = ' ';
                    output[outputPos] = '\0';
                }
            }
            // Push current operator onto stack
            operatorStack[++stackTop] = token;
        }
    }

    // Pop remaining operators from stack
    while (stackTop >= 0)
    {
        if (operatorStack[stackTop] != '(' && operatorStack[stackTop] != ')')
        {
            output[outputPos++] = operatorStack[stackTop--];
            output[outputPos++] = ' ';
            output[outputPos] = '\0';
        }
        else
        {
            stackTop--;
        }
    }
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