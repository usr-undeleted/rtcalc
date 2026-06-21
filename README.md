# Real time CLI calculation tool (rtcalc)
POSIX CLI tool that, in real time, take in user input and calculate the formula provided.  

### Features and usage
A manual is provided under docs/ in both manpage and markdown formats.

## Compilation
```
cargo build --release
```
To run the program:
```
./target/release/rtcalc
```  
Note: The `long-double` Cargo feature can be enabled for increased precision (`--features long-double`), though this has no effect on stable Rust (which lacks `f128`). On nightly with `f128` stabilized, it will switch from `f64` to 128-bit floats.

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
3. Making constants whenever possible to standardize definitions.
### Contributor guidelines
1. AI usage is limited to debugging and helping with code, not writing slop automatically. AI code will be rejected.
2. Your code has to be up to quality or better than the code present in the project.
3. If your code isn't readable, your code won't be accepted.
4. Don't try to reinvent the wheel. Huge code changes will only lead to complications down the line.
5. DO NOT FORGET TO BUMP VERSION, UPDATE MANUAL, AND README
### Project files
- src/
Contains Rust source modules: `main.rs` (entry point + terminal loop), `definitions.rs` (types/constants), `functions.rs` (validation/calculation/syntax coloring), `utilities.rs` (helpers).
- docs/
Contains documentation, in both markdown format and manpage format.
