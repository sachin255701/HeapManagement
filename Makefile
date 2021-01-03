CC=       	gcc
CFLAGS= 	-g -gdwarf-2 -std=gnu99 -Wall
LDFLAGS=
LIBRARIES=      lib/libmalloc-ff.so \
		lib/libmalloc-bf.so \
		lib/libmalloc-wf.so

all:    $(LIBRARIES)

lib/libmalloc-ff.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DFIT=0 -o $@ $< $(LDFLAGS)

lib/libmalloc-bf.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DFIT=1 -o $@ $< $(LDFLAGS)

lib/libmalloc-wf.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DFIT=2 -o $@ $< $(LDFLAGS)

clean:
	rm -f $(LIBRARIES)

.PHONY: all clean
