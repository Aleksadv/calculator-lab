cmake_minimum_required(VERSION 3.15)
project(CalculatorLab VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set(SOURCES
    src/main.cpp
    src/calculator/Tokenizer.cpp
    src/calculator/ShuntingYard.cpp
    src/calculator/Evaluator.cpp
    src/plugins/PluginLoader.cpp
)
add_executable(calc ${SOURCES})


target_include_directories(calc PRIVATE include src)

add_subdirectory(plugins_src/funcsin)
add_subdirectory(plugins_src/funcdeg)
add_subdirectory(plugins_src/funcln)

if(WIN32)
    set_target_properties(calc PROPERTIES SUFFIX ".exe")
endif()

add_executable(calc_selftests
    tests/calc_selftest.cpp
    src/calculator/Tokenizer.cpp
    src/calculator/ShuntingYard.cpp
    src/calculator/Evaluator.cpp
    src/plugins/PluginLoader.cpp
)
target_include_directories(calc_selftests PRIVATE include src)

# На интеграционных тестах хотим, чтобы плагины были собраны
add_dependencies(calc_selftests funcdeg funcln funcsin) # доб. сюда остальные плагины, если есть