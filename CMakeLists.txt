cmake_minimum_required(VERSION 3.14)
project(modern_pdfium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Options ──
option(PDFIUM_ROOT "Path to PDFium installation" "")
option(BUILD_EXAMPLES "Build example programs" OFF)

# ── Library ──
add_library(modern_pdfium STATIC)

target_include_directories(modern_pdfium
  PUBLIC
    include
)

file(GLOB MODERN_PDFIUM_SRC "src/*.cpp")
target_sources(modern_pdfium PRIVATE ${MODERN_PDFIUM_SRC})

# ── Link PDFium ──
if(PDFIUM_ROOT)
  # User-provided PDFium path
  target_include_directories(modern_pdfium
    PUBLIC
      "${PDFIUM_ROOT}/include"
  )
  target_link_directories(modern_pdfium
    PUBLIC
      "${PDFIUM_ROOT}/lib"
  )
  target_link_libraries(modern_pdfium
    PUBLIC
      pdfium
  )
else()
  # Fallback: find_package or system paths
  find_package(PDFium QUIET)
  if(PDFium_FOUND)
    target_link_libraries(modern_pdfium PUBLIC PDFium::PDFium)
  else()
    message(WARNING "PDFium not found. Set -DPDFIUM_ROOT=... to the pdfium_cmake_support repo root.")
  endif()
endif()

# ── Examples ──
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
