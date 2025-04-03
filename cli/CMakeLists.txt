cmake_minimum_required(VERSION 3.12)
project(homevault-cli)

add_executable(homevault-cli
        main.cpp
        CLISetup.cpp
        ../core/DirInfo.cpp
        ../core/ServerConnection.cpp
        ../include/hv/core/DirInfo.hpp
)

target_include_directories(homevault-cli PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(homevault-cli PRIVATE
        homevault-core
        CLI11::CLI11
)

target_compile_options(homevault-cli PRIVATE
        -Wall
        -Wextra
        -Werror
)

install(
        TARGETS homevault-cli
        RUNTIME DESTINATION bin
)
