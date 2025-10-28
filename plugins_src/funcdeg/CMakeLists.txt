project(funcdeg LANGUAGES CXX)
add_library(funcdeg SHARED funcdeg.cpp)
target_include_directories(funcdeg PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(funcdeg PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)
