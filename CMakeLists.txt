cmake_minimum_required(VERSION 3.20)

option(KRITIC_ENABLE_TESTING "Enable testing" ON)
project(kritic VERSION 1.0.1 DESCRIPTION "Unit testing library for C")

add_library(kritic STATIC
	src/attributes.c
	src/defaults.c
	src/kritic.c
	src/redirect.c
	src/scheduler.c
	src/timer.c
)

target_sources(kritic PUBLIC FILE_SET public_headers
	       TYPE HEADERS
	       BASE_DIRS include/
	       FILES
	       include/kritic/assert_types.h
	       include/kritic/attributes.h
	       include/kritic/defaults.h
	       include/kritic/kritic.h
	       include/kritic/redirect.h
	       include/kritic/scheduler.h
	       include/kritic/timer.h
)

target_include_directories(kritic PRIVATE include)

# Compile flags

## Warnings
target_compile_options(kritic PRIVATE
	-std=c99 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion
	-Wsign-conversion -Wcast-align -Wpointer-arith -Wformat=2 
	-Wstrict-prototypes -Wundef -Wdouble-promotion
)

## Optimization level
target_compile_options(kritic PRIVATE
    $<$<CONFIG:Debug>:-O0 -g -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O2 -fomit-frame-pointer -march=native>
)

## Sanitizers
target_compile_options(kritic PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# Install
include(GNUInstallDirs)

## pkg-config generation (NOTE: should go strictly after below include)
configure_file(
	kritic.pc.in
	"${CMAKE_CURRENT_BINARY_DIR}/kritic.pc"
	@ONLY
)

## Install the library
install(TARGETS kritic
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	FILE_SET public_headers
)

## Install .pc
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/kritic.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Testing
if (KRITIC_ENABLE_TESTING)
	enable_testing()
	include_directories(include)
	add_subdirectory(tests)
endif()
