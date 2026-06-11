cmake_minimum_required(VERSION 3.20)
project(modern_pdfium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use static MSVC runtime to match PDFium static libraries (/MT)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ── Options ──
option(PDFIUM_ROOT "Path to PDFium installation (pdfium_cmake_support root)" "")
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
      "${PDFIUM_ROOT}"
      "${PDFIUM_ROOT}/public"
      "${PDFIUM_ROOT}/public/cpp"
  )

  # PDFium static library directory (from cmake build output)
  set(PDFIUM_BUILD_LIB_DIR "${PDFIUM_ROOT}/build/cmake_build/lib")

  # PDFium dependency library directory
  set(PDFIUM_DEPS_LIB_DIR "${PDFIUM_ROOT}/deps/lib")

  # Link all PDFium internal static libraries
  file(GLOB PDFIUM_STATIC_LIBS "${PDFIUM_BUILD_LIB_DIR}/*.lib")
  target_link_libraries(modern_pdfium
    PUBLIC
      ${PDFIUM_STATIC_LIBS}
  )

  # Link dependency libraries
  target_link_directories(modern_pdfium
    PUBLIC
      "${PDFIUM_DEPS_LIB_DIR}"
  )
  target_link_libraries(modern_pdfium
    PUBLIC
      harfbuzz.lib
      icuuc.lib
      icudt.lib
      jpeg.lib
  )

  # Glob all abseil libs
  file(GLOB ABSL_LIBS "${PDFIUM_DEPS_LIB_DIR}/absl_*.lib")
  target_link_libraries(modern_pdfium
    PUBLIC
      ${ABSL_LIBS}
  )

  # Windows system libraries
  if(WIN32)
    target_link_libraries(modern_pdfium
      PUBLIC
        advapi32
        gdi32
        user32
    )
  endif()
else()
  # Fallback: find_package or system paths
  find_package(PDFium QUIET)
  if(PDFium_FOUND)
    target_link_libraries(modern_pdfium PUBLIC PDFium::PDFium)
  else()
    message(WARNING "PDFium not found. Set -DPDFIUM_ROOT=<path-to-pdfium_cmake_support>")
  endif()
endif()

# ── Examples ──
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
