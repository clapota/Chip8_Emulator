CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)

FILE(
        GLOB_RECURSE
        SRC
        src/*.cpp
)

add_executable(emu ${SRC})

target_link_libraries(emu sfml-window sfml-graphics sfml-system)