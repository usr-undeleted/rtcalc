#include <asm-generic/errno-base.h>
#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "functions.h"
#include "definitions.h"
#include "utilities.h"

struct termios backup = { 0 };
int retCode = 0;
volatile sig_atomic_t needsRedraw = 0;

int main (int argc, char *argv[]) {
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not being operated in a terminal - terminating.\n");
        return USER_MISTAKE;
    }

    unsigned long long precision = 6;
    char *prompt = ">>> ";
    // see definitions for the flags
    unsigned char globalFlags = 0;

    // environment variables
    // maybe only allocate if needed in the future with <alloca.h>?
    struct variable variables[MAX_VARIABLES + 1] = { 0 };
    unsigned short varCount = 0; // how many slots were used up + index

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

        } else if (!strcmp(argv[i], "syntax-highlighting")) {
            // define syntax highlightning
            globalFlags |= USE_PRETTY_COLORS;

        } else if (!strncmp(argv[i], "precision", 9)) {
            // define exact precision
            char *start = strchr(argv[i], '=');
            if (!start || !*(start + 1))
                helpMenu("\n" ERROR_CLR "\e[31mError: Precision not defined properly.\e[0,\n", USER_MISTAKE);

            char *check = ++start;
            precision = strtoul(start, &check, 10);

            if (check == start || *check != '\0')
                helpMenu("\n" ERROR_CLR "Error: Precision is an invalid number.\e[0,\n", USER_MISTAKE);

        } else if (!strncmp(argv[i], "define", 5)) {
            // define env variables
            // if there are already too many
            if ((varCount + 1) > MAX_VARIABLES)
                helpMenu("\n" ERROR_CLR "\e[31mError: Too many variables defined.\e[0,\n", USER_MISTAKE);

            char *name = strchr(argv[i], ':');
            if (!name)
                helpMenu("\n" ERROR_CLR "\e[31mError: Definition used incorrectly.\e[0,\n", USER_MISTAKE);
            name++; // name is at start of name
            if (!*name)
                helpMenu("\n" ERROR_CLR "\e[31mError: Definition name was not set.\e[0,\n", USER_MISTAKE);

            char *valuePtr = strchr(argv[i], '=');
            if (!valuePtr)
                helpMenu("\n" ERROR_CLR "\e[31mError: Definition has no value.\e[0,\n", USER_MISTAKE);
            char *eq = valuePtr;

            if (valuePtr == (name))
                helpMenu("\n" ERROR_CLR "\e[31mError: Definition name is invalid.\e[0,\n", USER_MISTAKE);

            if (*(valuePtr + 1) == '\0')
                helpMenu("\n" ERROR_CLR "\e[31mError: Definition value is invalid.\e[0,\n", USER_MISTAKE);

            valuePtr++;

            // normalize comma decimal to dot
            for (char *p = valuePtr; *p; p++) if (*p == ',') *p = '.';

            // run calculation stack
            int childHighestPrio = 0;
            char ret = 0;
            if ((ret = validateBuffer(valuePtr, &childHighestPrio, variables)))
                helpMenu("\n" ERROR_CLR "\e[31mError: Variable couldn't be expanded due to an invalid formula.\e[0,\n", USER_MISTAKE);

            // define
            variables[varCount].value = calculateBuffer(valuePtr, childHighestPrio, variables);
            variables[varCount].name  = name;
            // set null terminator, allowed under C99 section 5.1.2.2.1 :tongue:
            *eq = '\0';
            variables[varCount].len   = strlen(name);

            varCount++;

        } else {
            helpMenu("\n\e[31mError: Improper flag used.\e[0m\n", USER_MISTAKE);
        }
    }
    const size_t promptLen = strlen(prompt);

    // handle signals
    signal(SIGINT,   handleCtrlC);
    signal(SIGWINCH, handleTermResize);

    struct termios newMode = { 0 };
    // populate backup and define new mode
    if (tcgetattr(STDIN_FILENO, &backup) == -1) {
        fprintf(stderr, "Failed to backup term attributes: %s\n", strerror(errno));
        return CODE_MISTAKE;
    }
    atexit(restoreTerminal); // set up putting the terminal back to normal

    // change terminal mode to non-canonical for magic
    newMode = backup;
    newMode.c_lflag &= ~(ICANON | ECHO); // don't wait for new line and dont say anything
    newMode.c_cc[VMIN]  = 1; // get one char at a time
    newMode.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newMode) == -1) {
        fprintf(stderr, "Failed to change terminal mode: %s\n", strerror(errno));
        return CODE_MISTAKE;
    }
    putchar('\n');

    // get the absolute row
    printf("\e[6n");
    char getRowBuf[16] = { 0 };
    uint32_t startRow = 0; // holds the absolute row of program start
    scanf("\e[%d;%*dR", &startRow);

    // main loop to pupulate main buffer
    char calcBuffer[BUFFER_SIZE] = { 0 };
    ssize_t cursorPos = 0;

    // show prompt
    printf("\n%s", prompt);
    char result[RESULT_SIZE] = { 0 };
    size_t resSize = 0;

    // term info
    struct winsize ws = { 0 };
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0) {
        fprintf(stderr, "Failed to get terminal info: %s\n", strerror(errno));
        return CODE_MISTAKE;
    }

    while(1) {
        int highestPrio = 0;
        uint8_t ret = validateBuffer(calcBuffer, &highestPrio, variables); // find errors
        if (cursorPos >= 4096) ret = E_INPUT_SIZE_LIMIT; // input size limit

        // define result
        if (!ret) {
            if (cursorPos == 0 && calcBuffer[0] == '\0') {
                memset(result, '\0', sizeof(result));
                memcpy(result, WELCOME, sizeof(result));

            } else {

                defaultPrecision calc = calculateBuffer(calcBuffer, highestPrio, variables);
                if ((resSize = snprintf(NULL, 0, FORMAT_WITH_PRECISION, (int)precision, calc)) >= 2048) ret = 12;

                if (ret) {
                    snprintf(result, sizeof(result), "Can't calculate: %s", retToStr(ret));

                } else {
                    snprintf(result, sizeof(result), FORMAT_WITH_PRECISION, (int)precision, calc);
                }

            }
        } else {
            snprintf(result, sizeof(result), "Can't calculate: %s", retToStr(ret));
        }

        printf("\e[%d;1H\e[J"           // return cursor back to start
               "\e[A\r\e[2K%s\r\e[B%s", // replace result and prompt
               startRow, result, prompt
        );

        // print buffer itself
        if (globalFlags & USE_PRETTY_COLORS) {
            // yes colors >:3
            if (!ret) printBufColored(calcBuffer);
            else printf("%s%s\e[0m", ERROR_CLR, calcBuffer);

        } else {
            // no colors :(
            printf("%s%s\e[0m", ret ? ERROR_CLR : "", calcBuffer);
        }

        // we don't the user to type with a big ass buffer
        if (ret >= E_INPUT_SIZE_LIMIT) continue;

        ssize_t len = strlen(calcBuffer);

        // if sigwinch was detected
        if (needsRedraw) {
            needsRedraw = 0;
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0) {
                fprintf(stderr, "Failed to get terminal info: %s\n", strerror(errno));
                return CODE_MISTAKE;
            }
            // move everything back to start and clear
            printf("\e[%d;1H\e[J", startRow);
            continue;
        }

        // move cursor, wrapping on terminal edge
        ssize_t remaining = labs(cursorPos - len);
        ssize_t currentCol = (promptLen + 1 + len) % ws.ws_col;
        if (!(currentCol - 1)) putchar('\n');
        // actual movement
        while (remaining > 0) {
            printf("\e[D");
            remaining--;
            currentCol--;

            // if we hit the edge
            if (currentCol == 1 && remaining > 0) {
                printf("\e[A"   // move on line up
                    "\e[999C"); // move all the way to the right
                currentCol = ws.ws_col;
            }
        }

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

                            if (cursorPos > 0) {
                                if (strchr(DELIMITERS, calcBuffer[cursorPos - 1])) {
                                    // if the cursor is already at a delimiter,
                                    // move only delimiters on loop
                                    while (cursorPos > 0 && strchr(DELIMITERS, calcBuffer[cursorPos - 1])) {
                                        // move on loop
                                        cursorPos--;
                                    }

                                } else {
                                    while (cursorPos > 0) {
                                        // skip trailling whitespace
                                        if (!leftWhite) {
                                            if (!isspace(calcBuffer[cursorPos - 1])) leftWhite = 1;
                                        } else {
                                            if (isspace(calcBuffer[cursorPos - 1]) ||
                                                strchr(DELIMITERS, calcBuffer[cursorPos - 1])) break;
                                        }

                                        // move on loop
                                        cursorPos--;
                                    }
                                }
                            }

                        } else if (!strcmp(seq, ";5C")) {
                            // control+right, aka move right (word)
                            uint8_t rightWhite = 0;

                            if (cursorPos < len) {
                                if (strchr(DELIMITERS, calcBuffer[cursorPos])) {
                                    while (cursorPos < len && strchr(DELIMITERS, calcBuffer[cursorPos])) {
                                        cursorPos++;
                                    }

                                } else {
                                    while (cursorPos < len) {
                                        // skip trailling whitespace
                                        if (!rightWhite) {
                                            if (!isspace(calcBuffer[cursorPos])) rightWhite = 1;
                                        } else {
                                            if (isspace(calcBuffer[cursorPos]) ||
                                                strchr(DELIMITERS, calcBuffer[cursorPos])) break;
                                        }

                                        // move on loop
                                        cursorPos++;
                                    }
                                }
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

            if (cursorPos > 0) {
                if (strchr(DELIMITERS, calcBuffer[cursorPos - 1])) {
                    // if the cursor is already at a delimiter,
                    // delete only delimiters on loop
                    while (cursorPos > 0 && strchr(DELIMITERS, calcBuffer[cursorPos - 1])) {
                        // delete on loop
                        memmove(&calcBuffer[cursorPos - 1], &calcBuffer[cursorPos], len - cursorPos);
                        cursorPos--;
                        calcBuffer[--len] = '\0';
                    }

                } else {
                    while (cursorPos > 0) {
                        // skip trailling whitespace
                        if (!leftWhite) {
                            if (!isspace(calcBuffer[cursorPos - 1])) leftWhite = 1;
                        } else {
                            if (isspace(calcBuffer[cursorPos - 1]) ||
                                strchr(DELIMITERS, calcBuffer[cursorPos - 1])) break;
                        }

                        // delete on loop
                        memmove(&calcBuffer[cursorPos - 1], &calcBuffer[cursorPos], len - cursorPos);
                        cursorPos--;
                        calcBuffer[--len] = '\0';
                    }
                }
            }

        } else if (c == 0x18) {
            // ctrl + x, aka clear line
            len = 0;
            cursorPos = 0;
            memset(calcBuffer, '\0', sizeof(calcBuffer));

        } else if (c == 0xB) {
            // ctrl + k, aka clear everything right of cursor position
            if (calcBuffer[0] != '\0') {
                memset(calcBuffer + cursorPos, '\0', len - cursorPos);
                len = cursorPos;
            }

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
                if (c == ',') c = '.';
                memmove(&calcBuffer[cursorPos + 1], &calcBuffer[cursorPos], len - cursorPos + 1);
                calcBuffer[cursorPos++] = c;
            }
            len++;
        }
    }

    putchar('\n');
    return retCode;
}
