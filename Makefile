# === Tools ===
CC := clang

# === Compiler flags ===
DIAG_FLAGS := -Wall -Wextra -Wpedantic -Werror
OPT_FLAGS  := -O2 -fomit-frame-pointer -march=native
CFLAGS     := $(DIAG_FLAGS) $(OPT_FLAGS)
LDFLAGS    := -lc

# === Paths ===
KRITIC        := src/kritic.c
KRITIC_OBJ    := build/kritic.o
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
$(KRITIC_OBJ): $(KRITIC)
	@if [ ! -e "build" ]; then \
		mkdir build; \
	fi
	@printf " $(GREEN)$(BOLD)Compiling$(RESET) $<\n"
	@$(CC) $(CFLAGS) -c $< -o $@

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

clean:
	@if [ -d "build" ]; then \
		rm -rf build; \
	fi

.PHONY: all clean selftest
