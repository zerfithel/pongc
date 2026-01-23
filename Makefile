# pongc

CC = gcc
TARGET = pongc
CFLAGS = -Iinclude -Wall -Wextra -std=c23 -O2
LDFLAGS = -lm -lSDL2 -lSDL2_ttf -lenet
SRC = src/main.c \
			src/game.c \
			src/ball.c \
			src/signals.c \
			src/client.c \
			src/server.c \
			src/utils.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean: $(TARGET)
	rm -f $(TARGET)

.PHONY: all clean
