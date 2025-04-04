cmake_minimum_required(VERSION 3.12)
project(homevault-core)

add_library(homevault-core STATIC
    HomeVaultClient.cpp
    FileInfo.cpp
    webdav/WebDAVClient.cpp
    webdav/WebDAVClient.hpp
    webdav/WebDAVParser.cpp
    webdav/WebDAVParser.hpp
    webdav/WebDAVResponse.cpp
    webdav/WebDAVResponse.hpp
    webdav/WebDAVResource.cpp
    webdav/WebDAVResource.hpp
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
    pugixml
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
