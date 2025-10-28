project(funcln LANGUAGES CXX)
add_library(funcln SHARED funcln.cpp)
target_include_directories(funcln PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(funcln PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)
