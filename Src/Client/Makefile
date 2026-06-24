CC = gcc
CPPFLAGS += -I. -I../Game -I../../Headers -MMD -MP -Dcria_socket=cria_raw_socket -include socket.h
CFLAGS += -Wall
LDLIBS += -lncurses
RM ?= rm -f
RMDIR ?= rm -rf
MKDIR_P ?= mkdir -p

TARGET = exe_client
BUILD_DIR = build

CLIENT_SRCS = main.c
GAME_SRCS = ../Game/game.c ../Game/draw.c ../Game/aux_game.c ../Game/client.c
CONTROL_SRCS = ../Control/game_protocol.c ../Control/protocol.c ../Control/kermit.c ../Control/socket.c

CLIENT_OBJS = $(patsubst %.c,$(BUILD_DIR)/Client/%.o,$(CLIENT_SRCS))
GAME_OBJS = $(patsubst ../Game/%.c,$(BUILD_DIR)/Game/%.o,$(GAME_SRCS))
CONTROL_OBJS = $(patsubst ../Control/%.c,$(BUILD_DIR)/Control/%.o,$(CONTROL_SRCS))
OBJS = $(CLIENT_OBJS) $(GAME_OBJS) $(CONTROL_OBJS)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

$(BUILD_DIR)/Client/%.o: %.c | $(BUILD_DIR)/Client
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Game/%.o: ../Game/%.c | $(BUILD_DIR)/Game
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Control/%.o: ../Control/%.c | $(BUILD_DIR)/Control
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Client $(BUILD_DIR)/Game $(BUILD_DIR)/Control:
	$(MKDIR_P) $@

clean:
	$(RMDIR) $(BUILD_DIR)
	$(RM) $(TARGET) *.o ../Game/*.o ../Control/*.o

rebuild: clean all

-include $(DEPS)

.PHONY: all clean rebuild
