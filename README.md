ASM-Preprocessor
================

A textual-substitution preprocessor written to "Include" multiple assembly source files into a single source file for compilation.

Written to overcome limitations with the Include feature of the MIPS assembly language IDE named MARS.

When run on an ASM source file, (although technically, the program supports any ASCII text file) the program finds and replaces any statements of the format "@Include <filename.extension>" with the contents of the included file.

How to Compile:
 - Using any c99-compliant compiler, compile "preprocessor.c" into an executable file. A batch file named "CompileAndRun.bat" which does this using GCC, and then tries to run

How to Run:
  - Run the executable with a ASCII text file as input. If any statements of the format "@Include <filename.extension>" are found within, the program will search for a file of the same name and substitute the contents of that file in the Include statement's place.
  
Optional command line flags:
 "-i [filename]" Loads filename as the input file
 "-o [filename]" The created output file will be named filename
 "-d" Prints verbose debug / info messages while running
