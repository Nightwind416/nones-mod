CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic
LDFLAGS := -lm -lSDL3
REL_FLAGS := -O3 -flto=auto -D DISABLE_DEBUG -D DISABLE_CPU_LOG
DBG_FLAGS := -ggdb -Og -D DISABLE_CPU_LOG
# For profiling
#DBG_FLAGS := -ggdb -Og -D DISABLE_DEBUG -D DISABLE_CPU_LOG
# For memory checks
#DBG_FLAGS := -ggdb -O2 -fsanitize=address -D DISABLE_DEBUG -D DISABLE_CPU_LOG


ifeq ($(OS), Windows_NT)
	OS_NAME := windows
	ARCHIVE_FMT := .zip
	COPY_CMD := copy /Y
	BIN_EXT := .exe
else
	COPY_CMD := cp
	BIN_EXT :=
endif

OS_NAME ?= $(shell uname -s | tr '[:upper:]' '[:lower:]')
ARCH := $(if $(filter windows,$(OS_NAME)),x64,$(shell uname -m))

BIN := nones
VERSION := 0.3.0-mod

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=%.o)

BUILD_DIR := build
REL_DIR := $(BUILD_DIR)/release
DBG_DIR := $(BUILD_DIR)/debug

DBG_OBJS := $(addprefix $(DBG_DIR)/, $(OBJS))
REL_OBJS := $(addprefix $(REL_DIR)/, $(OBJS))

REL_BIN := $(REL_DIR)/$(BIN)
DBG_BIN := $(DBG_DIR)/$(BIN)

# Compiler and flags
.PHONY: all clean build-dll build-exe build-all debug run

# Default target: clean, build DLL, build executable
all: clean build-all

release: $(REL_BIN) build/release/nones.dll
ifeq ($(OS_NAME), windows)
	copy /Y $(subst /,\\,$<).exe $(BIN).exe
else
	copy /Y $(subst /,\\,$<) $(BIN)
endif
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll

$(REL_BIN): $(REL_OBJS)
	$(CC) $(REL_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(REL_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(REL_DIR)) mkdir $(subst /,\\,$(REL_DIR))
	$(CC) $(REL_FLAGS) $(CFLAGS) -c -o $@ $<

build/release/nones.dll: $(REL_OBJS)
	$(CC) -shared -o $@ $^ -lSDL3 -Llib/SDL3/lib
	$(COPY_CMD) $(subst /,\\,$@) nones.dll

debug: $(DBG_BIN)
ifeq ($(OS_NAME), windows)
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll
endif
	@cp $< $(BIN)

$(DBG_BIN): $(DBG_OBJS)
	$(CC) $(DBG_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(DBG_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(DBG_DIR)) mkdir $(subst /,\\,$(DBG_DIR))
	$(CC) $(DBG_FLAGS) $(CFLAGS) -c -o $@ $<

# command: make run ROM=path/to/rom.nes
run:
ifndef ROM
	@echo Usage: make run ROM=path/to/rom.nes
	@exit 1
else
	$(BIN) "$(ROM)"
endif

clean:
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	if exist $(BIN).exe del /Q $(BIN).exe
	if exist SDL3.dll del /Q SDL3.dll
	if exist nones.dll del /Q nones.dll

# build variants
build-dll: build/release/nones.dll
	copy /Y $(subst /,\\,$<) nones.dll

build-exe: $(REL_BIN)
	copy /Y $(subst /,\\,$<).exe $(BIN).exe
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll

build-all: build-dll build-exe
	@echo "All release targets built successfully"
