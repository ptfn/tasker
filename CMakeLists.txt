cmake_minimum_required(VERSION 3.22.1)

project(tasker)

find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "~/.local/bin")
add_executable(tasker src/main.c)

target_link_libraries(tasker ${CURSES_LIBRARIES})
target_link_libraries(tasker ${SQLite3_LIBRARIES})
