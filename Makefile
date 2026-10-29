# Make File

CC = gcc

CFLAGS = -Iinclude -g -D_REENTRANT
LIBS = -lSDL3 -lSDL3_ttf


SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

TARGET = epong

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
