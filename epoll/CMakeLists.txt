cmake_minimum_required(VERSION 3.00)
project(poggers)

include_directories(include)
file(GLOB MAIN
        "include/*.h"
        "src/*.c"    
)

add_executable(main ${MAIN})