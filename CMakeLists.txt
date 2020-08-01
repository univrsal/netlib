cmake_minimum_required(VERSION 3.7)
project(netlib VERSION 1.0.1 LANGUAGES C)

option(NETLIB_ENABLE_TESTS "Compile test binaries (default: ON)" ON)
option(NETLIB_ENABLE_STATIC "Compile netlib_static library (default: OFF)" OFF)
option(NETLIB_ENABLE_SHARED "Compile netlib_shared library (default: ON)" ON)
option(NETLIB_ENABLE_INSTALL "Register files for install target (default: ON)" ON)

if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(netlib_PLATFORM_DEPS
            ws2_32
            iphlpapi)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(netlib_PLATFORM_DEPS
            wsock32
            iphlpapi)
    endif()
endif()

if(UNIX)
endif()
add_library(netlib OBJECT
    ./src/netlib.c
    ./src/netlib_tcp.c
    ./src/netlib_udp.c
    ./src/netlib_select.c
    ./src/platform.h
)

set_property(TARGET netlib PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(netlib
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (NETLIB_ENABLE_INSTALL)
    include(GNUInstallDirs)
endif()

if (NETLIB_ENABLE_SHARED)
    add_library(netlib_shared SHARED $<TARGET_OBJECTS:netlib>)
    set_target_properties(netlib_shared PROPERTIES OUTPUT_NAME netlib)
    target_link_libraries(netlib_shared ${netlib_PLATFORM_DEPS})
    if (NETLIB_ENABLE_INSTALL)
        install(TARGETS netlib_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if (NETLIB_ENABLE_STATIC)
    add_library(netlib_static STATIC $<TARGET_OBJECTS:netlib>)
    set_target_properties(netlib_static PROPERTIES OUTPUT_NAME netlib)
    target_link_libraries(netlib_static ${netlib_PLATFORM_DEPS})
    if (NETLIB_ENABLE_INSTALL)
        install(TARGETS netlib_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if (NETLIB_ENABLE_SHARED OR NETLIB_ENABLE_STATIC)
    if (NETLIB_ENABLE_INSTALL)
        install(FILES ./include/netlib.h
            DESTINATION include)
    endif()
endif()


target_link_libraries(netlib
    ${netlib_PLATFORM_DEPS})

# Tests

if (NETLIB_ENABLE_TESTS)
    add_executable(netlib_client "tests/client.c")
    add_executable(netlib_server "tests/server.c")

    if (NETLIB_ENABLE_STATIC)
        add_dependencies(netlib_client netlib_static)
        target_link_libraries(netlib_client netlib_static)
        add_dependencies(netlib_server netlib_static)
        target_link_libraries(netlib_server netlib_static)
    elseif (NETLIB_ENABLE_SHARED)
        add_dependencies(netlib_client netlib_shared)
        target_link_libraries(netlib_client netlib_shared)
        add_dependencies(netlib_server netlib_shared)
        target_link_libraries(netlib_server netlib_shared)
    else()
        add_dependencies(netlib_client netlib)
        target_link_libraries(netlib_client netlib)
        add_dependencies(netlib_server netlib)
        target_link_libraries(netlib_server netlib)
    endif()

    if (NETLIB_ENABLE_INSTALL)
    install(TARGETS netlib_client netlib_server DESTINATION bin)
    endif()
endif()


if (UNIX)
    configure_file("./pc/netlib.pc.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" @ONLY)
    if (NETLIB_ENABLE_INSTALL)
        install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
endif()

include_directories(./include)

set_target_properties(netlib PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})
