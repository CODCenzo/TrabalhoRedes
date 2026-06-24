CC = gcc
CFLAGS = -Wall -I../../Headers
LIBS = -lncurses

TARGET = exe_client

CLIENT_SRC = main.c
GAME_SRC = ../Game/game.c ../Game/draw.c ../Game/aux_game.c ../Game/client.c
CONTROL_SRC = ../Control/game_protocol.c ../Control/protocol.c ../Control/kermit.c
SRC = $(CLIENT_SRC) $(GAME_SRC) $(CONTROL_SRC)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ) ../Control/socket.o
