include_directories(include/)


add_executable(bench_boottime boottime/main.cpp)
target_link_libraries(bench_boottime wasp)

add_executable(bench_run run/main.cpp)
target_link_libraries(bench_run wasp)

add_executable(bench_process process/main.cpp)
target_link_libraries(bench_process wasp)

add_executable(bench_pthread pthread/main.cpp)
target_link_libraries(bench_pthread wasp pthread)


add_executable(bench_create create/main.cpp)
target_link_libraries(bench_create wasp)

add_executable(bench_create_cache create_cache/main.cpp)
target_link_libraries(bench_create_cache wasp)


add_executable(bench_create_cache_async create_cache_async/main.cpp)
target_link_libraries(bench_create_cache_async wasp)

add_executable(bench_vmrun vmrun/main.cpp)
target_link_libraries(bench_vmrun wasp)


add_executable(bench_image_size image_size/main.cpp)
target_link_libraries(bench_image_size wasp)
