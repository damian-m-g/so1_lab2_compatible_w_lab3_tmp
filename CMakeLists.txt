# Verify cmake version
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(
  "ShellProject"
  VERSION 1.0.0
  DESCRIPTION "A nice shell project!"
  LANGUAGES C
)

# Enable testing
include(CTest)

# Flags for compiling
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -Wpedantic -Werror -Wunused-parameter -Wmissing-prototypes -Wstrict-prototypes")

# Include headers
include_directories(include)

# include source files
file(GLOB SOURCES "src/*.c")

# Find dependencies
find_package(cJSON REQUIRED)
find_package(unity REQUIRED)

add_subdirectory(submodules/metrics)

# add_executable => .exe/library
# add_library => .a/.so/.dll STATIC SHARED
add_executable(${PROJECT_NAME} ${SOURCES} src/main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE cjson::cjson unity::unity)

if(RUN_COVERAGE EQUAL 1)
  add_subdirectory(tests)
endif()
