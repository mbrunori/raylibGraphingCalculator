#include "./shuntingYard.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 256
#define MAX_OPS 256

//================
//-----BUFFER-----
//================

//appends a token to the buffer, adding a space and a EOF
void outAppend(Output* o, const char* s)
{
    size_t n = strlen(s);

    //overflow check
    if(o -> len + n + 2 >= sizeof(o -> out)) //+2 for the ' ' and  '\0'
    {
        fprintf(stderr, "buffer overflow\n");
        exit(1);
    }

    memcpy(o->out + o->len, s, n); //copy in out starting pos + the lenght
    o->len += n;
    o->out[o->len++] = ' ';
    o->out[o->len] = '\0'; 
    
}

void outTrimTrailingSpace(Output* o)
{
    if (o->len > 0 && o->out[o->len - 1] == ' ') {
        o->len--;
        o->out[o->len] = '\0';
    }
}

//========================
//-----OPERATOR STACK-----
//========================

void stackInit(OpStack *s) { s->top = -1; }
int  stack_empty(const OpStack *s) { return s->top < 0; }
char stack_peek(const OpStack *s) { return s->items[s->top]; }
void stack_push(OpStack *s, char c) {
    if (s->top + 1 >= MAX_OPS) {
        fprintf(stderr, "Operator stack overflow\n");
        exit(1);
    }
    s->items[++s->top] = c;
}
char stack_pop(OpStack *s) {
    if (stack_empty(s)) {
        fprintf(stderr, "Operator stack underflow\n");
        exit(1);
    }
    return s->items[s->top--];
}

//=======================
//-----SHUNTING YARD-----
//=======================

// unary minus is encoded as '~' (highest precedence, right associative)

//helpers
int isOp(char op) {
    return(op=='+' || op=='-' || op=='*' || op=='/' || op=='^' || op=='~' || op=='A' || op=='r');
}

int precedence(char op) {
    switch (op) {
        case '~': return 5; // unary minus
        case 'A': return 5; // abs
        case 'r': return 5; // sqrt function
        case '^': return 4;
        case '*':
        case '/': return 3;
        case '+':
        case '-': return 2;
        default:  return -1;
    }
}

int rightAssociative(char op) {
    return (op == '^' || op == '~' || op == 'A' || op == 'r');
}
int isIdentStart(unsigned char c)
{
    return isalpha(c);   // letters only for now
}

