# Hue
A lisp-like that can fit into your pocket

## Example
```lisp
(do int
    (puts "Hello there")
    (int i)
    (set i (add 1 2 3))
    (if i (do void
        (puts "If works")
    ))
    (return i) ; Return i
)
```
Note: the print function is not provided anywhere. It is assumed that the programmer added it after code generation.

## Why
Choose Hue because:
* smol: Written less than 200 lines of C++-code
* sped: Generates C-code, the fastest in the universe
* strong, independent: By default code generated by Hue does not have any library dependencies. It can always call C-functions tho.
* familiar errors: As long as you follow basic syntax rules, Hue won't send you any errors. But your C-compiler will.

![smol](https://github.com/Not-Nik/HUE/blob/master/mm.jpg)  

## Syntax specifics
HUE statements are written like so:
`(<statement name> <statement arguments)`
Arguments can be more statements or simply primitives.
Primitives are how you know them already: Numbers and strings:
* `0`
* `42`
* `36.3636`
* `"This is a string""`

## Lambdas
Similarly to lisp, HUE features lambdas. They are created with the `do` statement and can return a value using the return statement.
These lambdas can be used anywhere including in `if` and `while` conditions and variable assignments. They can contain any instruction
like a loop or even another lambda.
Also they are the only feature HUE has that C doesn't.

## Hacking HUE
If you want to add custom code generation to HUE simply do so by adding an overruling for a specific statement name.
An overruling is a function that takes an expression of type `Expr` and returns a string of type `std::string`. If you fell the urge
to generate code that shouldn't be directly in-place like functions or structs, simply add a string to `functions`
