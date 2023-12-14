
cmake_minimum_required (VERSION 3.20)

# add_subdirectory("")

add_library (VendorSandbox INTERFACE)

target_include_directories(VendorSandbox INTERFACE
  ./
)

target_link_libraries(VendorSandbox INTERFACE
  
)
