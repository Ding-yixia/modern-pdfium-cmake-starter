cmake_minimum_required(VERSION 3.14)
project(modern_pdfium LANGUAGES CXX)

add_library(modern_pdfium STATIC)

target_include_directories(modern_pdfium PUBLIC include)

file(GLOB MODERN_PDFIUM_SRC "src/*.cpp")
target_sources(modern_pdfium PRIVATE ${MODERN_PDFIUM_SRC})
