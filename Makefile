CC := gcc
SDL3_INC := -Ilib/SDL3/include
SDL3_LIB := -Llib/SDL3/lib
CFLAGS := -std=c11 -Wall -Wextra -pedantic $(SDL3_INC)
LDFLAGS := -lm -lSDL3 $(SDL3_LIB)
REL_FLAGS := -O2 -D DISABLE_DEBUG -D DISABLE_CPU_LOG
DBG_FLAGS := -ggdb -Og -D DISABLE_CPU_LOG
# For profiling
#DBG_FLAGS := -ggdb -Og -D DISABLE_DEBUG -D DISABLE_CPU_LOG
# For memory checks
#DBG_FLAGS := -ggdb -O2 -fsanitize=address -D DISABLE_DEBUG -D DISABLE_CPU_LOG

ifeq ($(POSIX),1)
	OS := $(shell uname -s | tr '[:upper:]' '[:lower:]')
	ARCH := $(shell uname -m)
	BIN := nones
	VERSION := 0.2.0
	TARBALL_NAME := $(BIN)-$(VERSION)-$(OS)-$(ARCH).tar.gz
	# POSIX compatible version of $(wildcard)
	SRCS := $(shell echo src/*.c)
else
	BIN := nones.exe
	SRCS := $(wildcard src/*.c)
endif
OBJS := $(SRCS:src/%.c=%.o)

BUILD_DIR := build
REL_DIR := $(BUILD_DIR)/release
DBG_DIR := $(BUILD_DIR)/debug

DBG_OBJS := $(addprefix $(DBG_DIR)/, $(OBJS))
REL_OBJS := $(addprefix $(REL_DIR)/, $(OBJS))

REL_BIN := $(REL_DIR)/$(BIN)
DBG_BIN := $(DBG_DIR)/$(BIN)



.PHONY: all clean release debug run tarball dll release-dll release-exe release-posix release-all


all: release



release: $(REL_BIN) build/release/nones.dll
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll

dll: build/release/nones.dll


$(REL_BIN): $(REL_OBJS)
	$(CC) $(REL_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(REL_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(REL_DIR)) mkdir $(subst /,\\,$(REL_DIR))
	$(CC) $(REL_FLAGS) $(CFLAGS) -c -o $@ $<

build/release/nones.dll: $(REL_OBJS)
	$(CC) -shared -o $@ $^ -lSDL3 -Llib/SDL3/lib
	copy /Y $(subst /,\\,$@) nones.dll

debug: $(DBG_BIN)
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll


$(DBG_BIN): $(DBG_OBJS)
	$(CC) $(DBG_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(DBG_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(DBG_DIR)) mkdir $(subst /,\\,$(DBG_DIR))
	$(CC) $(DBG_FLAGS) $(CFLAGS) -c -o $@ $<


run:
	$(BIN)



ifeq ($(POSIX),1)
	@if [ -d "$(BUILD_DIR)" ]; then rm -r $(BUILD_DIR); else echo 'Nothing to clean up'; fi
	@if [ -f "$(BIN)" ]; then rm $(BIN); fi
	@if [ -f "$(TARBALL_NAME)" ]; then rm $(TARBALL_NAME); fi
else
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	@if exist $(BIN) del /Q $(BIN)
	@REM Tarball is not created by default on Windows
endif

tarball:
	@if [ -f "$(BIN)" ]; then \
		strip $(BIN); \
		tar -czf $(TARBALL_NAME) $(BIN) "LICENSE" "README.md"; \
		echo "Created tarball $(TARBALL_NAME)..."; \
	else \
		echo "Please run 'make' before creating a tarball."; \
	fi

# Release variants
release-dll: build/release/nones.dll
	copy /Y $(subst /,\\,$<) nones.dll

release-exe: $(REL_BIN)
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll

release-posix:
	$(MAKE) POSIX=1 release

release-all: release-dll release-exe
	@echo "All release targets built successfully"
