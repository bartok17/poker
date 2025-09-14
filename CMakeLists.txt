cmake_minimum_required(VERSION 3.21) # for TARGET_RUNTIME_DLLS on Windows
project(poker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to use system SFML first. If not found, auto-fetch SFML.
option(USE_SYSTEM_SFML "Use SFML from the system with find_package. If OFF or not found, fetch SFML." ON)

if(USE_SYSTEM_SFML)
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
endif()

if(NOT SFML_FOUND)
    message(STATUS "SFML not found on system. Fetching SFML 2.6.1...")
    include(FetchContent)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW TRUE
    )
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SFML)
endif()

# Sources
set(SOURCES
    src/main.cpp
    src/card.cpp
    src/comparer.cpp
    src/deck.cpp
    src/hand.cpp
    src/playerAI.cpp
    src/ui.cpp
)

add_executable(poker ${SOURCES})

target_link_libraries(poker PRIVATE sfml-graphics sfml-window sfml-system)

# Help runtime linker find SFML from build tree
if(APPLE)
    set_target_properties(poker PROPERTIES BUILD_RPATH "@loader_path")
elseif(UNIX AND NOT APPLE)
    set_target_properties(poker PROPERTIES BUILD_RPATH "$ORIGIN")
endif()

# On Windows, copy dependent DLLs next to the executable after build
if(WIN32)
    add_custom_command(TARGET poker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:poker> $<TARGET_FILE_DIR:poker>
        COMMAND_EXPAND_LISTS
    )
endif()