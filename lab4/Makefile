CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -ldl

all: libfuncs1.so libfuncs2.so program1 program2

# Сборка библиотек
libfuncs1.so: libfuncs1.c
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $<

libfuncs2.so: libfuncs2.c
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $<

# Программа с линковкой
program1: program1.c libfuncs1.so
	$(CC) $(CFLAGS) -o $@ program1.c -L. -l:libfuncs1.so -Wl,-rpath,.

# Программа с динамической загрузкой
program2: program2.c
	$(CC) $(CFLAGS) -o $@ program2.c $(LDFLAGS)

clean:
	rm -f *.so program1 program2

run1: program1
	./program1

run2: program2 libfuncs1.so libfuncs2.so
	./program2

.PHONY: all clean run1 run2