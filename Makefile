# === Tools ===
CC := clang

# === Compiler flags ===
DIAG_FLAGS := -std=c99 -Wall -Wextra -Wpedantic -Werror
OPT_FLAGS  := -O2 -fomit-frame-pointer -march=native
CFLAGS     := $(DIAG_FLAGS) $(OPT_FLAGS)
LDFLAGS    := -lc

# === Paths ===
KRITIC_SRC    := src/kritic.c src/redirect.c
KRITIC_OBJ    := $(patsubst src/%.c, build/%.o, $(KRITIC_SRC))
RELEASE_DIR   := build/release
RELEASE_LIB   := $(RELEASE_DIR)/libkritic.a
RELEASE_HDR   := $(RELEASE_DIR)/kritic.h
RELEASE_TAR   := build/kritic-$(shell git describe --tags --always).tar.gz
RELEASE_ZIP   := build/kritic-$(shell git describe --tags --always).zip
ifeq ($(OS),Windows_NT)
	SELFTEST_EXE  := build/selftest.exe
else
	SELFTEST_EXE  := build/selftest
endif

# === ANSI Colors ===
RESET := \033[0m
BOLD  := \033[1m
GREEN := \033[32m
CYAN  := \033[36m

# === Self-test Sources ===
TEST_SRCS := $(wildcard tests/*.c) $(wildcard tests/**/*.c)
TEST_OBJS := $(patsubst tests/%.c, build/tests/%.o, $(TEST_SRCS))

all: $(KRITIC_OBJ)

# Build KritiC itself
build/%.o: src/%.c
	@if [ ! -e "build" ]; then \
		mkdir build; \
	fi
	@printf " $(GREEN)$(BOLD)Compiling$(RESET) $<\n"
	@$(CC) $(CFLAGS) -I. -c $< -o $@

# Compile self-tests
build/tests/%.o: tests/%.c
	@if [ ! -e "$@" ]; then \
		mkdir -p $(dir $@); \
	fi
	@if [ -f "$@" ] && [ "$@" -nt "$<" ]; then \
		printf " $(GREEN)$(BOLD)Skipping$(RESET) %s\n" "$<"; \
	else \
		printf " $(GREEN)$(BOLD)Compiling$(RESET) %s\n" "$<"; \
		$(CC) $(CFLAGS) -I. -c "$<" -o "$@" || exit $$?; \
	fi


# Build self-test executable
$(SELFTEST_EXE): $(KRITIC_OBJ) $(TEST_OBJS)
	@printf " $(GREEN)$(BOLD)Linking$(RESET)   self-test executable\n"
	@$(CC) $(CFLAGS) -I. $^ -o $@
	@printf " $(GREEN)$(BOLD)Built$(RESET)     $@\n"

# Run self-test suite
selftest: $(SELFTEST_EXE)
	@printf " $(CYAN)$(BOLD)Testing$(RESET)   KritiC...\n"
	@$(SELFTEST_EXE)

# Create static library
$(RELEASE_LIB): $(KRITIC_OBJ)
	@printf " $(GREEN)$(BOLD)Archiving$(RESET) libkritic.a\n"
	@if [ ! -e "build" ]; then \
		mkdir build; \
	fi
	@if [ ! -e "$(RELEASE_DIR)" ]; then \
		mkdir $(RELEASE_DIR); \
	fi
	@ar rcs $@ $^

# Copy public headers
$(RELEASE_HDR): kritic.h
	@printf " $(GREEN)$(BOLD)Copying$(RESET)   kritic.h\n"
	@if [ -d "$(RELEASE_DIR)" ]; then \
		cp "$<" "$@"; \
	fi

# Bundle release directory
release: $(RELEASE_LIB) $(RELEASE_HDR)
	@printf " $(GREEN)$(BOLD)Packing$(RESET)   $(RELEASE_TAR)\n"
	@tar -czf $(RELEASE_TAR) -C $(RELEASE_DIR) .
	@printf " $(CYAN)$(BOLD)Archive$(RESET)   ready: $(RELEASE_TAR)\n"


# Clean artifacts
clean:
	@if [ -d "build" ]; then \
		rm -rf build; \
	fi

.PHONY: all clean selftest release 
