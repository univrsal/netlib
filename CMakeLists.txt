cmake_minimum_required(VERSION 3.7)
project(netlib VERSION 1.0.1 LANGUAGES C)

option(ENABLE_TESTS "Compile test binaries (default: ON)" ON)
option(ENABLE_STATIC "Compile static library (default: OFF)" OFF)
option(ENABLE_SHARED "Compile shared library (default: ON)" ON)

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

include(GNUInstallDirs)

if (ENABLE_SHARED)
    add_library(shared SHARED $<TARGET_OBJECTS:netlib>)
    set_target_properties(shared PROPERTIES OUTPUT_NAME netlib)
    target_link_libraries(shared ${netlib_PLATFORM_DEPS})
    install(TARGETS shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if (ENABLE_STATIC)
    add_library(static STATIC $<TARGET_OBJECTS:netlib>)
    set_target_properties(static PROPERTIES OUTPUT_NAME netlib)
    target_link_libraries(shared ${netlib_PLATFORM_DEPS})
    install(TARGETS static DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if (ENABLE_SHARED OR ENABLE_STATIC)
    install(FILES ./include/netlib.h
        DESTINATION include)
endif()


target_link_libraries(netlib
    ${netlib_PLATFORM_DEPS})

# Tests

if (ENABLE_TESTS)
    add_executable(netlib_client "tests/client.c")
    add_executable(netlib_server "tests/server.c")

    if (ENABLE_STATIC)
        add_dependencies(netlib_client static)
        target_link_libraries(netlib_client static)
        add_dependencies(netlib_server static)
        target_link_libraries(netlib_server static)
    elseif (ENABLE_SHARED)
        add_dependencies(netlib_client shared)
        target_link_libraries(netlib_client shared)
        add_dependencies(netlib_server shared)
        target_link_libraries(netlib_server shared)
    else()
        add_dependencies(netlib_client netlib)
        target_link_libraries(netlib_client netlib)
        add_dependencies(netlib_server netlib)
        target_link_libraries(netlib_server netlib)
    endif()

    install(TARGETS netlib_client netlib_server DESTINATION bin)
endif()


if (UNIX)
    configure_file("./pc/netlib.pc.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

include_directories(./include)

set_target_properties(netlib PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})
