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


BIN := nones.exe
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=%.o)

BUILD_DIR := build
REL_DIR := $(BUILD_DIR)/release
DBG_DIR := $(BUILD_DIR)/debug

DBG_OBJS := $(addprefix $(DBG_DIR)/, $(OBJS))
REL_OBJS := $(addprefix $(REL_DIR)/, $(OBJS))

REL_BIN := $(REL_DIR)/$(BIN)
DBG_BIN := $(DBG_DIR)/$(BIN)



.PHONY: all clean release debug run tarball dll


all: release



release: $(REL_BIN) build/release/nones.dll
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\lib\SDL3.dll SDL3.dll

dll: build/release/nones.dll


$(REL_BIN): $(REL_OBJS)
	$(CC) $(REL_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(REL_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(REL_DIR)) mkdir $(subst /,\\,$(REL_DIR))
	$(CC) $(REL_FLAGS) $(CFLAGS) -c -o $@ $<

build/release/nones.dll: $(REL_OBJS)
	$(CC) -shared -o $@ $^ -lSDL3 -Llib/SDL3/lib

debug: $(DBG_BIN)
	copy /Y $(subst /,\\,$<) $(BIN)
	copy /Y lib\SDL3\lib\SDL3.dll SDL3.dll


$(DBG_BIN): $(DBG_OBJS)
	$(CC) $(DBG_FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(DBG_DIR)/%.o: src/%.c
	@if not exist $(subst /,\\,$(DBG_DIR)) mkdir $(subst /,\\,$(DBG_DIR))
	$(CC) $(DBG_FLAGS) $(CFLAGS) -c -o $@ $<


run:
	$(BIN)


clean:
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	@if exist $(BIN) del /Q $(BIN)
	@if [ -f "$(TARBALL_NAME)" ]; then rm $(TARBALL_NAME); fi

tarball:
	@if [ ! -f "$(BIN)" ]; then \
		echo "Please run make before creating a tarball."; \
	else \
		echo "Creating tarball $(TARBALL_NAME)..."; \
		strip $(BIN); \
		tar -czf $(TARBALL_NAME) $(BIN); \
	fi
