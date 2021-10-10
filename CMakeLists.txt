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


install(TARGETS wasp LIBRARY DESTINATION lib)

# add_executable(ex_minimal example/minimal.cpp)
# target_link_libraries(ex_minimal wasp)

# add_executable(ex_minimal_cache example/minimal_cache.cpp)
# target_link_libraries(ex_minimal_cache wasp)

# add_executable(ex_vmrun example/vmrun.cpp)
# target_link_libraries(ex_vmrun wasp)

# add_executable(ex_cache example/cache.cpp)
# target_link_libraries(ex_cache wasp)


# add_executable(ex_js example/js/host.cpp)
# target_link_libraries(ex_js wasp)



# add_executable(ex_fib example/fib/host.cpp)
# target_link_libraries(ex_fib wasp)

# add_executable(ex_imagesize example/imagesize.cpp)
# target_link_libraries(ex_imagesize wasp)

# add_subdirectory(example/pl)


add_subdirectory(bench/)


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