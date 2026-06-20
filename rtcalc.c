#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "definitions.h"
#include "utilities.h"

struct termios backup = { 0 };
int retCode = 0;
char *prompt = ">>> ";

// look for invalid characters
int validateBuffer(char *buffer, int *highestPrio) {
    char *ptr = buffer;
    ssize_t openParentheses = 0; // find '(' and ')'
    uint8_t lastWasParen = 0; // reject '()'
    uint8_t twoNum = 0; // for example, '2   2' is invalid, since no op
    size_t nums = 0, ops = 0;
    uint8_t mode = 0; // 0 for nums, 1 for ops

    while (*ptr) {
        skipWhitespace((const char **)&ptr);
        if (*ptr == '\0') break;
        lastWasParen = 0;

        // functions
        if (!mode) {
            if (getFuncIndex(ptr) != -1) {
                // square root
                int ret = 0;
                char *open = ptr;
                char *close = ptr;
                if ((close = findFuncClose(ptr, &open, &ret)) == NULL || ret) return ret;

                // check if we have empty brackets
                uint8_t isEmpty = 1;
                char *val = open + 1;
                while (val != close) {
                    if (!*val) break;
                    if (!isspace(*val)) {
                        isEmpty = 0;
                        break;
                    }
                    val++;
                }
                if (isEmpty) return 11;

                // make child to be checked
                size_t childLen = close - open - 1;
                char child[childLen + 1];
                memset(child, '\0', childLen + 1);
                memcpy(child, open + 1, childLen);
                if ((ret = validateBuffer(child, NULL))) return ret;

                ptr = close + 1;
                nums++;
                mode = !mode;
                continue;

            }
        }

        // invalid chars
        if (!strchr(VALID_LIST, *ptr)) return 1;

        // find unclosed parentheses
        switch (*ptr) {
            case '(': {
                if (lastWasParen) return 8;
                lastWasParen = 1;

                openParentheses++;
                ptr++;
                continue;
            }
            case ')': {
                if (lastWasParen) return 8;

                if (openParentheses) {
                    openParentheses--;
                } else {
                    return 3;
                }
                ptr++;
                continue;
            }
        }

        // trees
        if (!mode) {
            // numbers
            nums++;
            char *prev = ptr;
            strtod(ptr, &ptr);
            if (ptr == prev) return 4; // pointer didnt move

        } else {
            // operators
            ops++;
            if (!strchr(OPERATIONS, *ptr)) return 5; // invalid operator

            if (highestPrio != NULL) {
                uint8_t prio = getPriority(*ptr), tru = 0;
                if (prio > *highestPrio) *highestPrio = prio;
            }

            ptr++;

        }

        // change modes
        mode = !mode;
    }

    if (nums > (ops + 1)) {
        // too manu numbers
        return 7;
    } else if (ops > (nums - 1)) {
        // too many operators
        return 6;
    }

    if (openParentheses > 0) return 2;
    return 0;
}

// count the number of tokens on formula to find
// correct mem allocate size
size_t countTokens(const char *buf) {
    int count = 0;
    char *ptr = (char *)buf;
    uint8_t mode = 0; // same mode logic as validator

    while (*ptr) {
        skipWhitespace((const char **)&ptr);

        // functions
        if (!mode) {
            if (getFuncIndex(ptr) != -1) {
                char *open = ptr;
                char *close = findFuncClose(ptr, &open, NULL);

                ptr = close + 1;
                count++;
                mode = !mode;
                continue;
            }
        }

        // parentheses
        if (*ptr == '(' || *ptr == ')') {
            ptr++;
            count++;
            continue;
        }

        // switch type
        if (!mode) {
            // numbers
            strtod(ptr, &ptr); // moves pointer
        } else {
            // operators
            ptr++;
        }

        count++;
        mode = !mode;
    }

    return count;
}

