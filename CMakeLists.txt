cmake_minimum_required(VERSION 3.2)
project(renderer)

add_subdirectory(./3rdParty)

file(GLOB_RECURSE SOURCES ./src/*.h ./src/*.cpp)
add_executable(renderer ${SOURCES})
target_include_directories(renderer PRIVATE 
    ./3rdParty/Assimp
    ./3rdParty/freetype/include
    ./3rdParty/glad/include
    ./3rdParty/glfw/include
    ./3rdParty/glm/
    ./3rdParty/imgui/include
    )
target_link_libraries(renderer PRIVATE 
    ${CMAKE_SOURCE_DIR}/lib/libassimp.dll.a
    freetype
    glad
    glfw
    imgui
    )
set_target_properties(renderer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)