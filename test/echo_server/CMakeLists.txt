cmake_minimum_required(VERSION 3.10)

add_executable(kernel
        main.c
        printf.c)

enable_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CAN_USE_ASSEMBLER TRUE)
    file(GLOB_RECURSE ENGINE_ASM_FILES "*.asm")
    set(ENGINE_SOURCES ${ENGINE_SOURCES} ${ENGINE_ASM_FILES})
endif(CMAKE_ASM_NASM_COMPILER_LOADED)
