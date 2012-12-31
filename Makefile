
CFLAGS ?= -m32 -g

all: main
run: force main
	./main


main: main.c yield.o
	$(CC) $(CFLAGS) $^ -o $@

factorial: factorial.c yield.o
	$(CC) $(CFLAGS) $^ -o $@

dbuf: dbuf.c yield.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

yield.o: yield.c
	$(CC) $(CFLAGS) $^ -c -o $@

.PHONY: force
force:
	touch main.c

