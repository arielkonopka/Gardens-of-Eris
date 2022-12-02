cmake_minimum_required(VERSION 3.25.0)
project(GoEoOL CXX)
find_package(PkgConfig REQUIRED)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_FLAGS -O3 )
set(DataDir "GoEoOL/data")
include_directories("./include")
file(GLOB bElems CONFIGURE_DEPENDS "./include/*.h" "./src/*.cpp")
add_executable(GoEoOL ./main.cpp ${bElems} )

find_package(Boost COMPONENTS system filesystem REQUIRED)
pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_image-5 allegro_ttf-5 allegro_font-5)
target_link_libraries(GoEoOL ${ALLEGRO5_LIBRARIES} ${Boost_LIBRARIES})

link_directories(${Boost_LIBRARY_DIR} ${ALLEGRO5_LIBRARY_DIR} )
include_directories(${ALLEGRO5_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
#[[ copy the data files when creating a package ]]
file(COPY ${DataDir} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#add_dependencies(GoEoOL Boost ALLEGRO5)

