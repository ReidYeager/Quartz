
cmake_minimum_required (VERSION 3.20)

add_subdirectory("opal")
add_subdirectory("diamond")
add_subdirectory("peridot")
set(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory("spdlog")

add_library (VendorQuartz INTERFACE)

target_include_directories(VendorQuartz INTERFACE
  ./opal/include/
  ./diamond/src/
  ./peridot/include/
  ./spdlog/include/
)

target_link_libraries(VendorQuartz INTERFACE
  opal
  diamond
  Peridot
  spdlog
)
