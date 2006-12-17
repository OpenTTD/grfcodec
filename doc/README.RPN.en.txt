NFORenum -- RPN calculator
==========================

Most of you are familiar with infix notation, eg "4 + 2" or "4 + (2 * 2)".
Infix can be expressed as "left operation right"
RPN (Reverse Polish Notation, http://tinyurl.com/akj8w) uses
"left right operation", eg "4 2 +" or "4 2 2 * +". RPN has the advantage that
no parentheses are required in the expression; each well-formed expression
can only be evaluated in one and only one way.

In NFORenum, an RPN expression is introduced by an open parenthesis, and
accepts as operators +, -, *, and /. Completion of an RPN expression is
signaled by a close parenthesis, and the result of the RPN expression is
returned. If extra numbers are listed, they are simply ignored
(eg (1 2 3 +) is 5). If a number is listed at the end of an expression, that
number is returned (eg (1 2 + 3) and (3) are both 3).  Division is integer
division, and rounded down; (5 3 /) is 1.

In RPN expressions, a previously defined variable (variables are defined by
@@LET) may be used in place of any number, both in real sprites and in @@LET
commands. Variables should be valid C identifiers (first character must be A..Z,
a..z or _, all others must be A..Z, a..z, 0..9 or _), and are case sensitive.
Variables with 2 leading underscores are reserved for possible future use by
NFORenum.

An RPN expression may be used in a real sprite at any place where an decimal
integer is expected(xpos, ypos, ysize, xsize, xrel, and yrel), and on the
right side of the @@LET command.

RPN expressions may not be nested.

Malformed RPN expressions will cause the real sprite to be commented out,
just as if it were missing metadata. The calculator will do its best to let
you know what went wrong.

Currently, negative numbers may not be used in RPN expressions. If you want,
eg "-5", in an RPN expression, use "0 5 -" instead. Alternatively,
"@@LET var=-5", and use "var" instead.

The RPN parser has the following known limitations:
1) Negative numbers are not accepted.
  (use "0 <num> -" instead, or assign to a variable.)
2) Only the four basic operators are accepted: +, -, *, /
