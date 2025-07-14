# === Tools ===
CC := clang

# === Build Mode ===
MODE ?= release
PLATFORM ?= $(shell uname -o | tr '[:upper:]' '[:lower:]')

# === Install Path ===
PREFIX ?= /usr/local

# === Version ===
VERSION = $(shell git describe --tags --always)

# === Compiler flags ===
DIAG_FLAGS    := -std=c99 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion \
                 -Wsign-conversion -Wcast-align -Wpointer-arith -Wformat=2 \
				 -Wstrict-prototypes -Wundef -Wdouble-promotion
OPT_FLAGS     := -O2 -fomit-frame-pointer -march=native
DEBUG_FLAGS   := -g -fsanitize=address,undefined -fno-omit-frame-pointer -O0

ifeq ($(MODE),debug)
  CFLAGS      := $(DIAG_FLAGS) $(DEBUG_FLAGS)
  LDFLAGS     := -fsanitize=address,undefined
else
  CFLAGS      := $(DIAG_FLAGS) $(OPT_FLAGS)
endif

# === Paths ===
KRITIC_SRC    := src/kritic.c src/redirect.c src/timer.c src/scheduler.c src/attributes.c src/defaults.c
KRITIC_OBJ    := $(patsubst src/%.c, build/%.o, $(KRITIC_SRC))
RELEASE_DIR   := build/release
RELEASE_LIB   := $(RELEASE_DIR)/libkritic.a
RELEASE_HDR   := $(RELEASE_DIR)/kritic.h
RELEASE_TAR   := build/kritic-$(VERSION)-$(PLATFORM).tar.gz
RELEASE_ZIP   := build/kritic-$(VERSION)-windows.zip
EXPECTED_OUTP := docs/expected_output.txt
ACTUAL_OUTP   := actual_output.txt

# === Platform-specific settings ===
ifeq ($(PLATFORM),windows)
  CC := x86_64-w64-mingw32-gcc
endif

ifeq ($(OS),Windows_NT)
	SELFTEST_EXE := build/selftest.exe
else
	LDFLAGS      := -lpthread
	SELFTEST_EXE := build/selftest
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
$(KRITIC_OBJ): | announce_build_mode

# Prints the mode before building starts
announce_build_mode:
	@printf " $(CYAN)$(BOLD)Building$(RESET)  in %s mode...\n" "$(MODE)"

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
	@$(CC) $(CFLAGS) $(LDFLAGS) -I. $^ -o $@
	@printf " $(GREEN)$(BOLD)Built$(RESET)     $@\n"

# Run self-test suite
selftest: $(SELFTEST_EXE)
	@printf " $(CYAN)$(BOLD)Testing$(RESET)   KritiC...\n"
ifeq ($(OS),Windows_NT)
	@UBSAN_OPTIONS=print_stacktrace=1 $(SELFTEST_EXE)
else
	@ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 $(SELFTEST_EXE)
endif

# Bundle release directory
release: clean
	@if [ ! -e "$(RELEASE_DIR)" ]; then \
		mkdir -p $(RELEASE_DIR); \
	fi
	@$(MAKE) install PLATEFORM=$(PLATEFORM) PREFIX=$(RELEASE_DIR) --no-print-directory
ifeq ($(PLATFORM),windows)
	@printf " $(GREEN)$(BOLD)Packing$(RESET)   $(RELEASE_ZIP)\n"
	@if [ ! -e "build" ]; then \
		mkdir build; \
	fi
	@if [ -d "build" ]; then \
		cd build && zip -r $(notdir $(RELEASE_ZIP)) release; \
	fi
	@printf " $(CYAN)$(BOLD)Archive$(RESET)   ready: $(RELEASE_ZIP)\n"
else
	@printf " $(GREEN)$(BOLD)Packing$(RESET)   $(RELEASE_TAR)\n"
	@tar -czf $(RELEASE_TAR) -C $(RELEASE_DIR) .
	@printf " $(CYAN)$(BOLD)Archive$(RESET)   ready: $(RELEASE_TAR)\n"
endif

# Build static library
static: $(KRITIC_OBJ)
	@printf " $(GREEN)$(BOLD)Archiving$(RESET) build/libkritic.a\n"
	@ar rcs build/libkritic.a $(KRITIC_OBJ)

# Build dynamic library
dynamic: $(KRITIC_OBJ)
	@printf " $(GREEN)$(BOLD)Creating$(RESET)  dynamic library build/libkritic.so\n"
	@$(CC) $(LDFLAGS) $(KRITIC_OBJ) -o build/libkritc.so

# Compare test output to expected snapshot
selftest-check:
	@printf " $(CYAN)$(BOLD)Verifying$(RESET) self-test output against $(EXPECTED_OUTP)...\n"
	@$(MAKE) clean --no-print-directory
	@chmod +x tools/sanitize.sh
	@$(MAKE) selftest 2>&1 | ./tools/sanitize.sh > $(ACTUAL_OUTP)
	@diff -u $(EXPECTED_OUTP) $(ACTUAL_OUTP) || \
	(printf "\nOutput mismatch detected.\nUpdate $(EXPECTED_OUTP) if intentional.\n"; exit 1)
	@printf " $(GREEN)$(BOLD)Verified$(RESET)  $(ACTUAL_OUTP) matches expected self-test results.\n"

# Clean artifacts
clean:
	@if [ -d "build" ]; then \
		rm -rf build; \
	fi

ifeq ($(OS),Windows_NT)
install:
	@printf " $(BOLD)$(GREEN)Skipping$(RESET)  install on Windows\n"

uninstall:
	@printf " $(BOLD)$(GREEN)Skipping$(RESET)  uninstall on Windows\n"
else

install: static
	@printf " $(GREEN)$(BOLD)Installing$(RESET) in $(PREFIX)\n"
	@printf " $(GREEN)$(BOLD)Installing$(RESET) kritic.a\n"
	@if [ ! -e "$(PREFIX)/lib" ]; then \
		mkdir -p $(PREFIX)/lib; \
	fi
	@cp build/libkritic.* $(PREFIX)/lib
	@printf " $(GREEN)$(BOLD)Installing$(RESET) kritic.h\n"
	@if [ ! -e "$(PREFIX)/include" ]; then \
		mkdir -p $(PREFIX)/include; \
	fi
	@cp kritic.h $(PREFIX)/include
	@printf " $(GREEN)$(BOLD)Installing$(RESET) kritic.pc\n"
	@if [ ! -e "$(PREFIX)/lib/pkgconfig" ]; then \
		mkdir -p $(PREFIX)/lib/pkgconfig; \
	fi
	@echo "prefix=$(PREFIX)" > $(PREFIX)/lib/pkgconfig/kritic.pc
	@echo "version=$(VERSION)" >> $(PREFIX)/lib/pkgconfig/kritic.pc
	@cat kritic.pc >>  $(PREFIX)/lib/pkgconfig/kritic.pc

uninstall:
	@printf " $(GREEN)$(BOLD)Uninstalling$(RESET) in $(PREFIX)\n"
	@printf " $(GREEN)$(BOLD)Uninstalling$(RESET) kritic.a\n"
	@rm -f $(PREFIX)/lib/libkritic.*
	@printf " $(GREEN)$(BOLD)Uninstalling$(RESET) kritic.h\n"
	@rm -f $(PREFIX)/include/kritic.h
	@printf " $(GREEN)$(BOLD)Uninstalling$(RESET) kritic.pc\n"
	@rm -f $(PREFIX)/lib/pkgconfig/kritic.pc
endif

.PHONY: all clean announce_build_mode selftest release selftest-check install
