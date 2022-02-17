cmake_minimum_required(VERSION 3.13)

project(WaspHypervisor C CXX)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")



include_directories(./include/)

add_library(wasp SHARED
	src/Virtine.cpp
	src/Cache.cpp
	src/binding.cpp
	)

add_library(wasp_js_x SHARED
	src/Virtine.cpp
	src/Cache.cpp
	test/js/wasp_js_x.cpp
	)


install(TARGETS wasp LIBRARY DESTINATION lib)
install(
		DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*")




add_subdirectory(test/)


find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)


add_llvm_library(VirtinePass MODULE
	pass/pass.cpp
	pass/VirtineCompiler.cpp
	pass/command.cpp
	PLUGIN_TOOL
	opt
)
