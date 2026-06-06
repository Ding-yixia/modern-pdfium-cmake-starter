# Examples for modern-pdfium-cmake-starter
add_executable(pdfium_basic basic.cpp)
target_link_libraries(pdfium_basic PRIVATE modern_pdfium)

add_executable(pdfium_text text.cpp)
target_link_libraries(pdfium_text PRIVATE modern_pdfium)

add_executable(pdfium_bookmark bookmark.cpp)
target_link_libraries(pdfium_bookmark PRIVATE modern_pdfium)

add_executable(pdfium_annot annotation.cpp)
target_link_libraries(pdfium_annot PRIVATE modern_pdfium)

add_executable(pdfium_search search.cpp)
target_link_libraries(pdfium_search PRIVATE modern_pdfium)

add_executable(pdfium_render render.cpp)
target_link_libraries(pdfium_render PRIVATE modern_pdfium)
