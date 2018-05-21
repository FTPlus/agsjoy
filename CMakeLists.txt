cmake_minimum_required (VERSION 2.6)

option (USE_SDL "Use the SDL library instead of native APIs." OFF)
option (USE_MM "Use the windows Multi Media API (windows only)." OFF)
option (USE_DX "Use the directX API (windows only)." OFF)

project(agsjoy)
add_subdirectory(src)
add_subdirectory(test)