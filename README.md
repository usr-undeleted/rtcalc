# Real time CLI calculation tool (rtcalc)
POSIX CLI tool that, in real time, takes user input and calculates the formula provided.  
Written in **x86-64 NASM assembly** (was C, now asm).

### Features and usage
A manual is provided under docs/ in both manpage and markdown formats.

## Compilation
Requires `nasm` and `clang`.

```
make
```
To run:
```
./bin/rtcalc
```

## Code guidelines
### Code principles
1. Avoid dynamic memory allocation as much as possible. Stack errors are much more forgiving than allocated memory errors.
2. Splitting tasks across multiple functions, specially whenever those actions are done more than once.
3. Readability and good practices.
4. Friendly user UX.
### Specific design decisions
1. x86-64 SysV AMD64 ABI calling convention
2. NASM Intel syntax with %include-macro infrastructure
3. Recursive descent expression parser (not tokenize-then-reduce)
4. Links only libc + libm (no external dependencies)
5. Custom macros for prologue/epilogue, whitespace skipping, VLA helpers

### Project files
- src/ Contains assembly source files and equates include file.
- build/ Intermediate object files.
- docs/ Contains documentation, in both markdown format and manpage format.
