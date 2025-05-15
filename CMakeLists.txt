cmake_minimum_required(VERSION 3.10)
project(poker)

set(CMAKE_CXX_STANDARD 17)

# Add SFML
set(SFML_DIR "/opt/homebrew/lib/cmake/SFML")
find_package(SFML 2 COMPONENTS graphics window system REQUIRED)


# Add source files
set(SOURCES
    src/main.cpp
    src/card.cpp
    src/comparer.cpp
    src/deck.cpp
    src/hand.cpp
    src/playerAI.cpp
)

# Add the executable
add_executable(poker ${SOURCES})

# Link libraries
target_link_libraries(poker sfml-graphics sfml-window sfml-system)