# Makefile for Cookie by psastras

CC = g++

ifeq ($(BUILD_TYPE), debug)
    CFLAGS = -Wall -g -std=c++0x
else
    CFLAGS = -O3 -msse2 -ffast-math -fno-signed-zeros -fno-trapping-math -std=c++0x
endif


ifeq ($(OS),Windows_NT)
    LDFLAGS = -L/usr/local/lib -lglew32 -lglu32 -lopengl32 -lgdi32 -lfftw3f -L3rdparty/fftw
else
    LDFLAGS = -lm -lX11 -lGL -lGLU -lfftw3f -lfftw3f_threads
endif

INCLUDES = -IC:/Qt/qtcreator-2.2.82/mingw/include

SRCDIR := src
VSMLDIR := 3rdparty/VSML
OBJFILES := $(patsubst $(VSMLDIR)/%.cpp,$(VSMLDIR)/%.o,$(wildcard $(VSMLDIR)/*.cpp)) $(patsubst $(SRCDIR)/%.cpp,$(SRCDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
COMPILE = $(CC) $(CFLAGS) $(INCLUDES) -c

all: water
water: $(OBJFILES)
	$(CC) $(LDFLAGS) -o bin/water.exe $(OBJFILES) $(LDFLAGS)


%.o: %.cpp
	$(COMPILE) -o $@ $<

.PHONY: clean
clean:
	rm -f $(SRCDIR)/*.o
