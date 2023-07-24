# Totally Not Another Calculator

This is tnac - Totally (Absolutely, Positively, Definitely) Not Another Calculator.
Well, except it kind of is. Written entirely in C++.

This project was a challenge to myself, and it is basically an answer to the question:
*"Were I to write a compilery thing, what would it be, and how crazy would I be able to make it?"*

At any rate, it's been a good excersice on writing parsers.

Under the hood it has an expression evaluator which understands a bunch of maths over some kind of data.

Except, it is starting to look like a functional programming language of some sort.
I guess, I should rename it to IAPL (*"I accidentally a programming language"*) or something.

## Features

### Types

- 64-bit signed integers (`long long` aka `std::int64_t`)
- Floating point with (`double`)
- Complex numbers (via `std::complex`)
- Ordinary fractions
- Booleans (`bool`)
- Arrays
- Undefined (you can call those `null` or something similar)

Types are (mostly) inter-convertible and calculated dynamically.

From this point on, I will refer to integer and floating-point types simply
as `ints` and `floats` for brevity.

### Operations

- Unary `+` and `-`
- Addition (`+`)
- Subtraction (`-`)
- Multiplication (`*`)
- Division (`/`)
- Modulo (`%`)
- Bitwise negation (`~`)
- Bitwise and (`&`)
- Bitwise or (`|`)
- Bitwise xor (`^`)
- Power (`**`)
- Root (`//`)
- Logical not (`!`)
- Logical... err... opposite of not (`?`)
- Logical and (`&&`)
- Logical or (`||`)
- Equality comparison (`==`, `!=`)
- Relational operators (`<`, `>`, `<=`, `>=`)

Not all operations are supported for all types.
Using something unsupported results in a quiet "undefined" value.
Given the dynamic nature of types, this seems a reasonable choice since we normally
can't predict what kind of value we'll get.

The only exception are `!` and `?`. They accept anything and try to cast it to `bool`.
Any zero values and `undefined` are `false`, others are `true`.

Integer division is implicitly done via the floating type to avoid division by zero.
An expression like `42 / 0` will result in an `infinity` value.

Bitwise operations are supported for `ints` and values of other types wich are 'int-like'.
Being an 'int-like' means being able to convert to int without loss of data.

For example, `2.5 & 2` will yield an undefined value, while `2.0 & 2` is equivalent to `2 & 2`.
Similarly, a complex value of `2.0 + 0i` is considered an 'int-like',
whereas `2.0 + i` is not.

### Driver

The driver is a basic CLI interface which works in interactive mode by default.
It displays a prompt allowing the user to enter expressions which it will then
evaluate and produce a result.

The diver also supports certain commands. More on them later.

You can make the program run a program from a file and print out the result like this:

```tnac myfile.whatevz```

This will run, interpret the code, print the result, and exit.

You can also make it do the interpreting and printing, but stay in interactive mode by adding `-i`:

```tnac myfile.whatevz -i```

## Syntax

This section will contain syntax basics along with some examples.
For a taste of actual grammatics behind all this, see [grammar](stuff/grammar.md)

### Comments

Any sequence of characters enclosed in `'s becomes a comment:
```
`I am a comment`

`
  I am a comment
  too
`
```

### Numeric literals

Numbers can be of `bool`, `int` or `float` type.
Numbers can include any sequence of digits, a dot (`.`), or a prefix.

A number with exactly one dot surrounded by digits (for example, `1.111`) is parsed as a `float`.

`Ints` come in different flavours:

1. Decimal. Anything your normal integer can be (`1`, `42`, `1337`, etc)
2. Binary. It is, well, a binary - a sequence of `1`'s and `0`'s prefixed with `0b` or `0B` (`0b11100111`)
3. Octal. Digits `[0 ... 7]` prefixed with `0` (`0123`)
4. Hexadecimal. Digits and non-case-sensitive letters `[A ... F]` prefixed with `0x` or `0X` (`0xff12a`)

Boolean literals are `_true` and `_false`.

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

Type instantiations:
- `_cplx`
- `_frac`
- `_int`
- `_flt`
- `_bool`

Anonimous function decl:
- `_fn`

Result expression:
- `_result`

Return expression:
- `_ret`

Constants:
- `_true` - boolean `true`
- `_false` - boolean `false`
- `_i` - imaginary unit `i`
- `_pi` - the value of `pi`
- `_e` - the value of `e`

Note: the `_i` constant can be used to construct complex numbers indirectly.
This:
```
_cplx(1, 2)
```
is exactly the same as this:
```
1 + 2*_i
```

### Expressions

Expressions can include:

- Numbers
- Identifier names
- Expressions enclosed in parentheses (`(`, `)`)
- Unary operators `+`, `-`, `~`, `!`, `?`
- Binary operators `+`, `-`, `*`, `/`, `%`, `&`, `|`, `^`, `**`, `//`, `||`, `&&`, `<`, `>`, `<=`, `>=`, `==`, `!=`
- Assignments `=`
- Absolute value calculations - `| <expr> |`
- Keywords
- Invocations
- Declarations

