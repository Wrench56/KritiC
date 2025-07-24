enable_testing()

set(EXPECTED_OUTPUT ${CMAKE_SOURCE_DIR}/docs/expected_output.txt)
set(ACTUAL_OUTP actual_out)

macro(add_kritic_test name file)
	add_executable(${name} ${file})
	target_link_libraries(${name} kritic)
	add_test(NAME ${name}
		COMMAND bash -c "$<TARGET_FILE:${name}> 2>&1 | ${CMAKE_SOURCE_DIR}/tools/sanitize.sh > ${ACTUAL_OUTP} && diff -u ${EXPECTED_OUTPUT} ${ACTUAL_OUTP}"
	)
endmacro(add_kritic_test)

add_kritic_test(test_assert assertions.c)
add_kritic_test(test_attr attributes.c)
add_kritic_test(test_core core.c)
add_kritic_test(indirect indirect.c)
add_kritic_test(io io.c)
