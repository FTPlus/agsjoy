
add_definitions(-DTHIS_IS_THE_PLUGIN=1)

include_directories(${PROJECT_SOURCE_DIR}/include/)

project(agsjoy)

if (WIN32)
	# Compile runtime statically so that the dll does not require the redist kit
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif ()

add_library(agsjoy SHARED agsplugin.cpp API.cpp Joystick.cpp version.rc)
set_target_properties(agsjoy PROPERTIES CXX_VISIBILITY_PRESET hidden)

if (WIN32)
	target_link_libraries(agsjoy winmm)
	set_target_properties(agsjoy PROPERTIES PREFIX "")
else()
	target_link_libraries(agsjoy )
endif()
