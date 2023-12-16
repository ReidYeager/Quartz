
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_subdirectory("vendor")

file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h")
file(GLOB_RECURSE CppSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp")

add_library (Quartz STATIC
  ${HeaderFiles}
  ${CppSourceFiles}
)

target_include_directories (Quartz PUBLIC
  ./src/
  VendorQuartz
  VendorMain
)

target_link_libraries(Quartz
  VendorQuartz
  VendorMain
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  target_compile_definitions(Quartz PUBLIC "QTZ_CONFIG_DEBUG")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  target_compile_definitions(Quartz PUBLIC "QTZ_CONFIG_RELEASE")
endif()

if (WIN32)
  target_compile_definitions(Quartz PUBLIC "QTZ_PLATFORM_WIN32")
else()
  message(FATAL_ERROR "Must compile on windows")
endif()
