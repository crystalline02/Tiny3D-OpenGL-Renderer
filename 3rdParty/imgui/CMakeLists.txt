project(imgui)
file(GLOB_RECURSE SRC_IMGUI ./*.h ./*.cpp)
add_library(imgui STATIC ${SRC_IMGUI})
target_include_directories(imgui PRIVATE 
    ./include
    ../glfw/include
    )
set_target_properties(imgui PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)