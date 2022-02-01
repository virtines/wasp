include_directories(include/)


add_executable(fib fib/main.cpp)
target_link_libraries(fib wasp)


add_executable(js js/host.cpp)
target_link_libraries(js wasp)


add_executable(boottime boottime/main.cpp)
target_link_libraries(boottime wasp)

add_executable(run run/main.cpp)
target_link_libraries(run wasp)

add_executable(process_overhead process/main.cpp)
target_link_libraries(process_overhead wasp)

add_executable(pthread_overhead pthread/main.cpp)
target_link_libraries(pthread_overhead wasp pthread)


add_executable(create create/main.cpp)
target_link_libraries(create wasp)

add_executable(create_cache create_cache/main.cpp)
target_link_libraries(create_cache wasp)


add_executable(create_cache_async create_cache_async/main.cpp)
target_link_libraries(create_cache_async wasp)

add_executable(vmrun vmrun/main.cpp)
target_link_libraries(vmrun wasp)


add_executable(image_size image_size/main.cpp)
target_link_libraries(image_size wasp)

add_executable(echo_server echo_server/host.cpp)
target_link_libraries(echo_server wasp curl)

