cmake_minimum_required(VERSION 3.12)
project(homevault-core)

add_library(homevault-core STATIC
    Homevault.cpp
    ApiClient.cpp
    FilesystemCrawler.cpp
    Models.cpp
)

target_include_directories(homevault-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/hv>
        $<INSTALL_INTERFACE:include>

    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(homevault-core PUBLIC 
    ${CURLPP_LDFLAGS}
    nlohmann_json::nlohmann_json
)

target_compile_options(homevault-core PRIVATE 
    -Wall 
    -Wextra
    -Werror
)

# Install public headers
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include
    DESTINATION include
)

# Install the library
install(TARGETS homevault-core
    EXPORT homevault-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Create and install CMake config files
install(
    EXPORT homevault-targets
    FILE homevault-config.cmake
    NAMESPACE homevault::
    DESTINATION lib/cmake/homevault
)
