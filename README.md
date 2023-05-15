# Totally Not Another Calculator (under construction)
This is tnac - Totally (Absolutely, Positively, Definitely) Not Another Calculator.
Well, except it kind of is. Written entirely in C++.

Under the hood it has an expression evaluator and which understands a range of operations with numbers.

## Features (will be expanded)

### Types

1. 64-bit signed integers (`long long` aka `std::int64_t`)
2. Floating point with (`double`)
3. Complex numbers (via `std::complex`)
4. Ordinary fractions

Types are (mostly) inter-convertible and calculated dynamically.

From this point on, I will refer to integer and floating-point types simply as "ints" and "floats" for brevity.

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

Not all operations are supported for all types. Using something unsupported results in a quiet "undefined" value.
Given the dynamic nature of types, this seems a reasonable choice since we normally can't predict what kind of value we'll get.

Integer division is implicitly done via the floating type to avoid division by zero.
An expression like `42 / 0` will result in an `infinity` value.

Bitwise operations are supported for ints and values of other types wich are 'int-like'.
Being an 'int-like' means being able to convert to int without loss of data.

For example, `2.5 & 2` will yield an undefined value, while `2.0 & 2` is equivalent to `2 & 2`. Similarly, a complex value of `2.0 + 0i` is concidered an 'int-like',
whereas `2.0 + i` is not.

### Driver

The driver is a basic CLI interface which works in interactive mode.
It displays a prompt allowing the user to enter expressions with it will then evaluate and produce a result.

The diver also supports certain commands. More on them later.

## Syntax (will be expanded)

### Numeric literals

Numbers can be of int or float type. They can include any sequence of digits, a dot (`.`), or a prefix.

A number with exactly one dot surrounded by digits (for example, `1.111`) is parsed as a float.

Ints come in different flavours:

1. Decimal. Anything your normal int can be (`1`, `42`, `1337`, etc)
2. Binary. It is, well, a binary - a sequence of `1`'s and `0`'s prefixed with `0b` or `0B` (`0b11100111`)
3. Octal. Digits `[0 ... 7]` prefixed with `0` (`0123`)
4. Hexadecimal. Digits and non-case-sensitive letters `[A ... F]` prefixed with `0x` or `0X` (`0xff12`)

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

A list of supported keywords:

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

**Parentheses** can enclose any valid expression

**Unary** operators can be applied to numbers, invocations, identifiers, of the `_result` keyword.

Examples: `-1`, `-(2+2)`, `+(10 / 4)`, `-_result`, `+_complex(1,2)`