Multiple expressions can be chained one after another by using `:`.

This:
```
2 + 2 :
3 + 3 :
4 + 4
```
will evaluate the 3 expressions listed one by one from the first one to the last.

**Parentheses** can enclose any valid expression.

**Absolute value** works with any valid expression, pretty much as paren does

Examples:
- `| -2 |` yields `2`
- `| _cplx(3, 4) |` yields 5.0

**Unary** operators can be applied to literals, invocations, identifiers, or the `_result` keyword.

The `_result` keyword gets replaced with the most recent evaluation result.
For example, the expression `2 + 2 + _result` evaluates to `8`, since `_result`
expands to the evaluation result of `2 + 2`.

Examples: `-1`, `-(2+2)`, `+(10 / 4)`, `-_result`, `+_cplx(1,2)`

**Binary** operators can combine pretty much any valid expressions.

Examples: `2+2`, `-(_frac(1, 2) + 1) * 2`

**Invocations** are things with arguments.
They can create values of certain types, or call functions.

Currently, there are these invocation expressions:

- `_frac(<expr>, <expr>)` - instantiates an ordinary fraction.
Both the arguments (numerator and denominator) are mandatory. Any expressions will work, however,
value types other than int will convert and, potentially, lose data.
Examples: `_frac(1, 2)` is `1/2`, `_frac(4-2, 6/2)` is `2/3`.

- `_cplx(<expr>, <expr>)` - instantiates a complex number.
The first argument is the real part, and the second is imaginary.
None of the arguments are mandatory, they'll be 0 by default. Examples:
`_cplx()` is `0 + 0i`, `_cplx(1)` is `1 + 0i`, `_cplx(2+2, 1/2)` is `4 + 0.5i`

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

#### Functions

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

Just because internal functions are not visible outside of their scope,
doesn't mean they are not reachable. Example (evaluates to `21`):
```
func()
  internal(a, b)
    a + b;
  internal ;

a = func() :
a(10, 11)
```

Also, this is the same as the one above, just neater:
```
func()
  internal(a, b)
    a + b;
  ;

func()(10, 11)
```


Functions can be arguments of other functions, too.
This will result in `11`:
```
func(x)
  x() + 1;

other_func()
  10;

func(other_func)
```

Functions can be anonimous, kinda like lambdas in C++. You can't access them by name,
but you can do anything else with them what you can do with normal functions.

Such functions are declared via the `_fn` keyword:
```
a = _fn(x, y) x * y;
a(2, 3)
```
prints `6`

Also:
```
some_func(x, y) x(y);
some_func(_fn(x) x * x;, 10)
```
prints `100`

## _ret

The `_ret` keyword can be used to prematurely return to the caller.
This works either in functions, or in the global scope (in which case, the execution will finish):
```
func()
  _ret 41 :
  0 `unreachable`
;

`This is the global scope`
_ret func() + 1 : `evaluates to 42`
1 `unreachable`
```

## Arrays

### Basics

Arrays don't have any special keywords or anything. Creating them is simple:
```
[
  1,
  2 / 4,
  _cplx(3,4),
  some_func(42)
]
```
This will create an array of 4 elements containing an `int`, a `float`, a `complex`,
and whatever `some_func` might return.

You can also save an array to a variable:
```
a = [ 1, 2, 3 ]
```
Or create an array of arrays:
```
[
  [ 1, 2 ],
  [ 3, 4]
]
```

Overall, arrays behave just like any other types.
For example, they can participate in any valid expression:
```
[1, 2, 3] * 2
```
is going to result in:
```
[2, 4, 6]
```

Unaries also work as you'd expect:
```
-[1, -1, 2]
```
will give:
```
[-1, 1, -2]
```

Binaries where both operands are arrays will create larger arrays where
the given binary operator is applied to each pair of elements:
```
[1, 2, 3] ** [ 3, 4 ]
```
will give:
```
[ 1, 1, 8, 16, 27, 81 ]
```
The resulting array's size is `<size of the lhs> * <size of the rhs>`

### Array calls

Let's assume, we have some functions like these:
```
sum(a, b) a + b;
dif(a, b) |a - b|;
mul(a, b) a * b;
div(a, b) a / b;
```

Since functions are data types, we can put them in an array too:
```
arr = [ sum, dif ]
```
The above declaration will create an arry of two functions.
Now, since there are functions there, the array itself becomes callable:
```
arr(2,3)
```
will give us:
```
[ 5, 1 ]
```
This will apply the given arguments to each function in the array
and return an array of results.

We can also do crazy things like this:
```
[ sum, [ dif, mul ], div ](42, 69)
```
which will give us:
```
[ 111, [ 27, 2898 ], 0.608696 ]
```
Any non-functions in an array participating in a call will be ignored.
Any functions with a wrong number of params will be ignored too.
```
[
  _fn(a) a + 1;,
  0,
  _fn(a) a * 2;,
  _fn(a, b) a + b;
](2)
```
will give us
```
[3, 4]
```
since we supply one argument, and there are two functions having one parameter in the array.

