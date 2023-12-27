cmake_minimum_required(VERSION 3.2)
project(Renderer)
# set(CMAKE_BUILD_TYPE  Release)
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_CPP)
# 注意！链接库的书写顺序也很重要！target_link_libraries链接多个库时如果顺序不对有可能编译不成功！
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_subdirectory(${CMAKE_SOURCE_DIR}/imgui)
set(LIBS 
    ${CMAKE_SOURCE_DIR}/lib/libassimp.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libglad.a 
    ${CMAKE_SOURCE_DIR}/lib/libglfw3.a 
    imgui
    ${CMAKE_SOURCE_DIR}/lib/libfreetype.dll.a
    )
add_executable(renderer ${SRC_CPP})
target_include_directories(renderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(renderer PRIVATE ${LIBS})
