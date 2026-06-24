CC = gcc
CPPFLAGS += -I. -I../../Headers -MMD -MP
CFLAGS += -Wall
LDLIBS += -lncurses
RM ?= rm -f
RMDIR ?= rm -rf
MKDIR_P ?= mkdir -p

TARGET = main
BUILD_DIR = build

GAME_SRCS = main.c game.c draw.c aux_game.c client.c
CONTROL_SRCS = ../Control/game_protocol.c ../Control/protocol.c ../Control/kermit.c ../Control/socket.c
SRCS = $(GAME_SRCS) $(CONTROL_SRCS)
GAME_OBJS = $(patsubst %.c,$(BUILD_DIR)/Game/%.o,$(GAME_SRCS))
CONTROL_OBJS = $(patsubst ../Control/%.c,$(BUILD_DIR)/Control/%.o,$(CONTROL_SRCS))
OBJS = $(GAME_OBJS) $(CONTROL_OBJS)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

$(BUILD_DIR)/Game/%.o: %.c | $(BUILD_DIR)/Game
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Control/%.o: ../Control/%.c | $(BUILD_DIR)/Control
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Game $(BUILD_DIR)/Control:
	$(MKDIR_P) $@

clean:
	$(RMDIR) $(BUILD_DIR)
	$(RM) $(TARGET) *.o ../Control/*.o

rebuild: clean all

-include $(DEPS)

.PHONY: all clean rebuild
