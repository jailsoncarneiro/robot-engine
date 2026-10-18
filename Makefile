CC = gcc
CFLAGS = -Wall -O2 -Iinclude
LDFLAGS = -lmujoco -lglfw -lGL -lm

SRC = src/main.c
TARGET = build/engine

all:
	@mkdir -p build
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -rf build