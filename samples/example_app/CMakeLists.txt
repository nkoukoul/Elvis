# CMakeList.txt : CMake project for new_elvis, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("example_app")
set(CMAKE_CXX_FLAGS "-std=c++17")
# Add source to this project's executable.
add_executable (server "main.cpp" "my_controllers.cpp" "my_controllers.h" "my_models.cpp" "my_models.h")

target_link_libraries (server LINK_PUBLIC Elvis)
target_include_directories(server PUBLIC ${CMAKE_SOURCE_DIR})
# TODO: Add tests and install targets if needed.
