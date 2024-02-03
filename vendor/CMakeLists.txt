
cmake_minimum_required (VERSION 3.20)

add_subdirectory("opal")
add_subdirectory("diamond")
add_subdirectory("peridot")
set(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory("spdlog")

add_library (VendorQuartz INTERFACE)

define_property(GLOBAL PROPERTY VendorQuartzSource INHERITED)
set_property(GLOBAL PROPERTY VendorQuartzSource
  ${CMAKE_CURRENT_SOURCE_DIR}/header_only/stb_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/header_only/stb_image_write.h
  ${CMAKE_CURRENT_SOURCE_DIR}/header_only/tiny_obj_loader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/header_only/tinyexr.h

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
)

target_include_directories(VendorQuartz INTERFACE
  ./opal/include/
  ./diamond/src/
  ./peridot/include/
  ./spdlog/include/
  ./imgui/
  ./header_only/
)

target_link_libraries(VendorQuartz INTERFACE
  opal
  diamond
  spdlog
)
