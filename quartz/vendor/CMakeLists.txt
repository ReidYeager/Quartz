
cmake_minimum_required (VERSION 3.20)

add_subdirectory("spdlog")

add_library (VendorQuartz INTERFACE)

target_include_directories(VendorQuartz INTERFACE
  ./spdlog/include/
)

target_link_libraries(VendorQuartz INTERFACE
  spdlog
)
