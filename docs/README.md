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

Multiple expressions can be chained one after another by using `:`.

This:
```
2 + 2 :
3 + 3 :
4 + 4
```
will evaluate the 3 expressions listed one by one from the first one to the last.

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

### Declarations

You can declare certain types of things.
These are the *things* available:

1. Variables
2. Functions

#### Variables

**Variable declarations** are similar to simple assign expressions.

For example, this declares variables `a` and `b`:
```
a = 10 :
b = a + 1
```

This code also declares two variables and gives them the same value:
```
a = b = 42
```

The under-the-hood difference between them is that declarations are produced
for names which haven't been previously declared, whereas assignments operate
on already existing names.

Here's another example:
```
a = 10 :
a = 11
```

The above example contains a declaration and an assignment expression.
Generally, we don't care about distinguishing between those.
Any unknown name followed by an init (`= <something>`) will become a new variable.

Before a variable can be used, it must be declared.

This is ok:
```
a = 10 :
a + 5
```

This is a syntax error:
```
a + 5
```

#### Functions (calls are not implemented yet)

**Function declarations** go like this:
```
func([<identifier>, ...])
  [<expr> : ... ] ;
```

For example, this function will return the sum of its parameters:
```
sum(a, b)
  a + b ;
```

Function result is inferred from the last evaluated expression.

This is a pointless function which does absolutely nothing:
```
stupid() ;
```

Expressions that declare functions don't need a `:` at the end, as the `;` implies it.

This:
```
func(a, b)
  a + b ;
func(1, 2) + 1
```

Is exactly the same as this:
```
func(a, b)
  a + b ; :
func(1, 2) + 1
```

Function declarations can appear inside other functions, the nesting is unlimited.
Such internal functions won't be visible outside the one in which they are declared.
```
parent_func(a, b)
  child_func(a)
    a * 2 ;
  child_func(a + b) ;
```

A call like this will return `10`:
```
parent_func(2, 3)
```

Also, functions are their own value type:
```
func(param1, param2)
  param1 + param2 ;

variable = func :
variable(2, 3)
```

## Commands

Commands are special directives to the driver. By default, none are declared.
Clients should add their own commands according to their needs.

Commands have names and arguments. They begin at the name and end at `:` or
end of input. Arguments can be any tokens separated by whitespace characters.
Accepted tokens as well as the number of arguments are defined by the client.

In addition to tokens supported by tnac, arguments can also be string literals -
any sequences of characters separated by `'`'s.

Names are identifiers preceded by `#` - `#command_name`

Here's an example of a command which takes an int literal, a string and an identifier:
```
#command 42 'string of chars' x
```

Commands can appear between expressions, or directly after an expression:
```
a = 0 :
#command 'between exprs' :
b = a + 1 #command 'trailing' :
c = b
```

tnac driver defines the following commands:

* `#exit` - takes no arguments, exits the main loop
* `#result [<identifier>]` - prints the last evaluation result.
Its optional parameter is an identifier which sets the desired base for the printed value.
Supported modes:
  * `bin` - base 2
  * `oct` - base 8
  * `dec` - base 10
  * `hex` - base 16
* `ast [<string>] [<identifier>]` - prints the ast. Arguments are optional.
The first provides a file name to print the ast to, the second controls whether
the entire ast, or only the last parsed expression will be printed. See examples below.
* `#list [<string>]` - prints the entire source code. The optional string argument
is the file name to print it to
* `#vars [<string>]` - prints a list of all variables in existence. The optional string argument
is the file name to print it to
* `#bin`, `#oct`, `#dec`, `#hex` - these make sense when supplied right after
an expression. They control the numeric base of the printed value.

### Command examples

The `#ast` command (given the sequence of inputs below):

input 1: `a = 10`

input 2: `b = a + 1`

input 3: `a + b`

`#ast` prints this to stdout:
```
<scope>
|-Declaration  <VarName: a>
| `-Literal expression '10' <value: 10>
|-Declaration  <VarName: b>
| `-Binary expression '+'
|   |-Id expression 'a'
|   `-Literal expression '1' <value: 1>
`-Assign expression '='
  |-Id expression 'a'
  `-Binary expression '+'
    |-Id expression 'a'
    `-Id expression 'b'
```
`#ast 'out.txt'` produces the same output and prints it to a file named
`out.txt` and located in the current working directory

`#ast 'out.txt' current` same as above, but only prints the last assign expression

`#ast '' current` prints this to stdout:
```
Assign expression '='
|-Id expression 'a'
`-Binary expression '+'
  |-Id expression 'a'
  `-Id expression 'b'
```

`#vars` prints this to stdout:
```
a : 21
b : 11
```

`#vars 'out.txt'` - same, but uses the `out.txt` file

`#list` prints this to stdout:
```
a = 10 :
b = a + 1 :
a = a + b
```

`#list 'out.txt'` - same, but uses the `out.txt` file

`#result` prints `21`

`#result hex` prints `0x15`

Base-controlling commands work like this:
```
0b110011 ^ 0b101101 #bin
```
will print `0b11110`

The same, but with a `hex` instead:
```
0b110011 ^ 0b101101 #hex
```
will print `0x1e`