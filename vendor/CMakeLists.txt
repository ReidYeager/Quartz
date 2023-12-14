
cmake_minimum_required (VERSION 3.20)

# add_subdirectory("")

add_library (VendorMain INTERFACE)

target_include_directories(VendorMain INTERFACE
  ./
)

target_link_libraries(VendorMain INTERFACE
  
)
