# The headless agent library: the game's objects plus the agent interface (agentGame.h), with no
# window, sound or keyboard. Programs that play the game link goe-agent; Python uses agent/python.
set(GOE_AGENT_OBJECTS GardenOfErisLib)
if(GOE_BUILD_PYTHON)
  # A Python extension module is a shared library, so its code must be position independent.
  # The game's objects are compiled a second time for it; the game itself stays as it is.
  add_library(GardenOfErisPicLib OBJECT ${SOURCE_FILES})
  set_target_properties(GardenOfErisPicLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
  set(GOE_AGENT_OBJECTS GardenOfErisPicLib)
endif()
add_library(goe-agent STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/agentFeatures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/agentGame.cpp
    $<TARGET_OBJECTS:${GOE_AGENT_OBJECTS}>)
target_include_directories(goe-agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(goe-agent PUBLIC ${LIBRARIES})
if(GOE_BUILD_PYTHON)
  set_target_properties(goe-agent PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(GOE_BUILD_TESTS)
  add_executable(agent-test ${CMAKE_CURRENT_SOURCE_DIR}/tests/agent-test.cpp)
  target_link_libraries(agent-test goe-agent GTest::gtest_main)
  # run from elsewhere on purpose: the game is told where its data is
  add_test(NAME agent-test COMMAND agent-test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(agent-test PROPERTIES ENVIRONMENT "GOE_DATA_DIR=${CMAKE_SOURCE_DIR}/GoEoOL")
endif()

if(GOE_BUILD_PYTHON)
  add_subdirectory(python)
endif()
