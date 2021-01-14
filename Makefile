all:
		gcc -fPIC -w -c freq-byte.c
		gcc -shared -o freq-byte.so freq-byte.o
		gcc main.c -o lab1 -ldl -w
