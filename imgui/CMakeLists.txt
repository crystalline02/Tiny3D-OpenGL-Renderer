aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_IMGUI)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
add_library(imgui STATIC ${SRC_IMGUI})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_SOURCE_DIR}/include)