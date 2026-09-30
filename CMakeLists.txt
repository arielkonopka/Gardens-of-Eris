cmake_minimum_required(VERSION 3.16)
project(GardenOfEris LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Link-time optimization lets the compiler inline the many small getters across files
option(ENABLE_LTO "Build with link-time optimization" ON)
if(ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_ok OUTPUT ipo_msg LANGUAGES CXX)
  if(ipo_ok)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    # MinGW's lto-wrapper cannot run link-time jobs in parallel and warns about every serial one,
    # so link as a single partition there
    if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      add_link_options(-flto-partition=one)
    endif()
  else()
    message(STATUS "LTO not available: ${ipo_msg}")
  endif()
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /MTd /RTC1 /wd4514")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -g -O3 -pthread")
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
# std::stacktrace, used by the crash report, lives in libstdc++exp on GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND LIBRARIES stdc++exp)
endif()

target_link_libraries(GardenOfEris ${LIBRARIES})

# Set the source files for the unit tests using wildcard
file(GLOB TEST_SOURCE_FILES ${CMAKE_SOURCE_DIR}/unitTests/*.cpp)

# The unit tests use GoogleTest
find_package(GTest REQUIRED)

# Include directory for header files in tests
include_directories(${CMAKE_SOURCE_DIR}/include)

# Loop over each test source file and create an executable
foreach(TEST_SOURCE_FILE ${TEST_SOURCE_FILES})
    # Get the name of the test from the source file
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)

    # Add the executable target for each test
    add_executable(${TEST_NAME} ${TEST_SOURCE_FILE} $<TARGET_OBJECTS:GardenOfErisLib>)

    # Specify the required libraries
    target_link_libraries(${TEST_NAME}
        ${LIBRARIES}
        GTest::gtest_main
    )

    # Add the test
    # Tests read data/skins.json relative to the game directory
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/GoEoOL)
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


# Headless benchmark (not a test): level generation, memory, tick, save and load times
add_executable(goe-bench ${CMAKE_SOURCE_DIR}/benchmarks/goe-bench.cpp $<TARGET_OBJECTS:GardenOfErisLib>)
target_link_libraries(goe-bench ${LIBRARIES})