char* shuntingYard(const char* str)
{
    //initialization
    Output out = {.out={0}, .len = 0};
    OpStack ops; stackInit(&ops);

    //we keep track of the previous token type for detecting unary minus
    //0:start/none, 1=number, 2=operator, 3=left parenthesis, 4=right parenthesis
    int prev = 0;

    for(size_t i = 0; str[i] != '\0';)
    {
        unsigned char ch = (unsigned char)str[i]; //unsigned to avoid c's autoassumptions

        //skip whitespaces
        if(isspace(ch)) {i++; continue;}

        //numbers
        if(isdigit(ch))
        {
            char buf[64];
            size_t j = 0;

            while (isdigit((unsigned char)str[i]))
            {
                //error handling
                if(j+1 >= sizeof(buf))
                {
                    fprintf(stderr, "number token too long\n");
                    exit(1);
                }

                buf[j++] = str[i++];
            }

            buf[j] = '\0';
            outAppend(&out, buf);
            prev = 1;
            continue;
        }

        //parentheses
        //left
        if(ch == '(')
        {
            stack_push(&ops, ch);
            i++;
            prev = 3;
            continue;
        }
        //right
        if(ch == ')')
        {
            //pops until '()'
            int found = 0;
            while(!stack_empty(&ops))
            {
                char t = stack_pop(&ops);
                if(t=='('){found=1; break;}

                char opbuf[2] = {t, '\0'};
                outAppend(&out, opbuf);
            }

            if(!found)
            {
                fprintf(stderr, "syntax error: closing a non-opened paretheses\n");
                exit(1);
            }

            if (!stack_empty(&ops) && stack_peek(&ops) == 'r') 
            {
                char t = stack_pop(&ops);
                char opbuf[2] = { t, '\0' };
                outAppend(&out, opbuf);
            }


            i++;
            prev = 4;
            continue;
        }

        if (ch == '|')
        {
            // Decide if this '|' is opening or closing.
            // Opening if we're "expecting an operand":
            // start, after operator, after '('
            int isOpening = (prev == 0 || prev == 2 || prev == 3);

            if (isOpening)
            {
                // treat like '(' marker
                stack_push(&ops, '|');
                i++;
                prev = 3; // like left parenthesis
                continue;
            }
            else
            {
                // closing bar: pop until matching '|'
                int found = 0;
                while (!stack_empty(&ops))
                {
                    char t = stack_pop(&ops);
                    if (t == '|') { found = 1; break; }

                    char opbuf[2] = { t, '\0' };
                    outAppend(&out, opbuf);
                }

                if (!found)
                {
                    fprintf(stderr, "syntax error: closing '|' without opening '|'\n");
                    exit(1);
                }

                // emit ABS operator
                char absbuf[2] = { 'A', '\0' };
                outAppend(&out, absbuf);

                i++;
                prev = 4; // behaves like a closed group / operand
                continue;
            }
        }


        if(ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='^')
        {
            char o1 = (char)ch;

            //unary minus detection
            if(o1 == '-' && (prev == 0 || prev == 2 || prev == 3))
            {
                o1 = '~';
            }

            while (!stack_empty(&ops))
            {
                char o2 = stack_peek(&ops);
                if (!isOp(o2) || o2 == '(') break;

                int p1 = precedence(o1);
                int p2 = precedence(o2);

                //pop until:
                //  o2 has a higher precedence than o1
                //  they have equal precedence is left-associative

                if(p2 > p1 || (p1 == p2 && !rightAssociative(o1)))
                {
                    (void)stack_pop(&ops);
                    char opbuf[2] = {o2, '\0'};
                    outAppend(&out, opbuf);
                }
                else
                    break;
            }
            stack_push(&ops, o1);
            i++;
            prev = 2;
            continue;
        }

        // sqrt function: r(...)
        if (ch == 'r')
        {
            // require it to be used like a function call: r(
            size_t k = i + 1;
            while (isspace((unsigned char)str[k])) k++;

            if (str[k] != '(') {
                fprintf(stderr, "syntax error: 'r' must be followed by '('\n");
                exit(1);
            }

            // push function marker onto operator stack
            stack_push(&ops, 'r');
            i++;        // consume 'r'
            prev = 2;   // behaves like an operator (expects '(' then operand)
            continue;
        }

        // identifiers (variables)
        if (isIdentStart(ch))
        {
            char buf[64];
            size_t j = 0;

            while (isalnum((unsigned char)str[i]))
            {
                if (j + 1 >= sizeof(buf)) {
                    fprintf(stderr, "identifier too long\n");
                    exit(1);
                }
                buf[j++] = str[i++];
            }

            buf[j] = '\0';
            outAppend(&out, buf);
            prev = 1;   // behaves like a number
            continue;
        }

        fprintf(stderr, "unexpected char '%c'\n", str[i]);
        exit(1);
    }

    //pop all remaining operators
    while (!stack_empty(&ops)) {
        char t = stack_pop(&ops);
        if (t == '(' || t == ')') {
            fprintf(stderr, "mismatched parentheses\n");
            exit(1);
        }
        char opbuf[2] = {t, '\0'};
        outAppend(&out, opbuf);
    }

    //remove unnecessary
    outTrimTrailingSpace(&out);

    //allocate and return the result
    char* res = (char*)malloc(out.len + 1);
    if (!res) { fprintf(stderr, "out of memory\n"); exit(1); }
    memcpy(res, out.out, out.len + 1);
    return res;
}

void addExplicitMult(char **str) 
{
    if (!str || !*str) return;

    size_t len = strlen(*str);
    char *input = *str;

    // worst case: every character needs a '*', so double size
    char *newStr = malloc(len * 2 + 1);
    if (!newStr) return;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char curr = input[i];
        char next = (i + 1 < len) ? input[i + 1] : '\0';

        newStr[j++] = curr;

        if ((isdigit(curr) || isalpha(curr) || curr == ')') &&
            (isdigit(next) || isalpha(next) || next == '(')) {
            newStr[j++] = '*';
        }
    }

    newStr[j] = '\0';

    free(*str);
    *str = newStr;
}