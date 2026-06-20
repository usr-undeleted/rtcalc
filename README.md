# Real time CLI calculation tool (rtcalc)
POSIX CLI tool that, in real time, take in user input and calculate the formula provided.  

## Features
- Multi-variable calculation 
```x + y + z```
- Parentheses support 
```x + (y * z)```
- Addition (+), subtraction (-), multiplication (*), division (/), powers (^) and square roots (sqrt[x])
- Error system for invalid formulas
- Basic shell functionalities:
1. CTRL+A -> Move to input start
2. CTRL+E -> Move to input end
3. CTRL+X -> Clear buffer
4. CTRL+W -> Delete word by word
5. CTRL+K -> Clear everything right of cursor
6. Arrow keys -> Move side to side
7. CTRL+Arrow keys -> Move word by word
8. Syntax highlighting (off by default)

## Compilation
All testing is done with the `clang` compiler. Bug reports compiled without clang won't be considered. This also applies to compiling flags.  
To first compile the program, run:
```
clang -o rtcalc rtcalc.c -lm -std=gnu99
```
To finally run the program, run:
```
./rtcalc
```
Compiling notes:  
- Defining "`USE_LONG_DOUBLE`" increases precision by switching from a regular `double` floating point to a `long double` floating point, going from 64 bits to 128 bits. Note that switching might carry no difference, as implementation may vary from architecture to architecture.

## Plans
- Even more shell-like navigation features
- New math functions

## Manual
1. Basic usage
- Any invalid characters or functions will deny the calculation of input.
- Sample formulas: 
```
>>> 10 * 2 ^ (2)
```
```
>>> sqrt[2^5] / (15 / 2)
```
```
>>> 2 ^ 102 / 19 * 0.542 / 100000000
```
2. Details
- Input is limited to 4096 characters. Blowing past that limit will prevent all calculation.
3. Implemented functions and operators
Operators:  
- Addition (+)
- Subtraction (-)
- Multiplication (*)
- Division (/)
- Powers (^)
- Modulo (%)
Functions:  
- Square roots (sqrt[x])
- Cube roots (cbrt[x])
- Sines, Cosines and Tangents (sin[x], cos[x], tan[x])
- Natural logarithms (log[x])
- Hyperbolic functions
- Floor and ceiling
- Gamma

## Code guidelines
### Code principles
1. Avoid dynamic memory allocation as much as possible. Stack errors are much more forgiving than allocated memory errors.
2. Splitting tasks across multiples functions, specially whenever those actions are done more than once.
3. Readability and good practices.
4. Friendly user UX.
### Specific design decisions
1. Variables and functions named like: aVariable, oneThing, etc.
2. Keeping input buffer size capped at a reasonable limit (like 4kb).
3. Making macros whenever possible to standardize definitions.
### Contributor guidelines
1. The AGENTS.md in the project isn't an excuse to let agents write code for you. ONLY agent-assisted code will be accepted. AI usage is limited to debugging and helping with code, not writing slop automatically.
2. Your code has to be up to quality or better than the code present in the project.
3. If your code isn't readable, your code won't be accepted.
4. Don't try to reinvent the wheel. Huge code changes will only lead to complications down the line.
5. DO NOT FORGET TO UPDATE README AND RELEASE VERSION
