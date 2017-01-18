# Derive
One more progam[me] for differentiation.

C++11 is required (range-based for). (gcc 6.1.1 is OK, I think.)

A single file - no issues with compilation (I hope).

'windows.h' - was used to set the position of the cursor (void locate(int, int) - is not used now).

Input file - 'deriv.in': one expression - one line (parameter may be 'x', 'X').

Output file - 'deriv.out': representative derivations for each test (may contain 'e').


Implemented:
 - Unary op-s: '-' (unary), ln, sin, cos, tg, ctg, arcsin, arctg
 - Binary op-s: '+', '-', '**', '/', '****' ('^')
 - Brackets
 - Сrutches for x^const
 - simple simplifications
