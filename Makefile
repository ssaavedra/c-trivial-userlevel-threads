
CFLAGS ?= -m32 -g

STRIP ?= /bin/true

all: main
run: force main
	./main


main: main.c yield.o sthread.o
	$(CC) $(CFLAGS) $^ -o $@

factorial: factorial.c yield.o
	$(CC) $(CFLAGS) $^ -o $@

dbuf: dbuf.c yield.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

yield.o: yield.c
	$(CC) $(CFLAGS) $^ -c -o $@
	$(STRIP) -X -x -g $@

sthread.o: sthread.c
	$(CC) $(CFLAGS) $^ -c -o $@
	$(STRIP) -X -x -g $@

sthread_s.o: sthread.c
	$(CC) $(CFLAGS) -fPIC $^ -c -o $@
	$(STRIP) -X -x -g $@

yield_s.o: yield.c
	$(CC) $(CFLAGS) -fPIC $^ -c -o $@
	$(STRIP) -X -x -g $@

sthread.a: yield.o sthread.o
	$(AR) rcs $@ $^

libsthread.so: yield_s.o sthread_s.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$@ -o $@ $^
	$(STRIP) -X -x -g $@


.PHONY: force
force:
	touch main.c