// orchestrate everything together
double calculateBuffer(const char *buf, const int highestPrio) {
    // each token eventually gets reduced to result
    size_t count = countTokens(buf);
    struct calcToken tokens[count];
    memset(tokens, '\0', count * sizeof(struct calcToken));

    // populate array
    char *ptr = (char *)buf;
    int j = 0; // what token we are in
    uint8_t mode = 0;
    unsigned int parenLevel = 0;
    while (*ptr) {
        skipWhitespace((const char **)&ptr);

        // functions
        if (!mode) {
            if (getFuncIndex(ptr) != -1) {
                char *open = ptr;
                char *close = findFuncClose(ptr, &open, NULL);
                size_t childPrio = 0;

                // make child
                size_t childLen = close - open - 1;
                char child[childLen + 1];
                memset(child, '\0', sizeof(child));
                memcpy(child, open + 1, childLen);
                // get child highest prio
                for (int i = 0; i < childLen; i++) {
                    if (!strchr(OPERATIONS, child[i])) continue;
                    int loopPrio = getPriority(child[i]);
                    if (loopPrio > childPrio) childPrio = loopPrio;
                }

                tokens[j].type = NUMBER;
                switch (getFuncIndex(ptr)) {
                    case SQUARE_ROOT: tokens[j].val = sqrt(calculateBuffer(child, childPrio)); break;
                    case CUBE_ROOT:   tokens[j].val = cbrt(calculateBuffer(child, childPrio)); break;
                    case SINE:        tokens[j].val = sin (calculateBuffer(child, childPrio)); break;
                    case COSINE:      tokens[j].val = cos (calculateBuffer(child, childPrio)); break;
                    case TANGENT:     tokens[j].val = tan (calculateBuffer(child, childPrio)); break;
                    case N_LOG:       tokens[j].val = log (calculateBuffer(child, childPrio)); break;
                }
                ptr = close + 1;
                j++;
                mode = !mode;
                continue;

            }
        }

        // parentheses
        if (*ptr == '(' || *ptr == ')') {
            tokens[j].type = PARENTHESES;
            tokens[j].ptr = ptr;
            switch (*ptr) {
                case '(': tokens[j].depth = ++parenLevel; break;
                case ')': tokens[j].depth = parenLevel--; break;
            }

            ptr++;
            j++;
            continue;
        }

        // trees
        if (!mode) {
            // numbers
            tokens[j].type = NUMBER;
            tokens[j].val  = strtod(ptr, &ptr);
            j++;

        } else {
            // operators
            tokens[j].type = OPERATOR;
            tokens[j].op   = *ptr;
            ptr++;
            j++;
        }

        // break early if there is nothing fowards
        if (*ptr == '\0') break;

        mode = !mode;
    }

    // compression loop to complete parentheses
    for (int i = 0; i < count; i++) {
        // get values inside a () trough recursive function calls
        if (tokens[i].type == PARENTHESES && *(tokens[i].ptr) == '(') {
            // compress [(][1][+][1][)] to [2][SKIP][SKIP][SKIP][SKIP] (finding the center would be too hard)
            char *start = tokens[i].ptr + 1;
            unsigned int sDepth = tokens[i].depth;
            char *end = start;
            int childPrio = 0;
            int endIdx = 0; // becomes l

            for (int l = i + 1; l < count; l++) {
                int foundPrio = getPriority(tokens[l].op);
                if (foundPrio > childPrio) childPrio = foundPrio;

                if (tokens[l].depth == sDepth) {
                    end = tokens[l].ptr - 1;
                    endIdx = l;
                    break;
                }
            }

            // the buffer passed on
            size_t childLen = end - start + 1;
            char child[childLen + 1];
            memset(child, '\0', sizeof(child));
            memcpy(child, start, childLen);

            // get final
            tokens[i].type = NUMBER;
            tokens[i].val = calculateBuffer(child, childPrio);
            // set skips
            for (int l = i + 1; l <= endIdx; l++) {
                tokens[l].type = SKIP;
            }

            continue;
        }
    }

    // repeat parse loop, looking for highest prio
    for (int prio = highestPrio; prio >= 1; prio--) {

        // get (example) [2][+][2], compress to [SKIP][4][SKIP]
        for (int i = 0; i < count; i++) {
            if (tokens[i].type != OPERATOR ||
                getPriority(tokens[i].op) != prio ||
                tokens[i].type == SKIP) continue;

            // get left value
            double left = 0;
            int lidx = 0;
            for (int j = i - 1; j >= 0; j--) {
                if (tokens[j].type == NUMBER) {
                    left = tokens[j].val;
                    lidx = j;
                    break;
                }
            }

            // get right value
            double right = 0;
            int ridx = 0;
            for (int j = i + 1; j < count; j++) {
                if (tokens[j].type == NUMBER) {
                    right = tokens[j].val;
                    ridx = j;
                    break;
                }
            }

            tokens[i].val = calculateTrio(left, tokens[i].op, right);
            tokens[i].type = NUMBER;
            tokens[lidx].type = SKIP;
            tokens[ridx].type = SKIP;
        }
    }

    double result = 0;
    int i = 0;
    while (i < count) {
        if (tokens[i].type == NUMBER) {
            //free(tokens);
            return tokens[i].val;
        }
        i++;
    }
    return 0;
}

