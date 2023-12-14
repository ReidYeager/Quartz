
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

Project ("QuartzEngine")

add_subdirectory("vendor")
add_subdirectory("quartz")
add_subdirectory("sandbox")
