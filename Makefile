LUA     = 5.1
PREFIX  = /usr/local
LIBDIR  = $(PREFIX)/lib/lua/$(LUA)

LUA_CFLAGS  = $(shell pkg-config --cflags lua$(LUA))
CFLAGS  = -fPIC $(LUA_CFLAGS) -I/usr/include/
LIBS    = $(shell pkg-config --libs lua$(LUA)) -lsox

libsox.so: lua-libsox.o
	$(CC) -shared $(CFLAGS) -o $@ lua-libsox.o $(LIBS)

install:
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp libsox.so $(DESTDIR)$(LIBDIR)

docs: libsox.so docs/config.ld
	ldoc -c docs/config.ld -d html -a .

clean:
	rm -rf *.o *.so

.PHONY: libsox.so
