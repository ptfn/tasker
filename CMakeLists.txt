cmake_minimum_required(VERSION 3.22.1)

project(tasker)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "~/.local/bin")
add_executable(tasker src/main.c src/commands.c src/graphics.c src/utils.c src/tasker.h)

target_link_libraries(tasker m ${CURSES_LIBRARIES})
