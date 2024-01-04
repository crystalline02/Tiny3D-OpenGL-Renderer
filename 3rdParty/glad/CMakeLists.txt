project(glad LANGUAGES C)
add_library(glad STATIC 
    ./src/glad.c)
target_include_directories(glad PRIVATE ./include)
set_target_properties(glad PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)