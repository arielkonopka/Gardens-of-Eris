# The Python package goe: the extension module goe._goe (nanobind) plus the files in goe/.
#   - from the main build with -DGOE_BUILD_PYTHON=ON: the package is staged in
#     <build>/agent/python/package and ctest runs its tests as python-tests;
#   - by pip from this folder (pyproject.toml, scikit-build-core), which builds the repository's
#     root CMakeLists.txt and installs the package.
find_package(Python 3.9 COMPONENTS Interpreter Development.Module REQUIRED)
if(NOT nanobind_ROOT)
  execute_process(COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
                  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
endif()
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_goe NB_STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.cpp)
target_link_libraries(_goe PRIVATE goe-agent)
if(NOT MSVC)
  # nanobind's own headers trip these in every file that includes them
  target_compile_options(_goe PRIVATE -Wno-shadow -Wno-pedantic)
  if(TARGET nanobind-static)
    target_compile_options(nanobind-static PRIVATE -Wno-shadow -Wno-pedantic)
  endif()
endif()
# where the game's data is unless GOE_DATA_DIR or the data_dir argument says otherwise
target_compile_definitions(_goe PRIVATE GOE_DEFAULT_DATA_DIR="${PROJECT_SOURCE_DIR}/GoEoOL")

file(GLOB GOE_PY_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/goe/*.py)
set(GOE_PY_STAGE ${CMAKE_CURRENT_BINARY_DIR}/package/goe)
set_target_properties(_goe PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${GOE_PY_STAGE})
add_custom_target(goe-py-stage ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GOE_PY_STAGE}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GOE_PY_FILES} ${GOE_PY_STAGE}
    COMMENT "Staging the goe Python package")
add_dependencies(_goe goe-py-stage)

install(TARGETS _goe LIBRARY DESTINATION goe COMPONENT python)
install(FILES ${GOE_PY_FILES} DESTINATION goe COMPONENT python)

if(GOE_BUILD_TESTS)
  add_test(NAME python-tests
           COMMAND ${Python_EXECUTABLE} -m pytest -q ${CMAKE_CURRENT_SOURCE_DIR}/tests
           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(python-tests PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/package")
endif()
