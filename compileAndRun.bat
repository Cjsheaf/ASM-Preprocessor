echo off

gcc -std=c99 -c "C Files\preprocessor.c" -o "Object Files\preprocessor.o"

gcc -std=c99 "Object Files\preprocessor.o" -o preprocessor

preprocessor -i code.asm -o code_out.asm -d
pause