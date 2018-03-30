#GA makefile excute make -f mk to generate GA.o file
main.o: functions.c header.h #dependency list
		gcc -m32 -o main.o *.c -lm #command to be executed
