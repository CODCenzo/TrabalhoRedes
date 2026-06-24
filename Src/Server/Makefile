CC = gcc
CFLAGS = -Wall -I../../Headers
LIBS = -lncurses

TARGET = exe_server

SRC = main.c ../Game/game.c ../Game/draw.c ../Game/aux_game.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
