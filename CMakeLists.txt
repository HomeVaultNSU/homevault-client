cmake_minimum_required(VERSION 3.12)
project(homevault VERSION 1.0.0)

include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CLI "Build homevault-cli" ON)

find_package(CLI11 CONFIG REQUIRED)
pkg_check_modules(CURLPP REQUIRED curlpp)

add_subdirectory(core)
if (BUILD_CLI)
    add_subdirectory(cli)
endif()

install(TARGETS homevault-cli homevault-core
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
