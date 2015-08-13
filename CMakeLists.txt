cmake_minimum_required (VERSION 2.6)

project(scs)

add_definitions ("-Wall")
add_executable (scs main.cpp unit.cpp input.cpp map.cpp status.cpp)
add_library (torneko3 STATIC unit.cpp input.cpp map.cpp status.cpp)
target_link_libraries (scs boost_program_options)
set(CMAKE_CXX_FLAGS "-std=c++0x -O3")