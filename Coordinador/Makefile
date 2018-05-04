CC=gcc

BUILD_DIR=.build

SOURCES=$(shell find . -iname "*.c" | tr '\n' ' ')
OBJS=$(SOURCES:./src/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/Coordinador

$(BUILD_DIR)/Coordinador: create-dirs $(OBJS)
	$(CC)  -o $(BUILD_DIR)/Coordinador $(OBJS) -lpelao -lcommons

$(BUILD_DIR)/%.o: src/%.c
	$(CC) -c -o "$@" "$<"

create-dirs:
	mkdir --parents $(BUILD_DIR)

debug: CC += -DDEBUG -g
debug: all

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all create-dirs clean debug
