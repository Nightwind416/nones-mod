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
VERSION := 0.2.0
ARCHIVE_FMT ?= .tar.gz
ARCHIVE := $(BIN)-$(VERSION)-$(OS_NAME)-$(ARCH)$(ARCHIVE_FMT)

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=%.o)

BUILD_DIR := build
REL_DIR := $(BUILD_DIR)/release
DBG_DIR := $(BUILD_DIR)/debug

DBG_OBJS := $(addprefix $(DBG_DIR)/, $(OBJS))
REL_OBJS := $(addprefix $(REL_DIR)/, $(OBJS))

REL_BIN := $(REL_DIR)/$(BIN)
DBG_BIN := $(DBG_DIR)/$(BIN)


.PHONY: all clean release debug run tarball win_zip

all: release

release: $(REL_BIN) build/release/nones.dll
ifeq ($(OS_NAME), windows)
	copy /Y $(subst /,\\,$<).exe $(BIN).exe
else
	copy /Y $(subst /,\\,$<) $(BIN)
endif
	copy /Y lib\SDL3\bin\SDL3.dll SDL3.dll

dll: build/release/nones.dll

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

run:
	$(BIN)

ifeq ($(POSIX),1)
	@if [ -d "$(BUILD_DIR)" ]; then rm -r $(BUILD_DIR); else echo 'Nothing to clean up'; fi
	@if [ -f "$(BIN)" ]; then rm $(BIN); fi
	@if [ -f "$(ARCHIVE)" ]; then rm $(ARCHIVE); fi
	@if [ -f "SDL3.dll" ]; then rm "SDL3.dll"; fi

tarball:
	@if [ -f "$(BIN)" ]; then \
		strip $(BIN); \
		tar -czf $(ARCHIVE) $(BIN) "LICENSE" "README.md"; \
		echo "Created tarball $(ARCHIVE)..."; \
	else \
		echo "Please run 'make' before creating a tarball."; \
	fi
endif

# Release variants
release-dll: build/release/nones.dll
	copy /Y $(subst /,\\,$<) nones.dll

release-exe: $(REL_BIN)
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\lib\SDL3.dll SDL3.dll

release-all: release-dll release-exe
	@echo "All release targets built successfully"

win_zip:
	@if [ -f "$(BIN)" ]; then \
		strip $(BIN).exe; \
		7z a $(ARCHIVE) $(BIN).exe "SDL3.dll" "LICENSE" "README.md"; \
		echo "Created zip $(ARCHIVE)..."; \
	else \
		echo "Please run 'make' before creating a zip."; \
	fi
