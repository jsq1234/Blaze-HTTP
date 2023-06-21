cmake_minimum_required(VERSION 3.00)
project(Web_Server_C)

add_compile_options(-mtune=native -O2)

include_directories(include)
file(GLOB MAIN
        "include/*.h"
        "src/*.c"    
)

add_executable(main ${MAIN})

