CC      = cc
CC_WIN  = x86_64-w64-mingw32-gcc

PREFIX ?= /usr/local

LDFLAGS = -L$(PREFIX)/lib
INCLUDES = -I$(PREFIX)/include
INCLUDES_WIN = -I/usr/local/x86_64-w64-mingw32/include

SRC     = src/main.c src/render.c src/player.c src/maze.c

LIB     = bin/aschii.a
LIB_WIN = bin/win_aschii.a

CFLAGS  = -O0 -g
LDLIBS  = -lSDL2 -lSDL2_mixer

.PHONY: linux windows

$(info PREFIX='$(PREFIX)')
$(info LDFLAGS='$(LDFLAGS)')

linux:
	$(CC) $(SRC) $(LIB) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LDLIBS) -lm -lpthread -o bin/linux/45_seconds

windows:
	$(CC_WIN) $(SRC) $(LIB_WIN) $(CFLAGS) \
		$(INCLUDES_WIN) \
		-L/usr/local/x86_64-w64-mingw32/lib \
		$(LDLIBS) \
		-Wl,-e,mainCRTStartup -Wl,--subsystem,console \
		-o bin/windows/45_seconds.exe

