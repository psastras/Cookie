# Makefile for Water 2011 by psastras


CC = g++

CFLAGS = -Wall -O3

COMPILE = $(CC) $(CFLAGS) -c
LDFLAGS = -lX11 -lGL -lGLU
SRCDIR := src/
OBJFILES := $(patsubst src/%.cpp,src/%.o,$(wildcard src/*.cpp))

all: water

water: $(OBJFILES)
	$(CC) $(LDFLAGS) -o bin/water $(OBJFILES)


%.o: %.cpp

	$(COMPILE)  -o $@ $<

.PHONY: clean
clean:
	rm -f $(SRCDIR)*.o
