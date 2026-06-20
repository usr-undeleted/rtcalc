# Real time CLI calculation tool (rtcalc)
POSIX CLI tool that, in real time, take in user input and calculate the formula provided.  

### Features and usage
A manual is provided under docs/ in both manpage and markdown formats.

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
5. DO NOT FORGET TO BUMP VERSION, UPDATE MANUAL, AND README
### Project files
- src/
Contains headers and main .c file.
- docs/
Contains documentation, in both markdown format and manpage format.
