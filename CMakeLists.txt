cmake_minimum_required(VERSION 3.12)
project(GardenOfEris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -Og -pthread")

# Set the source files for the game elements using wildcard
file(GLOB SOURCE_FILES src/*.cpp)

# Build a library from the src files
add_library(GardenOfErisLib OBJECT ${SOURCE_FILES})

# Set the source file for the main game executable
set(MAIN_FILE main.cpp)

# Add the executable target for the game
add_executable(GardenOfEris ${MAIN_FILE} $<TARGET_OBJECTS:GardenOfErisLib>)

# Include directory for header files
target_include_directories(GardenOfEris PRIVATE include)

# Specify the required libraries
target_link_libraries(GardenOfEris
    stdc++
    allegro
    allegro_image
    allegro_font
    allegro_ttf
    m
    openal
    sndfile
    pthread
)

# Set the source files for the unit tests using wildcard
file(GLOB TEST_SOURCE_FILES unitTests/*.cpp)

# Include directory for header files in tests
include_directories(${Boost_INCLUDE_DIRS})

# Find Boost Test Framework
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# Include the src directory
include_directories(include)

# Build a library from the src files
#add_library(GardenOfErisLib OBJECT ${SOURCE_FILES})

# Loop over each test source file and create an executable
foreach(TEST_SOURCE_FILE ${TEST_SOURCE_FILES})
    # Get the name of the test from the source file
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)

    # Add the executable target for each test
    add_executable(${TEST_NAME} ${TEST_SOURCE_FILE} $<TARGET_OBJECTS:GardenOfErisLib>)

    # Include directory for header files
    target_include_directories(${TEST_NAME} PRIVATE include)

    # Specify the required libraries
    target_link_libraries(${TEST_NAME}
        stdc++
        allegro
        allegro_image
        allegro_font
        allegro_ttf
        m
        openal
        sndfile
        pthread
        ${Boost_LIBRARIES}
    )
endforeach(TEST_SOURCE_FILE)
