project(cdtext)
add_subdirectory(minikin)
add_library(cdtext 
    textcontainer.cc
    )
target_link_libraries(cdtext minikin)

target_include_directories(
    cdtext
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/minikin/include
    ${PROJECT_SOURCE_DIR}/minikin/libs
    ${CMAKE_BINARY_DIR}/include
    ${HARFBUZZ_INCLUDE_DIRS}
    ${FRIBIDI_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    #AFTER
    BEFORE
    )
