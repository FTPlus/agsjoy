
add_definitions(-DTHIS_IS_THE_PLUGIN=1)

include_directories(${PROJECT_SOURCE_DIR}/include/)

project(agsjoy)

add_library(agsjoy SHARED agsplugin.cpp API.cpp Joystick.cpp version.rc)
set_target_properties(agsjoy PROPERTIES CXX_VISIBILITY_PRESET hidden)

if (USE_SDL)
	message(STATUS "Using SDL!")
	add_definitions (-DSDL_VERSION)
	if (WIN32)
		target_link_libraries(agsjoy )
	else()
		target_link_libraries(agsjoy )
	endif()
elseif (USE_MM)
	message(STATUS "Using winMM!")
	add_definitions (-DWINMM_VERSION)
	target_link_libraries(agsjoy winmm)
elseif (USE_DX)
	message(STATUS "Using DirectX!")
	add_definitions (-DDX8_VERSION)
	target_link_libraries(agsjoy )
elseif (WIN32)
	target_link_libraries(agsjoy winmm)
else()
	target_link_libraries(agsjoy )
endif()

if (WIN32)
	# Compile runtime statically so that the dll does not require the redist kit
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	# remove 'lib' prefix in minGW builds
	set_target_properties(agsjoy PROPERTIES PREFIX "")
endif()
