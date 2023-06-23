cmake_minimum_required(VERSION 3.00)
project(Web_Server_C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -Wall -Wextra -pedantic -O2 -g3")

include_directories(include)
file(GLOB MAIN
        "include/*.h"
        "src/*.c"    
)

add_executable(main ${MAIN})

