# Human Math Expression Parser

## About
This project is a simple math expression parser which handles many cases where some operator is implicitly present. The included test.cpp is very handy in using this library

*Note* : A C++ 17 compliant compiler is required to build the project

---
## Usage
Include header file
`#include "expression_parser.h"`
and then execute
`expressionParser::solve("some string");`

*Note* : All the exceptions are handled by expressionParser::exception object

---
## Examples

This library is space ignorant. However, spaces must not be used for function names.  
For e.g. : `sin50` is valid, but `s in 50` is not

| Expression | solve(Expression) |
| ---------- | ----------------- |
| (2+3)5 | 25.0 |
| 25sin 3.1415 | 0.0 |
| 4!5 | 120 |
| sin 50(3.1415) | −0.8242 |

<sup><sub>sin 50(3.1415) is interpreted as 3.1415(sin 50)</sub></sup>


Compile test.cpp to solve any expression through command line

## Known Bugs
* Let me know if there are any
