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
#include "functions.h"
#include "definitions.h"
#include "utilities.h"

struct termios backup = { 0 };
int retCode = 0;
unsigned long long precision = 6;
char *prompt = ">>> ";
// see definitions for the flags
unsigned char globalFlags = 0;

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

            } else if (!strcmp(argv[i], "syntax-highlighting")) {
                // define syntax highlightning
                globalFlags |= USE_PRETTY_COLORS;

            } else if (!strncmp(argv[i], "precision", 9)) {
                // define exact precision
                char *start = strchr(argv[i], '=');
                if (!start || !*(start + 1))
                    helpMenu("\n\e[31mError: Precision not defined properly.\e[0,\n", USER_MISTAKE);

                char *check = ++start;
                precision = strtoul(start, &check, 10);

                if (check == start || *check != '\0')
                    helpMenu("\n\e[31mError: Precision is an invalid number.\e[0,\n", USER_MISTAKE);

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

                defaultPrecision calc = calculateBuffer(calcBuffer, highestPrio);
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

        printf("\e[u\e[J"               // return cursor back to start
               "\e[A\r\e[2K%s\r\e[B%s", // replace result and prompt
               result, prompt
        );

        // print buffer itself
        if (globalFlags & USE_PRETTY_COLORS) {
            // yes colors >:3
            if (!ret) printBufColored(calcBuffer);
            else printf("\e[31m%s\e[0m", calcBuffer);

        } else {
            // no colors :(
            printf("%s%s\e[0m", ret ? "\e[31m" : "", calcBuffer);
        }

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
                memmove(&calcBuffer[cursorPos + 1], &calcBuffer[cursorPos], len - cursorPos + 1);
                calcBuffer[cursorPos++] = c;
            }
            len++;
        }
    }

    putchar('\n');
    return retCode;
}
