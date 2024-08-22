cmake_minimum_required(VERSION 3.12)
project(GardenOfEris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the compiler flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -pthread")

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /MTd /RTC1 /wd4514")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -pthread")
endif()

# Set the source files for the game elements using wildcard
file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Build a library from the src files
add_library(GardenOfErisLib OBJECT ${SOURCE_FILES})

# Set the source file for the main game executable
set(MAIN_FILE ${CMAKE_SOURCE_DIR}/main.cpp)

# Add the executable target for the game
add_executable(GardenOfEris ${MAIN_FILE} $<TARGET_OBJECTS:GardenOfErisLib>)

# Include directory for header files
target_include_directories(GardenOfEris PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Specify the required libraries
set(LIBRARIES
    stdc++
    allegro
    allegro_image
    allegro_font
    allegro_ttf
    allegro_primitives
    m
    openal
    sndfile
    pthread
)

target_link_libraries(GardenOfEris ${LIBRARIES})

# Set the source files for the unit tests using wildcard
file(GLOB TEST_SOURCE_FILES ${CMAKE_SOURCE_DIR}/unitTests/*.cpp)

# Find Boost Test Framework
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# Include directory for header files in tests
include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)

# Loop over each test source file and create an executable
foreach(TEST_SOURCE_FILE ${TEST_SOURCE_FILES})
    # Get the name of the test from the source file
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)

    # Add the executable target for each test
    add_executable(${TEST_NAME} ${TEST_SOURCE_FILE} $<TARGET_OBJECTS:GardenOfErisLib>)

    # Specify the required libraries
    target_link_libraries(${TEST_NAME}
        ${LIBRARIES}
        ${Boost_LIBRARIES}
    )

    # Add the test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach(TEST_SOURCE_FILE)

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${TEST_SOURCE_FILES}
)

# SonarQube Analysis
option(ENABLE_SONARQUBE "Enable SonarQube analysis" OFF)
if(ENABLE_SONARQUBE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-abs-path")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/CoverageReports
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND gcovr -r ${CMAKE_SOURCE_DIR} --object-directory ${CMAKE_BINARY_DIR} -e "CMakeFiles" --sonarqube -o ${CMAKE_BINARY_DIR}/CoverageReports/coverageReport.xml
        DEPENDS run_tests
    )
endif()

