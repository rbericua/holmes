CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -Werror -fsanitize=address,undefined -g -MMD -MP
LDFLAGS := -lncurses

SRC_DIR := src
BENCH_DIR := bench
BUILD_DIR := build

TARGET := $(BUILD_DIR)/holmes
BENCH_TARGET := $(BUILD_DIR)/benchmark

SRCS := $(shell find $(SRC_DIR) -type f -name *.c)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))
OBJS_NO_MAIN := $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(OBJS))

BENCH_OBJ := $(BUILD_DIR)/$(BENCH_DIR)/main.o

DEPS := $(patsubst %.o, %.d, $(OBJS) $(BENCH_OBJS))

.PHONY: all
all: makedirs $(TARGET) $(BENCH_TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Linked $@"

$(BENCH_TARGET): $(BENCH_OBJ) $(OBJS_NO_MAIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Linked $@"

$(BUILD_DIR)/%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo "Compiled $@"

-include $(DEPS)

.PHONY: makedirs
makedirs:
	@mkdir -p $(addprefix $(BUILD_DIR)/, $(shell find $(SRC_DIR) -type d))
	@mkdir -p $(BUILD_DIR)/$(BENCH_DIR)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Removed build artifacts"

.PHONY: rebuild
rebuild:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory
