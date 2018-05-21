

include_directories(${PROJECT_SOURCE_DIR}/include/)
link_directories(${PROJECT_SOURCE_DIR}/lib/)

project(joytest)

add_executable(joytest main.cpp engine.cpp)
target_link_libraries(joytest agsjoy)