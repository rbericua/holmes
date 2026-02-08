CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -Werror -fsanitize=address,undefined -g
LDFLAGS := -lncurses

SRC_DIR := src
BUILD_DIR := build

TARGET := $(BUILD_DIR)/holmes

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all
all: makedirs $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Linked $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo "Compiled $@"

.PHONY: makedirs
makedirs:
	@mkdir -p $(subst $(SRC_DIR), $(BUILD_DIR), $(shell find $(SRC_DIR) -type d))

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

.PHONY: rebuild
rebuild:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory all
