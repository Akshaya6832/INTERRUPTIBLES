ARTIFACT = TRINETRA_WARNING_GOVERNOR
PLATFORM ?= aarch64le
BUILD_PROFILE ?= debug
CONFIG_NAME = $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)
CC = qcc -Vgcc_nto$(PLATFORM)
INCLUDES = -Iincludes -I../TRINETRA_COMMON/includes
CFLAGS = -Wall -Wextra -fmessage-length=0 $(INCLUDES)
LIBS = -lm
ifeq ($(BUILD_PROFILE),debug)
CFLAGS += -g -O0
endif
ifeq ($(BUILD_PROFILE),release)
CFLAGS += -O2
endif

SRC = src/warning_governor.c \
	src/severity.c \
	src/confidence.c \
	src/hysteresis.c \
	src/warning_state.c \
	../TRINETRA_COMMON/src/trinetra_ipc.c
OBJ = $(OUTPUT_DIR)/warning_governor.o \
	$(OUTPUT_DIR)/severity.o \
	$(OUTPUT_DIR)/confidence.o \
	$(OUTPUT_DIR)/hysteresis.o \
	$(OUTPUT_DIR)/warning_state.o \
	$(OUTPUT_DIR)/trinetra_ipc.o
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)

all: $(TARGET)

$(OUTPUT_DIR):
	mkdir -p $@

$(OUTPUT_DIR)/%.o: src/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/trinetra_ipc.o: ../TRINETRA_COMMON/src/trinetra_ipc.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

clean:
	rm -rf build
rebuild: clean all

print-config:
	@echo "PROJECT=$(ARTIFACT)"
	@echo "PLATFORM=$(PLATFORM)"
	@echo "BUILD_PROFILE=$(BUILD_PROFILE)"
	@echo "TARGET=$(TARGET)"

.PHONY: all clean rebuild print-config
