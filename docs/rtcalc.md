% rtcalc(1) rtcalc release 1.28.7
% undeleted
% June 2026

# NAME

rtcalc - Interactive CLI math program

# SYNOPSIS

`rtcalc` [flags]

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

`define:[name]=[val]`
: Defines a new variable of [name] with a value ([value]). The value field is calculated in the same way that a formula would interactively, meaning that even functions apply. And naturally, you may also use variables defined before in the formula.

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

Defined variables may be integrated seamlessly, being treated as regular operands.

`{x} * {y}`

Specific formulas will required two arguments. Separate each with a comma.

`logx[x , y]`

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

Note that all trigonometric functions return in radians.

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

`tan[x]`
: Perform the hyperbolic tangent of 'x'.

`asin[x]`
: Perform the reverse sine of 'x'.

`acos[x]`
: Perform the reverse cosine of 'x'.

`atan[x]`
: Perform the reverse tangent of 'x'.

`asinh[x]`
: Perform the reverse hyperbolic sine of 'x'.

`acosh[x]`
: Perform the reverse hyperbolic cosine of 'x'.

`atanh[x]`
: Perform the reverse hyperbolic tangent of 'x'.

`atan2[x]`
: Get the angle theta from the conversion of rectangular coordinates (x, y) to polar coordinates (r, theta).

`csc[x]`
: Perform the cosecant of 'x'.

`sec[x]`
: Perform the secant of 'x'.

`cot[x]`
: Perform the cotangent of 'x'.

`csch[x]`
: Perform the hyperbolic cosecant of 'x'.

`sech[x]`
: Perform the hyperbolic secant of 'x'.

`coth[x]`
: Perform the hyperbolic cotangent of 'x'.

## Conversion

`deg[x]`
: Convert radians to degrees.

`rad[x]`
: Convert degrees to radians.

## Rounding

`floor[x]`
: Get the floor of 'x'.

`ceil[x]`
: Get the ceiling of 'x'.

`trunc[x]`
: Get the integer part of 'x'.

`round[x]`
: Round 'x' to the nearest integer.

## Exponential

`sqrt[x]`
: Perform the square root of 'x'.

`cbrt[x]`
: Perform the cube root of 'x'.

`log[x]`
: Perform the natural logarithm of 'x'.

`log10[x]`
: Perform the base 10 logarithm of 'x'.

`log2[x]`
: Perform the base 2 logarithm of 'x'.

`logx[x, y]` (aka `log_x()`)
: Perform the base 'y' logarithm of 'x'.

`exp[x]`
: Perform the value of E ^ 'x'.

`exp2[x]`
: Perform the value of 2 ^ 'x'.

`pow[x, y]`
: Get the result of 'x' to the power of 'y'. Equivalent to operator '`^`'.

## Geometry

`hypot[x, y]`
: Get the length of the length of the hypotenuse of a right angle triangle with sides of length 'x' and 'y', from origin (0, 0).

## Special Functions

`gamma[x]`
: Perform the gamma of 'x'.

`lgamma[x]`
: Perform the natural logarithm of the absolute gamma of 'x'.

`erf[x]`
: Perform the value of the error function at 'x'.

`erfc[x]`
: Perform the value of the complementary error function at 'x'.

## Miscellaneous

`abs[x]`
: Get the absolute of 'x'.

`fmax[x, y]`
: Get the highest value of a floating 'x' and 'y'.

`fmin[x, y]`
: Get the lowest value of a floating 'x' and 'y'.

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

# SPECIFIC DETAILS

The input buffer has a limit of 2048 characters.

The result buffer has a limit of 1024 characters.

# DEVELOPER NOTES
I hope that you like this program <3
