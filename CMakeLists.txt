cmake_minimum_required(VERSION 3.10)
project(flightparse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

add_executable(flightparse
    src/main.cpp
    src/fdr_parser.cpp
    src/validator.cpp
    src/analyzer.cpp
    src/report_generator.cpp
)

target_include_directories(flightparse PRIVATE src)