int main (int argc, char *argv[]) {
    if (argc > 1) {
        // loop trough stuffies and match argument
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "help")) {
                helpMenu(NULL, 0);

            } else if (!strncmp(argv[i], "prompt", 6)) {
                // set custom prompt
                char *start = strchr(argv[i], '=');
                if (!start || !*(start + 1))
                    helpMenu("\n\e[31mError: Prompt not defined properly.\e[0,\n", USER_MISTAKE);
                start++;
                prompt = start;

            } else {
                helpMenu("\n\e[31mError: Improper flag used.\e[0m\n", USER_MISTAKE);

            }
        }
    }

    // handle signal
    signal(SIGINT, handleCtrlC);

    struct termios newMode = { 0 };
    // populate backup and define new mode
    if (tcgetattr(STDIN_FILENO, &backup) == -1) {
        fprintf(stderr, "Failed to backup term attributes: %s\n", strerror(errno));
        return CODE_MISTAKE;
    }
    atexit(restoreTerminal);

    newMode = backup;
    newMode.c_lflag &= ~(ICANON | ECHO); // don't wait for new line and dont say anything
    newMode.c_cc[VMIN]  = 1; // get one char at a time
    newMode.c_cc[VTIME] = 0;
    // set changes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &newMode);

    // main loop to pupulate main buffer
    char calcBuffer[BUFFER_SIZE] = { 0 };
    unsigned int cursorPos = 0;

    // show prompt + save cursor pos
    printf("\n%s\e[s", prompt);
    char result[RESULT_SIZE] = { 0 };
    size_t resSize = 0;

    while(1) {
        int highestPrio = 0;
        uint8_t ret = validateBuffer(calcBuffer, &highestPrio); // find errors
        if (cursorPos >= 4096) ret = 9; // input size limit

        // define result
        if (!ret) {
            if (cursorPos == 0 && calcBuffer[0] == '\0') {
                memset(result, '\0', sizeof(result));
                memcpy(result, "Welcome to the realtime math CLI tool!", sizeof(result));

            } else {

                double calc = calculateBuffer(calcBuffer, highestPrio);
                if ((resSize = snprintf(NULL, 0, "%lf", calc)) >= 2048) ret = 12;

                if (ret) {
                    snprintf(result, sizeof(result), "Can't calculate: %s", retToStr(ret));

                } else {
                    snprintf(result, sizeof(result), "%lf", calc);
                }

            }
        } else {
            snprintf(result, sizeof(result), "Can't calculate: %s", retToStr(ret));
        }

        printf("\e[u\e[J"                  // move the cursor back to the start
               "\e[A\r\e[2K%s\r\e[B%s"  // move cursor to result, print the message, return back to start
               "%s%s\e[0m",                // print the buffer
               result, prompt, ret ? "\e[31m" : "", calcBuffer);

        if (ret == 9) continue;

        int len = strlen(calcBuffer);
        // move the cursor
        // only do it when cursor isn't already at the end
        if (len > cursorPos) printf("\e[%dD", len - cursorPos);

        char c = getchar();
        // prevent newlines
        if (c == '\n' || c == 0x0C) continue;

        // escape character
        if (c == 0x1B) {
            c = getchar();
            if (c == '[') {
                c = getchar();

                switch (c) {
                    // movement keys
                    case 'C': {
                        // right arrow
                        if (cursorPos < len) cursorPos++;
                        continue;
                        break;
                    }
                    case 'D': {
                        // left arrow
                        if (cursorPos > 0) cursorPos--;
                        continue;
                        break;
                    }

                    case '1': {
                        char seq[8] = {0};
                        int i = 0;
                        while ((seq[i] = getchar()) != EOF && i < 7) {
                            if (seq[i] >= 'A' && seq[i] <= '~') break; // final char
                            i++;
                            c = seq[i];
                        }

                        if (!strcmp(seq, ";5D")) {
                            // control+left, aka move left (word)
                            uint8_t leftWhite = 0;

                            while (cursorPos > 0) {
                                // skip trailling whitespace
                                if (!leftWhite) {
                                    if (!isspace(calcBuffer[cursorPos - 1])) leftWhite = 1;
                                } else {
                                    if (isspace(calcBuffer[cursorPos - 1])) break;
                                }

                                // move on loop
                                cursorPos--;
                            }
                        } else if (!strcmp(seq, ";5C")) {
                            // control+right, aka move right (word)
                            uint8_t rightWhite = 0;

                            if (cursorPos != len) cursorPos += 1;
                            while (cursorPos <= len) {
                                // skip trailling whitespace
                                if (!rightWhite) {
                                    if (!isspace(calcBuffer[cursorPos])) rightWhite = 1;
                                } else {
                                    if (isspace(calcBuffer[cursorPos])) break;
                                }

                                // move on loop
                                cursorPos++;
                            }
                        } else {
                            continue;
                        }
                        break;
                    }

                    default: continue; break;
                }
            }
        }

        if (c == 127 || c == 0x8) {
            // backspace
            if (cursorPos > 0) {
                // move everything back, accounting for cursorPos
                memmove(&calcBuffer[cursorPos - 1], &calcBuffer[cursorPos], len - cursorPos);
                cursorPos--;
                calcBuffer[--len] = '\0';
            }

        } else if (c == 0x17) {
            // ctrl + w, aka delete word
            uint8_t leftWhite = 0;

            while (cursorPos > 0) {
                // skip trailling whitespace
                if (!leftWhite) {
                    if (!isspace(calcBuffer[cursorPos - 1])) leftWhite = 1;
                } else {
                    if (isspace(calcBuffer[cursorPos - 1])) break;
                }

                // delete on loop
                memmove(&calcBuffer[cursorPos - 1], &calcBuffer[cursorPos], len - cursorPos);
                cursorPos--;
                calcBuffer[--len] = '\0';
            }

        } else if (c == 0x18) {
            // ctrl + k, aka clear line
            len = 0;
            cursorPos = 0;
            memset(calcBuffer, '\0', sizeof(calcBuffer));

        } else if (c == 0x1) {
            // ctrl + a, aka move to start
            cursorPos = 0;

        } else if (c == 0x5) {
            // ctrl + e , aka move to end
            cursorPos = len;

        } else {
            // append (if character is valid, ofc)
            // we move the buffer to make space, like for when the cursor isnt at the end
            if (isprint(c)) {
                memmove(&calcBuffer[cursorPos + 1], &calcBuffer[cursorPos], len - cursorPos + 1);
                calcBuffer[cursorPos++] = c;
            }
            len++;
        }
    }

    putchar('\n');
    return retCode;
}
