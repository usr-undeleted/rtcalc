% rtcalc(1) rtcalc 1.14.4
% undeleted
% June 2026

# NAME

rtcalc - Interactive CLI math program

# SYNOPSIS

`calc` [flags]

# DESCRIPTION

Takes in user input interactively and in real time, calculating the formula provided by the user and outputting it.

# OPTIONS

`help`
: Show a help menu, providing basic usage info.

`prompt=[str]`
: Replace default shell prompt with user provided one. Note that the space beetwen input and prompt is NOT automatically provided.

`syntax-highlighting`
: Enable input syntax highlighting, coloring different components of the input.

`precision=[num]`
: Sets the printed result's floating point precision to a user set value.

# BASIC USAGE

After starting the program, type in the formula to be calculated, following basic math principles. The program will automatically error out on invalid formulas or syntax.

# SYNTAX AND EXAMPLES

Type in two operands and an operator.

`x + y`

The program supports more than one operand, as long as there are enough operators.

`x + y - z`

Each operator has an order of operations, that being:

`+`, `-`: Third priority.

`*`, `/`, `%`: Second priority.

`^`: First priority.

You may introduce parentheses.

`x * y ^ (z + z)`

Parentheses may be nested.

`x + (x + (y * (z + x)))`

Functions are implemented. Their results are treated as operands.

`sqrt[x] + y`

You may write formulas inside of functions.

`sqrt[x * y]`

# OPERATORS

`x + y`
: Perform addition.

`x - y`
: Perform subtraction.

`x * y`
: Perform addition.

`x / y`
: Perform addition.

`x % y`
: Perform division, and return the rest.

`x ^ y`
: Perform exponentiation.

# FUNCTIONS

## Trigonometric

`sin[x]`
: Perform the sine of 'x'.

`cos[x]`
: Perform the cosine of 'x'.

`tan[x]`
: Perform the tangent of 'x'.

`sinh[x]`
: Perform the hyperbolic sine of 'x'.

`cosh[x]`
: Perform the hyperbolic cosine of 'x'.

`tanh[x]`
: Perform the hyperbolic tangent of 'x'.

## Rounding

`floor[x]`
: Get the floor of 'x'.

`ceil[x]`
: Get the ceiling of 'x'.

## Exponential

`sqrt[x]`
: Perform the square root of 'x'.

`cbrt[x]`
: Perform the cube root of 'x'.

## Special Functions

`gamma[x]`
: Perform the gamma of 'x'.

# NAVIGATION

The program offers basic shell-like navigation options.

`CTRL + A`
: Move cursor to start of input buffer.

`CTRL + E`
: Move cursor to end of input buffer.

`CTRL + K`
: Clear everything right of cursor.

`CTRL + X`
: Clear input buffer.

`CTRL + W`
: Delete token by token.

`RIGHT ARROW`
: Move cursor to the right.

`LEFT ARROW`
: Move cursor to the left.

`CTRL + RIGHT ARROW`
: Move cursor to the right.

`CTRL + LEFT ARROW`
: Move cursor to the left.

# DEVELOPER NOTES
I hope that you like this program <3
