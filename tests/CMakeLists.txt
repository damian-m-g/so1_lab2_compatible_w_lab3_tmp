# Lógica para generación de pruebas unitarias
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage --coverage")
# Linking with coverage
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")

file(GLOB TEST_SOURCES "shell_project.test.c")
file(GLOB SHELL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.c")
# "main.c" won't be tested
list(FILTER SHELL_SOURCES EXCLUDE REGEX ".*main.c$")

# Add executable for unit test
enable_testing()
add_executable(shell_tests ${TEST_SOURCES} ${SHELL_SOURCES})

# Link test libraries & others
target_link_libraries(shell_tests PRIVATE unity::unity cjson::cjson)

# Enable testing
add_test(NAME shell_tests COMMAND ${CMAKE_BINARY_DIR}/tests/shell_tests)
