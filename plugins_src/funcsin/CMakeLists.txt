project(funcsin LANGUAGES CXX)
add_library(funcsin SHARED funcsin.cpp)
target_include_directories(funcsin PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(funcsin PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)
