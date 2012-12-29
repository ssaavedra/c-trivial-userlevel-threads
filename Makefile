
CFLAGS ?= -m32

all: main
run: force main
	./main


main: main.c yield.o
	$(CC) $(CFLAGS) $^ -o $@

yield.o: yield.c
	$(CC) $(CFLAGS) $^ -c -o $@

.PHONY: force
force:
	touch main.c

