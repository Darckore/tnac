# Totally Not Another Calculator (under construction)

This is tnac - Totally (Absolutely, Positively, Definitely) Not Another Calculator.
Well, except it kind of is. Written entirely in C++.

This project was a challenge to myself, and it is basically an answer to the question:
*"Were I to write a compilery thing, what would it be, and how crazy would I be able to make it?"*

At any rate, it's been a good excersice on writing parsers.

Under the hood it has an expression evaluator which understands a bunch of maths over some kind of data.

## Features (will be expanded)

### Types

1. 64-bit signed integers (`long long` aka `std::int64_t`)
2. Floating point with (`double`)
3. Complex numbers (via `std::complex`)
4. Ordinary fractions

Types are (mostly) inter-convertible and calculated dynamically.

From this point on, I will refer to integer and floating-point types simply
as `ints` and `floats` for brevity.

### Operations

1. Unary `+` and `-`
2. Addition (`+`)
3. Subtraction (`-`)
4. Multiplication (`*`)
5. Division (`/`)
6. Modulo (`%`)
7. Bitwise negation (`~`)
7. Bitwise and (`&`)
7. Bitwise or (`|`)
7. Bitwise xor (`^`)

Not all operations are supported for all types.
Using something unsupported results in a quiet "undefined" value.
Given the dynamic nature of types, this seems a reasonable choice since we normally
can't predict what kind of value we'll get.

Integer division is implicitly done via the floating type to avoid division by zero.
An expression like `42 / 0` will result in an `infinity` value.

Bitwise operations are supported for `ints` and values of other types wich are 'int-like'.
Being an 'int-like' means being able to convert to int without loss of data.

For example, `2.5 & 2` will yield an undefined value, while `2.0 & 2` is equivalent to `2 & 2`.
Similarly, a complex value of `2.0 + 0i` is considered an 'int-like',
whereas `2.0 + i` is not.

### Driver

The driver is a basic CLI interface which works in interactive mode.
It displays a prompt allowing the user to enter expressions with it will then
evaluate and produce a result.

The diver also supports certain commands. More on them later.

## Syntax (will be expanded)

This section will contain syntax basics along with some examples.
For a taste of actual grammatics behind all this, see [grammar](stuff/grammar.md)

### Numeric literals

Numbers can be of `int` or `float` type.
They can include any sequence of digits, a dot (`.`), or a prefix.

A number with exactly one dot surrounded by digits (for example, `1.111`) is parsed as a `float`.

`Ints` come in different flavours:

1. Decimal. Anything your normal integer can be (`1`, `42`, `1337`, etc)
2. Binary. It is, well, a binary - a sequence of `1`'s and `0`'s prefixed with `0b` or `0B` (`0b11100111`)
3. Octal. Digits `[0 ... 7]` prefixed with `0` (`0123`)
4. Hexadecimal. Digits and non-case-sensitive letters `[A ... F]` prefixed with `0x` or `0X` (`0xff12a`)

### Identifiers

Identifier names are case-sensitive.

They can include any digit, any Latin letter (`[A ... Z][ a ... z]`), and an undersore (`_`).
The first character of a name must be a letter.

These names are all valid:
`a`, `vaRiAble`, `my_super_var111111`

These are not:
`0a`, `_something`

### Keywords

Keywords are reserved names. They follow the rules for identifiers, except they always start with `_`

These are the supported keywords:

- `_result`
- `_complex`
- `_fraction`

### Expressions

Expressions can include:

1. Numbers
2. Identifier names
3. Expressions enclosed in parentheses (`(`, `)`)
4. Unary operators `+`, `-`, `~`
5. Binary operators `+`, `-`, `*`, `/`, `%`, `&`, `|`, `^`
6. Assignments `=`
7. Keywords
8. Invocations
9. Declarations

**Parentheses** can enclose any valid expression.

**Unary** operators can be applied to numbers, invocations, identifiers, or the `_result` keyword.

The `_result` keyword gets replaced with the most recent evaluation result.
For example, the expression `2 + 2 + _result` evaluates to `8`, since `_result`
expands to the evaluation result of `2 + 2`.

Examples: `-1`, `-(2+2)`, `+(10 / 4)`, `-_result`, `+_complex(1,2)`

**Binary** operators can combine pretty much any valid expressions.

Examples: `2+2`, `-(_fraction(1, 2) + 1) * 2`

**Invocations** are things with arguments.
They can create values of certain types, or call functions.

Currently, there are these invocation expressions:

- `_fraction(<expr>, <expr>)` - instantiates an ordinary fraction.
Both the arguments (numerator and denominator) are mandatory. Any expressions will work, however,
value types other than int will convert and, potentially, lose data.
Examples: `_fraction(1, 2)` is `1/2`, `_fraction(4-2, 6/2)` is `2/3`.

- `_complex(<expr>, <expr>)` - instantiates a complex number.
The first argument is the real part, and the second is imaginary.
None of the arguments are mandatory, they'll be 0 by default. Examples:
`_complex()` is `0 + 0i`, `_complex(1)` is `1 + 0i`, `_complex(2+2, 1/2)` is `4 + 0.5i`

- `func_name([<expr>, ...])` - calls a previously defined function.
Example: `my_func(1+1, 2)`

**Assignments** assign a value to a variable. Example: `var = 42`.
They also can be chained: `var1 = var2 = 42`.

**Declarations** declare stuff. See the next section.