On the other hand:
```
[
  _fn(a) a + 1;,
  0,
  _fn(a) a * 2;,
  _fn(a, b) a + b;
](2, 3)
```
will return:
```
[ 5 ]
```

## Conditionals

Conditional expressions come in two kinds - full notation, and shorthand notation.

### Full conditionals

The full notation goes like this:
```
{ <condition> }
  { <pattern> } -> [<expr> : ... ];
  ...
;
```

The condition value is subsequently compared with the specified patterns,
and the first one which evaluates to `true` becomes the resulting value.

If none of the patterns match, the default one is applied.
If nothing matches at all, the value is undefined.

Pattern bodies (the thing after `->`) are optional. If unspecified, they result in undefined values.

Patterns look like this:
1. `{ [equality or relational operator] <expr> }`
2. `{ ! }`
3. `{ ? }`
4. `{}`

`Option 1` applies the specified binary operator to the condition value on the left, and the following expression on the right.
If no operator is specified, `==` is the default
For example:
```
{ 10 }
  { < 0 } -> -1 ; `this is false since 10 is not less than 0`
  { 10 }  ->  1 ; `this is true, the implicit == is used`
;
```

Another example with a default pattern (`Option 4`):
```
factorial(arg)
  { arg }
    { < 0 }  -> ;
    { 0 }    ->  1  ;
    {}       ->  arg * factorial(arg - 1) ;
  ;
;
```
This one calculates the factorial.
The `{ < 0 }` pattern will evaluate to `true` for
any value less than `0`. The resulting value of the entire function is `undefined` in this case.
The `{ 0 }` will compare `arg` for equality with `0`
The `{}` one is default. It will run if the others evaluate to `false` and recurse
while `arg` is positive.

If we call the above function like this, we'll get a valid result:
```
factorial(5) `returns 120`
```

But what happens if we pass in something weird, like a function?
This
```
factorial(_fn();)
```
will result in this:
```
{}       ->  arg * factorial(arg - 1) ;
                   ^~~ Stack overflow
```

Since functions can't participate in arithmetics, none of the non-default patterns will match.

This is why unary patterns exist. `Option 2` will apply the logical not operation to the condition's value.
Since absolutely everything converts to `bool`, this will work just fine:

```
factorial(arg)
  { arg }
    { < 0 }  -> ;
    { ! }    ->  1  ;
    {}       ->  arg * factorial(arg - 1) ;
  ;
;
```
The `factorial(_fn();)` example will return `undefined`.

### Short conditionals

If you just need to check an expression in a "bool-like" context, it is inconvenient to write
a full conditional will patterns and stuff like that.
Enter shorthand notation:
```
{ <condition> } -> { [<expr-if-true>, [<expr-if-false>] }
```

The condition gets evaluated, then cast to `bool`.
According to the result:
- if the condition is `true`, and the optional `expr-if-true` branch exists, the result is the `expr-if-true` value
- if the condition is `true`, and the optional `expr-if-true` branch doesn't exists, the result is the `condition` value **before** conversion to `bool`
- if the condition is `false`, and the optional `expr-if-false` branch exists, the result is the `expr-if-false` value
- if the condition is `false`, and the optional `expr-if-false` branch doesn't exists, the result is an `undefined` value

Example:
```
test(x)
  { x + 1 } -> { 1, 0 }
;
```
This function will happily eat anything you throw at it and return either `0` or `1`

Also:
```
{ 0 }  -> { , 42 } : `42`
{ 23 } -> { , 42 } : `23`
{ 23 } ->      { } : `23, this one is kinda pointless`
{ 23 } ->   { 42 } : `42`
{ 0 }  ->   { 42 } : `undefined`
```

### Conditionals are expressions

They are primary expressions, to be precise. As such, you can use them anywhere literals can be

```
a = 10 :
b = { a > 0 } -> { 1, -1 } `b = 1`
```

A bit more interesting example:
```
a = 10 :
{ a > 0 } -> { _fn(x) x + 1;, _fn(x) x - 1; }(a)
```
This will result in `11` since `a > 0 == true`, so the result of the expression will evaluate to
function `_fn(x) x + 1;`, which will then accept `a` as its argument and increment it.

## Commands

Commands are special directives that are not, technically, a part of tnac.
By default, none are declared. Clients should add their own commands according to their needs.

Commands have names and arguments. They begin at the name and end at `:` or
end of input. Arguments can be any tokens separated by whitespace characters.
Accepted tokens as well as the number of arguments are defined by the client.

In addition to tokens supported by tnac, arguments can also be string literals -
any sequences of characters separated by `'`'s.

Names are identifiers preceded by `#` - `#command_name`

Here's an example of a command which takes an int literal, a string, and an identifier:
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

Given the sequence of inputs below:

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