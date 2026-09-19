# Common source is linked into each TRINETRA executable.
# This project is not a runtime process.
PLATFORM ?= aarch64le
CC = qcc -Vgcc_nto$(PLATFORM)
CFLAGS = -Wall -Wextra -Iincludes
BUILD_DIR = build/$(PLATFORM)-debug
all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/trinetra_ipc.c -o $(BUILD_DIR)/trinetra_ipc.o
clean:
	rm -rf build
.PHONY: all clean
