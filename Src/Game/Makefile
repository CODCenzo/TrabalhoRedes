CC = gcc
CFLAGS = -Wall -I../../Headers
LIBS = -lncurses

TARGET = main

GAME_SRC = main.c game.c draw.c aux_game.c client.c
CONTROL_SRC = ../Control/game_protocol.c ../Control/protocol.c ../Control/kermit.c ../Control/socket.c
SRC = $(GAME_SRC) $(CONTROL_SRC)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
