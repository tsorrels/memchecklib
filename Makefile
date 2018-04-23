CC=gcc

CFLAGS = -g -c -Wall -pedantic

all: lib exampleprog

lib: memchecklib.a

memchecklib.a: memcheck.o memcheck.h
	ar rcs memchecklib.a memcheck.o

memcheck.o: memcheck.c
	$(CC) -c -o memcheck.o memcheck.c 

exampleprog: exampleprog.c memchecklib.a
	$(CC) -o exampleprog exampleprog.c memchecklib.a -lcrypto

clean:
	rm *.o
	rm exampleprog
	rm *.a

%.o:    %.c
	$(CC) $(CFLAGS) $*.c
