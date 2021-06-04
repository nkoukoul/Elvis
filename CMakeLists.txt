# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("new_elvis")

# Include sub-projects.
add_subdirectory ("samples/example_app")
add_subdirectory ("elvis")
