# tnac grammar

This is the complete tnac grammar

```
scope:
  expr-list

expr-list:
  expr
  expr-list : expr

expr:
  decl-expr
  assign-expr

decl-expr:
  var-decl
  func-decl

assign-expr:
  bit-or-expr
  id-expr assign-operator assign-expr

bit-or-expr:
  bit-xor-expr
  bit-or-expr '|' bit-xor-expr

bit-xor-expr:
  bit-and-expr
  bit-xor-expr '^' bit-and-expr

bit-and-expr:
  additive-expr
  bit-and-expr '&' additive-expr

additive-expr:
  multiplicative-expr
  additive-expr additive-operator multiplicative-expr

multiplicative-expr:
  unary-expr
  multiplicative-expr multiplicative-operator unary-expr

unary-expr:
  call-expr
  unary-operator primary-expr

primary-expr:
  result-expr
  literal-expr
  id-expr
  typed-expr
  anonimous-function
  '(' expr ')'

var-decl:
  undeclared-identifier '=' expr

func-decl:
  undeclared-identifier func-params func-body

func-body:
  ''
  expr-list ';'

func-params:
  '(' func-param-list ')'

func-param-list:
  ''
  param-decl
  func-param-list ',' param-decl

param-decl:
  undeclared-identifier

undeclared-identifier:
  identifier (not previously declared)

result-expr:
  _result

id-expr:
  identifier (previously declared)

literal-expr:
  number

anonimous-function:
  _function func-params func-body

call-expr:
  primary-expr
  call-expr args

typed-expr:
  type-name args

type-name:
  type-keyword

type-keyword: one of
  _complex
  _fraction

args:
  '(' arg-list ')'

arg-list:
  ''
  expr
  arg-list ',' expr

command:
  # id-sequence

keyword:
  '_' id-sequence

identifier:
  identifier-start id-sequence

number:
  int-num
  float-num

float-num:
  digit-sequence '.' digit-sequence

int-num:
  binary-int-num
  octal-int-num
  decimal-int-num
  hex-int-num

binary-int-num:
  '0b' binary-digit-sequence

oct-int-num:
  '0' oct-digit-sequence

decimal-int-num:
  non-zero-digit digit-sequence

hex-int-num:
  '0x' hex-digit-sequence

binary-digit-sequence:
  binary-digit
  binary-digit-sequence binary-digit

oct-digit-sequence:
  oct-digit
  oct-digit-sequence oct-digit

digit-sequence:
  digit
  digit-sequence digit

hex-digit-sequence:
  hex-digit
  hex-digit-sequence hex-digit

assign-operator: one of
  =

multiplicative-operator: one of
  * / %

additive-operator: one of
  + -

unary-operator: one of
  + - ~

operator: one of
  + - ~ * / % | ^ & =

string-literal:
  ' anything '

id-sequence:
  identifier-char
  id-sequence identifier-char

identifier-start: one of
  a b c d e f g h i j k l m n o p q r s t u v w x y z
  A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

identifier-char: one of
  a b c d e f g h i j k l m n o p q r s t u v w x y z
  A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
  _ 1 2 3 4 5 6 7 8 9 0

binary-digit: one of
  0 1

oct-digit: one of
  0 1 2 3 4 5 6 7

non-zero-digit: one of
  1 2 3 4 5 6 7 8 9

digit: one of
  0 1 2 3 4 5 6 7 8 9

hex-digit: one of
  0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F

```