cmake_minimum_required(VERSION 3.12)
project(homevault-client VERSION 0.0.1)

include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CLI "Build homevault-cli" ON)

find_package(CLI11 CONFIG REQUIRED)
pkg_check_modules(CURLPP REQUIRED curlpp)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

else()
    message(STATUS "Doxygen not found, documentation will not be built")
endif()

add_subdirectory(core)
if (BUILD_CLI)
    add_subdirectory(cli)
endif()

install(TARGETS homevault-cli homevault-core
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
