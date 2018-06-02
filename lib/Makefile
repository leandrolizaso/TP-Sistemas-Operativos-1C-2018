CC=gcc

BUILD_DIR=_build
SOURCES=sockets.c protocolo.c
HEADERS=sockets.h protocolo.h
OBJS=$(SOURCES:%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/libpelao.so

$(BUILD_DIR)/libpelao.so: create-dirs $(OBJS)
	$(CC) -shared -o $(BUILD_DIR)/libpelao.so $(OBJS)

$(BUILD_DIR)/%.o: %.c
	$(CC) -c -fPIC -o "$@" "$<"

create-dirs:
	mkdir --parents $(BUILD_DIR)

debug: CC += -DDEBUG -g
debug: all

clean:
	rm -rf $(BUILD_DIR)

install: all
	cp -vu $(BUILD_DIR)/libpelao.so /usr/lib
	mkdir --parents /usr/include/pelao
	cp -vu $(HEADERS) /usr/include/pelao

uninstall:
	rm -v -f /usr/lib/libpelao.so
	rm -v -rf /usr/include/pelao

.PHONY: all create-dirs clean debug install uninstall
