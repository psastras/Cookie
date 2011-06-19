# Makefile for Water 2011 by psastras


CC = g++

CFLAGS = -Wall -O3

COMPILE = $(CC) $(CFLAGS) -c
LDFLAGS = -lm -lX11 -lGL -lGLU -lfftw3
SRCDIR := src
VSMLDIR := 3rdparty/VSML/
OBJFILES := $(patsubst $(SRCDIR)/%.cpp,$(SRCDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp $(VSMLDIR)/*.cpp))

all: water

water: $(OBJFILES)
	$(CC) -o bin/water $(OBJFILES) $(LDFLAGS)


%.o: %.cpp

	$(COMPILE)  -o $@ $<

.PHONY: clean
clean:
	rm -f $(SRCDIR)/*.o
