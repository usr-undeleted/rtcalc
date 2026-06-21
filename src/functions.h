#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "definitions.h"
#include "utilities.h"

// the meat of the project

// look for invalid characters
static inline int validateBuffer(char *buffer, int *highestPrio, const struct variable *variables) {
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

        // number-like
        if (!mode) {
            // variables
            if (*ptr == '{') {
                char *close = ptr;
                // open is ptr
                if (!(close = findVarClose(ptr))) return 13;
                // form buffer comparator
                size_t varLen = close - ptr - 1;
                char bufComp[varLen + 1];
                memset(bufComp, '\0', sizeof(bufComp));
                memcpy(bufComp, ptr + 1, varLen);

                // loop trough list and find matching var name
                int i = 0;
                uint8_t fail = 1;
                while (i < MAX_VARIABLES && variables[i].name != NULL) {
                    // dont even bother with strncmp if the lengths dont match
                    if (variables[i].len != varLen) {
                        i++;
                        continue;
                    };

                    // finally compare
                    if (!strncmp(bufComp, variables[i].name, variables[i].len)) {
                        fail = 0;
                        break;
                    }

                    i++;
                }
                if (fail) return 14;

                ptr = close + 1;
                nums++;
                mode = !mode;
                continue;
            }

            // functions
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
                if ((ret = validateBuffer(child, NULL, variables))) return ret;

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
// contains flag that change its usage, see header
static inline size_t countTokens(const char *buf, const char flags) {
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

                if (flags & CT_FLAG_READ_BRACKETS) {
                    ptr = open;
                } else {
                    ptr = close + 1;
                }
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

        // variable curly brackets
        if (flags & CT_FLAG_READ_CURLY_BRACKETS && *ptr == '{') {
            ptr = findVarClose(ptr);
            ptr++;
            count += 3;
            continue;
        }

        // function brackets
        if (flags & CT_FLAG_READ_BRACKETS && (*ptr == '[' || *ptr == ']')) {
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
static inline defaultPrecision calculateBuffer(const char *buf, const int highestPrio, const struct variable *variables) {
    // each token eventually gets reduced to result
    size_t count = countTokens(buf, CT_FLAG_EMPTY);
    struct calcToken tokens[count];
    memset(tokens, '\0', count * sizeof(struct calcToken));

    // populate array
    char *ptr = (char *)buf;
    int j = 0; // what token we are in
    uint8_t mode = 0;
    unsigned int parenLevel = 0;
    while (*ptr) {
        skipWhitespace((const char **)&ptr);

        // semi-numbers
        if (!mode) {
            // variables
            if (*ptr == '{') {
                char *close = findVarClose(ptr);
                // open is ptr
                // form buffer comparator
                size_t varLen = close - ptr - 1;
                char bufComp[varLen + 1];
                memset(bufComp, '\0', sizeof(bufComp));
                memcpy(bufComp, ptr + 1, varLen);

                // loop trough list and find matching var name
                int i = 0;
                while (i < MAX_VARIABLES && variables[i].name != NULL) {
                    // dont even bother with strncmp if the lengths dont match
                    if (variables[i].len != varLen) {
                        i++;
                        continue;
                    };

                    // finally compare
                    if (!strncmp(bufComp, variables[i].name, variables[i].len)) {
                        tokens[j].val  = variables[i].value;
                        tokens[j].type = NUMBER;
                        break;
                    }

                    i++;
                }

                ptr = close + 1;
                j++;
                mode = !mode;
                continue;
            }

            // functions
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
                    case SQUARE_ROOT: tokens[j].val = sqrt (calculateBuffer(child, childPrio, variables)); break;
                    case CUBE_ROOT:   tokens[j].val = cbrt (calculateBuffer(child, childPrio, variables)); break;
                    case SINE:        tokens[j].val = sin  (calculateBuffer(child, childPrio, variables)); break;
                    case COSINE:      tokens[j].val = cos  (calculateBuffer(child, childPrio, variables)); break;
                    case TANGENT:     tokens[j].val = tan  (calculateBuffer(child, childPrio, variables)); break;
                    case SINE_H:      tokens[j].val = sinh (calculateBuffer(child, childPrio, variables)); break;
                    case COSINE_H:    tokens[j].val = cosh (calculateBuffer(child, childPrio, variables)); break;
                    case TANGENT_H:   tokens[j].val = tanh (calculateBuffer(child, childPrio, variables)); break;
                    case SINE_R:      tokens[j].val = asin (calculateBuffer(child, childPrio, variables)); break;
                    case COSINE_R:    tokens[j].val = acos (calculateBuffer(child, childPrio, variables)); break;
                    case TANGENT_R:   tokens[j].val = atan (calculateBuffer(child, childPrio, variables)); break;
                    case SINE_RH:     tokens[j].val = asinh(calculateBuffer(child, childPrio, variables)); break;
                    case COSINE_RH:   tokens[j].val = acosh(calculateBuffer(child, childPrio, variables)); break;
                    case TANGENT_RH:  tokens[j].val = atanh(calculateBuffer(child, childPrio, variables)); break;
                    case N_LOG:       tokens[j].val = log  (calculateBuffer(child, childPrio, variables)); break;
                    case FLOOR:       tokens[j].val = floor(calculateBuffer(child, childPrio, variables)); break;
                    case CEILING:     tokens[j].val = ceil (calculateBuffer(child, childPrio, variables)); break;
                    case GAMMA:       tokens[j].val = gamma(calculateBuffer(child, childPrio, variables)); break;
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
            tokens[i].val = calculateBuffer(child, childPrio, variables);
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
            return tokens[i].val;
        }
        i++;
    }
    return 0;
}

// read buffer, tokenize it, and print by parts
static inline void printBufColored(const char *buf) {
    // create array
    size_t count = countTokens(buf, CT_FLAG_READ_BRACKETS | CT_FLAG_READ_CURLY_BRACKETS);
    struct colorToken tokens[count];
    memset(tokens, '\0', count * sizeof(struct colorToken));

    // populate array
    char *ptr = (char *)buf;
    int j = 0; // what token we are in
    uint8_t mode = 0;
    while (*ptr && j < count) {
        skipWhitespace((const char **)&ptr);

        // functions
        if (!mode && getFuncIndex(ptr) != -1) {
            char *close = findFuncClose(ptr, NULL, NULL);
            tokens[j].type = SC_FUNCTION;
            tokens[j].ptr  = ptr;

            ptr = strchr(ptr, '[');
            j++;
            continue;
        }

        // parentheses
        if (*ptr == '(' || *ptr == ')') {
            tokens[j].type = SC_PARENTHESES;
            tokens[j].ptr  = ptr;

            ptr++;
            j++;
            continue;
        }

        // variables, cover them fully
        if (*ptr == '{') {
            // opening
            tokens[j].type = SC_CURLY_BRACKETS;
            tokens[j].ptr  = ptr;
            j++;

            // contents
            tokens[j].type = SC_VARIABLES;
            tokens[j].ptr  = ptr + 1;
            ptr = findVarClose(ptr++);
            j++;

            // ending
            tokens[j].type = SC_CURLY_BRACKETS;
            tokens[j].ptr  = ptr;
            ptr++;
            j++;

            mode = !mode;
            continue;
        }

        // brackets
        if (*ptr == '[' || *ptr == ']') {
            tokens[j].type = SC_BRACKETS;
            tokens[j].ptr  = ptr;

            ptr++;
            j++;
            continue;
        }

        // trees
        if (!mode) {
            // numbers
            tokens[j].type = SC_NUMBER;
            tokens[j].ptr  = ptr;
            strtod(ptr, &ptr);
            j++;

        } else {
            // operators
            tokens[j].type = SC_OPERATOR;
            tokens[j].ptr  = ptr;
            ptr++;
            j++;
        }

        // break early if there is nothing fowards
        if (*ptr == '\0') break;

        mode = !mode;
    }
    // define last "null-terminating" array item
    skipWhitespace((const char **)&ptr);
    tokens[j].type = SC_EMPTY;
    tokens[j].ptr  = ptr;

    // print array
    for (int i = 0; i < count; i++) {
        if (tokens[i].type == SC_EMPTY) break;

        if      (tokens[i].type == SC_NUMBER)            printf("%s", NUMBER_CLR);
        else if (tokens[i].type == SC_OPERATOR)          printf("%s", OPERATOR_CLR);
        else if (tokens[i].type == SC_PARENTHESES)       printf("%s", PAREN_CLR);
        else if (tokens[i].type == SC_BRACKETS)          printf("%s", BRACKETS_CLR);
        else if (tokens[i].type == SC_FUNCTION)          printf("%s", FUNCTION_CLR);
        else if (tokens[i].type == SC_CURLY_BRACKETS)    printf("%s", CURLY_BRACKETS_CLR);
        else if (tokens[i].type == SC_VARIABLES)         printf("%s", VARIABLE_CLR);
        else printf("%s", RESET);

        // only print up to before next pointer
        int len = tokens[i + 1].ptr - tokens[i].ptr;
        printf("%.*s\e[0m", len, tokens[i].ptr);
    }
}

#endif
